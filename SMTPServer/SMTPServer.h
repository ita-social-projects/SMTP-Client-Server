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

#include "../CPPLogger/CPPLogger.h"
#include "../XMLParser/XMLParser.h"

constexpr auto WINSOCK_VER = 2;
constexpr auto BUF_SIZE = 128;
constexpr auto WELCOME = 220;
constexpr auto SERVER_CLOSED = 221;

class SMTPServer
{
public:
	SMTPServer() = default;

	bool Initialize();
	bool SetSocketSettings();
	void ServerStart();

private:
	void AcceptConnections();
	static void WorkWithClient(SOCKET client_socket);

private:
	SOCKET m_server_socket = INVALID_SOCKET;
	std::unique_ptr<ThreadPool> m_thread_pool;

	Logger LOG;
};

