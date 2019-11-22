#pragma once
#include "stdafx.h"
#include <windows.h> 
#include <stdio.h>
#include <vector>
#include <boost/archive/tmpdir.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost\serialization\vector.hpp>
#include <boost/serialization/assume_abstract.hpp>

#define BUFFERS_SIZE 4096

class Streamer
{
	// Necessary for serialization. Otherwise cannot Streamer object members are
	// unaccessible
	friend std::ostream & operator<<(std::ostream &os, const Streamer &gp);
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int /* file_version */) {
		ar &code & name;
	}

public:
	Streamer() = default;
	Streamer::~Streamer() {};
	Streamer(int argCode, std::string argName) : code(argCode), name(argName) { }

	int GetCode() { return this->code; }
	std::string GetName() { return this->name; }
private:
	int code;
	std::string name;
};

typedef struct
{
	OVERLAPPED aoOverlap;
	HANDLE ahPipeInstance;
	TCHAR aTchRequest[BUFFERS_SIZE];
	DWORD aDwRead;
	TCHAR aTchReply[BUFFERS_SIZE];
	DWORD aDwToWrite;
} PipeInstanceType, *PPipeInstanceType;