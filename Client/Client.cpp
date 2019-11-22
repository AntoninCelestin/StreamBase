#include "stdafx.h"
#include "Client.h"

using std::cout;
using std::endl;

Client::Client()
{
	// Named pipe file.
	lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");
}


Client::~Client()
{
}

// Flag used to stop async thread.
bool Client::_stopThread;

int Client::SendAsync(const MessageWrapper *messageWrapper)
{
	BOOL   fSuccess = FALSE;
	DWORD  dwMode;

	Client::_stopThread = false;

	PPipeInstanceType lpPipeInst;
	DWORD dwWait, cbRet;

	if (ConectToServer(1))
	{
		// Try to open a named pipe; wait for it, if necessary. 
		// The pipe connected; change to message-read mode. 
		dwMode = PIPE_READMODE_MESSAGE;
		fSuccess = SetNamedPipeHandleState(
			ahPipe,    // pipe handle 
			&dwMode,  // new pipe mode 
			NULL,     // don't set maximum bytes 
			NULL);    // don't set maximum time 

		if (!fSuccess)
		{
			_tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
			return -1;
		}

		// Allocate storage for this instance. 

		lpPipeInst = (PPipeInstanceType)GlobalAlloc(
			GPTR, sizeof(PipeInstanceType));
		if (lpPipeInst == NULL)
		{
			printf("GlobalAlloc failed (%d)\n", GetLastError());
			return 0;
		}

		std::wstring stemp1 = MessageWrapper::SerializeMessage(messageWrapper);
		LPCWSTR sw = stemp1.c_str();

		lpPipeInst->ahPipeInstance = ahPipe;
		StringCchCopy(lpPipeInst->aTchReply, BUFFERS_SIZE, sw);

		lpPipeInst->aDwToWrite = (lstrlen(lpPipeInst->aTchReply) + 1) * sizeof(TCHAR);

		//write data to named pipe server
		fSuccess = WriteFileEx(
			lpPipeInst->ahPipeInstance,
			lpPipeInst->aTchReply,
			lpPipeInst->aDwToWrite,
			(LPOVERLAPPED)lpPipeInst,
			(LPOVERLAPPED_COMPLETION_ROUTINE)Client::CompletedWriteRoutine);

		if (!fSuccess)
		{
			_tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
			Client::DisconnectAndClose(lpPipeInst);
			return -1;
		}
			
		while (!Client::_stopThread)
		{
			dwWait = WaitForSingleObjectEx(
				ahConnectEvent,  // event object to wait for 
				INFINITE,        // waits indefinitely 
				TRUE);

			//wait for async completion
			switch (dwWait)
			{
			case 0:
				fSuccess = GetOverlappedResult(
					ahPipe,     // pipe handle 
					&aoConnect, // OVERLAPPED structure 
					&cbRet,     // bytes transferred 
					FALSE);     // does not wait 
				if (!fSuccess)
				{
					printf("ConnectNamedPipe (%d)\n", GetLastError());
					return 0;
				}
				break;
			case WAIT_IO_COMPLETION:
				break;
			default:
				printf("WaitForSingleObjectEx (%d)\n", GetLastError());
				return 0;
			}
		}
	}

	return 0;
}

int Client::SendSync(const MessageWrapper * messageWrapper)
{
	
	BOOL   fSuccess = FALSE;
	DWORD  aDwRead, cbWritten, dwMode;
	TCHAR  chBuf[BUFFERS_SIZE];
	PPipeInstanceType lpPipeInst;

	// Try to open a named pipe; wait for it, if necessary. 

	if (ConectToServer(2))
	{
		// The pipe connected; change to message-read mode. 

		dwMode = PIPE_READMODE_MESSAGE;
		fSuccess = SetNamedPipeHandleState(
			ahPipe,    // pipe handle 
			&dwMode,  // new pipe mode 
			NULL,     // don't set maximum bytes 
			NULL);    // don't set maximum time 
		if (!fSuccess)
		{
			_tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
			return -1;
		}

		// Allocate storage for this instance. 

		lpPipeInst = (PPipeInstanceType)GlobalAlloc(
			GPTR, sizeof(PipeInstanceType));
		if (lpPipeInst == NULL)
		{
			printf("GlobalAlloc failed (%d)\n", GetLastError());
			return 0;
		}

		// Send a message to the pipe server. 
		std::wstring stemp1 = MessageWrapper::SerializeMessage(messageWrapper);
		//prepare the serialized message to be send
		LPCWSTR sw = stemp1.c_str();
		
		//stores the pipe handle
		lpPipeInst->ahPipeInstance = ahPipe;
		StringCchCopy(lpPipeInst->aTchReply, BUFFERS_SIZE, sw);
		lpPipeInst->aDwToWrite = (lstrlen(lpPipeInst->aTchReply) + 1) * sizeof(TCHAR);
		//write data to named pipe server
		fSuccess = WriteFile(
			lpPipeInst->ahPipeInstance,
			lpPipeInst->aTchReply,
			lpPipeInst->aDwToWrite,
			&cbWritten,
			NULL
		);

		if (!fSuccess)
		{
			_tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
			return -1;
		}

		do
		{
			// Read from the pipe. 
			fSuccess = ReadFile(
				ahPipe,    // pipe handle 
				chBuf,    // buffer to receive reply 
				BUFFERS_SIZE * sizeof(TCHAR),  // size of buffer 
				&aDwRead,  // number of bytes read 
				NULL);    // not overlapped 

			if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
				break;

		} while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 

		if (!fSuccess)
		{
			_tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
			return -1;
		}
		//deserialize message to be processed
		auto resp = MessageWrapper::DeserializeMessage(chBuf);
		//processes named pipe server response
		ProcessResponse(resp);
		//close the pipe handle
		CloseHandle(ahPipe);
	}
	return 0;
}

void Client::ProcessResponse(MessageWrapper* mw)
{
	switch (mw->GetCommand())
	{
	case Commands::COMMAND_RESPONSE_SERVER_OK:
		cout << "\nMessage was successfully sent to the server!\n" << endl;
		break;
	case Commands::COMMAND_RESPONSE_LIST_OF_STREAMERS:
		if (mw->GetStreamersCount() > 0)
		{
			cout << "Printing the list of streamers from server" << endl;
			for (auto item : mw->GetStreamers())
			{
				cout << "Streamer name: " << item.GetName() << " Streamer Code : " << item.GetCode() << endl;
			}
		}
		else
		{
			cout << "No streamers stored on server.." << endl;
		}
		break;
	case Commands::COMMAND_RESPONSE_LIST_OF_DATA :
			if (mw->GetDataCount() > 0)
			{
				cout << "Printing the list of data from server" << endl;
				for (auto item : mw->GetData())
				{
					cout << "Data stored on server : " << item << endl;
				}
			}
			break;
	default:
		break;
	}
}

bool Client::ConectToServer(int type)
{
	bool conn = false;

	//verify the connection type
	if(type == 1)
		conn = ConnectAsync();
	else
		conn = ConnectSync();

	while (!conn)
	{
		cout << "Was not possible connect to the server - check if server is available press any key to try again..." << endl;
		_getch();

		if (type == 1)
			conn = ConnectAsync();
		else
			conn = ConnectSync();
	}

	return conn;
}

bool Client::ConnectAsync()
{
	if (this->Initialize())
	{
		while (1)
		{
			ahPipe = CreateFile(
				lpszPipename,   // pipe name 
				GENERIC_READ |  // read and write access 
				GENERIC_WRITE,
				0,              // no sharing 
				NULL,           // default security attributes
				OPEN_EXISTING,  // opens existing pipe 
				FILE_FLAG_OVERLAPPED,              // default attributes 
				NULL);          // no template file 

								// Break if the pipe handle is valid. 

			if (ahPipe != INVALID_HANDLE_VALUE)
			{
				return true;
			}


			// Exit if an error other than ERROR_PIPE_BUSY occurs. 

			if (GetLastError() != ERROR_PIPE_BUSY)
			{
				_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
				return false;
			}

			// All pipe instances are busy, so wait for 20 seconds. 

			if (!WaitNamedPipe(lpszPipename, 20000))
			{
				printf("Could not open pipe: 20 second wait timed out.");
				return false;
			}
		}
	}
	else
		return false;
}

bool Client::ConnectSync()
{
	while (1)
	{
		ahPipe = CreateFile(
			lpszPipename,   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE,
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			0,              // default attributes 
			NULL);          // no template file 

							// Break if the pipe handle is valid. 

		if (ahPipe != INVALID_HANDLE_VALUE)
			return true;

		// Exit if an error other than ERROR_PIPE_BUSY occurs. 

		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			_tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
			return false;
		}

		// All pipe instances are busy, so wait for 20 seconds. 

		if (!WaitNamedPipe(lpszPipename, 20000))
		{
			printf("Could not open pipe: 20 second wait timed out.");
			return false;
		}
	}
	return true;
}

//creates named pipe connection event 
bool Client::Initialize()
{
	bool res = true;
	// Create one event object for the connect operation. 
	ahConnectEvent = CreateEvent(
		NULL,    // default security attribute
		TRUE,    // manual reset event 
		TRUE,    // initial state = signaled 
		NULL);   // unnamed event object 

	if (ahConnectEvent == NULL)
	{
		printf("CreateEvent failed with %d.\n", GetLastError());
		res = false;
	}

	aoConnect.hEvent = ahConnectEvent;
	return res;
}

//callback method to be called when read process is finalized
VOID Client::CompletedReadRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap)
{

	PPipeInstanceType lpPipeInst;
	lpPipeInst = (PPipeInstanceType)lpOverLap;
	auto s = MessageWrapper::DeserializeMessage(lpPipeInst->aTchRequest);
	ProcessResponse(s);
	Client::_stopThread = true;
	Client::DisconnectAndClose(lpPipeInst);
}

//callback method to be called when write process is finalized
VOID Client::CompletedWriteRoutine(DWORD dwErr, DWORD cbWritten, LPOVERLAPPED lpOverLap)
{
	PPipeInstanceType lpPipeInst;
	BOOL fRead = FALSE;

	// lpOverlap points to storage for this instance. 
	lpPipeInst = (PPipeInstanceType)lpOverLap;

	if ((dwErr == 0) && (cbWritten == lpPipeInst->aDwToWrite))
		fRead = ReadFileEx(
			lpPipeInst->ahPipeInstance,
			lpPipeInst->aTchRequest,
			BUFFERS_SIZE * sizeof(TCHAR),
			(LPOVERLAPPED)lpPipeInst,
			(LPOVERLAPPED_COMPLETION_ROUTINE)CompletedReadRoutine);
}

VOID Client::DisconnectAndClose(PPipeInstanceType lpPipeInst)
{
	FlushFileBuffers(lpPipeInst->ahPipeInstance);

	DisconnectNamedPipe(lpPipeInst->ahPipeInstance);

	CloseHandle(lpPipeInst->ahPipeInstance);

	// Release the storage for the pipe instance. 
	if (lpPipeInst != NULL)
		GlobalFree(lpPipeInst);
}