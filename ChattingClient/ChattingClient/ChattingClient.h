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
	SOCKET			g_socket;
	//Overlap			recv_over;
	//RecvBuffInfo	recv_buff;

	WSABUF			send_wsabuf;
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
	void		Run();
	void		RecvThread();
	int			Recvn(SOCKET s, char* buf, int len, int flags);

	int			PacketRessembly(DWORD packetSize);
	void		PacketProcess(protobuf::io::CodedInputStream& input_stream, const ChattingClient& handler);
	void		ProcessPacket(unsigned char *packet);
	void		ProcessEneterChannelPacket(const Protocols::Enter_Channel message) const;
	void		ProcessCreateRoomPacket(unsigned char *packet);
	void		ProcessNotifyExistRoomPacket(unsigned char *packet);
	void		ProcessRoomListPacket(unsigned char *packet);
	void		ProcessRoomChatPacket(unsigned char *packet);
	void		ProcessEnterRoomPacket(unsigned char *packet);
	void		ProcessChannelChatPacket(unsigned char *packet);
	void		ProcessNotifyEnterRoomPacket(unsigned char *packet);
	void		ProcessNotifyLeaveRoomPacket(unsigned char *packet);

	int			WsaRecv();
	//void		SendBroadcast(google::protobuf::MessageLite * msg);
	void		SendPacket(unsigned char *packet, int size);
	void		SendChannelMovePacket(int channel);
	void		SendChannelChattingPacket(char* message, int channel, int len);
	void		SendCreateRoomPacket(int room);
	void		SendRoomMovePacket(unsigned char *packet);
	void		SendRoomUserListPacket(int room);
	void		SendRoomChattingPacket(char* message, int room, int len);
	void		SendEnterRoomPacket(int room);
	void		SendLeaveRoomPacket(int room);

	void		CloseSocket();
	void		err_display(char *msg, int err_no);
	void		SetMenu();
	void		ChattingThreadStart(int roomIndex);
	void		ChattingMenu(int roomIndex);
	void		ChattingCommand(int roomIndex);
	inline bool	GetRecvStart() { return recv_start; }
};

