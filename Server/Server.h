#pragma once
#include "stdafx.h"
#include "MessageWrapper.h"
#include <strsafe.h>

#define BUFFER_SIZE 512

class Server
{
public:
	virtual void Initialize();
	static VOID GetAnswerToRequest(LPTSTR, LPTSTR, MessageWrapper*);
	static DWORD WINAPI InstanceThread(LPVOID);
private:
	static vector<Streamer> aServerStreamers;
	static vector<string> aServerData;
};
