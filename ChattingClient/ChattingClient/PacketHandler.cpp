#include "stdafx.h"


PacketHandler::PacketHandler()
{
}


PacketHandler::~PacketHandler()
{
}

void PacketHandler::SendBroadcast(google::protobuf::MessageLite * msg)
{
	
}

void PacketHandler::PrintMessage(const protobuf::Message & message) const
{
	std::string textFormatStr;
	protobuf::TextFormat::PrintToString(message, &textFormatStr);
	printf("%s\n", textFormatStr.c_str());
}

void PacketHandler::PacketProcess(protobuf::io::CodedInputStream & input_stream, const PacketHandler & handler)
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
		Channel_P::channel_chatting message;
		if (false == message.ParseFromCodedStream(&payload_input_stream))
			break;

		message.id();
		handler.Handle(message);
		


		/*switch (messageHeader.type)
		{
		case simple::LOGIN:
		{
		simple::Login message;
		if (false == message.ParseFromCodedStream(&payload_input_stream))
		break;
		handler.Handle(message);
		}
		break;
		case simple::CHAT:
		{
		simple::Chat message;
		if (false == message.ParseFromCodedStream(&payload_input_stream))
		break;
		handler.Handle(message);
		}
		break;
		case simple::MOVE:
		{
		simple::Move message;
		if (false == message.ParseFromCodedStream(&payload_input_stream))
		break;
		handler.Handle(message);
		}
		break;
		}*/
	}
}

