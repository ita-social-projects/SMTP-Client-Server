// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "SMTPServer.h"

void SMTPServer::AcceptConnections()
{
	SOCKET client_socket;

	while (true)
	{
		SOCKADDR client_info;
		int client_info_len = sizeof(client_info);

		if (INVALID_SOCKET == (client_socket = accept(m_server_socket, &client_info, &client_info_len)))
		{
			LOG_WARN << "Error with client socket accepting!\n";
		}

		else
		{		
			m_thread_pool->AddTask(WorkWithClient, client_socket);
		}
	}
}

void SMTPServer::WorkWithClient(SOCKET client_socket)
{
	MailSession mail_session(client_socket);
	char buf[BUF_SIZE];
	int len;
	ZeroMemory(&buf, sizeof(buf));

	mail_session.SendResponse(WELCOME);

	while (len = recv(mail_session.get_client_socket(), (char*)&buf, sizeof(buf), 0))
	{
		if (SERVER_CLOSED == mail_session.Processes(buf))
		{
			closesocket(mail_session.get_client_socket());
			break;
		}

		ZeroMemory(&buf, sizeof(buf));
	}
}

bool SMTPServer::Initialize()
{
	WSADATA wsa_data;

	if (auto res = WSAStartup(MAKEWORD(WINSOCK_VER, WINSOCK_VER), &wsa_data) != 0)
	{
		LOG_FATAL << "Error with winsock initializing!\n";
		return false;
	}

	return true;
}

bool SMTPServer::SetSocketSettings()
{
	SOCKADDR_IN server_info;
	XMLParser xml_parser;

	m_thread_pool = std::make_unique<ThreadPool>(xml_parser.GetMaxWorkingThreads());
	m_server_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (m_server_socket == INVALID_SOCKET)
	{
		LOG_FATAL << "Error with socket initializing!\n";
		return false;
	}

	ZeroMemory(&server_info, sizeof(server_info));

	server_info.sin_family = AF_INET;
	server_info.sin_port = htons(xml_parser.GetListenerPort());
	server_info.sin_addr = *(LPIN_ADDR)(gethostbyname(xml_parser.GetIpAddress().c_str())->h_addr_list[0]);

	if (bind(m_server_socket, (sockaddr*)&server_info, sizeof(server_info)) == SOCKET_ERROR)
	{
		LOG_FATAL << "Error with socket binding!\n";
		return false;
	}

	return true;
}

void SMTPServer::ServerStart()
{
	if (listen(m_server_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		LOG_FATAL << "Error with server starting!\n";
		exit(WSAGetLastError());
	}

	LOG_INFO << "Server started!\n\n";

	AcceptConnections();
}