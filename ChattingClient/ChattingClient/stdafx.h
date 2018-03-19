#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")

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

#define SERVERIP	"127.0.0.1"
#define SERVERPORT	9000
#define BUF_SIZE	1024


enum EVENT_TYPE
{
	OV_SEND = 1,
	OV_RECV = 2
};


enum MENU_SELECT
{
	CHANNEL_MOVE = 1,
	ROOM_CREATE = 2,
	ENTER_ROOM_INIT = 3,
	IN_ROOM_USER_LIST = 4,
	ROOM_MOVE = 5,
	EXIT_SERVER = 10
};

//#include "C:\Users\song\Documents\ChattingServer-채널매니저x\ChattingServer-채널매니저x\ChattingServer\ChattingServer\protocol.h"
#include "C:\Users\songyikim\Source\Repos\ChattingServer\ChattingServer\protocol.h"
#include "User.h"
#include "ChattingClient.h"
