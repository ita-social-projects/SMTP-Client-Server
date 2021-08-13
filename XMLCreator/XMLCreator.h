#pragma once
#include <iostream>
#include <array>

#include "..\\XMLParser\XMLParser.h"

constexpr const char* FILE_NAME = "Config.xml";

class IXMLCreator
{
public:
	virtual bool SetServerName(const std::string& new_value) = 0;
	virtual bool SetServerDisplayName(const std::string& new_value) = 0;
	virtual bool SetListenerPort(const std::string& new_value) = 0;
	virtual bool SetIpAddress(const std::string& new_value) = 0;
	virtual bool SetBlockingSockets(const std::string& new_value) = 0;
	virtual bool SetSocketTimeOut(const std::string& new_value) = 0;
	virtual bool SetLogFilename(const std::string& new_value) = 0;
	virtual bool SetLogLevel(const std::string& new_value) = 0;
	virtual bool SetLogFlush(const std::string& new_value) = 0;
	virtual bool SetThreadIntervalTime(const std::string& new_value) = 0;
	virtual bool SetMaxWorkingThreads(const std::string& new_value) = 0;
	virtual bool Write(const char* filename) = 0;
};

class XMLCreator : public IXMLCreator, public XMLParser
{
private:
	struct Symbols
	{
		const char TAG1 = '<';
		const char TAG2 = '>';
		const std::string END_TAG = "</";
		const char TAB = '\t';
		const char NEW_LINE = '\n';
	};

	bool CheckIfSet(const std::string& tag, const std::string& new_value);
	void SetValueByTag(const std::shared_ptr<XMLNode>& current_node, const std::string& tag_needed, const std::string& data, bool& set_flag);
	void WriteTreeToFile(std::shared_ptr<XMLNode> current, size_t depth, std::ofstream& file_name, Symbols& character);
	bool CreateStandartTree();
	template <const int T>
	void CreateBranch(std::shared_ptr<XMLNode> parent, std::shared_ptr<XMLNode> new_child, std::array<std::string, T>& tags, std::array<std::string, T>& values);

public:
	bool SetServerName(const std::string& new_value) override;
	bool SetServerDisplayName(const std::string& new_value) override;
	bool SetListenerPort(const std::string& new_value) override;
	bool SetIpAddress(const std::string& new_value) override;
	bool SetBlockingSockets(const std::string& new_value) override;
	bool SetSocketTimeOut(const std::string& new_value) override;
	bool SetLogFilename(const std::string& new_value) override;
	bool SetLogLevel(const std::string& new_value) override;
	bool SetLogFlush(const std::string& new_value) override;
	bool SetThreadIntervalTime(const std::string& new_value) override;
	bool SetMaxWorkingThreads(const std::string& new_value) override;
	bool Write(const char* filename = FILE_NAME) override;
};
