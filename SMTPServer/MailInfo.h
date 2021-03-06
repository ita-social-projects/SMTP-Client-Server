#pragma once

#include <iostream>

#include <string>
#include <map>
#include <algorithm>

#include "../SqlServerLib/SQLServer.h"

auto constexpr DB_SERVER_NAME = "localhost\\SQLSOFT";
auto constexpr DB_DATABASE_NAME = "SMTP-Client-Server";
auto constexpr DB_USERNAME = "Tomas";
auto constexpr DB_PASSWORD = "password";

class MailInfo
{
public:
	MailInfo() = default;

	void set_login(const std::string& login);
	void set_password(const std::string& password);
	void set_mail_from(const std::string& mail_from);
	void set_rcpt_to(const std::string& rcpt_to);
	void set_subject(const std::string& subject);
	void set_text(const std::string& text);

	const std::string& get_login() const;
	const std::string& get_password() const;
	const std::string& get_mail_from() const;
	const std::string& get_rcpt_to() const;
	const std::string& get_subject() const;
	const std::string& get_text() const;

	void CutSymbols(std::string& str);

	void SaveToDatabase();

	bool ConnectToDB();
	bool TakeDataFromDB();
	bool IsUserExist();

private:
	SQLServer m_db;

	std::string m_mail_from;
	std::string m_rcpt_to;
	std::string m_subject;
	std::string m_text;
	std::string m_login;
	std::string m_password;

	std::map<std::string, std::string> m_data_from_db;
};

