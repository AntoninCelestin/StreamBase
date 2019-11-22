#include "stdafx.h"
#include "Server.h"

vector<Streamer> Server::aServerStreamers;
vector<string> Server::aServerData;

void Server::Initialize()
{
	BOOL   fConnected = FALSE;
	DWORD  dwThreadId = 0;
	HANDLE ahPipe = INVALID_HANDLE_VALUE, hThread = NULL;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");

	// The main loop creates an instance of the named pipe and 
	// then waits for a client to connect to it. When the client 
	// connects, a thread is created to handle communications 
	// with that client, and this loop is free to wait for the
	// next client connect request. It is an infinite loop.

	for (;;)
	{
		_tprintf(TEXT("\nPipe Server: Main thread awaiting client connection on %s\n"), lpszPipename);
		ahPipe = CreateNamedPipe(
			lpszPipename,             // pipe name 
			PIPE_ACCESS_DUPLEX,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			BUFFER_SIZE,              // output buffer size 
			BUFFER_SIZE,              // input buffer size 
			0,                        // client time-out 
			NULL);                    // default security attribute 

		if (ahPipe == INVALID_HANDLE_VALUE)
		{
			_tprintf(TEXT("CreateNamedPipe failed, GLE=%d.\n"), GetLastError());
			return;
		}

		// Wait for the client to connect; if it succeeds, 
		// the function returns a non null value. 
		// If the function returns zero,
		// GetLastError returns ERROR_PIPE_CONNECTED. 

		fConnected = ConnectNamedPipe(ahPipe, NULL) ?
			TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

		if (fConnected)
		{
			printf("\nClient connected, creating a processing thread.\n");

			hThread = CreateThread(
				NULL,					// no security attribute 
				0,						// default stack size 
				Server::InstanceThread, // thread proc
				(LPVOID)ahPipe,			// thread parameter 
				0,						// not suspended 
				&dwThreadId);			// returns thread ID 

			if (hThread == NULL)
			{
				_tprintf(TEXT("CreateThread failed, GLE=%d.\n"), GetLastError());
				return;
			}
			else CloseHandle(hThread);
		}
		else
			// The client could not connect, so close the pipe. 
			CloseHandle(ahPipe);
	}
}

// This routine is a thread processing function to read from and reply to a client
// via the open pipe connection passed from the main loop. This allows
// the main loop to continue executing, potentially creating more threads of
// of this procedure to run concurrently, depending on the number of incoming
// client connections.
DWORD WINAPI Server::InstanceThread(LPVOID lpvParam)
{
	HANDLE hHeap = GetProcessHeap();
	TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFFER_SIZE * sizeof(TCHAR));
	TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFFER_SIZE * sizeof(TCHAR));

	DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
	BOOL fSuccess = FALSE;
	HANDLE ahPipe = NULL;

	// Do some extra error checks since the app will keep running even if this
	// thread fails.
	if (lpvParam == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL value in lpvParam.\n");
		printf("   Exiting InstanceThread.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		return (DWORD)-1;
	}

	if (pchRequest == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL heap allocation.\n");
		printf("   Exiting InstanceThread.\n");
		if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
		return (DWORD)-1;
	}

	if (pchReply == NULL)
	{
		printf("\nERROR - Pipe Server Failure:\n");
		printf("   InstanceThread got an unexpected NULL heap allocation.\n");
		printf("   Exiting InstanceThread.\n");
		if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
		return (DWORD)-1;
	}

	// The thread's parameter is a handle to a pipe object instance. 
	ahPipe = (HANDLE)lpvParam;

	// Loop until done reading
	while (1)
	{
		// Read client requests from the pipe. This simplistic code only allows messages
		// up to BUFFER_SIZE characters in length.
		fSuccess = ReadFile(
			ahPipe,							// handle to pipe 
			pchRequest,						// buffer to receive data 
			BUFFER_SIZE * sizeof(TCHAR),	// size of buffer 
			&cbBytesRead,					// number of bytes read 
			NULL);							// not overlapped I/O 

		if (!fSuccess || cbBytesRead == 0)
		{
			if (GetLastError() == ERROR_BROKEN_PIPE)
			{
				_tprintf(TEXT("InstanceThread: client disconnected.\n"), GetLastError());
			}
			else
			{
				_tprintf(TEXT("InstanceThread ReadFile failed, GLE=%d.\n"), GetLastError());
			}
			break;
		}


		PPipeInstanceType lpPipeInst = (PPipeInstanceType)GlobalAlloc(
			GPTR, sizeof(PipeInstanceType));
		if (lpPipeInst == NULL)
		{
			printf("GlobalAlloc failed (%d)\n", GetLastError());
			return 0;
		}

		MessageWrapper *response = new MessageWrapper();
		// Process the incoming message.
		Server::GetAnswerToRequest(pchRequest, pchReply, response);

		std::wstring stemp1 = MessageWrapper::SerializeMessage(response);
		LPCWSTR sw = stemp1.c_str();

		lpPipeInst->ahPipeInstance = ahPipe;
		StringCchCopy(lpPipeInst->aTchReply, BUFFER_SIZE, sw);
		lpPipeInst->aDwToWrite = (lstrlen(lpPipeInst->aTchReply) + 1) * sizeof(TCHAR);

		fSuccess = WriteFile(
			lpPipeInst->ahPipeInstance, // handle to pipe 
			lpPipeInst->aTchReply,		// buffer to write from 
			lpPipeInst->aDwToWrite,		// number of bytes to write 
			&cbWritten,					// number of bytes written 
			NULL);						// not overlapped I/O 

		if (!fSuccess || lpPipeInst->aDwToWrite != cbWritten)
		{
			_tprintf(TEXT("InstanceThread WriteFile failed, GLE=%d.\n"), GetLastError());
			break;
		}
	}

	// Flush the pipe to allow the client to read the pipe's contents 
	// before disconnecting. Then disconnect the pipe, and close the 
	// handle to this pipe instance. 

	FlushFileBuffers(ahPipe);
	DisconnectNamedPipe(ahPipe);
	CloseHandle(ahPipe);

	HeapFree(hHeap, 0, pchRequest);
	HeapFree(hHeap, 0, pchReply);

	printf("Exiting InstanceThread.\n");
	return 1;
}

// This routine is a simple function to print the client request to the console
// and populate the reply buffer with a default data string. This is where you
// would put the actual client request processing code that runs in the context
// of an instance thread. Keep in mind the main thread will continue to wait for
// and receive other client connections while the instance thread is working.

VOID Server::GetAnswerToRequest(LPTSTR pchRequest,
	LPTSTR pchReply,
	MessageWrapper* mw
	)
{
	auto s = MessageWrapper::DeserializeMessage(pchRequest);

	switch (s->GetCommand())
	{
		case Commands::COMMAND_SEND_DATA_SYNC :
		case Commands::COMMAND_SEND_DATA_ASYNC:
			for(auto el : s->GetData())
				Server::aServerData.push_back(el);
			mw->SetCommand(Commands::COMMAND_RESPONSE_SERVER_OK);
			break;
		case Commands::COMMAND_SEND_OBJECT_SYNC:
		case Commands::COMMAND_SEND_OBJECT_ASYNC:
			for (auto el : s->GetStreamers())
				Server::aServerStreamers.push_back(el);
			mw->SetCommand(Commands::COMMAND_RESPONSE_SERVER_OK);
			break;
		case Commands::COMMAND_REQUEST_OBJECT_ASYNC:
		case Commands::COMMAND_REQUEST_OBJECT_SYNC:
			mw->SetListOfStreamers(Server::aServerStreamers);
			mw->SetCommand(Commands::COMMAND_RESPONSE_LIST_OF_STREAMERS);
			break;
		case Commands::COMMAND_REQUEST_DATA_ASYNC:
		case Commands::COMMAND_REQUEST_DATA_SYNC:
			mw->SetListOfData(Server::aServerData);
			mw->SetCommand(Commands::COMMAND_RESPONSE_LIST_OF_DATA);
			break;
	default:
		break;
	}

	_tprintf(TEXT("Client Request String:\"%s\"\n"), pchRequest);

	// Check the outgoing message to make sure it's not too long for the buffer.
	if (FAILED(StringCchCopy(pchReply, BUFFER_SIZE, TEXT("default answer from server"))))
	{
		pchReply[0] = 0;
		printf("StringCchCopy failed, no outgoing message.\n");
		return;
	}
}