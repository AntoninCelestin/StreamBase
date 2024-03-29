// ClientMain.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Client.h"

int main()
{
	//create a smart pointer for client
	auto client = std::make_unique<Client>();
	//flag to signalize when aplication should stop
	bool stop = false;
	//default value for option
	int option = 0;

	cout << "Connected to the server. Please select an option:" << endl;
	while (!stop)
	{
		cout << "1 - Send async streamer object." << endl;
		cout << "2 - Send sync streamer object." << endl;
		cout << "3 - Send async data ." << endl;
		cout << "4 - Send sync data." << endl;
		cout << "5 - Retrieve streamer objects from server async" << endl;
		cout << "6 - Retrieve streamer objects from server sync" << endl;
		cout << "7 - Retrieve data from server async" << endl;
		cout << "8 - Retrieve data from server sync" << endl;
		cout << "9 - Quit " << endl;

		cin >> option;

		while (!cin.good())
		{
			cin.clear();
			cin.ignore(INT32_MAX, '\n');
			option = 0;
		}
		MessageWrapper *mw = new MessageWrapper();
		switch (option)
		{
			//async streamer object to the server
		case 1:
			mw->SetCommand(Commands::COMMAND_SEND_OBJECT_ASYNC);
			SubMenuStreamer(mw);
			client->SendAsync(mw);
			break;
			//sync streamer object to the server
		case 2:
			mw->SetCommand(Commands::COMMAND_SEND_OBJECT_SYNC);
			SubMenuStreamer(mw);
			client->SendSync(mw);
			break;
			//async data to the server (string data)
		case 3:
			mw->SetCommand(Commands::COMMAND_SEND_DATA_ASYNC);
			SubMenuSomeData(mw);
			client->SendAsync(mw);
			break;
			//sync data to the server (string data)
		case 4:
			mw->SetCommand(Commands::COMMAND_SEND_DATA_SYNC);
			SubMenuSomeData(mw);
			client->SendSync(mw);
			break;
			//asynchronous call to retrieve streamers object from server
		case 5:
			mw->SetCommand(Commands::COMMAND_REQUEST_OBJECT_ASYNC);
			client->SendAsync(mw);
			break;
			//synchronous call to retrieve streamers object from server
		case 6:
			mw->SetCommand(Commands::COMMAND_REQUEST_OBJECT_SYNC);
			client->SendSync(mw);
			break;
			//asynchronous call to retrieve data from server
		case 7:
			mw->SetCommand(Commands::COMMAND_REQUEST_DATA_ASYNC);
			client->SendAsync(mw);
			break;
			//synchronous call to retrieve data from server
		case 8:
			mw->SetCommand(Commands::COMMAND_REQUEST_DATA_SYNC);
			client->SendSync(mw);
			break;
		case 9:
			stop = true;
			break;
		default:
			cout << "Invalid option" << endl;
			break;

		}
		//release memory
		delete mw;
	}
}


//Show submenu streamer option
void SubMenuStreamer(MessageWrapper * result)
{
	char name[100];
	int code = 0;
	bool flagStop = false;

	cout << "Enter streamer's name :" << endl;
	cin.ignore();
	cin.getline(name,sizeof(name));
	cout << "Enter streamer's code:" << endl;
	cin >> code;
	while (!cin.good())
	{
		cout << "Invalid values" <<endl;
		cin.clear();
		cin.ignore(INT32_MAX, '\n');
		cout << "Enter streamer's code:" << endl;
		cin >> code;
	} 
	
	const Streamer st(code, name);
	result->SetStreamer(st);
}

//Show submenu to send some data to the server
void SubMenuSomeData(MessageWrapper * result)
{
	char data[100];
	cout << "Write some data to send to the server:" << endl;
	cin.ignore();
	cin.getline(data, sizeof(data));

	result->SetSomeData(data);
}