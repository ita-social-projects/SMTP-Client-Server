#pragma once

#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <string>
#include <ctime>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include "base64.h"
#include "..\CPPLogger\CPPLogger.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")

constexpr auto	DEFAULT_SSL_PORT	= "465";	// default port for connection through ssl secure connection
constexpr auto	DEFAULT_BUFFER_SIZE	= 10240;	// size of buffer, where will be storing answers from server
const int		MAX_FILE_DESCRIPTOR	= 1;		// this value will be used in select method, to set max file descriptor we want to monitor
// class which will be created and then throwned if any kind of error occurs
// it stores error code
class SMTPErrorClass
{
public:
	// enum class which defines different kinds of errors
	enum class SMTPErrorEnum
	{
		SMTP_NO_ERROR = 0,
		SSL_ERROR,
		WSA_STARTUP_ERROR,
		WSA_HOSTNAME_ERROR,
		WSA_INVALID_SOCKET_ERROR,
		WSA_SELECT_ERROR,
		WSA_SEND_ERROR,
		WSA_RECEIVE_ERROR,
		SERVER_NOT_RESPONDING,
		SERVER_HELLO_ANSWER,
		SERVER_AUTH_LOGIN_ANSWER,
		SERVER_AUTHORIZATION_FAILED,
		SERVER_MAIL_FROM_ANSWER,
		SERVER_RCPT_TO_ANSWER,
		SERVER_DATA_ANSWER,
		SERVER_LETTER_ANSWER,
		SERVER_QUIT_ANSWER,
		SEND_MSG_EMPTY,
		LACK_OF_BUFFER_MEMORY,
		CONNECTION_CLOSED,
		STRING_ARGUMENT_EMPTY,
		UNDEF_USER_LOGIN,
		UNDEF_USER_PASSWORD,
		SERVER_NOT_READY,
		UNDEF_SERVER_CHOICE,
		INVALID_LOGIN
	};

	SMTPErrorClass(SMTPErrorEnum error) : m_error_code(error) {}
	std::string GetErrorText() const;	
	
private:
	SMTPErrorEnum m_error_code;
};

class SMTPClientClass
{
public:
	SMTPClientClass();
	~SMTPClientClass();		

	enum class SMTPServerResponce
	{
		SERVER_READY			= 220,
		SERVER_QUIT				= 221,
		SERVER_AUTH_SUCCESSFUL	= 235,
		SERVER_OKAY				= 250,
		SERVER_AUTH_LOGIN		= 334,
		SERVER_START_MAIL		= 354
	};	
	
	// Sets user e-mail
	bool	set_login(const std::string&);
	// Sets port to connect
	bool	set_port(const std::string&);
	// Sets time (in sec) for how long client will be waiting for server respone
	void	set_server_timeout(const int);
	// Sets user password for e-mail
	bool	set_password(const std::string&);
	// Sets e-mail of recepient
	bool	set_recepient_email(const std::string&);
	// Sets message subject
	bool	set_subject(const std::string&);
	// Sets message data
	bool	set_letter_message(const std::string&);
	// Set ServerChoice
	bool	set_smtp_address(const std::string&);
	// Starts the procedure of sending all part of e-mail message
	bool	Send();		

protected:	
	
	// Initializes Winsock2, creates socket and tries to connect to server
	virtual bool OpenConnection();	
	// Sends data through non-secure connection
	virtual bool SendData(const std::string&);
	// Receives response from server through non-secure connection
	virtual bool ReceiveData();
	// Checks response code in server answer
	int		GetResponseCode() const;
	// Sends 'ehlo gmail.com'
	void	SendHello();
	// Sends 'auth login'
	void	SendAuthLogin();
	// Sends 'mail from:<user e-mail>'
	void	SendMailFrom();
	// Sends 'rcpt to:<recepient e-mail>'
	void	SendRcptTo();
	// Sends 'subject:'
	void	SendSubject();
	// Sends 'quit'
	void	SendQuit();
	// Sends message to server that signals about ending of data transwer
	void	SendEndingDot();
	// Sends quit message and closes the socket
	void	DisconnectFromServer();
	// Flush receive_buffer
	void	FlushBuffer();

protected:

	SOCKET						m_socket;
	std::unique_ptr<char[]>		m_receive_buffer;	
	bool						m_connect_status;	
	std::string					m_smtp_address;
	std::string					m_login;
	std::string					m_password;
	std::string					m_recepient_email;
	std::string					m_subject;
	std::string					m_letter_message;
	int							m_server_timeout;
	Logger						LOG;

private:
	std::string					m_port;
};

class SMTPSecureClientClass final : public SMTPClientClass
{
public:
	SMTPSecureClientClass();
	~SMTPSecureClientClass();

private:

	bool	OpenConnection() override;
	// Initializes SSL library
	bool	InitSSLCTX();  
	// Frees SSL aquired resources and setting pointers to NULL
	void	FreeOpenSSLResources(); 
	// Tries to connect to server through secure(ssl) connection
	bool	OpenSSLConnect(); 
	// Sends data through secure connection
	bool	SendData(const std::string& msg_to_send) override;	
	// Receives response from server through secure connection 
	bool	ReceiveData() override; 

private:
	
	SSL_CTX* m_ctx;
	SSL*	 m_ssl;	
};
