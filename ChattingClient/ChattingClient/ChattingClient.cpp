#include "stdafx.h"

BOOL g_time = true;
BOOL menu_enable = true;

ChattingClient::ChattingClient()
{
	//recv_over.wsabuf.buf = reinterpret_cast<CHAR*>(recv_over.iocp_buff);
	//recv_over.wsabuf.len = sizeof(recv_over.iocp_buff);

	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;

	/*bool opend = ServerConnect();

	if (true == opend)
	{

	}*/
}


ChattingClient::~ChattingClient()
{
}


bool ChattingClient::ServerConnect()
{
	int retval;

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);//���� �ʱ�ȭ

	//g_socket = socket(AF_INET, SOCK_STREAM, 0);
	g_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (g_socket == INVALID_SOCKET) err_display("socket() Error! : ", WSAGetLastError());

	//connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	
	//retval = connect(g_socket, (SOCKADDR *)&serveraddr, sizeof(serveraddr));
	retval = WSAConnect(g_socket, (SOCKADDR*)&serveraddr, sizeof(serveraddr), NULL, NULL, NULL, NULL);
	if (retval == SOCKET_ERROR)
	{
		err_display("connect() Error! : ", WSAGetLastError());
		return false;
	}
	std::cout << "Server Connected Success!" << std::endl << std::endl;	

	std::thread recv_thread{ &ChattingClient::RecvThread, this };
	std::thread send_thread{ &ChattingClient::SetMenu, this };
	
	recv_thread.join();
	send_thread.join();

	return true;
}

void ChattingClient::Run()
{
	ChattingThreadStart(1);

	do
	{

	} while (false == isQuit);
}

int ChattingClient::Recvn(SOCKET s, char* buf, int len, int flags)
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

void ChattingClient::RecvThread()
{
	int retval =0;
	DWORD iobyte, ioflag = 0;
	char buffer[BUF_SIZE];

	while (true)
	{
		//std::cout << "recv thread" << std::endl;
		//retval = recv(g_socket, buffer, sizeof(buffer), ioflag);
		retval = WSARecv(g_socket, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
		if (retval == SOCKET_ERROR) {
			err_display("recv() Error! : ", WSAGetLastError());
			return;
		}
		std::vector<char*> recv_vector;
		ProcessPacket(reinterpret_cast<unsigned char *>(&recv_buffer));

		recv_start = true;
	}

	closesocket(g_socket);//���� �ݱ�    
	WSACleanup();//���� ����ȭ
}

// ���� ��Ŷ ó��
void ChattingClient::ProcessPacket(unsigned char *packet)
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
	case ROOM_CHATTING: {
		ProcessRoomChatPacket(packet);
		break;
	}
	case ENTER_ROOM: {
		ProcessEnterRoomPacket(packet);
		break;
	}
	case ROOM_LIST: {
		ProcessRoomListPacket(packet);
		break;
	}
	default:
		break;
	}
}

void ChattingClient::ProcessEneterChannelPacket(unsigned char *packet)
{
	Enter_Channel *enter_packet = reinterpret_cast<Enter_Channel*>(packet);
	std::cout << "======================================" << std::endl;
	std::cout << enter_packet->channelIndex << "�� ä�ο� �����ϼ̽��ϴ�." << std::endl;
	std::cout << "======================================" << std::endl;
	printf("\n");
	my_id = enter_packet->id;		// Ŭ�� id ����

	menu_enable = true;
	//SetMenu();
}

// �� ���� ��Ŷ ó��
void ChattingClient::ProcessCreateRoomPacket(unsigned char *packet)
{
	Create_Room *create_packet = reinterpret_cast<Create_Room*>(packet);
	menu_enable = true;
}

// �����ϴ� ������ Ȯ���ϴ� ��Ŷ ó��
void ChattingClient::ProcessNotifyExistRoomPacket(unsigned char *packet)
{
	Notify_Exist_Room *exist_packet = reinterpret_cast<Notify_Exist_Room*>(packet);
	char enter_choice[5];

	printf("\n");
	if (exist_packet->exist == true)		// �̹� �����ϴ� ���� �� -> ���� ���� �� ���� ���常 ������ ����
	{
		std::cout << "[ " << exist_packet->roomIndex << "�� ���� �̹� �ֽ��ϴ�. ]" << std::endl;
		std::cout << "[ " << exist_packet->roomIndex << "�� �濡 �����Ͻðڽ��ϱ�? (Y/N) : ";
		std::cin >> enter_choice;
		//fgets(enter_choice, MSG_SIZE + 1, stdin);

		if (0 == strcmp(enter_choice, "Y")) {				// ä�ù� ���� O
			SendEnterRoom(exist_packet->roomIndex);			
			menu_enable = true;
		}
		else if(0 == strcmp(enter_choice, "N")){			// ä�ù� ���� X
			menu_enable = true;
			return;
		}
		else
		{
			printf("�� �� �� �����Դϴ�.\n");
			menu_enable = true;
		}
	}
	else									// �������� �ʴ� ���� �� -> ���� ���� �� �ִ� ����
	{
		std::cout << "[ " << exist_packet->roomIndex << "�� ���� �����Ͽ����ϴ�. ]" << std::endl;
		std::cout << "[ " << exist_packet->roomIndex << "�� �濡 �����Ͽ����ϴ�. ]" << std::endl << std::endl;		
		menu_enable = true;
	}

	//menu_enable = true;
}

void ChattingClient::ProcessRoomListPacket(unsigned char * packet)
{
	Room_List *list_packet = reinterpret_cast<Room_List*>(packet);

	if (list_packet->roomCnt == 0)
	{
		std::cout << "������ ���� �����ϴ�." << std::endl;
		return;
	}

	std::cout << "========== ä�ù� ����Ʈ ==========" << std::endl;
	
	//printf("[ ")
	for (int i = 0; i < list_packet->roomCnt; ++i)
	{
		std::cout << "[ "<< list_packet->roomList[i] << " ] ,";
	}
}

void ChattingClient::ProcessRoomChatPacket(unsigned char * packet)
{
	Room_Chatting *chat_packet = reinterpret_cast<Room_Chatting*>(packet);

	printf("\n");
	std::cout << "[ " << chat_packet->id << " ] : ";
	
	for (int i = 0; i < chat_packet->size - 10; ++i)
	{
		std::cout << chat_packet->message[i];
	}
	printf("\n");
	menu_enable = true;
}

void ChattingClient::ProcessEnterRoomPacket(unsigned char * packet)
{
	Enter_Room *enter_packet = reinterpret_cast<Enter_Room*>(packet);

	if(true == enter_packet->isEnter)
		std::cout << enter_packet->roomIndex << "�� �濡 �����߽��ϴ�." << std::endl;
	else
		std::cout << enter_packet->roomIndex << "�� ���� �����ϴ�." << std::endl;
}

int ChattingClient::WsaRecv()
{
	DWORD iobyte, ioflag = 0;
	//return WSARecv(g_socket, &recv_over.wsabuf, 1, NULL, &flags, &recv_over.overlap, NULL);
	return WSARecv(g_socket, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
}

void ChattingClient::SendBroadcast(google::protobuf::MessageLite * msg)
{
	DWORD iobyte = 0;		

	//send_wsabuf.len = sizeof(*packet);
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}
}


void ChattingClient::SendPacket(unsigned char *packet, int size)
{
	DWORD iobyte = 0;
	int ioflag = 0;

	//int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	int ret = send(g_socket, (char*)packet, size, ioflag);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}	
	//menu_enable = true;
}

// ä�� �̵� ��Ŷ ����
void ChattingClient::SendChannelMovePacket(int channel)
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

// �� ���� ��Ŷ ����
void ChattingClient::SendCreateRoomPacket(int room)
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

}

void ChattingClient::SendRoomChatting(char* message, int room, int len)
{
	DWORD iobyte = 0;

	Room_Chatting* chatting_packet = reinterpret_cast<Room_Chatting *>(send_buffer);
	chatting_packet->type = ROOM_CHATTING;
	chatting_packet->id = my_id;
	chatting_packet->roomIndex = room;
	//strncpy(chatting_packet->message, message, len);
	strcpy(chatting_packet->message, message);
	chatting_packet->size = 10 + len;
	//*chatting_packet->message = *message;

	send_wsabuf.len = chatting_packet->size;
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}
}

void ChattingClient::SendEnterRoom(int room)
{
	DWORD iobyte = 0;
	Enter_Room *enter_packet = reinterpret_cast<Enter_Room *>(send_buffer);
	enter_packet->type = ENTER_ROOM;
	enter_packet->size = sizeof(Enter_Room);
	enter_packet->id = my_id;
	enter_packet->roomIndex = room;

	send_wsabuf.len = sizeof(*enter_packet);
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}
}

void ChattingClient::CloseSocket()
{
	closesocket(g_socket);
}

void ChattingClient::err_display(char *msg, int err_no)
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


void ChattingClient::SetMenu()
{
	int choice = 0;
	int room=0;

	while (1)
	{
		if (menu_enable == true)
		{
			//menu_enable = false;
			std::cout << "======================================" << std::endl;
			std::cout << "1. ä�� �̵�" << std::endl;
			std::cout << "2. ä�÷� ����" << std::endl;
			std::cout << "3. ä�÷� ����" << std::endl;
			std::cout << "4. ä�÷� �ȿ� �ִ� ���� ��� ����" << std::endl;
			std::cout << "5. ä�÷� �̵�" << std::endl;
			std::cout << "6. ä�� ä��" << std::endl;
			std::cout << "7. ä�ù� ä��" << std::endl;
			std::cout << "10. ����" << std::endl;
			std::cout << "======================================" << std::endl;

			std::cout << "�޴� ���� : ";
			std::cin >> choice;

			switch (choice)
			{
			case CHANNEL_MOVE:
				//system("cls");
				int channel;
				std::cout << std::endl;
				std::cout << "�� �� ä�η� �̵��Ͻðڽ��ϱ�? (0~4 ����) : " << std::endl;
				std::cin >> channel;
				if (channel > 4 || channel < 0) {
					printf("ä���� �߸� �����߽��ϴ�.\n");
					menu_enable = true;
					break;
				}
				SendChannelMovePacket(channel);
				std::cout << channel << "�� ä�η� �̵��߽��ϴ�." << std::endl;
				break;
			case ROOM_CREATE:
				//system("cls");				
				printf("�� �� ���� ����ðڽ��ϱ�? \n");
				std::cin >> room;
				SendCreateRoomPacket(room);
				menu_enable = false;
				break;
			case ENTER_ROOM_INIT:
				//system("cls");
				std::cout << "�� �� ������ �����Ͻðڽ��ϱ�? ";
				std::cin >> room;
				SendEnterRoom(room);
				//menu_enable = false;
				break;
			case IN_ROOM_USER_LIST:
				//system("cls");
				break;
			case ROOM_MOVE:
				//system("cls");
				break;	
			case CHANNEL_CHATTING_MENU:
				break;
			case ROOM_CHATTING_MENU:
			{
				char send_msg[MSG_SIZE];
				if (0 == room)
				{
					std::cout << "���� �濡 �������� �ʾҽ��ϴ�." << std::endl;
					break;
				}
				std::cout << "���� �޽��� : ";
				fgets(send_msg, 500, stdin);
				//std::cin >> send_msg;
				int chat_len = strlen(send_msg);
				menu_enable = false;
				SendRoomChatting(send_msg, room, chat_len);
				break;
			}
			case EXIT_SERVER:
				closesocket(g_socket);
				exit(-1);
				break;
			default:
				break;
			}
		}
	}
}

void ChattingClient::ChattingMenu(int roomIndex)
{	
	int retval = 0;
	DWORD iobyte = 0;
	char buf[BUF_SIZE], send_msg[MSG_SIZE];

	ChattingThreadStart(roomIndex);
}

void ChattingClient::ChattingCommand(int roomIndex)
{
	char buf[BUF_SIZE], send_msg[MSG_SIZE];
	
	while (true)
	{
		if (true == recv_start)
		{
			Channel_P::channel_chatting chatting_msg;

			//chatting_msg.set_id(my_id);
			//chatting_msg.set_target(0);	// Ŭ���̾�Ʈ���� set_target�� �ʿ����.
			//printf("���� �޽��� : ");
			//scanf("%s", send_msg);
			//chatting_msg.set_message(send_msg);


			//// �̸� �����ؾ� �ϴ� ������ ���̸� �˾Ƴ��� �����Ҵ�
			//int bufSize = chatting_msg.ByteSize();
			//unsigned char* outputBuf = new unsigned char[bufSize];

			//// ���ۿ� ����ȭ
			//int retval = chatting_msg.SerializeToArray(outputBuf, bufSize);
			//if (false == retval)
			//	printf("Serialize Falied! \n");

			//SendPacket(outputBuf, bufSize);
		}
	}
}

void ChattingClient::ChattingThreadStart(int roomIndex)
{
	
	std::thread chatting_thread{ &ChattingClient::ChattingCommand, this, roomIndex };

	chatting_thread.join();
}