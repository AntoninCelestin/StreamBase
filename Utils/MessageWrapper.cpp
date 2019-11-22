#include "stdafx.h"
#include "MessageWrapper.h"

MessageWrapper::MessageWrapper(Commands command)
{
	this->_command = command;
}

MessageWrapper::MessageWrapper(Streamer streamer, Commands command)
{
	this->_streamers.push_back(streamer);
	this->_command = command;
}

MessageWrapper::MessageWrapper(string data, Commands command)
{
	this->_data.push_back(data);
	this->_command = command;
}


MessageWrapper::~MessageWrapper()
{
	this->_streamers.empty();
	this->_data.empty();
}

Commands MessageWrapper::GetCommand()
{
	return this->_command;
}

// Only used on server side. Should be improved.
vector<string> MessageWrapper::GetData()
{
	return this->_data;
}

void  MessageWrapper::SetCommand(Commands command)
{
	this->_command = command;
}

// Only used on server side. Should be improved.
void MessageWrapper::SetListOfStreamers(vector<Streamer> streamers) 
{
	this->_streamers = streamers;
}

// Only used on server side. Should be improved.
void MessageWrapper::SetListOfData(vector<string> data)
{
	this->_data = data;
}

void MessageWrapper::SetStreamer(Streamer st)
{
	this->_streamers.push_back(st);
}

int MessageWrapper::GetStreamersCount()
{
	return this->_streamers.size();
}

int MessageWrapper::GetDataCount()
{
	return this->_data.size();
}

//set the regular message in message wrapper
void MessageWrapper::SetSomeData(string data)
{
	this->_data.push_back(data);
}

//deserialize message using boost lib
MessageWrapper* MessageWrapper::DeserializeMessage(LPTSTR request)
{
	std::string val;
	int qtd = lstrlen(request);

	for (int i = 0; i < qtd; i++)
		val.push_back(request[i]);

	std::stringstream MyStringStream;
	MyStringStream << val;

	boost::archive::text_iarchive ia(MyStringStream);

	MessageWrapper *result;
	ia >> result;
	return result;
}

//serialize message using boost lib
std::wstring MessageWrapper::SerializeMessage(const MessageWrapper *obj)
{
	std::stringstream MyStringStream;
	boost::archive::text_oarchive oa(MyStringStream);
	oa << obj;

	std::string data = MyStringStream.str();

	std::wstring stemp = std::wstring(data.begin(), data.end());
	return stemp;
}

vector<Streamer> MessageWrapper::GetStreamers()
{
	return this->_streamers;
}

std::ostream & operator<<(std::ostream & os, const Streamer & st)
{
	return os << ' ' << st.code << ' ' << st.name << '\'' << '"';
}

std::ostream & operator<<(std::ostream & os, const Commands & st)
{
	return os << ' ' << st << '\'' << '"';
}

std::ostream & operator<<(std::ostream & os, const MessageWrapper & wm)
{
	//serialize the list of streamers
	for (auto it : wm._streamers)
	{
		os << it;
	}
	//serialize the list of data
	for (auto dt : wm._data)
	{
		os << dt;
	}
	//serialize the command
	os << wm._command;
	
	return os;
}