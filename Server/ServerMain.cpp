// ServerMain.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Server.h"

int main()
{
	std::unique_ptr<Server> service = std::make_unique<Server>();
	service->Initialize();

	return 0;
}