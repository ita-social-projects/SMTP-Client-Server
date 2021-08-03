// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include "pch.h"
#include "SQLServer.h"


SQLServer::SQLServer(const ConnectParams& connect_string)
{
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
		params = connect_string;
		std::string con = connect_string.server + "@" + connect_string.database;
		m_connection.Connect(_TSA(con.c_str()), _TSA(connect_string.username.c_str()), _TSA(connect_string.password.c_str()), SA_SQLServer_Client);
		LOG_INFO << "Connected to SQL Server...\n";
	}

	return m_connection.isConnected();
}

void SQLServer::InsertEmail(const Email& email)
{
	std::string table_name{ "Users" };
	std::string column_name_id{ "Email_address" };
	std::string column_name_address{ "Password" };

	std::string insert_command{ "INSERT INTO " };
	std::string insert_statement = insert_command + table_name;

	insert_statement += " (" + column_name_id + ", " + column_name_address + ") ";

	insert_statement += "VALUES ('" + email.address + "', ";
	insert_statement += "'" + email.password + "')";

	SACommand insert(&m_connection);
	insert.setCommandText(_TSA(insert_statement.c_str()));
	insert.Execute();

}

void SQLServer::InsertMessage(const Message& message, const Email& email)
{

	std::string table_name{ "Message" };
	std::string column_name_content{ "Content" };
	std::string column_name_email_address{ "Email_address" };
	std::string column_name_date{ "was_sent" };

	std::string insert_command{ "INSERT INTO " };
	std::string insert_statement = insert_command + table_name;

	insert_statement +=
		"(" + column_name_content +
		", " + column_name_email_address +
		", " + column_name_date + ")";

	insert_statement += " VALUES ('" + message.content + "',\n";
	insert_statement += "'" + email.address + "',\n";
	insert_statement += " CONVERT(DATETIME,'" + message.date + "', 120))";;

	SACommand insert(&m_connection);
	insert.setCommandText(_TSA(insert_statement.c_str()));
	insert.Execute();

}

void SQLServer::SelectUsers()
{
	std::string table_name{ "Users" };
	std::string select_command{ "SELECT *\nFROM " };
	std::string select_statement = select_command + table_name;
	SACommand select(&m_connection, _TSA((select_statement).c_str()));
	select.Execute();

	while (select.FetchNext())
	{
		auto result1 = select.Field(_TSA("Email_address")).asString().GetMultiByteChars();
		auto result2 = select.Field(_TSA("Password")).asString().GetMultiByteChars();

		LOG_INFO << result1 << "\t" << result2;
		LOG_INFO << "\n";
	}

	m_connection.Commit();
}

void SQLServer::ClearTable(const std::string& table)
{
	std::string delete_command{ "DELETE FROM " };
	SACommand delete_statement(&m_connection, _TSA((delete_command + table).c_str()));
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