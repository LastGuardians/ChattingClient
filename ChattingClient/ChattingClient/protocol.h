#pragma once

enum PacketType {
	ENTER_CHANNEL		= 1,
	LEAVE_CHANNEL		= 2,
	CREATE_ROOM			= 3,
	NOTIFY_ENTER_ROOM	= 4,
	NOTIFY_LEAVE_ROOM	= 5,

};

struct Enter_Channel {
	BYTE			type = { ENTER_CHANNEL };
	BYTE			size;
	unsigned int	id;
	unsigned int	channelIndex;
};

struct Leave_Channel {
	BYTE			type = { LEAVE_CHANNEL };
	BYTE			size;
	unsigned int	id;
	unsigned int	channelIndex;
};

struct Create_Room {

};