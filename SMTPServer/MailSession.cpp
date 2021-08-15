// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "MailSession.h"

MailSession::MailSession(SOCKET client_socket)
{
	this->m_client_socket = client_socket;
}

const SOCKET& MailSession::get_client_socket() const
{
	return m_client_socket;
}

bool MailSession::ValidAdress(char* buf)
{
	size_t strlen_buf = strlen(buf);
	return (strlen_buf > MIN_MAIL_SIZE && strlen_buf < MAX_MAIL_SIZE && strchr(buf, '@'));
}

bool MailSession::ProcessConnectToDB()
{
	if (auto res_connect = m_mail_info.ConnectToDB())
	{
		if (auto take_res = m_mail_info.TakeDataFromDB())
		{
			return true;
		}
	}

	return false;
}

bool MailSession::IsEmpty()
{
	if (m_mail_info.get_login().empty() || m_mail_info.get_mail_from().empty() ||
		m_mail_info.get_password().empty() || m_mail_info.get_rcpt_to().empty() ||
		m_mail_info.get_subject().empty() || m_mail_info.get_text().empty())
	{
		return true;
	}

	return false;
}

void MailSession::ProcessSaveTo()
{
	m_mail_info.SaveToDatabase();
}

void MailSession::ProcessSaveToFile()
{
	m_mail_info.SaveToFile();
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

	switch (response_type)
	{
	case Responses::WELCOME_TO_CLIENT:
		strcpy(buf, WELCOME_STR);
		break;
	case Responses::SERVICE_CLOSING:
		strcpy(buf, CLOSING_CHANNEL_STR);
		break;
	case Responses::LOGIN_SUCCESS:
		strcpy(buf, LOGIN_STR);
		break;
	case Responses::OK:
		strcpy(buf, OK_STR);
		break;
	case Responses::BAD_SEQUENSE:
		strcpy(buf, BAD_SEQUENCE_STR);
		break;
	case Responses::COMMAND_NOT_IMPLEMENTED:
		strcpy(buf, COMMAND_N_IMPLEMENTED_STR);
		break;
	case Responses::EMAIL_N_RECEIVED:
		strcpy(buf, EMAIL_N_RCV_STR);
		break;
	case Responses::LOGIN_RCV:
		strcpy(buf, LOGIN_RCV_STR);
		break;
	case Responses::NO_USER:
		strcpy(buf, NO_USER_STR);
		break;
	case Responses::START_MAIL:
		strcpy(buf, START_MAIL_STR);
		break;
	case Responses::SYNTAX_ERROR:
		strcpy(buf, SYNTAX_ERROR_STR);
		break;
	case Responses::USER_NOT_LOCAL:
		strcpy(buf, USER_NOT_LOCAL_STR);
		break;
	default:
		strcpy(buf, NO_DESCRIPTION_STR);
		break;
	}

	send(m_client_socket, buf, sizeof(buf), 0);
	return response_type;
}

int MailSession::Processes(char* buf)
{

	if (_strnicmp(buf, HELO_COMMAND, FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessHELO(buf);
	}

	else if (_strnicmp(buf, EHLO_COMMAND, FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessHELO(buf);
	}

	else if (_strnicmp(buf, AUTH_COMMAND, FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessAUTH(buf);
	}

	else if (m_current_status == MailSessionStatus::LOGIN)
	{
		return ProcessAUTH(buf);
	}

	else if (m_current_status == MailSessionStatus::PASSWORD)
	{
		return ProcessAUTH(buf);
	}

	else if (_strnicmp(buf, MAIL_COMMAND, FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessMAIL(buf);
	}

	else if (_strnicmp(buf, RCPT_COMMAND, FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessRCPT(buf);
	}

	else if (_strnicmp(buf, DATA_COMMAND, FIRST_FOUR_SYMBOLS) == 0)
	{
		return ProcessDATA(buf);
	}

	else if (m_current_status == MailSessionStatus::SUBJECT)
	{
		return SubProcessEmail(buf);
	}

	else if (m_current_status == MailSessionStatus::DATA)
	{
		return SubProcessSubject(buf);
	}

	else if (_strnicmp(buf, QUIT_COMMAND, FIRST_FOUR_SYMBOLS) == 0)
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
	if (m_current_status != MailSessionStatus::EMPTY)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	if (strchr(buf, '.') == NULL)
	{
		return SendResponse(Responses::SYNTAX_ERROR);
	}

	m_current_status = MailSessionStatus::AUTH;

	return SendResponse(Responses::OK);
}

int MailSession::ProcessAUTH(char* buf)
{
	if (m_current_status == MailSessionStatus::AUTH)
	{
		m_current_status = MailSessionStatus::LOGIN;
		return SendResponse(Responses::LOGIN_RCV);
	}

	else if (m_current_status == MailSessionStatus::LOGIN)
	{
		return SubProcessLoginRecieve(buf);
	}

	else if (m_current_status == MailSessionStatus::PASSWORD)
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
	if (m_current_status != MailSessionStatus::AUTH_SUCCESS)
	{
		return SendResponse(Responses::BAD_SEQUENSE);
	}

	std::string address = CutAddress(buf);

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

	std::string address = CutAddress(buf);

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
	m_mail_info.set_login(buf);

	m_current_status = MailSessionStatus::PASSWORD;
	return SendResponse(Responses::LOGIN_RCV);
}

int MailSession::SubProcessPasswordRecieve(char* buf)
{
	m_mail_info.set_password(buf);

	if (m_mail_info.IsUserExist())
	{
		m_current_status = MailSessionStatus::AUTH_SUCCESS;
		return SendResponse(Responses::LOGIN_SUCCESS);
	}

	m_current_status = MailSessionStatus::EMPTY;
	return SendResponse(Responses::USER_NOT_LOCAL);
}

int MailSession::SubProcessEmail(char* buf)
{
	std::string text = buf;

	if (strstr(buf, SMTP_DATA_TERMINATOR))
	{
		size_t pos = text.find("\r\n", 0);
		text = text.substr(0, pos);
		m_mail_info.set_text(text);

		m_current_status = MailSessionStatus::QUIT;

		return 1;
	}

	return SendResponse(Responses::EMAIL_N_RECEIVED);
}

int MailSession::SubProcessSubject(char* buf)
{
	if (_strnicmp(buf, SUBJECT_COMMAND, FIRST_EIGHT_SYMBOLS) != 0)
	{
		return SendResponse(Responses::SYNTAX_ERROR);
	}

	std::string subject = CutSubject(buf);

	m_current_status = MailSessionStatus::SUBJECT;
	m_mail_info.set_subject(subject);

	Sleep(1);

	return SendResponse(Responses::OK);
}

int MailSession::ProcessQUIT()
{
	return SendResponse(Responses::SERVICE_CLOSING);
}
