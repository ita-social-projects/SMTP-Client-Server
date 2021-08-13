// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include "..\CPPLogger\CPPLogger.h"

constexpr auto SERVER_NAME_DB					= "ServTest";
constexpr auto SERVER_DISPLAY_NAME			= "ServTestserver";
constexpr unsigned int SERVER_LISTENER_PORT = 25;
constexpr auto SERVER_IP_ADDRESS			= "localhost";
constexpr unsigned int SOCKET_TIMEOUT		= 5;
constexpr auto LOG_FILENAME					= "C:\\Logs\\";
constexpr unsigned int LOG_LEVEL			= 2;
constexpr unsigned int THREAD_INTERVAL		= 30;
constexpr unsigned int MAX_WORKING_THREADS	= 10;
constexpr auto XML_FILE_PATH				= "Config.xml";

constexpr auto ATTR_ROOT					= "root";
constexpr auto ATTR_SERVER					= "Server";
constexpr auto ATTR_SERVER_NAME				= "servername";
constexpr auto ATTR_SERVER_DISPLAY_NAME		= "serverdisplayname";
constexpr auto ATTR_SERVER_LISTENER_PORT	= "listenerport";
constexpr auto ATTR_SERVER_IP_ADDRESS		= "ipaddress";
constexpr auto ATTR_COMMUNICATION_SETTINGS	= "communicationsettings";
constexpr auto ATTR_SOCKET_BLOCKING			= "blocking";
constexpr auto ATTR_SOCKET_TIMEOUT			= "socket_timeout";
constexpr auto ATTR_LOGGING					= "logging";
constexpr auto ATTR_LOG_FILENAME			= "filename";
constexpr auto ATTR_LOG_LEVEL				= "LogLevel";
constexpr auto ATTR_LOG_FLUSH				= "flush";
constexpr auto ATTR_TIME					= "time";
constexpr auto ATTR_THREAD_INTERVAL			= "Period_time";
constexpr auto ATTR_THREAD_POOL				= "threadpool";
constexpr auto ATTR_MAX_WORKING_THREADS		= "maxworkingthreads";



class IXMLParser
{
public:
	virtual std::string GetServerName() = 0;
	virtual std::string GetServerDisplayName() = 0;
	virtual unsigned int GetListenerPort() = 0;
	virtual std::string GetIpAddress() = 0;
	virtual bool UseBlockingSockets() = 0;
	virtual unsigned int GetSocketTimeOut() = 0;
	virtual std::string GetLogFilename() = 0;
	virtual unsigned int GetLogLevel() = 0;
	virtual bool UseLogFlush() = 0;
	virtual unsigned int GetThreadIntervalTime() = 0;
	virtual unsigned int GetMaxWorkingThreads()	= 0;
};

class XMLParser : public IXMLParser
{
public:
	struct XMLNode
	{
		std::string tag;
		std::string value;
		std::vector<std::shared_ptr<XMLNode>> children;
		std::shared_ptr<XMLNode> parent;
	};

	XMLParser() : m_root{ nullptr }
	{
		LOG = LOG->GetInstance();
	}

	~XMLParser()
	{			
		m_root = nullptr;		
	}

protected:
	bool ParseFile(const std::string& filename = XML_FILE_PATH);
	bool CreateElement(std::shared_ptr<XMLNode>& elem_pointer, const std::string& elem_tag, const std::string& elem_data);
	bool ChildAppend(std::shared_ptr<XMLNode>& parent, std::shared_ptr<XMLNode>& child);
	bool FileCheck(const std::filesystem::path& filename);
	void FindByTag(const std::shared_ptr<XMLNode>& current_node, const std::string& tag_needed, std::string& data) const;
	bool ValueCheck(const std::string& line, unsigned int& number) const;

public:
	std::string GetServerName() override;
	std::string GetServerDisplayName() override;
	unsigned int GetListenerPort() override;
	std::string GetIpAddress() override;
	bool UseBlockingSockets() override;
	unsigned int GetSocketTimeOut() override;
	std::string GetLogFilename() override;
	unsigned int GetLogLevel() override;
	bool UseLogFlush() override;
	unsigned int GetThreadIntervalTime() override;
	unsigned int GetMaxWorkingThreads() override;

protected:
	std::shared_ptr<XMLNode> m_root;
	Logger* LOG = nullptr;
};