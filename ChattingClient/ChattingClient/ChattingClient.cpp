#include "stdafx.h"

BOOL	g_time = true;
bool	menu_enable = false;

WSABUF	send_wsabuf;

ChattingClient::ChattingClient()
{
	send_wsabuf.buf = send_buffer;
	send_wsabuf.len = BUF_SIZE;
	recv_wsabuf.buf = recv_buffer;
	recv_wsabuf.len = BUF_SIZE;
}


ChattingClient::~ChattingClient()
{
}


bool ChattingClient::ServerConnect()
{
	int retval;

	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);//윈속 초기화

	g_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	if (g_socket == INVALID_SOCKET) err_display("socket() Error! : ", WSAGetLastError());

	//connect()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = inet_addr(SERVERIP);
	serveraddr.sin_port = htons(SERVERPORT);
	
	retval = WSAConnect(g_socket, (SOCKADDR*)&serveraddr, sizeof(serveraddr), NULL, NULL, NULL, NULL);
	if (retval == SOCKET_ERROR)
	{
		err_display("connect() Error! : ", WSAGetLastError());
		return false;
	}
	std::cout << "Server Connected Success!" << std::endl << std::endl;	

	std::thread recv_thread{ &ChattingClient::RecvThread, this };
	LoginToServer();
	std::thread send_thread{ &ChattingClient::SetMenu, this };
	
	recv_thread.join();
	send_thread.join();

	return true;
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
		retval = WSARecv(g_socket, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
		if (retval == SOCKET_ERROR) {
			err_display("recv() Error! : ", WSAGetLastError());
			return;
		}
		protobuf::io::ArrayInputStream input_array_stream(recv_buffer, BUF_SIZE);
		protobuf::io::CodedInputStream input_coded_stream(&input_array_stream);

		// 패킷 분석
		PacketProcess(input_coded_stream);

		menu_enable = true;
		recv_start = true;
	}

	closesocket(g_socket);//소켓 닫기    
	WSACleanup();//윈속 해제화
}

void ChattingClient::PacketProcess(protobuf::io::CodedInputStream & input_stream)
{
	MessageHeader messageHeader;

	// 헤더를 읽어냄
	while (input_stream.ReadRaw(&messageHeader, MessageHeaderSize))
	{
		// 직접 억세스 할수 있는 버퍼 포인터와 남은 길이를 알아냄
		const void* payload_ptr = NULL;
		int remainSize = 0;
		input_stream.GetDirectBufferPointer(&payload_ptr, &remainSize);
		if (remainSize < (signed)messageHeader.size)
			break;
		
		// 메세지 본체를 읽어내기 위한 스트림을 생성
		protobuf::io::ArrayInputStream payload_array_stream(payload_ptr, messageHeader.size);
		protobuf::io::CodedInputStream payload_input_stream(&payload_array_stream);


		// 메세지 본체 사이즈 만큼 포인터 전진
		input_stream.Skip(messageHeader.size);

		// 메세지 종류별로 역직렬화해서 적절한 메서드를 호출해줌
		switch (messageHeader.type)
		{
		case Protocols::ENTER_CHANNEL:
		{
			Protocols::Enter_Channel message;
		
			message.ParseFromCodedStream(&payload_input_stream);
			ProcessEneterChannelPacket(message);
			my_id = message.id();		// 클라 id 지정
			channel_index = message.channelindex();
			break;
		}
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
				room_index = message.roomindex();
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
			if(true == ProcessEnterRoomPacket(message))	// 방이 있을 때만
				room_index = message.roomindex();		// roomindex 지정
			break;
		}
		}
	}
}

// 수신 패킷 처리
void ChattingClient::ProcessPacket(unsigned char *packet)
{
	//switch (packet[0])
	//{
	//case ENTER_CHANNEL: {
	//	//ProcessEneterChannelPacket(packet);
	//	break;
	//}
	//case CREATE_ROOM: {
	//	ProcessCreateRoomPacket(packet);
	//	break;
	//}
	//case CHANGE_CHANNEL: {
	//	break;
	//}
	//case NOTIFY_ENTER_ROOM: {
	//	ProcessNotifyEnterRoomPacket(packet);
	//	break;
	//}
	//case NOTIFY_LEAVE_ROOM: {
	//	ProcessNotifyLeaveRoomPacket(packet);
	//	break;
	//}
	//case NOTIFY_EXIST_ROOM: {
	//	ProcessNotifyExistRoomPacket(packet);
	//	break;
	//}
	//case ROOM_CHATTING: {
	//	ProcessRoomChatPacket(packet);
	//	break;
	//}
	//case ENTER_ROOM: {
	//	ProcessEnterRoomPacket(packet);
	//	break;
	//}
	//case CHANNEL_CHATTING: {
	//	ProcessChannelChatPacket(packet);
	//	break;
	//}
	//case ROOM_LIST: {
	//	ProcessRoomListPacket(packet);
	//	break;
	//}
	//default:
	//	break;
	//}
}

void ChattingClient::ProcessEneterChannelPacket(const Protocols::Enter_Channel message) const 
{
	std::cout << "======================================" << std::endl;
	std::cout << message.channelindex() << "번 채널에 입장하셨습니다." << std::endl;
	std::cout << "======================================" << std::endl;
	printf("\n");

	menu_enable = true;
	//SetMenu();
}

// 방 생성 패킷 처리
void ChattingClient::ProcessCreateRoomPacket(const Protocols::Create_Room message) const
{
	menu_enable = true;
}

// 존재하는 방인지 확인하는 패킷 처리
bool ChattingClient::ProcessNotifyExistRoomPacket(const Protocols::Notify_Exist_Room message) const
{
	char enter_choice[5];

	printf("\n");
	if (message.exist() == true)		// 이미 존재하는 방일 때 -> 방을 만들 수 없고 입장만 가능한 상태
	{
		std::cout << "[ " << message.roomindex() << "번 방이 이미 있습니다. ]" << std::endl;
		std::cout << "[ " << message.roomindex() << "번 방에 입장하시겠습니까? (Y/N) : ";
		std::cin >> enter_choice;

		if (0 == strcmp(enter_choice, "Y")) {				// 채팅방 입장 O
			SendEnterRoomPacket(message.roomindex());
			menu_enable = true;
			return true;
		}
		else if(0 == strcmp(enter_choice, "N")){			// 채팅방 입장 X
			menu_enable = true;
			return false;
		}
		else
		{
			printf("잘 못 된 문자입니다.\n");
			menu_enable = true;
			return false;
		}
	}
	else									// 존재하지 않는 방일 때 -> 방을 만들 수 있는 상태
	{
		std::cout << "[ " << message.roomindex() << "번 방을 생성하였습니다. ]" << std::endl;
		std::cout << "[ " << message.roomindex() << "번 방에 입장하였습니다. ]" << std::endl << std::endl;
		menu_enable = true;
		return true;
	}

}

void ChattingClient::ProcessRoomListPacket(const Protocols::Room_List message) const
{	
	printf("\n");
	std::cout << "========== " << message.roomindex() << " 번 방 유저 리스트 ==========" << std::endl;

	for (int i = 0; i < message.userlist_size(); ++i)
	{
		if (message.userlist(i) == my_id)
		{
			std::cout << "[ " << message.userlist(i) << "번 유저 ] - Me" << std::endl;
		}
		else
			std::cout << "[ "<< message.userlist(i) << "번 유저 ]" << std::endl;
	}
	std::cout << "========================================= " << std::endl << std::endl;

	menu_enable = true;
}

void ChattingClient::ProcessRoomChatPacket(const Protocols::Room_Chatting message) const
{
	if (message.id() == my_id)
	{
		menu_enable = true;
		return;
	}
	printf("\n");
	std::cout << "[" << room_index <<"번 방] [ " << message.id() << " ] 번 유저 : ";
	
	std::cout << message.message() << std::endl;
	printf("\n");
	menu_enable = true;
}

bool ChattingClient::ProcessEnterRoomPacket(const Protocols::Enter_Room message) const
{	
	printf("\n");
	if (true == message.isenter())
	{
		std::cout << message.roomindex() << "번 방에 입장했습니다." << std::endl;		
		return true;
	}
	else
	{
		std::cout << message.roomindex() << "번 방이 없습니다." << std::endl;
		return false;
	}

	menu_enable = true;
}

void ChattingClient::ProcessChannelChatPacket(const Protocols::Channel_Chatting message) const
{
	if (message.id() == my_id)
	{
		menu_enable = true;
		return;
	}
	printf("\n");
	std::cout << "[" << channel_index <<" 채널] [ " << message.id() << " ] 번 유저 : ";

	std::cout << message.message() << std::endl;
	printf("\n");
	menu_enable = true;
}

void ChattingClient::ProcessNotifyEnterRoomPacket(const Protocols::Notify_Enter_Room message) const
{
	printf("\n");
	std::cout << "[ " << message.id() << " ] 번 유저가 입장하였습니다." << std::endl;
}

void ChattingClient::ProcessNotifyLeaveRoomPacket(const Protocols::Notify_Leave_Room message) const
{
	printf("\n");
	std::cout << "[ " << message.id() << " ] 번 유저가 퇴장하였습니다." << std::endl;
}

int ChattingClient::WsaRecv()
{
	DWORD iobyte, ioflag = 0;
	return WSARecv(g_socket, &recv_wsabuf, 1, &iobyte, &ioflag, NULL, NULL);
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
}

void ChattingClient::SendLoginPacket(char * id, int len)
{
	DWORD iobyte = 0;

	Protocols::User_Login login;
	login.set_user_id(id);

	size_t bufSize = login.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::USER_LOGIN;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = login.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	char* resultBuf = new char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);

	send_wsabuf.buf = resultBuf;
	send_wsabuf.len = header.size;
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}
}

// 채널 이동 패킷 전송
void ChattingClient::SendChannelMovePacket(int channel)
{
	DWORD iobyte = 0;

	Protocols::Change_Channel change;
	change.set_channelindex(channel);

	size_t bufSize = change.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::CHANGE_CHANNEL;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = change.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	char* resultBuf = new char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);

	send_wsabuf.buf = resultBuf;
	send_wsabuf.len = header.size;
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}
	menu_enable = true;
}

void ChattingClient::SendChannelChattingPacket(char * message, int channel, int len)
{
	DWORD iobyte = 0;

	Protocols::Channel_Chatting chatting;
	chatting.set_message(message);

	size_t bufSize = chatting.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::CHANNEL_CHATTING;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = chatting.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	char* resultBuf = new char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);

	send_wsabuf.buf = resultBuf;
	send_wsabuf.len = header.size;
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	
	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}
	menu_enable = true;
}

// 방 생성 패킷 전송
void ChattingClient::SendCreateRoomPacket(int room)
{
	Protocols::Create_Room create_room;
	create_room.set_roomindex(room);

	size_t bufSize = create_room.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::CREATE_ROOM;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = create_room.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	char* resultBuf = new char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);

	DWORD iobyte = 0;
	send_wsabuf.buf = resultBuf;
	send_wsabuf.len = header.size;
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);
	
	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}
}


void ChattingClient::SendRoomUserListPacket(int room)
{
	Protocols::Room_List room_list;
	room_list.set_roomindex(room);

	size_t bufSize = room_list.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::ROOM_LIST;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = room_list.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	char* resultBuf = new char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);

	DWORD iobyte = 0;
	send_wsabuf.buf = resultBuf;
	send_wsabuf.len = header.size;
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}
}

void ChattingClient::SendRoomChattingPacket(char* message, int room, int len)
{
	DWORD iobyte = 0;

	Protocols::Room_Chatting chatting;
	chatting.set_message(message);

	size_t bufSize = chatting.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::ROOM_CHATTING;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = chatting.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	char* resultBuf = new char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);

	send_wsabuf.buf = resultBuf;
	send_wsabuf.len = header.size;
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}
}

void ChattingClient::SendEnterRoomPacket(int room) const
{
	Protocols::Enter_Room enter_room;
	enter_room.set_roomindex(room);

	size_t bufSize = enter_room.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::ENTER_ROOM;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = enter_room.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	char* resultBuf = new char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);

	DWORD iobyte = 0;
	send_wsabuf.buf = resultBuf;
	send_wsabuf.len = header.size;
	int ret = WSASend(g_socket, &send_wsabuf, 1, &iobyte, 0, NULL, NULL);

	if (SOCKET_ERROR == ret) {
		if (ERROR_IO_PENDING != WSAGetLastError()) {
			err_display("WSASend() Error! : ", WSAGetLastError());
		}
	}
}

void ChattingClient::SendLeaveRoomPacket(int room)
{
	Protocols::Leave_Room leave_room;
	leave_room.set_roomindex(room);

	size_t bufSize = leave_room.ByteSizeLong();
	char* outputBuf = new char[bufSize];

	// 헤더 생성
	MessageHeader header;
	header.size = MessageHeaderSize + bufSize;
	header.type = Protocols::LEAVE_ROOM;
	char* header_seri = reinterpret_cast<char*>(&header);

	int rtn = leave_room.SerializeToArray(outputBuf, bufSize);

	// 전송 버퍼 생성
	char* resultBuf = new char[bufSize + MessageHeaderSize];
	memcpy(resultBuf, header_seri, MessageHeaderSize);
	memcpy(resultBuf + MessageHeaderSize, outputBuf, bufSize);

	DWORD iobyte = 0;
	send_wsabuf.buf = resultBuf;
	send_wsabuf.len = header.size;
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


void ChattingClient::SetMenu()
{
	int choice = 0;
	int room = 0;
	char send_msg[MSG_SIZE];

	while (1)
	{
		if (menu_enable == true)
		{
			std::cout << "======================================" << std::endl;
			std::cout << "1. 채널 이동" << std::endl;
			std::cout << "2. 채팅방 개설" << std::endl;
			std::cout << "3. 채팅방 입장" << std::endl;
			std::cout << "4. 채팅방 안에 있는 유저 목록 보기" << std::endl;
			std::cout << "6. 채널 채팅" << std::endl;
			std::cout << "7. 방 채팅" << std::endl;
			std::cout << "8. 채팅방 퇴장" << std::endl;
			std::cout << "10. 종료" << std::endl;
			std::cout << "======================================" << std::endl;

			std::cout << "메뉴 선택 : ";
			std::cin >> choice;
			if (choice / 1.00 != int(choice))
			{
				std::cout << "메뉴를 다시 선택하세요." << std::endl;
				continue;
			}
			
			switch (choice)
			{
			case CHANNEL_MOVE:
				system("cls");
				int channel;
				std::cout << std::endl;
				if (0 != room_index)
				{
					std::cout << "방에서 퇴장한 후에 채널을 이동하세요." << std::endl;
					break;
				}
				while (getchar() != '\n');
				std::cout << "몇 번 채널로 이동하시겠습니까? (0 ~ 4 선택) : " << std::endl;
				std::cin >> channel;
				if (channel > 4 || channel < 0) {
					printf("채널을 잘못 선택했습니다.\n\n");
					menu_enable = true;
					break;
				}
				SendChannelMovePacket(channel);
				std::cout << channel << "번 채널로 이동했습니다." << std::endl << std::endl;
				channel_index = channel;
				break;
			case ROOM_CREATE:
			{
				system("cls");
				if (0 != room_index)
				{
					std::cout << "이미 방에 입장 중입니다." << std::endl;
					break;
				}
				while (getchar() != '\n');
				printf("몇 번 방을 만드시겠습니까? \n");
				std::cin >> room;
				if (room < 1)
				{
					std::cout << "방은 1번 부터 생성 가능합니다." << std::endl << std::endl;
					break;
				}
				menu_enable = false;
				SendCreateRoomPacket(room);				
				break;
			}
			case ENTER_ROOM_INIT:
				if (0 != room_index)
				{
					std::cout << "이미 방에 입장 중입니다." << std::endl << std::endl;
					break;
				}
				system("cls");
				while (getchar() != '\n');
				std::cout << "몇 번 방으로 입장하시겠습니까? ";
				std::cin >> room;
				menu_enable = false;
				SendEnterRoomPacket(room);
				break;
			case IN_ROOM_USER_LIST:
				system("cls");
				if (0 == room_index)
				{
					std::cout << "아직 방에 입장하지 않았습니다." << std::endl << std::endl;
					break;
				}
				menu_enable = false;
				SendRoomUserListPacket(room_index);
				break;
			case CHANNEL_CHATTING_MENU:
			{
				while (getchar() != '\n');
				fputs("보낼 메시지 : ", stdout);
				//getchar();								// 입력할 때까지 대기
				std::cin.getline(send_msg, MSG_SIZE, '\n');
				
				int chat_len = strlen(send_msg);
				menu_enable = false;
				SendChannelChattingPacket(send_msg, channel_index, chat_len);
				break;
			}
			break;
			case ROOM_CHATTING_MENU:
			{
				char send_msg[MSG_SIZE];
				if (0 == room_index)
				{
					std::cout << "아직 방에 입장하지 않았습니다." << std::endl << std::endl;
					break;
				}
				while (getchar() != '\n');
				std::cout << "보낼 메시지 : ";
				std::cin.getline(send_msg, MSG_SIZE, '\n');
				int chat_len = strlen(send_msg);
				menu_enable = false;
				SendRoomChattingPacket(send_msg, room, chat_len);
				break;
			}
			break;
			case LEAVE_ROOM_MENU:
				system("cls");
				if (0 == room_index)
				{
					std::cout << "아직 방에 입장하지 않았습니다." << std::endl << std::endl;
					break;
				}
				std::cout << room_index <<" 번 방에서 퇴장합니다." << std::endl << std::endl;
				SendLeaveRoomPacket(room_index);
				room_index = 0;
				break;
			case EXIT_SERVER:
				closesocket(g_socket);
				exit(-1);
				break;
			default:
				std::cout << "메뉴를 다시 선택하세요." << std::endl;
				break;
			}
		}
	}
}

void ChattingClient::LoginToServer()
{
	char send_msg[MSG_SIZE];

	while (getchar() != '\n');
	std::cout << "아이디를 입력하세요. ";
	std::cin.getline(send_msg, MSG_SIZE, '\n');
	int login_len = strlen(send_msg);
	SendLoginPacket(send_msg, login_len);
}
