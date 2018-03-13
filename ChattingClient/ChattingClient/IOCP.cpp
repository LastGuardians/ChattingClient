#include "stdafx.h"

BOOL g_time = true;
BOOL menu_enable = false;

IOCP::IOCP()
{
	//recv_over.wsabuf.buf = reinterpret_cast<CHAR*>(recv_over.iocp_buff);
	//recv_over.wsabuf.len = sizeof(recv_over.iocp_buff);

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;

	ServerConnect();
}


IOCP::~IOCP()
{
}


void IOCP::ServerConnect()
{
	int retval;

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);//윈속 초기화

	//g_socket = socket(AF_INET, SOCK_STREAM, 0);
	g_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (g_socket == INVALID_SOCKET) err_display("socket() Error! : ", WSAGetLastError());

	//connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);

	//WSAEventSelect(g_socket, handle, FD_CONNECT);
	//retval = connect(g_socket, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	retval = WSAConnect(g_socket, (SOCKADDR*)&serveraddr, sizeof(serveraddr), NULL, NULL, NULL, NULL);
	if (retval == SOCKET_ERROR) err_display("connect() Error! : ", WSAGetLastError());
	std::cout << "Server Connected Success!" << std::endl << std::endl;
	//handle = WSACreateEvent();	
	

	std::thread recv_thread{ &IOCP::RecvThread, this };
	SetMenu();
	recv_thread.join();


}

int IOCP::Recvn(SOCKET s, char* buf, int len, int flags)
{
	int rcv = 0;
	char* ptr = buf;
	int left = len;

	while (left > 0)
	{
		rcv = recv(s, ptr, left, flags);
		if (rcv == SOCKET_ERROR)
		{
			return SOCKET_ERROR;
		}
		else if (rcv == 0)
		{
			break;
		}

		left -= rcv;
		ptr += rcv;
	}
	return (len - left);
}

void IOCP::RecvThread()
{
	int retval =0;
	DWORD iobyte, ioflag = 0;
	char buffer[BUF_SIZE];

	while (true)
	{
		//std::cout << "recv thread" << std::endl;
		//retval = recv(g_socket, buffer, sizeof(buffer), ioflag);
		//retval = Recvn(g_socket, buffer, retval, 0);
		retval = WSARecv(g_socket, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
		if (retval == SOCKET_ERROR) {
			err_display("recv() Error! : ", WSAGetLastError());
			break;
		}
		//BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);
		ProcessPacket(reinterpret_cast<unsigned char *>(&recv_buffer));

	}

	closesocket(g_socket);//소켓 닫기    
	WSACleanup();//윈속 해제화
}

// 수신 패킷 처리
void IOCP::ProcessPacket(unsigned char *packet)
{
	switch (packet[0])
	{
	case ENTER_CHANNEL: {
		ProcessEneterChannelPacket(packet);
		break;
	}
	case CREATE_ROOM: {
		ProcessCreateRoomPacket(packet);
		break;
	}
	case CHANGE_CHANNEL: {
		break;
	}
	case NOTIFY_EXIST_ROOM: {
		ProcessNotifyExistRoomPacket(packet);
		break;
	}
	default:
		break;
	}
}

void IOCP::ProcessEneterChannelPacket(unsigned char *packet)
{
	Enter_Channel *enter_packet = reinterpret_cast<Enter_Channel*>(packet);
	std::cout << "======================================" << std::endl;
	std::cout << enter_packet->channelIndex << "번 채널에 입장하셨습니다." << std::endl;
	printf("\n");
	my_id = enter_packet->id;

	menu_enable = true;
	//SetMenu();
}

// 방 생성 패킷 처리
void IOCP::ProcessCreateRoomPacket(unsigned char *packet)
{
	Create_Room *create_packet = reinterpret_cast<Create_Room*>(packet);
	menu_enable = true;
}

// 이미 존재하는 방 패킷 처리
void IOCP::ProcessNotifyExistRoomPacket(unsigned char *packet)
{
	Notify_Exist_Room *exist_packet = reinterpret_cast<Notify_Exist_Room*>(packet);
	char enter_choice;

	printf("\n");
	if (exist_packet->exist == true) {
		std::cout << "[ " << exist_packet->roomIndex << "번 방이 이미 있습니다. ]" << std::endl;
		std::cout << "[ " << exist_packet->roomIndex << "번 방에 입장하시겠습니까? (Y/N) : ";
		std::cin >> enter_choice;

		if (strcmp(&enter_choice, "Y")) {
			
		}
		else if(strcmp(&enter_choice, "N")){

		}
		else
		{

		}

	}
	else {
		std::cout << "[ " << exist_packet->roomIndex << "번 방을 생성하였습니다. ]" << std::endl;
		std::cout << "[ " << exist_packet->roomIndex << "번 방에 입장하였습니다. ]" << std::endl << std::endl;
		
		Room_Chatting chatting_packet;

		std::cout << "보낼 메시지 : ";
		std::cin >> chatting_packet.message;
		SendRoomChatting(chatting_packet.message, exist_packet->roomIndex);
	}

	menu_enable = true;
}

int IOCP::WsaRecv()
{
	DWORD iobyte, ioflag = 0;
	//return WSARecv(g_socket, &recv_over.wsabuf, 1, NULL, &flags, &recv_over.overlap, NULL);
	return WSARecv(g_socket, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
}


void IOCP::SendPacket(unsigned char *packet)
{
	DWORD iobyte = 0;

	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	//int ret = send(g_socket, &send_wsabuf,)

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}	
	//menu_enable = true;
}

// 채널 이동 패킷 전송
void IOCP::SendChannelMovePacket(int channel)
{
	DWORD iobyte = 0;

	Change_Channel* packet = reinterpret_cast<Change_Channel *>(send_buffer);
	packet->type = CHANGE_CHANNEL;
	packet->size = sizeof(Change_Channel);
	packet->id = my_id;
	packet->channelIndex = channel;
	
	send_wsabuf.len = sizeof(*packet);
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}
	menu_enable = true;
}

// 방 생성 패킷 전송
void IOCP::SendCreateRoomPacket(int room)
{
	DWORD iobyte = 0;

	Create_Room* packet = reinterpret_cast<Create_Room *>(send_buffer);
	packet->type = CREATE_ROOM;
	packet->size = sizeof(Create_Room);
	packet->id = my_id;
	packet->roomIndex = room;

	send_wsabuf.len = sizeof(*packet);
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}

	//menu_enable = true;
}

void IOCP::SendRoomChatting(char* message, int room)
{
	DWORD iobyte = 0;

	Room_Chatting* chatting_packet = reinterpret_cast<Room_Chatting *>(send_buffer);
	chatting_packet->type = ROOM_CHATTING;
	chatting_packet->size = sizeof(Room_Chatting);
	chatting_packet->id = my_id;
	chatting_packet->roomIndex = room;
	*chatting_packet->message = *message;

	send_wsabuf.len = sizeof(*chatting_packet);
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}

	//menu_enable = true;
}

void IOCP::CloseSocket()
{
	closesocket(g_socket);
}

void IOCP::err_display(char *msg, int err_no)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);
	printf("%s\n", msg, (LPCTSTR)lpMsgBuf);
	printf("err_no : %d\n", err_no);
	LocalFree(lpMsgBuf);
}


void IOCP::SetMenu()
{
	int choice = 0;

	while (1)
	{
		if (menu_enable == true)
		{
			menu_enable = false;
			std::cout << "======================================" << std::endl;
			std::cout << "1. 채널 이동" << std::endl;
			std::cout << "2. 채팅룸 개설" << std::endl;
			std::cout << "3. 채팅룸 이동" << std::endl;
			std::cout << "4. 채팅룸 안에 있는 유저 목록 보기" << std::endl;
			std::cout << "5. 채팅룸 입장" << std::endl;
			std::cout << "10. 종료" << std::endl;
			std::cout << "======================================" << std::endl;

			std::cout << "메뉴 선택 : ";
			std::cin >> choice;

			switch (choice)
			{
			case CHANNEL_MOVE:
				system("cls");
				int channel;
				std::cout << std::endl;
				std::cout << "몇번 채널로 이동하시겠습니까? (0~4 선택) : " << std::endl;
				std::cin >> channel;
				if (channel > 4 || channel < 0) {
					printf("채널을 잘못 선택하셨습니다.\n");
					continue;
				}
				SendChannelMovePacket(channel);
				std::cout << channel << "번 채널로 이동했습니다." << std::endl;
				break;
			case ROOM_CREATE:
				system("cls");
				int room;
				printf("몇 번방을 만드시겠습니까? \n");
				std::cin >> room;

				SendCreateRoomPacket(room);
				break;
			case ROOM_MOVE:
				system("cls");
				break;
			case IN_ROOM_USER_LIST:
				//system("cls");
				break;
			case ENTER_ROOM:
				int room;
				std::cout<<"몇 번방으로 입장하시겠습니까? "
				//system("cls");
				break;
			case EXIT_SERVER:
				closesocket(g_socket);
				exit(-1);
			default:
				break;
			}
		}
	}
}