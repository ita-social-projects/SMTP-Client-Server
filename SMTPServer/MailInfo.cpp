// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "MailInfo.h"

void MailInfo::set_login(const std::string& login)
{
	this->m_login = login;
}

void MailInfo::set_password(const std::string& password)
{
	this->m_password = password;
}

void MailInfo::set_mail_from(const std::string& mail_from)
{
	this->m_mail_from = mail_from;
}

void MailInfo::set_rcpt_to(const std::string& rcpt_to)
{
	this->m_rcpt_to = rcpt_to;
}

void MailInfo::set_subject(const std::string& subject)
{
	this->m_subject = subject;
}

void MailInfo::set_text(const std::string& text)
{
	this->m_text += "\n" + text;
}

const std::string& MailInfo::get_login() const
{
	return m_login;
}

const std::string& MailInfo::get_password() const
{
	return m_password;
}

const std::string& MailInfo::get_mail_from() const
{
	return m_mail_from;
}

const std::string& MailInfo::get_rcpt_to() const
{
	return m_rcpt_to;
}

const std::string& MailInfo::get_subject() const
{
	return m_subject;
}

const std::string& MailInfo::get_text() const
{
	return m_text;
}

void MailInfo::CutSymbols(std::string& str)
{
	size_t pos = str.find("\r\n", 0);

	if (pos != -1)
	{
		str = str.substr(0, pos);
	}
}

void MailInfo::SaveToFile()
{
	std::ofstream file("email.txt", std::ofstream::app);

	file << "From: " << m_mail_from << "\nTo: " << m_rcpt_to << "\nSubject: " << m_subject << m_text << "\n\n";

	file.close();
}

void MailInfo::ConnectToDB()
{
	ConnectParams conect_params;

	conect_params.server = DB_SERVER_NAME;
	conect_params.database = DB_DATABASE_NAME;
	conect_params.username = DB_USERNAME;
	conect_params.password = DB_PASSWORD;

	bool var = m_db.Connect(conect_params);

	/*m_db.Connect(conect_params);*/
}

bool MailInfo::TakeDataFromDB()
{
	m_db.SelectUsers(m_data_from_db);

	if (m_data_from_db.empty())
	{
		return false;
	}

	return true;
}

bool MailInfo::IsUserExist()
{
	CutSymbols(m_login);
	CutSymbols(m_password);

	std::pair<const std::string, std::string> data = std::make_pair(m_login, m_password);
	auto find_result = find(m_data_from_db.begin(), m_data_from_db.end(), data);

	if (find_result == m_data_from_db.end())
	{
		return false;
	}

	return true;
}
