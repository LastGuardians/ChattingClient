//#pragma once
//
//struct Overlap
//{
//	WSAOVERLAPPED	overlap = { 0 };
//	int				event_type = { OV_RECV };
//	WSABUF			wsabuf = { 0 };
//	unsigned char	iocp_buff[BUF_SIZE] = { 0 };
//};
//
//struct RecvBuffInfo {
//	unsigned char	buf[BUF_SIZE];
//	int				sizePrev = { 0 };
//	int				sizeCurr = { 0 };
//};
//
//class User
//{
//public:
//	User();
//	virtual ~User();
//
//private:
//	SOCKET			userSocket;
//	Overlap			recv_over;
//	RecvBuffInfo	recv_buff;
//
//public:
//	//void		SetUserInfo(SOCKET s, bool connect, int id);
//	int			PacketRessembly(DWORD packetSize);
//	void		ProcessPacket();
//	int			WsaRecv();
//	int			SendPacket(unsigned char *packet);
//	void		CloseSocket();
//};
//
