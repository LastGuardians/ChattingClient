#include "stdafx.h"

static bool	menu_enable = false;
WSABUF		send_wsabuf;

ChattingClient::ChattingClient()
{
	send_wsabuf.buf = _send_buffer;
	send_wsabuf.len = BUF_SIZE;
	_recv_wsabuf.buf = _recv_buffer;
	_recv_wsabuf.len = BUF_SIZE;
}

ChattingClient::~ChattingClient()
{
}


bool ChattingClient::ServerConnect()
{
	int retval;

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);//���� �ʱ�ȭ

	_g_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (_g_socket == INVALID_SOCKET) err_display("socket() Error! : ", WSAGetLastError());

	//connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	
	retval = WSAConnect(_g_socket, (SOCKADDR*)&serveraddr, sizeof(serveraddr), NULL, NULL, NULL, NULL);
	if (retval == SOCKET_ERROR)
	{
		err_display("connect() Error! : ", WSAGetLastError());
		return false;
	}
	std::cout << "Server Connected Success!" << std::endl << std::endl;	
	return true;
}

void ChattingClient::ThreadStart()
{
	std::thread recv_thread{ &ChattingClient::RecvThread, this };
	//LoginToServer();
	std::thread send_thread{ &ChattingClient::MenuStart, this };

	recv_thread.join();
	send_thread.join();
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

	while (true)
	{
		retval = WSARecv(_g_socket, &_recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
		if (retval == SOCKET_ERROR) {
			err_display("recv() Error! : ", WSAGetLastError());
			return;
		}
		protobuf::io::ArrayInputStream input_array_stream(_recv_buffer, BUF_SIZE);
		protobuf::io::CodedInputStream input_coded_stream(&input_array_stream);

		// ��Ŷ �м�
		PacketProcess(input_coded_stream);
	}

	closesocket(_g_socket);//���� �ݱ�    
	WSACleanup();//���� ����ȭ
}

void ChattingClient::PacketProcess(protobuf::io::CodedInputStream & input_stream)
{
	MessageHeader messageHeader;

	// ����� �о
	while (input_stream.ReadRaw(&messageHeader, MessageHeaderSize))
	{
		// ���� �＼�� �Ҽ� �ִ� ���� �����Ϳ� ���� ���̸� �˾Ƴ�
		const void* payload_ptr = NULL;
		int remainSize = 0;
		input_stream.GetDirectBufferPointer(&payload_ptr, &remainSize);
		if (remainSize < (signed)messageHeader.size)
			break;
		
		// �޼��� ��ü�� �о�� ���� ��Ʈ���� ����
		protobuf::io::ArrayInputStream payload_array_stream(payload_ptr, messageHeader.size);
		protobuf::io::CodedInputStream payload_input_stream(&payload_array_stream);


		// �޼��� ��ü ������ ��ŭ ������ ����
		input_stream.Skip(messageHeader.size);

		// �޼��� �������� ������ȭ�ؼ� ������ �޼��带 ȣ������
		switch (messageHeader.type)
		{
		case Protocols::ENTER_CHANNEL:
		{
			Protocols::Enter_Channel message;
		
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessEneterChannelPacket(message);
			_my_id = message.id();		// Ŭ�� id ����
			_channel_index = message.channelindex();
			break;
		}
	/*	case Protocols::USER_LOGIN:
		{
			Protocols::User_Login message;
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessLoginPacket(message);
			break;
		}*/
		case Protocols::NOTIFY_ENTER_ROOM:
		{
			Protocols::Notify_Enter_Room message;
			
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessNotifyEnterRoomPacket(message);
			break;
		}
		case Protocols::NOTIFY_LEAVE_ROOM:
		{
			Protocols::Notify_Leave_Room message;
			
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessNotifyLeaveRoomPacket(message);
			break;
		}
		case Protocols::NOTIFY_EXIST_ROOM:
		{
			Protocols::Notify_Exist_Room message;
		
			message.ParseFromCodedStream(&payload_input_stream);
			if(true == ProcessNotifyExistRoomPacket(message))
				_room_index = message.roomindex();
			break;
		}
		case Protocols::ROOM_CHATTING:
		{
			Protocols::Room_Chatting message;
			
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessRoomChatPacket(message);
			break;
		}
		case Protocols::ROOM_LIST:
		{
			Protocols::Room_List message;
			
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessRoomListPacket(message);
			break;
		}
		case Protocols::CHANNEL_CHATTING:
		{
			Protocols::Channel_Chatting message;
		
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessChannelChatPacket(message);
			break;
		}
		case Protocols::ENTER_ROOM:
		{
			Protocols::Enter_Room message;
	
			message.ParseFromCodedStream(&payload_input_stream);
			if(true == ProcessEnterRoomPacket(message))	// ���� ���� ����
				_room_index = message.roomindex();		// roomindex ����
			break;
		}
		}
	}
}


void ChattingClient::ProcessEneterChannelPacket(const Protocols::Enter_Channel message) 
{
	std::cout << "======================================" << std::endl;
	std::cout << message.channelindex() << "�� ä�ο� �����ϼ̽��ϴ�." << std::endl;
	std::cout << "======================================" << std::endl << std::endl;
	menu_enable = true;
	//MenuStart();
}

//void ChattingClient::ProcessLoginPacket(const Protocols::User_Login message) const
//{
//	if (message.has_user_id() == true)
//	{
//		if (true == message.success())
//		{
//			std::cout << message.user_id() << " ���� �α��� ����!" << std::endl;
//			menu_enable = true;
//		}
//		else
//		{
//			std::cout << message.user_id() << " ���� �α��� ����!" << std::endl;
//			LoginToServer();
//		}
//	}
//}

// �� ���� ��Ŷ ó��
void ChattingClient::ProcessCreateRoomPacket(const Protocols::Create_Room message) const
{
	menu_enable = true;
}

// �����ϴ� ������ Ȯ���ϴ� ��Ŷ ó��
bool ChattingClient::ProcessNotifyExistRoomPacket(const Protocols::Notify_Exist_Room message) const
{
	char enter_choice[5];

	std::cout << std::endl;
	if (message.exist() == true)		// �̹� �����ϴ� ���� �� -> ���� ���� �� ���� ���常 ������ ����
	{
		std::cout << "[ " << message.roomindex() << "�� ���� �̹� �ֽ��ϴ�. ]" << std::endl;
		std::cout << "[ " << message.roomindex() << "�� �濡 �����Ͻðڽ��ϱ�? (Y/N) : ";
		std::cin >> enter_choice;

		if (0 == strcmp(enter_choice, "Y")) {				// ä�ù� ���� O
			Protocols::Enter_Room enter_room;
			enter_room.set_roomindex(message.roomindex());
			enter_room.set_type(Protocols::ENTER_ROOM);

			SendPacketAssemble(enter_room.type(), enter_room);
			menu_enable = true;
			return true;
		}
		else if(0 == strcmp(enter_choice, "N")){			// ä�ù� ���� X
			menu_enable = true;
			return false;
		}
		else
		{
			printf("�� �� �� �����Դϴ�.\n");
			menu_enable = true;
			return false;
		}
	}
	else									// �������� �ʴ� ���� �� -> ���� ���� �� �ִ� ����
	{
		std::cout << "[ " << message.roomindex() << "�� ���� �����Ͽ����ϴ�. ]" << std::endl;
		std::cout << "[ " << message.roomindex() << "�� �濡 �����Ͽ����ϴ�. ]" << std::endl << std::endl;
		menu_enable = true;
		return true;
	}

}

void ChattingClient::ProcessRoomListPacket(const Protocols::Room_List message) const
{	
	printf("\n");
	std::cout << "========== " << message.roomindex() << " �� �� ���� ����Ʈ ==========" << std::endl;

	for (int i = 0; i < message.userlist_size(); ++i)
	{
		if (message.userlist(i) == _my_id)
		{
			std::cout << "[ " << message.userlist(i) << "�� ���� ] - Me" << std::endl;
		}
		else
			std::cout << "[ "<< message.userlist(i) << "�� ���� ]" << std::endl;
	}
	std::cout << "========================================= " << std::endl << std::endl;

	menu_enable = true;
}

void ChattingClient::ProcessRoomChatPacket(const Protocols::Room_Chatting message) const
{
	if (message.id() == _my_id)
	{
		menu_enable = true;
		return;
	}
	printf("\n");
	std::cout << "[" << _room_index <<"�� ��] [ " << message.id() << " ] �� ���� : ";
	
	std::cout << message.message() << std::endl;
	printf("\n");
	menu_enable = true;
}

bool ChattingClient::ProcessEnterRoomPacket(const Protocols::Enter_Room message) const
{	
	printf("\n");
	if (true == message.isenter())
	{
		std::cout << message.roomindex() << "�� �濡 �����߽��ϴ�." << std::endl;
		menu_enable = true;
		return true;
	}
	else
	{
		std::cout << message.roomindex() << "�� ���� �����ϴ�." << std::endl;
		menu_enable = true;
		return false;
	}
}

void ChattingClient::ProcessChannelChatPacket(const Protocols::Channel_Chatting message) const
{
	if (message.id() == _my_id)
	{
		menu_enable = true;
		return;
	}
	printf("\n");
	std::cout << "[" << _channel_index <<" ä��] [ " << message.id() << " ] �� ���� : ";

	std::cout << message.message() << std::endl;
	printf("\n");
	menu_enable = true;
}

void ChattingClient::ProcessNotifyEnterRoomPacket(const Protocols::Notify_Enter_Room message) const
{
	printf("\n");
	std::cout << "[ " << message.id() << " ] �� ������ �����Ͽ����ϴ�." << std::endl;
}

void ChattingClient::ProcessNotifyLeaveRoomPacket(const Protocols::Notify_Leave_Room message) const
{
	printf("\n");
	std::cout << "[ " << message.id() << " ] �� ������ �����Ͽ����ϴ�." << std::endl;
}

int ChattingClient::WsaRecv()
{
	DWORD iobyte, ioflag = 0;
	return WSARecv(_g_socket, &_recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
}

//void ChattingClient::SendLoginPacket(char * id, int len) const
//{
//	DWORD iobyte = 0;
//
//	Protocols::User_Login login;
//	login.set_user_id(id);
//
//	size_t bufSize = login.ByteSizeLong();
//	char* outputBuf = new char[bufSize];
//
//	// ��� ����
//	MessageHeader header;
//	header.size = MessageHeaderSize + bufSize;
//	header.type = Protocols::USER_LOGIN;
//	char* header_seri = reinterpret_cast<char*>(&header);
//
//	int rtn = login.SerializeToArray(outputBuf, bufSize);
//
//	// ���� ���� ����
//	char* resultBuf = new char[bufSize + MessageHeaderSize];
//	memcpy(resultBuf, header_seri, MessageHeaderSize);
//	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);
//
//	send_wsabuf.buf = resultBuf;
//	send_wsabuf.len = header.size;
//	int ret = WSASend(_g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
//
//	if (SOCKET_ERROR == ret) {
//		if (ERROR_IO_PENDING != WSAGetLastError()) {
//			err_display("WSASend() Error! : ", WSAGetLastError());
//		}
//	}
//}

void ChattingClient::SendPacketAssemble(int type, google::protobuf::Message & msg) const
{
	size_t bufSize = msg.ByteSizeLong();
	char resultBuf[100];
	int size = bufSize + MessageHeaderSize;

	// ��� ����
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = type;

	char* header_seri = reinterpret_cast<char*>(&header);

	memcpy(resultBuf, header_seri, MessageHeaderSize);
	msg.SerializeToArray(resultBuf + MessageHeaderSize, bufSize);

	DWORD iobyte = 0;
	send_wsabuf.buf = resultBuf;
	send_wsabuf.len = header.size;
	int ret = WSASend(_g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}
	//menu_enable = true;
}


void ChattingClient::CloseSocket()
{
	closesocket(_g_socket);
}

void ChattingClient::err_display(char *msg, int err_no) const
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
	printf("%s\n", msg);
	printf("err_no : %d\n", err_no);
	LocalFree(lpMsgBuf);
}


void ChattingClient::MenuStart()
{
	int choice = 0;
	while (1)
	{
		if (menu_enable == true)
		{
			std::cout << "======================================" << std::endl;
			std::cout << "1. ä�� �̵�" << std::endl;
			std::cout << "2. ä�ù� ����" << std::endl;
			std::cout << "3. ä�ù� ����" << std::endl;
			std::cout << "4. ä�ù� �ȿ� �ִ� ���� ��� ����" << std::endl;
			std::cout << "6. ä�� ä��" << std::endl;
			std::cout << "7. �� ä��" << std::endl;
			std::cout << "8. ä�ù� ����" << std::endl;
			std::cout << "10. ����" << std::endl;
			std::cout << "======================================" << std::endl;

			std::cout << "�޴� ���� : ";
			std::cin >> choice;
			if (choice / 1.00 != int(choice))
			{
				std::cout << "�޴��� �ٽ� �����ϼ���." << std::endl;
				continue;
			}
			
			switch (choice)
			{
			case CHANNEL_MOVE:
				MenuChannelMove();
				break;
			case ROOM_CREATE:
				MenuRoomCreate();
				break;
			case ENTER_ROOM_MENU:
				MenuEnterRoom();
				break;
			case IN_ROOM_USER_LIST:
				MenuRoomUserList();
				break;
			case CHANNEL_CHATTING_MENU:
				MenuChannelChatting();
				break;
			case ROOM_CHATTING_MENU:
				MenuRoomChatting();
				break;
			case LEAVE_ROOM_MENU:
				MenuLeaveRoom();
				break;
			case EXIT_SERVER:
				closesocket(_g_socket);
				exit(-1);
				break;
			default:
				std::cout << "�޴��� �ٽ� �����ϼ���." << std::endl;
				break;
			}
		}
	}
}

void ChattingClient::MenuChannelMove()
{
	system("cls");
	int channel;
	std::cout << std::endl;
	if (0 != _room_index)
	{
		std::cout << "�濡�� ������ �Ŀ� ä���� �̵��ϼ���." << std::endl;
		return;
	}
	while (getchar() != '\n');
	std::cout << "�� �� ä�η� �̵��Ͻðڽ��ϱ�? (0 ~ 4 ����) : " << std::endl;
	std::cin >> channel;
	if (channel > 4 || channel < 0) {
		printf("ä���� �߸� �����߽��ϴ�.\n\n");
		menu_enable = true;
		return;
	}

	Protocols::Change_Channel change;
	change.set_channelindex(channel);
	change.set_type(Protocols::CHANGE_CHANNEL);

	SendPacketAssemble(change.type(), change);
	//SendChannelMovePacket(channel);
	std::cout << channel << "�� ä�η� �̵��߽��ϴ�." << std::endl << std::endl;
	_channel_index = channel;
}

void ChattingClient::MenuRoomCreate()
{
	int room;
	system("cls");
	if (0 != _room_index)
	{
		std::cout << "�̹� �濡 ���� ���Դϴ�." << std::endl;
		return;
	}
	while (getchar() != '\n');
	printf("�� �� ���� ����ðڽ��ϱ�? \n");
	std::cin >> room;
	if (room < 1)
	{
		std::cout << "���� 1�� ���� ���� �����մϴ�." << std::endl << std::endl;
		return;
	}
	menu_enable = false;

	Protocols::Create_Room create_room;
	create_room.set_roomindex(room);
	create_room.set_type(Protocols::CREATE_ROOM);

	SendPacketAssemble(create_room.type(), create_room);
	//SendCreateRoomPacket(room);
}

void ChattingClient::MenuEnterRoom()
{
	int room;
	if (0 != _room_index)
	{
		std::cout << "�̹� �濡 ���� ���Դϴ�." << std::endl << std::endl;
		return;
	}
	system("cls");
	while (getchar() != '\n');
	std::cout << "�� �� ������ �����Ͻðڽ��ϱ�? ";
	std::cin >> room;
	menu_enable = false;

	Protocols::Enter_Room enter_room;
	enter_room.set_roomindex(room);
	enter_room.set_type(Protocols::ENTER_ROOM);

	SendPacketAssemble(enter_room.type(), enter_room);
	//SendEnterRoomPacket(room);
}

void ChattingClient::MenuRoomUserList()
{
	system("cls");
	if (0 == _room_index)
	{
		std::cout << "���� �濡 �������� �ʾҽ��ϴ�." << std::endl << std::endl;
		return;
	}
	menu_enable = false;

	Protocols::Room_List room_list;
	room_list.set_roomindex(_room_index);
	room_list.set_type(Protocols::ROOM_LIST);

	SendPacketAssemble(room_list.type(), room_list);
	//SendRoomUserListPacket(_room_index);
}

void ChattingClient::MenuChannelChatting()
{
	char send_msg[MSG_SIZE];

	while (getchar() != '\n');
	fputs("���� �޽��� : ", stdout);
	//getchar();								// �Է��� ������ ���
	std::cin.getline(send_msg, MSG_SIZE, '\n');

	int chat_len = strlen(send_msg);
	menu_enable = false;

	Protocols::Channel_Chatting chatting;
	chatting.set_message(send_msg);
	chatting.set_type(Protocols::CHANNEL_CHATTING);

	SendPacketAssemble(chatting.type(), chatting);
	//SendChannelChattingPacket(send_msg, _channel_index, chat_len);
}

void ChattingClient::MenuRoomChatting()
{
	char send_msg[MSG_SIZE];
	if (0 == _room_index)
	{
		std::cout << "���� �濡 �������� �ʾҽ��ϴ�." << std::endl << std::endl;
		return;
	}
	while (getchar() != '\n');
	std::cout << "���� �޽��� : ";
	std::cin.getline(send_msg, MSG_SIZE, '\n');
	int chat_len = strlen(send_msg);
	menu_enable = false;

	Protocols::Room_Chatting chatting;
	chatting.set_message(send_msg);
	chatting.set_type(Protocols::ROOM_CHATTING);

	SendPacketAssemble(chatting.type(), chatting);
	//SendRoomChattingPacket(send_msg, _room_index, chat_len);
}

void ChattingClient::MenuLeaveRoom()
{
	system("cls");
	if (0 == _room_index)
	{
		std::cout << "���� �濡 �������� �ʾҽ��ϴ�." << std::endl << std::endl;
		return;
	}
	std::cout << _room_index << " �� �濡�� �����մϴ�." << std::endl << std::endl;

	Protocols::Leave_Room leave_room;
	leave_room.set_roomindex(_room_index);
	leave_room.set_type(Protocols::LEAVE_ROOM);

	SendPacketAssemble(leave_room.type(), leave_room);
	//SendLeaveRoomPacket(_room_index);
	_room_index = 0;
}

void ChattingClient::LoginToServer() const
{
	char send_msg[MSG_SIZE];

	while (getchar() != '\n');
	std::cout << "���̵� �Է��ϼ���. ";
	
	std::cin.getline(send_msg, MSG_SIZE, '\n');
	int login_len = strlen(send_msg);
	//SendLoginPacket(send_msg, login_len);
}
