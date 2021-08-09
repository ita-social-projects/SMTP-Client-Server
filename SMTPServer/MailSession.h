#pragma once
#include <iostream>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#include "MailInfo.h"

constexpr auto MAX_ADDRESS_LENGTH = 256;
constexpr auto SMTP_DATA_TERMINATOR = "\r\n.\r\n";
constexpr auto FIRST_FOUR_SYMBOLS = 4;
constexpr auto FIRST_EIGHT_SYMBOLS = 8;
constexpr auto MIN_MAIL_SIZE = 2;
constexpr auto MAX_MAIL_SIZE = 255;
constexpr auto RESPONSE_BUF_SIZE = 64;

constexpr auto WELCOME_STR = "220 Welcome!\r\n";
constexpr auto CLOSING_CHANNEL_STR = "221 Service closing transmission channel\r\n";
constexpr auto LOGIN_STR = "235 Successfull login\r\n";
constexpr auto OK_STR = "250 OK\r\n";
constexpr auto LOGIN_RCV_STR = "334 Login has recieved\r\n";
constexpr auto START_MAIL_STR = "354 Start mail input; end with <CRLF>.<CRLF>\r\n";
constexpr auto SYNTAX_ERROR_STR = "501 Syntax error in parameters or arguments\r\n";
constexpr auto COMMAND_N_IMPLEMENTED_STR = "502 Command not implemented\r\n";
constexpr auto BAD_SEQUENCE_STR = "503 Bad sequence of commands\r\n";
constexpr auto NO_USER_STR = "550 No such user\r\n";
constexpr auto USER_NOT_LOCAL_STR = "551 User not local. Can not forward the mail\r\n";
constexpr auto NO_DESCRIPTION_STR = "No description\r\n";
constexpr auto EMAIL_N_RCV_STR = "504 Email not received!\r\n";

constexpr auto HELO_COMMAND = "HELO";
constexpr auto EHLO_COMMAND = "EHLO";
constexpr auto AUTH_COMMAND = "AUTH";
constexpr auto MAIL_COMMAND = "MAIL";
constexpr auto RCPT_COMMAND = "RCPT";
constexpr auto DATA_COMMAND = "DATA";
constexpr auto QUIT_COMMAND = "QUIT";
constexpr auto SUBJECT_COMMAND = "Subject:";




enum MailSessionStatus
{
	EMPTY,
	EHLO,
	AUTH,
	LOGIN,
	PASSWORD,
	AUTH_SUCCESS,
	MAIL_FROM, 
	RCPT_TO,
	DATA,
	SUBJECT,
	QUIT
};

enum Responses
{
	WELCOME_TO_CLIENT = 220,
	SERVICE_CLOSING,
	LOGIN_SUCCESS = 235,
	OK = 250,
	LOGIN_RCV = 334,
	START_MAIL = 354,
	SYNTAX_ERROR = 501,
	COMMAND_NOT_IMPLEMENTED,
	BAD_SEQUENSE,
	EMAIL_N_RECEIVED,
	NO_USER = 550,
	USER_NOT_LOCAL
};

class MailSession
{
public:
	MailSession() = delete;
	MailSession(SOCKET client_socket);

	const SOCKET& get_client_socket() const;

	int SendResponse(int response_type);
	int Processes(char* buf);

private:
	int ProcessNotImplemented();
	int ProcessHELO(char* buf);
	int ProcessAUTH(char* buf);
	int ProcessMAIL(char* buf);
	int ProcessRCPT(char* buf);
	int ProcessDATA(char* buf);

	int SubProcessLoginRecieve(char* buf);
	int SubProcessPasswordRecieve(char* buf);
	int SubProcessEmail(char* buf);
	int SubProcessSubject(char* buf);

	int ProcessQUIT();

	bool ValidAdress(char* buf);

	std::string CutSubject(char* buf);
	std::string CutAddress(char* buf);

private:
	SOCKET m_client_socket;
	MailInfo m_mail_info;
	int m_current_status = MailSessionStatus::EMPTY;
};

