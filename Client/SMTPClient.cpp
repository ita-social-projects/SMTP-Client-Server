// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "SMTPClient.h"

SMTPClientClass::SMTPClientClass()
{	
	m_receive_buffer = std::make_unique<char[]>(DEFAULT_BUFFER_SIZE);

	if (m_receive_buffer == nullptr)
	{
		LOG_ERROR << "Lack of memory for message buffer.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::LACK_OF_BUFFER_MEMORY);
	}
		
	SecureZeroMemory(m_receive_buffer.get(), DEFAULT_BUFFER_SIZE);
	
	m_socket			= INVALID_SOCKET;	
	m_connect_status	= false;	
	m_server_timeout	= 0;
}

SMTPClientClass::~SMTPClientClass()
{
	if (m_connect_status)
	{
		DisconnectFromServer();
	}
	
	WSACleanup();
}

bool	SMTPClientClass::OpenConnection()
{
	// Winsock initialization
	WSAData wsa_data;
	struct addrinfo* result = nullptr,
		* ptr = nullptr,
		hints;	
	
	int check_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (check_result != 0)
	{
		WSACleanup();
		LOG_ERROR << "Couldn't initialize Winsock2.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_STARTUP_ERROR);
	}

	SecureZeroMemory(&hints, sizeof(hints));
	hints.ai_family		= AF_UNSPEC; // unspecify what type of IP address we will be using (IPv4 or IPv6)
	hints.ai_socktype	= SOCK_STREAM;
	hints.ai_protocol	= IPPROTO_TCP;	

	
	check_result = getaddrinfo(m_smtp_address.c_str(), m_port.c_str(), &hints, &result);
	if (check_result != 0)
	{
		WSACleanup();
		LOG_ERROR << "Couldn't get addrinfo for smtp server.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_HOSTNAME_ERROR);
	}

	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connection to a server
		m_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (m_socket == INVALID_SOCKET)
		{
			WSACleanup();
			LOG_ERROR << "Invalid socket.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_INVALID_SOCKET_ERROR);
		}

		// Connect to a server
		check_result = connect(m_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (check_result == SOCKET_ERROR)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (m_socket == INVALID_SOCKET)
	{
		WSACleanup();
		LOG_ERROR << "Invalid socket.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_INVALID_SOCKET_ERROR);
	}

	LOG_INFO << "Successfully connected to the server.";
	return true;
}

bool	SMTPClientClass::SendData(const std::string &msg_to_send) 
{
	fd_set		fdwrite;
	timeval		time;
	int			result;
	u_int		index		= 0;
	u_int		msg_left	= (u_int)msg_to_send.size();	

	if (msg_to_send.empty())
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SEND_MSG_EMPTY);

	while ((int)msg_left > 0)
	{
		time.tv_sec = m_server_timeout;
		FD_ZERO(&fdwrite);

		FD_SET(m_socket, &fdwrite);

		result = select((int)m_socket + MAX_FILE_DESCRIPTOR, NULL, &fdwrite, NULL, &time);

		if (result == SOCKET_ERROR)
		{
			FD_CLR(m_socket, &fdwrite);
			LOG_ERROR << "Winsock select() error.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_SELECT_ERROR);
		}

		if (result == 0)
		{			
			FD_CLR(m_socket, &fdwrite);
			LOG_ERROR << "Server is not responding.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_NOT_RESPONDING);
		}

		if (result > 0 && FD_ISSET(m_socket, &fdwrite))
		{
			result = send(m_socket, &msg_to_send.c_str()[(size_t)index], (int)msg_left, 0);
			if (result == SOCKET_ERROR || result == 0)
			{
				FD_CLR(m_socket, &fdwrite);
				LOG_ERROR << "Winsock send() error.";
				throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_SEND_ERROR);
			}
			msg_left	-= result;
			index		+= result;
		}
	}

	FD_CLR(m_socket, &fdwrite);

	return true;
}

bool	SMTPClientClass::ReceiveData()
{
	int		result	= 0;
	fd_set	fdread;
	timeval	time;

	time.tv_sec		= m_server_timeout;

	FD_ZERO(&fdread);
	FD_SET(m_socket, &fdread);

	result = select((int)m_socket + MAX_FILE_DESCRIPTOR, &fdread, NULL, NULL, &time);

	if (result == SOCKET_ERROR)
	{
		FD_CLR(m_socket, &fdread);
		LOG_ERROR << "Winsock select() error.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_SELECT_ERROR);
	}

	if (result == 0)
	{		
		FD_CLR(m_socket, &fdread);
		LOG_ERROR << "Server is not responding.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_NOT_RESPONDING);
	}

	if (FD_ISSET(m_socket, &fdread))
	{
		result = recv(m_socket, m_receive_buffer.get(), DEFAULT_BUFFER_SIZE, 0);
		if (result == SOCKET_ERROR)
		{
			FD_CLR(m_socket, &fdread);
			LOG_ERROR << "Winsock recv() error.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_RECEIVE_ERROR);
		}
	}

	FD_CLR(m_socket, &fdread);
	m_receive_buffer.get()[(size_t)result] = 0;

	if (result == 0)
	{
		LOG_ERROR << "Connection with server closed during receive process.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::CONNECTION_CLOSED);
	}

	return true;

}

bool	SMTPClientClass::set_login(const std::string& s)
{
	if (s.empty())
	{
		LOG_ERROR << "User login is not specified.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::STRING_ARGUMENT_EMPTY);
	}
	m_login = s;

	return true;
}

bool	SMTPClientClass::set_port(const std::string& port)
{
	if (port.empty())
	{
		LOG_ERROR << "Server port is not specified.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::STRING_ARGUMENT_EMPTY);
	}
	m_port = port;

	return true;
}

void	SMTPClientClass::set_server_timeout(const int t)
{
	m_server_timeout = t;	
}

bool	SMTPClientClass::set_password(const std::string& s)
{
	if (s.empty())
	{
		LOG_ERROR << "User password was not specified.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::STRING_ARGUMENT_EMPTY);
	}
	m_password = s;

	return true;
}

bool	SMTPClientClass::set_recepient_email(const std::string& s)
{
	if (s.empty())
	{
		LOG_ERROR << "Recepient email was not specified.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::STRING_ARGUMENT_EMPTY);
	}
	m_recepient_email = s;

	return true;
}

void	SMTPClientClass::set_subject(const std::string& s)
{
	if (!s.empty())
	{
		m_subject = s;	
	}
}

bool	SMTPClientClass::set_letter_message(const std::string& s)
{
	if (s.empty())
	{
		LOG_ERROR << "Message data is empty.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::STRING_ARGUMENT_EMPTY);
	}
	m_letter_message = s;

	return true;
}

bool	SMTPClientClass::Send()
{
	OpenConnection();	

	if (m_socket == INVALID_SOCKET)
	{
		LOG_ERROR << "Invalid socket.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_INVALID_SOCKET_ERROR);
	}

	try
	{
		// Initiate full sending procedure		
		ReceiveData();
		if (GetResponseCode() != (int)SMTPServerResponce::SERVER_READY)
		{
			LOG_ERROR << "Server is not ready.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_NOT_READY);
		}
		FlushBuffer();
		m_connect_status = true;		

		SendHello();
		ReceiveData();
		if (GetResponseCode() != (int)SMTPServerResponce::SERVER_OKAY)
		{
			LOG_ERROR << "Server responded with error to EHLO";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_HELLO_ANSWER);
		}
		FlushBuffer();

		SendAuthLogin();
		ReceiveData();
		if (GetResponseCode() != (int)SMTPServerResponce::SERVER_AUTH_LOGIN)
		{
			LOG_ERROR << "Server responded with error to AUTH LOGIN";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_AUTH_LOGIN_ANSWER);
		}
		FlushBuffer();

		if (m_login.empty())
		{
			LOG_ERROR << "User login was not specified.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::UNDEF_USER_LOGIN);
		}		
		std::string encoded_login = base64_encode((const unsigned char*)m_login.c_str(), (u_int)m_login.size());
		encoded_login.append("\r\n");
		SendData(encoded_login);		

		ReceiveData();
		if (GetResponseCode() != (int)SMTPServerResponce::SERVER_AUTH_LOGIN)
		{
			LOG_ERROR << "Server responded with error to user login.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_AUTH_LOGIN_ANSWER);
		}
		FlushBuffer();

		if (m_password.empty())
		{
			LOG_ERROR << "User password was not specified.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::UNDEF_USER_PASSWORD);
		}
		
		std::string encoded_password = base64_encode((const unsigned char*)m_password.c_str(), (u_int)m_password.size());
		encoded_password.append("\r\n");
		SendData(encoded_password);
		

		ReceiveData();
		if (GetResponseCode() != (int)SMTPServerResponce::SERVER_AUTH_SUCCESSFUL)
		{
			LOG_ERROR << "Server responded with error to user authorization.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_AUTHORIZATION_FAILED);
		}
		FlushBuffer();

		SendMailFrom();
		ReceiveData();
		if (GetResponseCode() != (int)SMTPServerResponce::SERVER_OKAY)
		{
			LOG_ERROR << "Server responded with error to MAIL FROM:";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_MAIL_FROM_ANSWER);
		}
		FlushBuffer();

		SendRcptTo();	
		ReceiveData();
		if (GetResponseCode() != (int)SMTPServerResponce::SERVER_OKAY)
		{
			LOG_ERROR << "Server responded with error to RCPT TO:";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_RCPT_TO_ANSWER);
		}
		FlushBuffer();

		const std::string DATA = "DATA\r\n";
		SendData(DATA);
		ReceiveData();
		if (GetResponseCode() != (int)SMTPServerResponce::SERVER_START_MAIL)
		{
			LOG_ERROR << "Server responded with error to DATA.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_DATA_ANSWER);
		}
		FlushBuffer();

		if (!m_subject.empty())
		{
			SendSubject();
		}

		SendData(m_letter_message.append("\r\n"));
		SendEndingDot();

		ReceiveData();
		if (GetResponseCode() != (int)SMTPServerResponce::SERVER_OKAY)
		{
			LOG_ERROR << "Server responded with error to message ending procedure.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_LETTER_ANSWER);
		}

		SendQuit();
		ReceiveData();
		if (GetResponseCode() != (int)SMTPServerResponce::SERVER_QUIT)
		{
			LOG_ERROR << "Server responded with error to QUIT";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_QUIT_ANSWER);
		}
	}
	catch (const SMTPErrorClass&)
	{
		DisconnectFromServer();
		throw;
	}

	return true;
}

int		SMTPClientClass::GetResponseCode() const
{
	// Transform value from ASCII into integer
	const int	TO_HUNDREDS		= 100;
	const int	TO_TENS			= 10;
	const char	CHAR_TO_INT		= '0';
	const int	BUF_FIRST_CHAR	= 0;
	const int	BUF_SECOND_CHAR	= 1;
	const int	BUF_THIRD_CHAR	= 2;

	return (m_receive_buffer.get()[BUF_FIRST_CHAR] - CHAR_TO_INT) * TO_HUNDREDS + (m_receive_buffer.get()[BUF_SECOND_CHAR] - CHAR_TO_INT) * TO_TENS + m_receive_buffer.get()[BUF_THIRD_CHAR] - CHAR_TO_INT;
}

void	SMTPClientClass::SendHello()
{
	const char	AT_SIGN = '@';
	size_t		index;
	std::string	mail_domain;
	
	if (m_login.size())
	{
		index = m_login.find(AT_SIGN);
		if (index == m_login.size())
		{
			LOG_ERROR << "Domain of user email is invalid.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::INVALID_LOGIN);
		}

		mail_domain = m_login.substr(index + 1);
	}
	std::string hello = "ehlo ";
	hello.append(mail_domain);
	hello.append("\r\n");
	SendData(hello);
}

void	SMTPClientClass::SendAuthLogin()
{
	const std::string AUTH_LOGIN = "auth login\r\n";
	SendData(AUTH_LOGIN);
}

void	SMTPClientClass::SendMailFrom()
{
	std::string mail_from = "mail from:<";
	mail_from.append(m_login);
	mail_from.append(">\r\n");
	SendData(mail_from);
}

void	SMTPClientClass::SendRcptTo()
{
	std::string rcpt_to = "rcpt to:<";
	rcpt_to.append(m_recepient_email);
	rcpt_to.append(">\r\n");
	SendData(rcpt_to);
}

void	SMTPClientClass::SendSubject()
{
	std::string subject = "subject: ";
	subject.append(m_subject);
	subject.append("\r\n");
	SendData(subject);
}

void	SMTPClientClass::SendQuit()
{
	const std::string QUIT = "quit\r\n";
	SendData(QUIT);	
}

void	SMTPClientClass::SendEndingDot()
{
	const std::string DOT = "\r\n.\r\n";
	SendData(DOT);
}

void	SMTPClientClass::DisconnectFromServer()
{
	if (m_connect_status)
	{
		SendQuit();
		m_connect_status = false;
	}
	if (m_socket)
	{
		closesocket(m_socket);
	}

	m_socket = INVALID_SOCKET;
	LOG_INFO << "Successfully closed connection.";
}

void	SMTPClientClass::FlushBuffer()
{
	SecureZeroMemory(m_receive_buffer.get(), DEFAULT_BUFFER_SIZE);
}

bool SMTPClientClass::set_smtp_address(const std::string& s)
{
	if (s.size())
	{		
		m_smtp_address = s;
		return true;
	}	
	
	return false;
}

std::string SMTPErrorClass::GetErrorText() const
{
	switch (m_error_code)
	{
	case SMTPErrorEnum::SMTP_NO_ERROR:
		return "No errors";
	case SMTPErrorEnum::SSL_ERROR:
		return "Error during SSL connection";
	case SMTPErrorEnum::WSA_STARTUP_ERROR:
		return "Unable to initialize Winsock2";
	case SMTPErrorEnum::WSA_HOSTNAME_ERROR:
		return "Couldn't open connection to the chosen host";
	case SMTPErrorEnum::WSA_INVALID_SOCKET_ERROR:
		return "Invalid Winsock2 socket";
	case SMTPErrorEnum::WSA_SELECT_ERROR:
		return "Function select() error";
	case SMTPErrorEnum::WSA_SEND_ERROR:
		return "Function send() error";
	case SMTPErrorEnum::WSA_RECEIVE_ERROR:
		return "Function recv() error";
	case SMTPErrorEnum::SERVER_NOT_RESPONDING:
		return "Server not responding";
	case SMTPErrorEnum::SERVER_HELLO_ANSWER:
		return "Server returned error after sending EHLO";
	case SMTPErrorEnum::SERVER_AUTH_LOGIN_ANSWER:
		return "Server returned error after sending AUTH LOGIN";
	case SMTPErrorEnum::SERVER_AUTHORIZATION_FAILED:
		return "Server failed to authorizate user";
	case SMTPErrorEnum::SERVER_MAIL_FROM_ANSWER:
		return "Server returned error after sending MAIL FROM:";
	case SMTPErrorEnum::SERVER_RCPT_TO_ANSWER:
		return "Server returned error after sending RCPT TO:";
	case SMTPErrorEnum::SERVER_DATA_ANSWER:
		return "Server returned error after sending DATA";
	case SMTPErrorEnum::SERVER_LETTER_ANSWER:
		return "Server returned error after sending letter body";
	case SMTPErrorEnum::SERVER_QUIT_ANSWER:
		return "Server returned error after sending QUIT, but your letter should be delivered";
	case SMTPErrorEnum::SEND_MSG_EMPTY:
		return "Sending message is empty";
	case SMTPErrorEnum::LACK_OF_BUFFER_MEMORY:
		return "System couldn't allocate enough memory for receive buffer";
	case SMTPErrorEnum::CONNECTION_CLOSED:
		return "Couldn't finished operation, because connection with server closed";
	case SMTPErrorEnum::STRING_ARGUMENT_EMPTY:
		return "Couldn't set value, because argument was empty string";
	case SMTPErrorEnum::UNDEF_USER_LOGIN:
		return "User login is undefined";
	case SMTPErrorEnum::UNDEF_USER_PASSWORD:
		return "User password is undefined";
	case SMTPErrorEnum::SERVER_NOT_READY:
		return "Server returned 'NOT READY' after connection was open";
	case SMTPErrorEnum::UNDEF_SERVER_CHOICE:
		return "User didn't define server to connect";
	default:
		return "Undefined error id";
	}
}

SMTPSecureClientClass::SMTPSecureClientClass()
{	
	m_receive_buffer = std::make_unique<char[]>(DEFAULT_BUFFER_SIZE);
	
	if (m_receive_buffer == nullptr)
	{
		LOG_ERROR << "Lack of memory for message buffer.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::LACK_OF_BUFFER_MEMORY);
	}

	SecureZeroMemory(m_receive_buffer.get(), DEFAULT_BUFFER_SIZE);

	m_connect_status	= false;
	m_ctx				= nullptr;
	m_ssl				= nullptr;
	m_socket			= INVALID_SOCKET;	
}

SMTPSecureClientClass::~SMTPSecureClientClass()
{
	if (m_connect_status)
	{
		DisconnectFromServer();
	}

	FreeOpenSSLResources();
	WSACleanup();
}

bool SMTPSecureClientClass::OpenConnection()
{
	// Winsock initialization
	WSAData wsa_data;
	struct addrinfo* result = nullptr,
		* ptr = nullptr,
		hints;	

	int check_result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
	if (check_result != 0)
	{
		WSACleanup();
		LOG_ERROR << "Couldn't initialize Winsock2.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_STARTUP_ERROR);
	}

	SecureZeroMemory(&hints, sizeof(hints));

	hints.ai_family		= AF_UNSPEC; // unspecify what type of IP address we will be using (IPv4(AF_INET) or IPv6(AF_INET6))
	hints.ai_socktype	= SOCK_STREAM;
	hints.ai_protocol	= IPPROTO_TCP;

	
	check_result = getaddrinfo(m_smtp_address.c_str(), DEFAULT_SSL_PORT, &hints, &result);
	if (check_result != 0)
	{
		WSACleanup();
		LOG_ERROR << "Couldn't get addrinfo for smtp server.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_HOSTNAME_ERROR);
	}

	for (ptr = result; ptr != nullptr; ptr = ptr->ai_next)
	{
		// Create a SOCKET for connection to a server
		m_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (m_socket == INVALID_SOCKET)
		{
			WSACleanup();
			LOG_ERROR << "Invalid socket.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_INVALID_SOCKET_ERROR);
		}

		// Connect to a server
		check_result = connect(m_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (check_result == SOCKET_ERROR)
		{
			closesocket(m_socket);
			m_socket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (m_socket == INVALID_SOCKET)
	{
		WSACleanup();
		LOG_ERROR << "Invalid socket.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_INVALID_SOCKET_ERROR);
	}

	InitSSLCTX();
	OpenSSLConnect();

	return true;
}

bool SMTPSecureClientClass::InitSSLCTX()
{
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	m_ctx = SSL_CTX_new(SSLv23_client_method());
	if (m_ctx == nullptr)
	{
		LOG_ERROR << "SSL initialization error";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SSL_ERROR);
	}

	LOG_INFO << "Successfully initialized SSL.";
	return true;
}

void SMTPSecureClientClass::FreeOpenSSLResources()
{
	if (m_ssl != nullptr)
	{
		SSL_shutdown(m_ssl);
		SSL_free(m_ssl);
		m_ssl = nullptr;
	}
	if (m_ctx != nullptr)
	{
		SSL_CTX_free(m_ctx);
		m_ctx = nullptr;
	}
}

bool SMTPSecureClientClass::OpenSSLConnect()
{
	if (m_ctx == nullptr)
	{
		LOG_ERROR << "SSL error(CTX).";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SSL_ERROR);
	}

	m_ssl = SSL_new(m_ctx);
	if (m_ssl == nullptr)
	{
		LOG_ERROR << "SSL error(SSL).";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SSL_ERROR);
	}

	SSL_set_fd(m_ssl, (int)m_socket);
	SSL_set_mode(m_ssl, SSL_MODE_AUTO_RETRY);

	fd_set	fdwrite;
	fd_set	fdread;
	timeval	time;
	int		result = 0;
	int		write_blocked = 0;
	int		read_blocked = 0;

	while (true)
	{
		time.tv_sec = m_server_timeout;
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdread);

		if (write_blocked)
			FD_SET(m_socket, &fdwrite);

		if (read_blocked)
			FD_SET(m_socket, &fdread);

		if (write_blocked || read_blocked)
		{
			write_blocked = 0;
			read_blocked = 0;
			result = select((int)m_socket + MAX_FILE_DESCRIPTOR, &fdread, &fdwrite, NULL, &time);

			if (result == SOCKET_ERROR)
			{
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdread);
				LOG_ERROR << "Winsock select() error.";
				throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_SELECT_ERROR);
			}

			// timeout has happened
			if (result == 0)
			{
				FD_ZERO(&fdwrite);
				FD_ZERO(&fdread);
				LOG_ERROR << "Server is not responding.";
				throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_NOT_RESPONDING);
			}
		}

		result = SSL_connect(m_ssl);

		switch (SSL_get_error(m_ssl, result))
		{
		case SSL_ERROR_NONE:
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			return true;

		case SSL_ERROR_WANT_WRITE:
			write_blocked = 1;
			break;

		case SSL_ERROR_WANT_READ:
			read_blocked = 1;
			break;

		default:
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			LOG_ERROR << "SSL_connect error.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SSL_ERROR);
		}
	}

	LOG_INFO << "Successfully connected to the server through secured connection.";
	return true;
}

bool SMTPSecureClientClass::SendData(const std::string& msg_to_send)
{
	int		result;
	fd_set	fdwrite;
	fd_set	fdread;
	timeval	time;

	u_int	msg_left				= (u_int)msg_to_send.size();
	u_int	offset					= 0;
	int		write_blocked_on_read	= 0;

	if (msg_to_send.empty())
	{
		LOG_ERROR << "Message data is empty.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SEND_MSG_EMPTY);
	}

	while ((int)msg_left > 0)
	{
		time.tv_sec = m_server_timeout;
		FD_ZERO(&fdwrite);
		FD_ZERO(&fdread);

		FD_SET(m_socket, &fdwrite);

		if (write_blocked_on_read)
		{
			FD_SET(m_socket, &fdread);
		}

		result = select((int)m_socket + MAX_FILE_DESCRIPTOR, &fdread, &fdwrite, NULL, &time);

		if (result == SOCKET_ERROR)
		{
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			LOG_ERROR << "Winsock select() error.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_SELECT_ERROR);
		}

		if (result == 0)
		{
			FD_ZERO(&fdwrite);
			FD_ZERO(&fdread);
			LOG_ERROR << "Server is not responding.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_NOT_RESPONDING);
		}

		if (FD_ISSET(m_socket, &fdwrite) || (write_blocked_on_read && FD_ISSET(m_socket, &fdread)))
		{
			write_blocked_on_read = 0;

			result = SSL_write(m_ssl, (msg_to_send.c_str()) + (size_t)offset, (int)msg_left);

			switch (SSL_get_error(m_ssl, (int)result))
			{
			case SSL_ERROR_NONE:
				msg_left -= result;
				offset += result;
				break;

			case SSL_ERROR_WANT_WRITE:
				break;

			case SSL_ERROR_WANT_READ:
				write_blocked_on_read = 1;
				break;

			default:
				FD_ZERO(&fdread);
				FD_ZERO(&fdwrite);
				LOG_ERROR << "SSL_write error.";
				throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SSL_ERROR);
			}
		}
	}

	FD_ZERO(&fdwrite);
	FD_ZERO(&fdread);

	return true;
}

bool SMTPSecureClientClass::ReceiveData()
{
	fd_set	fdread;
	fd_set	fdwrite;
	timeval	time;

	const int	BUFFER_SIZE				= 1024;
	int			result					= 0;
	u_int		offset					= 0;
	int			read_blocked_on_write	= 0;
	bool		sending_finished		= false;

	while (!sending_finished)
	{
		time.tv_sec = m_server_timeout;
		FD_ZERO(&fdread);
		FD_ZERO(&fdwrite);

		FD_SET(m_socket, &fdread);

		if (read_blocked_on_write)
		{
			FD_SET(m_socket, &fdwrite);
		}

		result = select((int)m_socket + MAX_FILE_DESCRIPTOR, &fdread, &fdwrite, NULL, &time);

		if (result == SOCKET_ERROR)
		{
			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			LOG_ERROR << "Winsock select() error.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::WSA_SELECT_ERROR);
		}

		if (result == 0)
		{
			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);
			LOG_ERROR << "Server is not responding.";
			throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SERVER_NOT_RESPONDING);
		}

		if (FD_ISSET(m_socket, &fdread) || (read_blocked_on_write && FD_ISSET(m_socket, &fdwrite)))
		{
			while (true)
			{
				read_blocked_on_write = 0;

				char buffer[BUFFER_SIZE];
				int ssl_error;

				result = SSL_read(m_ssl, buffer, BUFFER_SIZE);
				ssl_error = SSL_get_error(m_ssl, (int)result);
				if (ssl_error == SSL_ERROR_NONE)
				{
					if (offset + result > DEFAULT_BUFFER_SIZE - 1)
					{
						FD_ZERO(&fdread);
						FD_ZERO(&fdwrite);
						LOG_ERROR << "Lack of memory to store the server responce.";
						throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::LACK_OF_BUFFER_MEMORY);
					}

					memcpy(m_receive_buffer.get() + (size_t)offset, buffer, (size_t)result);
					offset += result;

					if (SSL_pending(m_ssl))
					{
						continue;
					}
					else
					{
						sending_finished = true;
						break;
					}
				}
				else if (ssl_error == SSL_ERROR_ZERO_RETURN)
				{
					sending_finished = true;
					break;
				}
				else if (ssl_error == SSL_ERROR_WANT_READ)
				{
					break;
				}
				else if (ssl_error == SSL_ERROR_WANT_WRITE)
				{
					read_blocked_on_write = 1;
					break;
				}
				else
				{
					FD_ZERO(&fdread);
					FD_ZERO(&fdwrite);
					LOG_ERROR << "SSL error during receive procedure.";
					throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::SSL_ERROR);
				}
			}
		}
	}

	FD_ZERO(&fdread);
	FD_ZERO(&fdwrite);
	m_receive_buffer.get()[(size_t)offset] = 0;

	if (offset == 0)
	{
		LOG_ERROR << "Connection is closed during receive procedure.";
		throw SMTPErrorClass(SMTPErrorClass::SMTPErrorEnum::CONNECTION_CLOSED);
	}

	return true;
}
