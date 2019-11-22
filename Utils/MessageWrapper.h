#pragma once
#include "Commands.h"
#include "Streamer.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class MessageWrapper
{

	friend std::ostream & operator<<(std::ostream &os, const Streamer &st);
	friend std::ostream & operator<<(std::ostream &os, const Commands &cd);
	friend std::ostream & operator<<(std::ostream &os, const MessageWrapper &wm);

	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int /* file_version */) {
		ar &_command & _streamers &_data;
	}

public:
	MessageWrapper() = default;
	MessageWrapper(Commands command);
	MessageWrapper(Streamer streamer , Commands command);
	MessageWrapper(string data, Commands command);
	~MessageWrapper();
	Commands GetCommand();
	vector<Streamer> GetStreamers();
	vector<string> GetData();
	void SetListOfStreamers(vector<Streamer> streamers);
	void SetListOfData(vector<string> data);
	void SetStreamer(Streamer st);
	void SetCommand(Commands command);
	int GetStreamersCount();
	int GetDataCount();
	void SetSomeData(string data);
	static MessageWrapper* DeserializeMessage(LPTSTR request);
	static std::wstring SerializeMessage(const MessageWrapper *obj);
private:
	Commands _command;
	vector<Streamer> _streamers;
	vector<string> _data;
};

