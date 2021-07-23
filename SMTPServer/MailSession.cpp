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
	return (strlen_buf > MIN_MAIL_SIZE && strlen_buf < MAX_MAIL_SIZE && strchr(buf, '@'));
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

	switch (response_type)
	{
	case Responses::WELCOME_TO_CLIENT:
		strcpy(buf, "220 Welcome!\r\n");
		break;

	case Responses::SERVICE_CLOSING:
		strcpy(buf, "221 Service closing transmission channel\r\n");
		break;

	case Responses::LOGIN_SUCCESS:
		strcpy(buf, "235 Successful login\r\n");
		break;

	case Responses::OK:
		strcpy(buf, "250 OK\r\n");
		break;

	case Responses::LOGIN_RCV:
		strcpy(buf, "334 Ready to receive login\r\n");
		break;

	case Responses::START_MAIL:
		strcpy(buf, "354 Start mail input; end with <CRLF>.<CRLF>\r\n");
		break;

	case Responses::SYNTAX_ERROR:
		strcpy(buf, "501 Syntax error in parameters or arguments\r\n");
		break;

	case Responses::COMMAND_NOT_IMPLEMENTED:
		strcpy(buf, "502 Command not implemented\r\n");
		break;

	case Responses::BAD_SEQUENSE:
		strcpy(buf, "503 Bad sequence of commands\r\n");
		break;

	case Responses::NO_USER:
		strcpy(buf, "550 No such user\r\n");
		break;

	case Responses::USER_NOT_LOCAL:
		strcpy(buf, "551 User not local. Can not forward the mail\r\n");
		break;

	case Responses::EMAIL_N_RECEIVED:
		strcpy(buf, "521 Server does not accept mail\r\n");
		break;

	default:
		strcpy(buf, "No description\r\n");
		break;
	}

	send(m_client_socket, buf, sizeof(buf), 0);
	return response_type;
}

int MailSession::Processes(char* buf)
{
	switch (m_current_status)
	{
	case MailSessionStatus::LOGIN:
		return SubProcessLoginRecieve(buf);
		break;

	case MailSessionStatus::PASSWORD:
		return SubProcessPasswordRecieve(buf);
		break;

	case MailSessionStatus::DATA:
		return SubProcessSubject(buf);
		break;

	case MailSessionStatus::SUBJECT:
		return SubProcessEmail(buf);
		break;
	}


	if (_strnicmp(buf, "HELO", FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessHELO(buf);
	}

	else if (_strnicmp(buf, "AUTH LOGIN", FIRST_TEN_SYMBOLS) == 0)
	{
		return ProcessAUTH(buf);
	}

	else if (_strnicmp(buf, "ehlo", FIRST_FOUR_SYMBOLS) == 0)
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

int MailSession::ProcessAUTH(char* buf)
{
	if (m_current_status != MailSessionStatus::EHLO)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	m_current_status = MailSessionStatus::LOGIN;

	return SendResponse(Responses::LOGIN_RCV);
}

int MailSession::ProcessMAIL(char* buf)
{
	if (m_current_status != MailSessionStatus::AUTH_SUCCESS)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	std::string address;

	address = CutAddress(buf);

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
	if (m_current_status != MailSessionStatus::MAIL_FROM)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	std::string address;

	address = CutAddress(buf);

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
	if (m_current_status != MailSessionStatus::RCPT_TO)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	m_current_status = MailSessionStatus::DATA;
	return SendResponse(Responses::START_MAIL);
}

int MailSession::SubProcessLoginRecieve(char* buf)
{
	if (m_current_status != MailSessionStatus::LOGIN)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}
	
	m_current_status = MailSessionStatus::PASSWORD;

	return SendResponse(Responses::LOGIN_RCV);
}

int MailSession::SubProcessPasswordRecieve(char* buf)
{
	if (m_current_status != MailSessionStatus::PASSWORD)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	m_current_status = MailSessionStatus::AUTH_SUCCESS;

	return SendResponse(Responses::LOGIN_SUCCESS);
}

int MailSession::SubProcessEmail(char* buf)
{
	std::string text = buf;
	std::string result;

	size_t pos;

	if (strstr(buf, SMTP_DATA_TERMINATOR))
	{
		pos = text.find("\r\n", 0);

		if (pos != -1)
		{
			result = text.substr(0, pos);
			m_mail_info.set_text(result);
		}
	
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

	Sleep(1);
	return 0;
}

int MailSession::ProcessQUIT()
{
	m_mail_info.SaveToFile();

	return SendResponse(Responses::SERVICE_CLOSING);
}
