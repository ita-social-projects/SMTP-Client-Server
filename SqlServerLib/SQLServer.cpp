// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include "pch.h"
#include "framework.h"
#include "SQLServer.h"


SQLServer::SQLServer(const ConnectParams& connect_string)
{
	LOG = LOG->GetInstance();
	Connect(connect_string);
}

SQLServer::~SQLServer()
{
	Disconnect();
}


bool SQLServer::Connect(const ConnectParams& connect_string)
{
	bool check_connection = m_connection.isConnected();

	if (!check_connection)
	{
		m_params = connect_string;
		std::string con = connect_string.server + "@" + connect_string.database;
		m_connection.Connect(_TSA(con.c_str()), _TSA(connect_string.username.c_str()), _TSA(connect_string.password.c_str()), SA_SQLServer_Client);
		LOG_INFO << "Connected to SQL Server...\n";
	}

	return m_connection.isConnected();
}

void SQLServer::InsertEmail(const Email& email)
{
	const std::string TABLE_NAME{ "Users" };
	const std::string COLUMN_NAME_ID{ "Email_address" };
	const std::string COLUMN_NAME_ADDRESS{ "Password" };
	
	std::string insert_statement = INSERT_COMMAND + TABLE_NAME;

	insert_statement += " (" + COLUMN_NAME_ID + ", " + COLUMN_NAME_ADDRESS + ") ";

	
	std::shared_ptr<unsigned char[]> encrypted_pass;
	int encrypted_pass_len;
	unsigned int password_size = static_cast<unsigned int>(email.password.size());
	encrypted_pass_len = m_crypto.Encrypt((unsigned char*)email.password.c_str(), password_size, encrypted_pass);
	std::string str = (char*)encrypted_pass.get();
	insert_statement += "VALUES ('" + email.address + "', ";
	insert_statement += "'" + str + "')";

	SACommand insert(&m_connection);
	insert.setCommandText(_TSA(insert_statement.c_str()));
	insert.Execute();

}

void SQLServer::InsertMessage(const Message& message, const Email& email)
{

	const std::string TABLE_NAME{ "Message" };
	const std::string COLUMN_NAME_CONTENT{ "Content" };
	const std::string COLUMN_NAME_EMAIL_ADDRESS{ "Email_address" };
	const std::string COLUMN_NAME_DATE{ "was_sent" };

	std::string insert_statement = INSERT_COMMAND + TABLE_NAME;

	insert_statement +=
		"(" + COLUMN_NAME_CONTENT +
		", " + COLUMN_NAME_EMAIL_ADDRESS +
		", " + COLUMN_NAME_DATE + ")";

	insert_statement += " VALUES ('" + message.content + "',\n";
	insert_statement += "'" + email.address + "',\n";
	insert_statement += " CONVERT(DATETIME,'" + message.date + "', 120))";;

	SACommand insert(&m_connection);
	insert.setCommandText(_TSA(insert_statement.c_str()));
	insert.Execute();

}

void SQLServer::SelectUsers(std::map<std::string, std::string>& info)
{
	const std::string TABLE_NAME{ "Users" };
	const std::string SELECT_COMMAND{ "SELECT *\nFROM " };
	const std::string SELECT_STATEMENT = SELECT_COMMAND + TABLE_NAME;
	SACommand select(&m_connection, _TSA((SELECT_STATEMENT).c_str()));
	select.Execute();

	std::shared_ptr<unsigned char[]> decrypted_pass;

	while (select.FetchNext())
	{
		auto str1 = select.Field(_TSA("Email_address")).asString().GetMultiByteChars();
		auto pass = select.Field(_TSA("Password")).asString().GetMultiByteChars();

		int len = sizeof(pass);
		m_crypto.Decrypt((unsigned char*)pass, len, decrypted_pass);

		auto str2 = (char*)decrypted_pass.get();

		info.emplace(str1, str2);

		std::string tab = "\t";
		std::string str_for_log = str1 + tab + str2;
		LOG_INFO << str_for_log.c_str();
	}

	m_connection.Commit();
}

void SQLServer::ClearTable(const std::string& table)
{
	const std::string DELETE_COMMAND{ "DELETE FROM " };
	SACommand delete_statement(&m_connection, _TSA((DELETE_COMMAND + table).c_str()));
	delete_statement.Execute();
}

bool SQLServer::Disconnect()
{
	bool check_connection = m_connection.isConnected();

	if (check_connection) {

		m_connection.Disconnect();
		LOG_INFO << "Disconnected from SQL Server...\n";
	}

	return !m_connection.isConnected();
}