#pragma once

#include <iostream>

#include <string>
#include <fstream>

class MailInfo
{
public:
	MailInfo() = default;

	void set_mail_from(const std::string& mail_from);
	void set_rcpt_to(const std::string& rcpt_to);
	void set_subject(const std::string& subject);
	void set_text(const std::string& text);

	const char* get_mail_from() const;
	const char* get_rcpt_to() const;
	const char* get_subject() const;
	const char* get_text() const;

	void SaveToFile();

private:
	std::string m_mail_from;
	std::string m_rcpt_to;
	std::string m_subject;
	std::string m_text;
};

