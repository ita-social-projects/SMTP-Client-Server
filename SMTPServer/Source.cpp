// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include "SMTPServer.h"

int main()
{
	SMTPServer smtp_server;

	if (auto res_init = smtp_server.Initialize())
	{
		if (auto res_settings = smtp_server.SetSocketSettings())
		{
			smtp_server.ServerStart();
		}
	}

	return 0;
}