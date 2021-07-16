// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "MailInfo.h"

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

const char* MailInfo::get_mail_from() const
{
	return m_mail_from.c_str();
}

const char* MailInfo::get_rcpt_to() const
{
	return m_rcpt_to.c_str();
}

const char* MailInfo::get_subject() const
{
	return m_subject.c_str();
}

const char* MailInfo::get_text() const
{
	return m_text.c_str();
}

void MailInfo::SaveToFile()
{
	std::ofstream file("email.txt", std::ofstream::app);

	file << "From: " << m_mail_from << "\nTo: " << m_rcpt_to << "\nSubject: " << m_subject << "\n" << m_text;

	file.close();
}
