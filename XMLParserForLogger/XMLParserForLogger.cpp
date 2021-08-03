// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "framework.h"
#include "XMLParserForLogger.h"

bool XMLParserForLogger::ParseFile(const std::string& filename)
{
	struct Characters
	{
		const char TAG1 = '<';
		const char TAG2 = '>';
		const char SLASH = '/';
		const std::string EXCLAMATION = "<!";
		const std::string QUESTION = "<?";
		const std::string END_TAG_SYMBOLS = "</";
	};
	std::filesystem::path file_path = filename;
	if (!FileCheck(file_path))
	{
		return false;
	}

	std::ifstream file{ filename };
	if (!file.is_open())
	{
		return false;
	}

	std::string line;
	std::unique_ptr<Characters> symbol = std::make_unique<Characters>();
	bool tag_flag = false;
	bool first_flag = true;
	std::shared_ptr<XMLNode> current_node(nullptr);
	std::shared_ptr<XMLNode> parent_node(nullptr);
	while (std::getline(file, line))
	{
		if (line.empty())
		{
			continue;
		}
		if (line.find(symbol->QUESTION) != std::string::npos)
		{
			continue;
		}
		if (line.find(symbol->EXCLAMATION) != std::string::npos)
		{
			continue;
		}
		std::size_t tag_begin = line.find_first_of(symbol->TAG1);
		std::size_t tag_end = line.find_first_of(symbol->TAG2);
		std::size_t tag_close;
		if (tag_begin == std::string::npos || tag_end == std::string::npos)
		{
			return false;
		}
		std::string element_tag;
		std::string element_data;
		std::string close_tag;
		int one_step = 1;
		int two_steps = 2;
		if (line[tag_begin + one_step] != line.size() && line[tag_begin + one_step] != symbol->SLASH)
		{
			element_tag = line.substr(tag_begin + one_step, tag_end - (tag_begin + one_step));
		}
		else if (line[tag_begin + one_step] != line.size() && line[tag_begin + one_step] == symbol->SLASH)
		{
			close_tag = line.substr(tag_begin + two_steps, tag_end - (tag_begin + two_steps));
			if (parent_node != nullptr && parent_node->tag == close_tag)
			{
				current_node = parent_node;
				parent_node = current_node->parent;
				continue;
			}
		}
		tag_close = line.find(symbol->END_TAG_SYMBOLS, tag_end);
		if (tag_close != std::string::npos)
		{
			element_data = line.substr(tag_end + one_step, tag_close - tag_end - one_step);
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

bool XMLParserForLogger::CreateElement(std::shared_ptr<XMLNode>& elem_pointer, const std::string& elem_tag, const std::string& elem_data)
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

bool XMLParserForLogger::ChildAppend(std::shared_ptr<XMLNode>& parent, std::shared_ptr<XMLNode>& child)
{
	if (parent == nullptr || child == nullptr)
	{
		return false;
	}
	parent->children.push_back(child);
	child->parent = parent;
	return true;
}

bool XMLParserForLogger::FileCheck(const std::filesystem::path& filename)
{
	if (!std::filesystem::exists(filename))
	{
		return false;
	}
	if (!std::filesystem::is_regular_file(filename))
	{
		return false;
	}
	const std::string extension = ".xml";
	if (std::filesystem::path(filename).extension().string() != extension)
	{
		return false;
	}
	if (std::filesystem::is_empty(filename))
	{
		return false;
	}
	return true;
}

void XMLParserForLogger::FindByTag(const std::shared_ptr<XMLNode>& current_node, const std::string& tag_needed, std::string& data) const
{
	if (!current_node || current_node->children.empty())
	{
		return;
	}
	size_t child_size = current_node->children.size();
	for (size_t i = 0; i < child_size; i++)
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

bool XMLParserForLogger::ValueCheck(const std::string& line, unsigned int& number) const
{
	std::string data;
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

std::string XMLParserForLogger::GetLogFilename()
{
	if (m_root == nullptr)
	{
		bool parsed = ParseFile(FILE_PATH_FOR_XML);
		if (!parsed)
		{
			return LOG_FILE_NAME;
		}
	}
	std::string data;
	FindByTag(m_root, ATTR_LOG_FILE_NAME, data);
	if (!data.empty())
	{
		return data;
	}
	return LOG_FILE_NAME;
}

unsigned int XMLParserForLogger::GetLogLevel()
{
	if (m_root == nullptr)
	{
		bool parsed = ParseFile(FILE_PATH_FOR_XML);
		if (!parsed)
		{
			return LEVEL_FOR_LOG;
		}
	}
	unsigned int log_level = 0;
	bool flag = ValueCheck(ATTR_LEVEL_FOR_LOG, log_level);
	if (flag)
	{
		return log_level;
	}
	return LEVEL_FOR_LOG;
}

bool XMLParserForLogger::UseLogFlush()
{
	if (m_root == nullptr)
	{
		bool parsed = ParseFile(FILE_PATH_FOR_XML);
		if (!parsed)
		{
			return parsed;
		}
	}
	unsigned int flush = 0;
	bool flag = ValueCheck(ATTR_FLUSH_FOR_LOG, flush);
	return flag;
}