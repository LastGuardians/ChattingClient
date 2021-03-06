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


// ��Ŷ ���
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
	SOCKET			g_socket;
	char 			send_buffer[BUF_SIZE];
	WSABUF			recv_wsabuf;
	char 			recv_buffer[BUF_SIZE];

	int				my_id;
	int				channel_index;
	int				room_index = 0;

	HANDLE			handle;
	HANDLE			m_hiocp = { 0 };
	bool			m_b_server_shut_down = { false };
	bool			server_connected = { false };
	bool			isQuit = { false };
	bool			recv_start = { false };


public:
	bool		ServerConnect();
	void		RecvThread();
	int			Recvn(SOCKET s, char* buf, int len, int flags);

	void		PacketProcess(protobuf::io::CodedInputStream& input_stream);
	void		ProcessEneterChannelPacket(const Protocols::Enter_Channel message) const;
	void		ProcessCreateRoomPacket(const Protocols::Create_Room message) const;
	bool		ProcessNotifyExistRoomPacket(const Protocols::Notify_Exist_Room message) const;
	void		ProcessRoomListPacket(const Protocols::Room_List message) const;
	void		ProcessRoomChatPacket(const Protocols::Room_Chatting message) const;
	bool		ProcessEnterRoomPacket(const Protocols::Enter_Room message) const;
	void		ProcessChannelChatPacket(const Protocols::Channel_Chatting message) const;
	void		ProcessNotifyEnterRoomPacket(const Protocols::Notify_Enter_Room message) const;
	void		ProcessNotifyLeaveRoomPacket(const Protocols::Notify_Leave_Room message) const;

	int			WsaRecv();
	void		SendPacket(unsigned char *packet, int size);
	void		SendLoginPacket(char* id, int len);
	void		SendChannelMovePacket(int channel);
	void		SendChannelChattingPacket(char* message, int channel, int len);
	void		SendCreateRoomPacket(int room);
	void		SendRoomUserListPacket(int room);
	void		SendRoomChattingPacket(char* message, int room, int len);
	void		SendEnterRoomPacket(int room) const;
	void		SendLeaveRoomPacket(int room);

	void		CloseSocket();
	void		err_display(char *msg, int err_no) const;
	void		SetMenu();
	void		LoginToServer();
	inline bool	GetRecvStart() { return recv_start; }
};

