#include "stdafx.h"

int main()
{
	ChattingClient client;

	bool opened = client.ServerConnect();
	if (true == opened)
	{
		client.ThreadStart();
	}
}
