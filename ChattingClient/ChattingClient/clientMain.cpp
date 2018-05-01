#include "stdafx.h"

int main()
{
	ChattingClient client;

	bool opend = client.ServerConnect();
	if (true == opend)
	{
		client.ThreadStart();
	}
}
