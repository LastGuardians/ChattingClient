#pragma once


struct MessageHeader
{
	protobuf::uint32 size;
};
const int MessageHeaderSize = sizeof(MessageHeader);

class PacketHandler
{
public:
	PacketHandler();
	virtual ~PacketHandler();

	inline void Handle(const Channel_P::channel_chatting& message) const
	{
		PrintMessage(message);
	}

	void SendBroadcast(google::protobuf::MessageLite *msg);

protected:

	void PrintMessage(const protobuf::Message& message) const;
	void PacketProcess(protobuf::io::CodedInputStream& input_stream, const PacketHandler& handler);
};

