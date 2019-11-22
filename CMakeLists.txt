cmake_minimum_required(VERSION 3.8)
project(StreamBase)

set(CMAKE_CXX_STANDARD 17)

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif ()

include_directories(./Utils)
include_directories(./Client)
include_directories(./Server)

set(StreamBaseServer_SourceFiles Server/Server.h Server/stdafx.h Server/targetver.h Utils/Commands.h Utils/MessageWrapper.h Utils/stdafx.h Utils/Streamer.h Utils/targetver.h Server/Server.cpp Server/stdafx.cpp Utils/MessageWrapper.cpp Utils/stdafx.cpp)
add_executable(Server src/ServerMain.cpp ${StreamBaseServer_SourceFiles})

set(StreamBaseClient_SourceFiles Client/Client.h Client/stdafx.h Client/targetver.h Utils/Commands.h Utils/MessageWrapper.h Utils/stdafx.h Utils/Streamer.h Utils/targetver.h Client/Client.cpp Client/stdafx.cpp Utils/MessageWrapper.cpp Utils/stdafx.cpp)
add_executable(Client src/ClientMain.cpp ${StreamBaseClient_SourceFiles})