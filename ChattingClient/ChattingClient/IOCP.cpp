#include "stdafx.h"

BOOL g_time = true;

IOCP::IOCP()
{
	//recv_over.wsabuf.buf = reinterpret_cast<CHAR*>(recv_over.iocp_buff);
	//recv_over.wsabuf.len = sizeof(recv_over.iocp_buff);

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;

	ServerConnect();

	//SetMenu();
}


IOCP::~IOCP()
{
}


void IOCP::ServerConnect()
{
	int retval;

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);//���� �ʱ�ȭ

	g_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (g_socket == INVALID_SOCKET) err_display("socket() Error! : ", WSAGetLastError());

	//connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);

	//WSAEventSelect(g_socket, handle, FD_CONNECT);
	retval = WSAConnect(g_socket, (SOCKADDR*)&serveraddr, sizeof(serveraddr), NULL, NULL, NULL, NULL);
	if (retval == SOCKET_ERROR) err_display("connect() Error! : ", WSAGetLastError());
	std::cout << "Server Connected Success!" << std::endl << std::endl;
	//handle = WSACreateEvent();	
	DWORD iobyte, ioflag = 0;

	while (true)
	{
		if (g_time) {
			g_time = false;

			retval = WSARecv(g_socket, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
			if (retval == SOCKET_ERROR) {
				err_display("WsaRecv() Error! : ", WSAGetLastError());
				break;
			}
			//BYTE *ptr = reinterpret_cast<BYTE *>(recv_buffer);
			ProcessPacket(reinterpret_cast<BYTE *>(recv_buffer));
		}
	}


	//while (1)
	//{
	//	index = WSAWaitForMultipleEvents(1, &handle, FALSE, WSA_INFINITE, FALSE);
	//	//index -= WSA_WAIT_EVENT_0;
	//	WSAEnumNetworkEvents(g_socket, handle, &ev);

	//	switch (ev.lNetworkEvents)
	//	{
	//	case FD_CONNECT:
	//		WSAEventSelect(g_socket, handle, FD_READ);
	//		handle = WSACreateEvent();
	//		break;
	//	case FD_WRITE:
	//		//writeProc(index);
	//		break;
	//	case FD_READ:
	//		retval = WsaRecv();
	//		if (retval == SOCKET_ERROR) {
	//			err_display("WsaRecv() Error! : ", WSAGetLastError());
	//			break;
	//		}
	//		else if (retval == 0)
	//			break;
	//		break;
	//	case FD_CLOSE:
	//		//CloseProc(index);
	//		break;
	//	}
	//}

	
	closesocket(g_socket);//���� �ݱ�    
	WSACleanup();//���� ����ȭ
}


void IOCP::ProcessPacket(unsigned char *packet)
{
	switch (packet[0])
	{
	case ENTER_CHANNEL: {
		ProcessEneterChannelPacket(packet);
		break;
	}
	case ROOM_CREATE: {
		break;
	}
	case ROOM_MOVE: {
		break;
	}
	case IN_ROOM_USER_LIST: {
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
	std::cout << enter_packet->channelIndex << "�� ä�ο� �����ϼ̽��ϴ�." << std::endl;
	my_id = enter_packet->id;
	SetMenu();
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
	g_time = true;
}

void IOCP::SendChannelMovePacket(int room)
{
	DWORD iobyte = 0;

	Change_Channel* packet = reinterpret_cast<Change_Channel *>(send_buffer);
	packet->type = CHANGE_CHANNEL;
	packet->size = sizeof(Change_Channel);
	packet->id = my_id;
	packet->roomIndex = room;
	
	send_wsabuf.len = sizeof(*packet);
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}
	g_time = true;
	//SendPacket(reinterpret_cast<unsigned char*>(packet));
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
		std::cout << "======================================" << std::endl;
		std::cout << "1. ä�� �̵�" << std::endl;
		std::cout << "2. ä�÷� ����" << std::endl;
		std::cout << "3. ä�÷� �̵�" << std::endl;
		std::cout << "4. ä�÷� �ȿ� �ִ� ���� ��� ����" << std::endl;
		std::cout << "5. ����" << std::endl;
		std::cout << "======================================" << std::endl;

		std::cout << "�޴� ���� : ";
		std::cin >> choice;

		switch (choice)
		{
		case CHANNEL_MOVE:
			int room;
			std::cout << std::endl;
			std::cout << "��� ä�η� �̵��Ͻðڽ��ϱ�? (0~4 ����) : ";
			std::cin >> room;
			if (room > 4 || room < 0) {
				printf("���� �߸� �����ϼ̽��ϴ�.\n");
				continue;
			}
			SendChannelMovePacket(room);
			break;
		case ROOM_CREATE:
			system("cls");
			break;
		case ROOM_MOVE:
			system("cls");
			break;
		case IN_ROOM_USER_LIST:
			system("cls");
			break;
		default:
			break;
		}
	}
}