#pragma once
enum class Commands
{
	COMMAND_RESPONSE_SERVER_ERROR,
	COMMAND_RESPONSE_SERVER_OK,
	COMMAND_SEND_DATA_ASYNC,
	COMMAND_SEND_DATA_SYNC,
	COMMAND_SEND_OBJECT_ASYNC,
	COMMAND_SEND_OBJECT_SYNC,
	COMMAND_REQUEST_DATA_ASYNC,
	COMMAND_REQUEST_DATA_SYNC,
	COMMAND_REQUEST_OBJECT_ASYNC,
	COMMAND_REQUEST_OBJECT_SYNC,
	COMMAND_RESPONSE_LIST_OF_STREAMERS,
	COMMAND_RESPONSE_LIST_OF_DATA
};

