#pragma once
#include "stdafx.h"
#include "MessageWrapper.h"
#include <strsafe.h>
#include <conio.h>
#include <memory>
#include <wchar.h>

#define PIPE_TIMEOUT 5000

class Client
{
public:
	Client();
	~Client();
	int SendAsync(const MessageWrapper *messageWrapper);
	int SendSync(const MessageWrapper *messageWrapper);
	static VOID DisconnectAndClose(PPipeInstanceType);
	static VOID WINAPI CompletedReadRoutine(DWORD dwErr, DWORD cbBytesRead, LPOVERLAPPED lpOverLap);
	static VOID WINAPI CompletedWriteRoutine(DWORD dwErr, DWORD cbWritten, LPOVERLAPPED lpOverLap);
private:
	HANDLE ahPipe;
	HANDLE ahConnectEvent;
	OVERLAPPED aoConnect;
	static bool _stopThread;
	LPTSTR lpszPipename;
	bool ConnectAsync();
	bool ConnectSync();
	bool ConectToServer(int type);
	static void ProcessResponse(MessageWrapper* messageWrapper);
	bool Initialize();
};

