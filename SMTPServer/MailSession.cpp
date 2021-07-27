// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "MailSession.h"

MailSession::MailSession(SOCKET client_socket)
{
	this->client_socket = client_socket;
}

const SOCKET& MailSession::get_client_socket() const
{
	return client_socket;
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
	char buf[RESPONSE_BUF_SIZE];
	ZeroMemory(&buf, sizeof(buf));

	if (response_type == Responses::WELCOME_TO_CLIENT)
	{
		strcpy(buf, "220 Welcome!\r\n");
	}

	else if (response_type == Responses::SERVICE_CLOSING)
	{
		strcpy(buf, "221 Service closing transmission channel\r\n");
	}

	else if (response_type == Responses::LOGIN_SUCCESS)
	{
		strcpy(buf, "235 Successfull login\r\n");
	}

	else if (response_type == Responses::OK)
	{
		strcpy(buf, "250 OK\r\n");
	}

	else if (response_type == Responses::LOGIN_RCV)
	{
		strcpy(buf, "334 Login has recieved\r\n");
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

	send(client_socket, buf, sizeof(buf), 0);

	return response_type;
}

int MailSession::Processes(char* buf)
{

	if (_strnicmp(buf, "HELO", FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessHELO(buf);
	}

	else if (_strnicmp(buf, "EHLO", FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessHELO(buf);
	}

	else if (_strnicmp(buf, "AUTH", FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessAUTH(buf);
	}

	else if (current_status == MailSessionStatus::LOGIN)
	{
		return ProcessAUTH(buf);
	}

	else if (current_status == MailSessionStatus::PASSWORD)
	{
		return ProcessAUTH(buf);
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

	else if (current_status == MailSessionStatus::SUBJECT)
	{
		return SubProcessEmail(buf);
	}

	else if (current_status == MailSessionStatus::DATA)
	{
		return SubProcessSubject(buf);
	}

	else if (_strnicmp(buf, "QUIT", FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessQUIT();
	}

	else
	{
		return ProcessNotImplemented();
	}
}

int MailSession::ProcessNotImplemented()
{
	return SendResponse(Responses::COMMAND_NOT_IMPLEMENTED);
}

int MailSession::ProcessHELO(char* buf)
{
	if (current_status != MailSessionStatus::EMPTY)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	if (strchr(buf, '.') == NULL)
	{
		return SendResponse(Responses::SYNTAX_ERROR);
	}

	current_status = MailSessionStatus::AUTH;

	return SendResponse(Responses::OK);
}

int MailSession::ProcessAUTH(char* buf)
{
	if (current_status == MailSessionStatus::AUTH)
	{
		current_status = MailSessionStatus::LOGIN;
		return SendResponse(Responses::LOGIN_RCV);
	}

	else if (current_status == MailSessionStatus::LOGIN)
	{
		return SubProcessLoginRecieve(buf);
	}

	else if (current_status == MailSessionStatus::PASSWORD)
	{
		return SubProcessPasswordRecieve(buf);
	}

	else
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}
}

int MailSession::ProcessMAIL(char* buf)
{
	if (current_status != MailSessionStatus::AUTH_SUCCESS)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	std::string address = CutAddress(buf);

	if (!MailSession::ValidAdress((char*)address.c_str()))
	{
		return SendResponse(Responses::SYNTAX_ERROR);
	}

	current_status = MailSessionStatus::MAIL_FROM;
	mail_info.set_mail_from(address);

	return SendResponse(Responses::OK);
}

int MailSession::ProcessRCPT(char* buf)
{
	if (current_status != MailSessionStatus::MAIL_FROM)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	std::string address = CutAddress(buf);

	if (!MailSession::ValidAdress((char*)address.c_str()))
	{
		return SendResponse(Responses::SYNTAX_ERROR);
	}

	current_status = MailSessionStatus::RCPT_TO;
	mail_info.set_rcpt_to(address);

	return SendResponse(Responses::OK);
}

int MailSession::ProcessDATA(char* buf)
{
	if (current_status != MailSessionStatus::RCPT_TO)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	current_status = MailSessionStatus::DATA;
	return SendResponse(Responses::START_MAIL);
}

int MailSession::SubProcessLoginRecieve(char* buf)
{
	current_status = MailSessionStatus::PASSWORD;
	return SendResponse(Responses::LOGIN_RCV);
}

int MailSession::SubProcessPasswordRecieve(char* buf)
{
	current_status = MailSessionStatus::AUTH_SUCCESS;
	return SendResponse(Responses::LOGIN_SUCCESS);
}

int MailSession::SubProcessEmail(char* buf)
{
	std::string text = buf;

	if (strstr(buf, SMTP_DATA_TERMINATOR))
	{
		size_t pos = text.find("\r\n", 0);
		text = text.substr(0, pos);
		mail_info.set_text(text);

		current_status = MailSessionStatus::QUIT;

		return 1;
	}

	return SendResponse(Responses::EMAIL_N_RECEIVED);
}

int MailSession::SubProcessSubject(char* buf)
{
	if (_strnicmp(buf, "Subject:", FIRST_EIGHT_SYMBOLS) != 0)
	{
		return SendResponse(Responses::SYNTAX_ERROR);
	}

	std::string subject;
	subject = CutSubject(buf);

	current_status = MailSessionStatus::SUBJECT;
	mail_info.set_subject(subject);

	Sleep(1);

	return SendResponse(Responses::OK);
}

int MailSession::ProcessQUIT()
{
	mail_info.SaveToFile();
	return SendResponse(Responses::SERVICE_CLOSING);
}
