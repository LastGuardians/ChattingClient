#pragma once

enum PacketType {
	ENTER_CHANNEL		= 1,
	LEAVE_CHANNEL		= 2,
	CREATE_ROOM			= 3,
	NOTIFY_ENTER_ROOM	= 4,
	NOTIFY_LEAVE_ROOM	= 5,
	CHANGE_CHANNEL		= 6,
};

#pragma pack(push, 1)

struct Enter_Channel {
	BYTE			type = { ENTER_CHANNEL };
	BYTE			size;
	unsigned int	id;
	unsigned int	channelIndex; // (0~4 »çÀÌ°ª)
};

struct Leave_Channel {
	BYTE			type = { LEAVE_CHANNEL };
	BYTE			size;
	unsigned int	id;
	unsigned int	channelIndex;
};

struct Create_Room {
	BYTE			type = { CREATE_ROOM };
	BYTE			size;
	unsigned int	id;
	unsigned int	roomIndex;
};

struct Change_Channel {
	BYTE			type = { CHANGE_CHANNEL };
	BYTE			size;
	unsigned int	id;
	unsigned int	roomIndex;
};

#pragma pack(pop)