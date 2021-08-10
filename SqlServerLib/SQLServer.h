#pragma once
#include <iostream>
#include "SQLConnection.h"
#include "../CPPLogger/CPPLogger.h"
#include "../Crypto/SymmetricCrypto.h"


class SQLServer : public ISQLConnection
{
public:

	SQLServer() = default;
	SQLServer(const ConnectParams& connect_string);
	~SQLServer();

	bool Connect(const ConnectParams& connect_string) override;
	void InsertEmail(const Email& email) override;
	void InsertMessage(const Message& message, const Email& email) override;
	void SelectUsers() override;
	void ClearTable(const std::string& table) override;
	bool Disconnect() override;

private:
	SAConnection m_connection;
	SACommand m_command;
	ConnectParams m_params;

	Logger *LOG;
	SymmetricCrypto crypto;

};