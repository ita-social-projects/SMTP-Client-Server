// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <memory>

constexpr auto LOG_FILE_NAME			= "C:\\Logs\\";
constexpr unsigned int LEVEL_FOR_LOG	= 2;
constexpr auto FILE_PATH_FOR_XML		= "..\\Config.xml";

constexpr auto ATTR_LOG_FILE_NAME		= "filename";
constexpr auto ATTR_LEVEL_FOR_LOG		= "LogLevel";
constexpr auto ATTR_FLUSH_FOR_LOG		= "flush";


class IXMLParserForLogger
{
public:
	virtual std::string GetLogFilename() = 0;
	virtual unsigned int GetLogLevel() = 0;
	virtual bool UseLogFlush() = 0;
};

class XMLParserForLogger final : public IXMLParserForLogger
{
public:
	struct XMLNode
	{
		std::string tag;
		std::string value;
		std::vector<std::shared_ptr<XMLNode>> children;
		std::shared_ptr<XMLNode> parent;
	};

	XMLParserForLogger() : m_root{ nullptr }
	{
	}

	~XMLParserForLogger()
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
	std::string GetLogFilename() override;
	unsigned int GetLogLevel() override;
	bool UseLogFlush() override;

private:
	std::shared_ptr<XMLNode> m_root;
};