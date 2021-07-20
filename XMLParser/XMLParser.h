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

constexpr auto SERVER_NAME					= "ServTest";
constexpr auto SERVER_DISPLAY_NAME			= "ServTestserver";
constexpr unsigned int SERVER_LISTENER_PORT = 35000;
constexpr auto SERVER_IP_ADDRESS			= "127.0.0.1";
constexpr unsigned int SOCKET_TIMEOUT		= 5;
constexpr auto LOG_FILENAME					= "serverlog.txt";
constexpr unsigned int LOG_LEVEL			= 2;
constexpr unsigned int THREAD_INTERVAL		= 30;
constexpr unsigned int MAX_WORKING_THREADS	= 10;
constexpr auto XML_FILE_PATH				= "..\\Config.xml";

constexpr auto ATTR_SERVER_NAME				= "servername";
constexpr auto ATTR_SERVER_DISPLAY_NAME		= "serverdisplayname";
constexpr auto ATTR_SERVER_LISTENER_PORT	= "listenerport";
constexpr auto ATTR_SERVER_IP_ADDRESS		= "ipaddress";
constexpr auto ATTR_SOCKET_BLOCKING			= "blocking";
constexpr auto ATTR_SOCKET_TIMEOUT			= "socket_timeout";
constexpr auto ATTR_LOG_FILENAME			= "filename";
constexpr auto ATTR_LOG_LEVEL				= "LogLevel";
constexpr auto ATTR_LOG_FLUSH				= "flush";
constexpr auto ATTR_THREAD_INTERVAL			= "Period_time";
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

class XMLParser final : public IXMLParser
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
	}

	~XMLParser()
	{
		m_root = nullptr;
	}

private:
	bool ParseFile(const std::string& filename);
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

private:
	std::shared_ptr<XMLNode> m_root;
	Logger LOG;
};