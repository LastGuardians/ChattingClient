#pragma once
#define PROTOBUF_USE_DLLS
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#pragma comment(lib, "libprotobufd.lib")

#include <iostream>
#include <thread>
#include <WinSock2.h>
#include <winsock.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <thread>
#include <process.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <google/protobuf/text_format.h>
#include "Protocols.pb.h"

#define SERVERIP	"127.0.0.1"
#define SERVERPORT	9000
#define BUF_SIZE	1024

using namespace google;


enum EVENT_TYPE
{
	OV_SEND = 1,
	OV_RECV = 2
};


enum MENU_SELECT
{
	CHANNEL_MOVE			= 1,
	ROOM_CREATE				= 2,
	ENTER_ROOM_INIT			= 3,
	IN_ROOM_USER_LIST		= 4,
	ROOM_MOVE				= 5,
	CHANNEL_CHATTING_MENU	= 6,
	ROOM_CHATTING_MENU		= 7,
	LEAVE_ROOM_MENU			= 8,
	EXIT_SERVER				= 10
};

//#include "C:\Users\song\Documents\Visual Studio 2015\Projects\ChattingServer\ChattingServer\protocol.h"
#include "C:\Users\songyikim\Source\Repos\ChattingServer\ChattingServer\protocol.h"
#include "ChattingClient.h"