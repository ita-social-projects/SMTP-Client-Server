// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "MailSession.h"

MailSession::MailSession(SOCKET& client_socket)
{
	this->m_client_socket = client_socket;
}

const SOCKET& MailSession::get_socket() const
{
	return m_client_socket;
}

bool MailSession::ValidAdress(char* buf)
{
	size_t strlen_buf = strlen(buf);
	return (strlen_buf > 2 && strlen_buf < 255 && strchr(buf, '@'));
}

std::string MailSession::CutAddress(char* buf)
{
	std::string str_buf = buf;
	std::string result = "";

	size_t start_pos = str_buf.find('<', 0);
	size_t end_pos = str_buf.find('>', 0);

	for (size_t i = start_pos + 1; i < end_pos; i++)
	{
		result += str_buf.at(i);
	}

	return result;
}

void MailSession::SpecialSymbols(std::string& str)
{
	if (size_t pos = str.find("\r\n") != -1)
	{
		str.clear();
	}
}

std::string MailSession::CutSubject(char* buf)
{
	std::string str_buf = buf;
	std::string result = "";

	size_t start_pos = str_buf.find(':', 0);

	for (size_t i = start_pos + 2; i < str_buf.size(); i++)
	{
		result += str_buf.at(i);
	}

	return result;
}

int MailSession::SendResponse(int response_type)
{
	char buf[64];
	ZeroMemory(&buf, sizeof(buf));

	if (response_type == Responses::WELCOME)
	{
		strcpy(buf, "220 Welcome!\r\n");
	}

	else if (response_type == Responses::SERVICE_CLOSING)
	{
		strcpy(buf, "221 Service closing transmission channel\r\n");
	}

	else if (response_type == Responses::OK)
	{
		strcpy(buf, "250 OK\r\n");
	}

	else if (response_type == Responses::START_MAIL)
	{
		strcpy(buf, "354 Start mail input; end with <CRLF>.<CRLF>\r\n");
	}

	else if (response_type == Responses::SYNTAX_ERROR)
	{
		strcpy(buf, "501 Syntax error in parameters or arguments\r\n");
	}

	else if (response_type == Responses::COMMAND_NOT_IMPLEMENTED)
	{
		strcpy(buf, "502 Command not implemented\r\n");
	}

	else if (response_type == Responses::BAD_SEQUENSE)
	{
		strcpy(buf, "503 Bad sequence of commands\r\n");
	}

	else if (response_type == Responses::NO_USER)
	{
		strcpy(buf, "550 No such user\r\n");
	}

	else if (response_type == Responses::USER_NOT_LOCAL)
	{
		strcpy(buf, "551 User not local. Can not forward the mail\r\n");
	}

	else
	{
		strcpy(buf, "No description\r\n");
	}

	std::cout << "Sending: " << buf << "\n";
	send(m_client_socket, buf, sizeof(buf), 0);

	return response_type;
}

int MailSession::Processes(char* buf)
{

	if (m_current_status == MailSessionStatus::DATA)
	{
		return SubProcessSubject(buf);
	}

	if (m_current_status == MailSessionStatus::SUBJECT)
	{
		return SubProcessEmail(buf);
	}


	if (_strnicmp(buf, "HELO", FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessHELO(buf);
	}

	else if (_strnicmp(buf, "EHLO", FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessHELO(buf);
	}

	else if (_strnicmp(buf, "MAIL", FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessMAIL(buf);
	}

	else if (_strnicmp(buf, "RCPT", FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessRCPT(buf);
	}

	else if (_strnicmp(buf, "DATA", FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessDATA(buf);
	}

	else if (_strnicmp(buf, "QUIT", FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessQUIT();
	}

	else
	{
		return ProcessNotImplemented(false);
	}
}

int MailSession::ProcessNotImplemented(bool arg)
{
	return SendResponse(Responses::COMMAND_NOT_IMPLEMENTED);
}

int MailSession::ProcessHELO(char* buf)
{
	std::cout << "Received 'HELO' or 'ELHO'\n";

	if (m_current_status != MailSessionStatus::EMPTY)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	if (strchr(buf, '.') == NULL)
	{
		return SendResponse(Responses::SYNTAX_ERROR);
	}

	m_current_status = MailSessionStatus::EHLO;

	return SendResponse(Responses::OK);
}

int MailSession::ProcessMAIL(char* buf)
{
	std::cout << "Received 'MAIL FROM'\n";

	if (m_current_status != MailSessionStatus::EHLO)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	std::string address;

	address = CutAddress(buf);

	std::cout << "Message from: " << address << "\n";

	if (!MailSession::ValidAdress((char*)address.c_str()))
	{
		return SendResponse(Responses::SYNTAX_ERROR);
	}

	m_current_status = MailSessionStatus::MAIL_FROM;
	m_mail_info.set_mail_from(address);

	return SendResponse(Responses::OK);
}

int MailSession::ProcessRCPT(char* buf)
{
	std::cout << "Received 'RCPT TO'\n";

	if (m_current_status != MailSessionStatus::MAIL_FROM)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	std::string address;

	address = CutAddress(buf);

	std::cout << "Message from: " << address << "\n";

	if (!MailSession::ValidAdress((char*)address.c_str()))
	{
		return SendResponse(Responses::SYNTAX_ERROR);
	}

	m_current_status = MailSessionStatus::RCPT_TO;
	m_mail_info.set_rcpt_to(address);

	return SendResponse(Responses::OK);
}

int MailSession::ProcessDATA(char* buf)
{
	std::cout << "Received 'DATA'\n";

	if (m_current_status != MailSessionStatus::RCPT_TO)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	m_current_status = MailSessionStatus::DATA;
	return SendResponse(Responses::START_MAIL);
}

int MailSession::SubProcessEmail(char* buf)
{
	std::string text = buf;
	SpecialSymbols(text);

	m_mail_info.set_text(text);

	if (strstr(buf, SMTP_DATA_TERMINATOR))
	{
		std::cout << "Received DATA END\n";
		m_current_status = MailSessionStatus::QUIT;

		return SendResponse(Responses::OK);
	}

	return SendResponse(Responses::OK);
}

int MailSession::SubProcessSubject(char* buf)
{
	if (_strnicmp(buf, "Subject:", FIRST_EIGHT_SYMBOLS) != 0)
	{
		return SendResponse(Responses::SYNTAX_ERROR);
	}

	std::string subject;
	subject = CutSubject(buf);

	m_current_status = MailSessionStatus::SUBJECT;
	m_mail_info.set_subject(subject);

	return SendResponse(Responses::OK);
}

int MailSession::ProcessQUIT()
{
	m_mail_info.SaveToFile();
	std::cout << "Received 'QUIT'\n";
	return SendResponse(Responses::SERVICE_CLOSING);
}
