#pragma once
#include <string>
#include <SQLAPI.h>
#include <map>

struct User
{
	std::string address;
	std::string password;
};

struct Message
{
	std::string to;
	std::string subject_mail;
	std::string content;
};

struct ConnectParams
{
	std::string server;
	std::string database;
	std::string username;
	std::string password;
	std::string table;
};

class ISQLConnection abstract
{
public:

	virtual bool Connect(const ConnectParams& connect_string) = 0;
	virtual void InsertEmail(const User& email) = 0;
	virtual void InsertMessage(const Message& message, const User& email) = 0;
	virtual void SelectUsers(std::map<std::string, std::string>& info) = 0;
	virtual void ClearTable(const std::string& table) = 0;
	virtual bool Disconnect() = 0;
};