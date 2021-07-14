// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "framework.h"
#include "XMLParser.h"

bool XMLParser::ParseFile(const std::string& filename)
{
	struct Characters
	{
		const char tag1 = '<';
		const char tag2 = '>';
		const char slash = '/';
		const std::string exclamation = "<!";
		const std::string question = "<?";
		const std::string end_tag_symbols = "</";
	};
	std::filesystem::path file_path = filename;
	if (!FileCheck(file_path))
	{
		return false;
	}

	std::ifstream file{ filename };
	if (!file.is_open())
	{
		//std::cout << "Can't open file.\n";
		return false;
	}

	std::string line;
	std::unique_ptr<Characters> symbol = std::make_unique<Characters>();
	bool tag_flag = false;
	bool first_flag = true;
	std::shared_ptr<XMLNode> current_node = nullptr;
	std::shared_ptr<XMLNode> parent_node = nullptr;
	while (std::getline(file, line))
	{
		if (line.empty())
		{
			continue;
		}
		if (line.find(symbol->question) != std::string::npos)
		{
			continue;
		}
		if (line.find(symbol->exclamation) != std::string::npos)
		{
			continue;
		}
		std::size_t tag_begin = line.find_first_of(symbol->tag1);
		std::size_t tag_end = line.find_first_of(symbol->tag2);
		std::size_t tag_close;
		if (tag_begin == std::string::npos || tag_end == std::string::npos)
		{
			//std::cout << "XML file is not valid.\n";
			return false;
		}
		std::string element_tag;
		std::string element_data;
		std::string close_tag;
		int one = 1;
		int two = 2;
		if (line[tag_begin + one] != line.size() && line[tag_begin + one] != symbol->slash)
		{
			element_tag = line.substr(tag_begin + one, tag_end - (tag_begin + one));
		}
		else if (line[tag_begin + one] != line.size() && line[tag_begin + one] == symbol->slash)
		{
			close_tag = line.substr(tag_begin + two, tag_end - (tag_begin + two));
			if (parent_node != nullptr && parent_node->tag == close_tag)
			{
				current_node = parent_node;
				parent_node = current_node->parent;
				continue;
			}
		}
		tag_close = line.find(symbol->end_tag_symbols, tag_end);
		if (tag_close != std::string::npos)
		{
			element_data = line.substr(tag_end + one, tag_close - tag_end - one);
		}
		CreateElement(current_node, element_tag, element_data);
		if (first_flag)
		{
			m_root = current_node;
			parent_node = current_node;
			first_flag = false;
		}
		else
		{
			ChildAppend(parent_node, current_node);
			if (tag_close == std::string::npos)
			{
				parent_node = current_node;
				current_node = nullptr;
			}
		}
	}
	file.close();
	return true;
}

bool XMLParser::CreateElement(std::shared_ptr<XMLNode>& elem_pointer, const std::string& elem_tag, const std::string& elem_data)
{
	if (elem_tag.empty())
	{
		return false;
	}
	elem_pointer.reset(new XMLNode);
	elem_pointer->tag = elem_tag;
	elem_pointer->value = elem_data;
	return true;
}

bool XMLParser::ChildAppend(std::shared_ptr<XMLNode>& parent, std::shared_ptr<XMLNode>& child)
{
	if (parent == nullptr || child == nullptr)
	{
		return false;
	}
	parent->children.push_back(child);
	child->parent = parent;
	return true;
}

bool XMLParser::FileCheck(const std::filesystem::path& filename) const
{
	if (!std::filesystem::exists(filename))
	{
		//std::cout << "File does not exist.\n";
		return false;
	}
	if (!std::filesystem::is_regular_file(filename))
	{
		//std::cout << "File is not regular file.\n";
		return false;
	}
	const std::string extension = ".xml";
	if (std::filesystem::path(filename).extension().string() != extension)
	{
		//std::cout << "It's not XML file.\n";
		return false;
	}
	if (std::filesystem::is_empty(filename))
	{
		//std::cout << "File is empty.\n";
		return false;
	}
	return true;
}

void XMLParser::FindByTag(const std::shared_ptr<XMLNode>& current_node, const std::string& tag_needed, std::string& data)
{
	if (!current_node->children.size())
	{
		return;
	}
	for (size_t i = 0; i < current_node->children.size(); i++)
	{

		if (current_node->children[i]->tag == tag_needed)
		{
			data = current_node->children[i]->value;
			break;
		}
		else
		{
			FindByTag(current_node->children[i], tag_needed, data);
		}
	}
}

bool XMLParser::ValueCheck(const std::string& line, unsigned int& number)
{
	std::string data = "";
	FindByTag(m_root, line, data);
	if (!data.empty())
	{
		try
		{
			number = stoi(data);
		}
		catch (...)
		{
			return false;
		}
		return true;
	}
	return false;
}

std::string XMLParser::GetServerName()
{
	if (m_root == nullptr)
	{
		ParseFile(XML_FILE_PATH);
	}
	std::string line = "servername";
	std::string data = "";
	FindByTag(m_root, line, data);
	if (!data.empty())
	{
		return data;
	}
	return SERVER_NAME;
}

std::string XMLParser::GetServerDisplayName()
{
	if (m_root == nullptr)
	{
		ParseFile(XML_FILE_PATH);
	}
	std::string line = "serverdisplayname";
	std::string data = "";
	FindByTag(m_root, line, data);
	if (!data.empty())
	{
		return data;
	}
	return SERVER_DISPLAY_NAME;
}

unsigned int XMLParser::GetListenerPort()
{
	if (m_root == nullptr)
	{
		ParseFile(XML_FILE_PATH);
	}
	std::string line = "listenerport";
	unsigned int port = 0;
	bool flag = ValueCheck(line, port);
	if (flag)
	{
		return port;
	}
	return SERVER_LISTENER_PORT;
}

std::string XMLParser::GetIpAddress()
{
	if (m_root == nullptr)
	{
		ParseFile(XML_FILE_PATH);
	}
	std::string line = "ipaddress";
	std::string data = "";
	FindByTag(m_root, line, data);
	if (!data.empty())
	{
		return data;
	}
	return SERVER_IP_ADDRESS;
}

bool XMLParser::UseBlockingSockets()
{
	if (m_root == nullptr)
	{
		ParseFile(XML_FILE_PATH);
	}
	std::string line = "blocking";
	unsigned int block = 0;
	bool flag = ValueCheck(line, block);
	return flag;
}

unsigned int XMLParser::GetSocketTimeOut()
{
	if (m_root == nullptr)
	{
		ParseFile(XML_FILE_PATH);
	}
	std::string line = "socket_timeout";
	unsigned int time_out = 0;
	bool flag = ValueCheck(line, time_out);
	if (flag)
	{
		return time_out;
	}
	return SOCKET_TIMEOUT;
}

std::string XMLParser::GetLogFilename()
{
	if (m_root == nullptr)
	{
		ParseFile(XML_FILE_PATH);
	}
	std::string line = "filename";
	std::string data = "";
	FindByTag(m_root, line, data);
	if (!data.empty())
	{
		return data;
	}
	return LOG_FILENAME;
}

unsigned int XMLParser::GetLogLevel()
{
	if (m_root == nullptr)
	{
		ParseFile(XML_FILE_PATH);
	}
	std::string line = "LogLevel";
	unsigned int log_level = 0;
	bool flag = ValueCheck(line, log_level);
	if (flag)
	{
		return log_level;
	}
	return LOG_LEVEL;
}

bool XMLParser::UseLogFlush()
{
	if (m_root == nullptr)
	{
		ParseFile(XML_FILE_PATH);
	}
	std::string line = "flush";
	unsigned int flush = 0;
	bool flag = ValueCheck(line, flush);
	return flag;
}

unsigned int XMLParser::GetPeriodTime()
{
	if (m_root == nullptr)
	{
		ParseFile(XML_FILE_PATH);
	}
	std::string line = "Period_time";
	unsigned int time = 0;
	bool flag = ValueCheck(line, time);
	if (flag)
	{
		return time;
	}
	return PERIOD_TIME;
}

unsigned int XMLParser::GetMaxWorkingThreads()
{
	if (m_root == nullptr)
	{
		ParseFile(XML_FILE_PATH);
	}
	std::string line = "maxworkingthreads";
	unsigned int number = 0;
	bool flag = ValueCheck(line, number);
	if (flag)
	{
		return number;
	}
	return MAX_WORKING_THREADS;
}
