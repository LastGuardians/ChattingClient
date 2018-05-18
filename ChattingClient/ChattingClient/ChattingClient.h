#pragma once

struct Overlap
{
	WSAOVERLAPPED	overlap = { 0 };
	int				event_type = { OV_RECV };
	WSABUF			wsabuf = { 0 };
	unsigned char	iocp_buff[BUF_SIZE] = { 0 };
};

struct RecvBuffInfo {
	unsigned char	buf[BUF_SIZE];
	int				sizePrev = { 0 };
	int				sizeCurr = { 0 };
};


// 패킷 헤더
struct MessageHeader
{
	Protocols::PacketType	type;
	protobuf::uint32		size;
};
const int MessageHeaderSize = sizeof(MessageHeader);

class ChattingClient
{
public:
	ChattingClient();
	virtual ~ChattingClient();

private:
	SOCKET			_g_socket;
	char 			_send_buffer[BUF_SIZE];
	WSABUF			_recv_wsabuf;
	char 			_recv_buffer[BUF_SIZE];

	int				_my_id;
	int				_channel_index;
	int				_room_index = 0;

public:
	bool		ServerConnect();
	void		ThreadStart();
	void		RecvThread();
	int			Recvn(SOCKET s, char* buf, int len, int flags);

	void		PacketProcess(protobuf::io::CodedInputStream& input_stream);
	void		ProcessEneterChannelPacket(const Protocols::Enter_Channel message);
	//void		ProcessLoginPacket(const Protocols::User_Login message) const;
	void		ProcessCreateRoomPacket(const Protocols::Create_Room message) const;
	bool		ProcessNotifyExistRoomPacket(const Protocols::Notify_Exist_Room message) const;
	void		ProcessRoomListPacket(const Protocols::Room_List message) const;
	void		ProcessRoomChatPacket(const Protocols::Room_Chatting message) const;
	bool		ProcessEnterRoomPacket(const Protocols::Enter_Room message) const;
	void		ProcessChannelChatPacket(const Protocols::Channel_Chatting message) const;
	void		ProcessNotifyEnterRoomPacket(const Protocols::Notify_Enter_Room message) const;
	void		ProcessNotifyLeaveRoomPacket(const Protocols::Notify_Leave_Room message) const;

	int			WsaRecv();
	//void		SendLoginPacket(char* id, int len) const;
	void		SendChannelMovePacket(int channel);
	void		SendChannelChattingPacket(char* message, int channel, int len);
	void		SendCreateRoomPacket(int room);
	void		SendRoomUserListPacket(int room);
	void		SendRoomChattingPacket(char* message, int room, int len);
	void		SendEnterRoomPacket(int room) const;
	void		SendLeaveRoomPacket(int room);

	void		MenuStart();
	void		MenuChannelMove();
	void		MenuRoomCreate();
	void		MenuEnterRoom();
	void		MenuRoomUserList();
	void		MenuChannelChatting();
	void		MenuRoomChatting();
	void		MenuLeaveRoom();

	void		CloseSocket();
	void		err_display(char *msg, int err_no) const;	
	void		LoginToServer() const;
};

