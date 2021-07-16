#pragma once

#include <iostream>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <memory>
#include <thread>
#include <string>

#include "ThreadPool.h"
#include "MailSession.h"

#pragma comment(lib, "Ws2_32.lib")

#define SMTP_PORT 25
#define WELCOME 220

class SMTPServer
{
public:
	SMTPServer() : m_server_socket(INVALID_SOCKET) {}

	bool Initialize();
	bool SetSocketSettings();
	void ServerStart();

private:
	void AcceptConnections();
	static void WorkWithClient(SOCKET client_socket);

private:
	SOCKET m_server_socket;
	std::unique_ptr<ThreadPool> m_thread_pool;
};


