//#include "stdafx.h"
//
//
//User::User() : userSocket{0}
//{
//	recv_over.wsabuf.buf = reinterpret_cast<CHAR*>(recv_over.iocp_buff);
//	recv_over.wsabuf.len = sizeof(recv_over.iocp_buff);
//}
//
//
//User::~User()
//{
//}
//
//int User::SendPacket(unsigned char *packet)
//{
//	Overlap *over = new Overlap;
//	memset(over, 0, sizeof(Overlap));
//	over->event_type = OV_SEND;
//	over->wsabuf.buf = reinterpret_cast<CHAR *>(over->iocp_buff);
//	over->wsabuf.len = packet[1];
//	memcpy(over->iocp_buff, packet, packet[1]);
//
//	int ret = WSASend(userSocket, &over->wsabuf, 1, NULL, 0,
//		&over->overlap, NULL);
//	return ret;
//}
//
//int User::WsaRecv()
//{
//	DWORD flags = { 0 };
//	return WSARecv(userSocket, &recv_over.wsabuf, 1, NULL, &flags, &recv_over.overlap, NULL);
//
//}
//
//void User::CloseSocket()
//{
//	closesocket(userSocket);
//}
//
//int User::PacketRessembly(DWORD packetSize)
//{
//	unsigned char *buf_ptr = recv_over.iocp_buff;
//
//	int remained = packetSize;
//	while (0 < remained) {
//		if (0 == recv_buff.sizeCurr) { recv_buff.sizeCurr = buf_ptr[0]; }
//		int required = recv_buff.sizeCurr - recv_buff.sizePrev;
//		if (remained >= required) {
//			memcpy(recv_buff.buf + recv_buff.sizePrev, buf_ptr, required);
//			ProcessPacket();
//			buf_ptr += required;
//			remained -= required;
//			recv_buff.sizeCurr = 0;
//			recv_buff.sizePrev = 0;
//		}
//		else {
//			memcpy(recv_buff.buf + recv_buff.sizePrev, buf_ptr, remained);
//			buf_ptr += remained;
//			recv_buff.sizePrev += remained;
//			remained = 0;
//		}
//	}
//
//	return WsaRecv();
//}
//
//void User::ProcessPacket()
//{
//	cout << "받은 메세지 : " << recv_buff.buf[0] << endl;
//}