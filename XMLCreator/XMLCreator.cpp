// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "framework.h"
#include "XMLCreator.h"

bool XMLCreator::CheckIfSet(const std::string& tag, const std::string& new_value)
{
	if (new_value.empty())
	{
		return false;
	}
	if (m_root == nullptr)
	{
		bool parce_flag = ParseFile();
		if (!parce_flag)
		{
			bool is_tree_created = CreateStandartTree();
			if (!is_tree_created)
			{
				return false;
			}
		}
	}
	bool is_set = false;
	SetValueByTag(m_root, tag, new_value, is_set);
	if (!is_set)
	{
		return false;
	}
	return true;
}

void XMLCreator::SetValueByTag(const std::shared_ptr<XMLNode>& current_node, const std::string& tag_needed, const std::string& data, bool& set_flag)
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
			current_node->children[i]->value = data;
			set_flag = true;
			break;
		}
		else
		{
			SetValueByTag(current_node->children[i], tag_needed, data, set_flag);
		}
	}
}

void XMLCreator::WriteTreeToFile(std::shared_ptr<XMLNode> current, size_t depth, std::ofstream& file_name, Symbols& character)
{
	if (current == nullptr)
	{
		return;
	}
	for (size_t j = 0; j < depth; j++)
	{
		file_name << character.TAB;
	}
	file_name << character.TAG1 << current->tag << character.TAG2 << current->value;
	if (!current->children.empty())
	{
		file_name << character.NEW_LINE;
		depth++;
		for (size_t i = 0; i < current->children.size(); i++)
		{
			WriteTreeToFile(current->children[i], depth, file_name, character);
		}
		--depth;
		for (size_t j = 0; j < depth; j++)
		{
			file_name << character.TAB;
		}
	}
	file_name << character.END_TAG << current->tag << '>' << character.NEW_LINE;
}

bool XMLCreator::CreateStandartTree()
{
	if (m_root != nullptr)
	{
		return false;
	}

	const std::string LISTENER_PORT = std::to_string(SERVER_LISTENER_PORT);
	const std::string TIMEOUT = std::to_string(SOCKET_TIMEOUT);
	const std::string LEVEL = std::to_string(LOG_LEVEL);
	const std::string INTERVAL = std::to_string(THREAD_INTERVAL);
	const std::string WORKING_THREADS = std::to_string(MAX_WORKING_THREADS);
	const std::string BLOCKING = "0";

	const int ROOT_TAGS_SIZE = 5;
	const int SERVER_TAGS_SIZE = 4;
	const int COMMUNICATION_TAGS_SIZE = 2;
	const int LOGGING_TAGS_SIZE = 3;
	std::array<std::string, ROOT_TAGS_SIZE> root_tags{ ATTR_SERVER, ATTR_COMMUNICATION_SETTINGS, ATTR_LOGGING, ATTR_TIME, ATTR_THREAD_POOL };
	std::array<std::string, SERVER_TAGS_SIZE> server_tags{ ATTR_SERVER_NAME, ATTR_SERVER_DISPLAY_NAME, ATTR_SERVER_LISTENER_PORT, ATTR_SERVER_IP_ADDRESS };
	std::array<std::string, SERVER_TAGS_SIZE> server_values{ SERVER_NAME, SERVER_DISPLAY_NAME, LISTENER_PORT, SERVER_IP_ADDRESS };
	std::array<std::string, COMMUNICATION_TAGS_SIZE> communication_tags{ ATTR_SOCKET_BLOCKING, ATTR_SOCKET_TIMEOUT };
	std::array<std::string, COMMUNICATION_TAGS_SIZE> communication_values{ BLOCKING, TIMEOUT };
	std::array<std::string, LOGGING_TAGS_SIZE> logging_tags{ ATTR_LOG_FILENAME, ATTR_LOG_LEVEL, ATTR_LOG_FLUSH };
	std::array<std::string, LOGGING_TAGS_SIZE> logging_values{ LOG_FILENAME, LEVEL, BLOCKING };

	std::shared_ptr<XMLNode> parent_node = nullptr;
	std::shared_ptr<XMLNode> new_node = nullptr;
	std::string node_value;
	CreateElement(parent_node, ATTR_ROOT, node_value);
	m_root = parent_node;
	for (int i = 0; i < ROOT_TAGS_SIZE; i++)
	{
		CreateElement(new_node, root_tags[i], node_value);
		ChildAppend(parent_node, new_node);
	}
	enum class Children
	{
		SERVER = 0, COMMUNICATION, LOGGING, TIME, THREADPOOL
	};
	if (m_root == nullptr)
	{
		return false;
	}
	size_t children_size = m_root->children.size();
	for (size_t i = 0; i < children_size; i++)
	{
		parent_node = m_root->children[i];
		switch (i)
		{
		case (int)Children::SERVER:
			CreateBranch(parent_node, new_node, server_tags, server_values);
			break;
		case (int)Children::COMMUNICATION:
			CreateBranch(parent_node, new_node, communication_tags, communication_values);
			break;
		case (int)Children::LOGGING:
			CreateBranch(parent_node, new_node, logging_tags, logging_values);
			break;
		case (int)Children::TIME:
			CreateElement(new_node, ATTR_THREAD_INTERVAL, INTERVAL);
			ChildAppend(parent_node, new_node);
			break;
		case (int)Children::THREADPOOL:
			CreateElement(new_node, ATTR_MAX_WORKING_THREADS, WORKING_THREADS);
			ChildAppend(parent_node, new_node);
			break;
		default:
			break;
		}
	}
	return true;
}

template <const int T>
void XMLCreator::CreateBranch(std::shared_ptr<XMLNode> parent, std::shared_ptr<XMLNode> new_child, std::array<std::string, 
								T>& tags, std::array<std::string, T>& values)
{
	for (size_t i = 0; i < T; i++)
	{
		CreateElement(new_child, tags[i], values[i]);
		ChildAppend(parent, new_child);
	}
}

bool XMLCreator::SetServerName(const std::string& new_value)
{
	bool set = CheckIfSet(ATTR_SERVER_NAME, new_value);
	if (!set)
	{
		std::string log_massage = "Setting for <" + std::string(ATTR_SERVER_NAME) + "> was not set to " + std::string(FILE_NAME);
		LOG_INFO << log_massage.c_str();
		return false;
	}
	return true;
}

bool XMLCreator::SetServerDisplayName(const std::string& new_value)
{
	bool set = CheckIfSet(ATTR_SERVER_DISPLAY_NAME, new_value);
	if (!set)
	{
		std::string log_massage = "Setting for <" + std::string(ATTR_SERVER_DISPLAY_NAME) + "> was not set to " + std::string(FILE_NAME);
		LOG_INFO << log_massage.c_str();
		return false;
	}
	return true;
}

bool XMLCreator::SetListenerPort(const std::string& new_value)
{
	bool set = CheckIfSet(ATTR_SERVER_LISTENER_PORT, new_value);
	if (!set)
	{
		std::string log_massage = "Setting for <" + std::string(ATTR_SERVER_LISTENER_PORT) + "> was not set to " + std::string(FILE_NAME);
		LOG_INFO << log_massage.c_str();
		return false;
	}
	return true;
}

bool XMLCreator::SetIpAddress(const std::string& new_value)
{
	bool set = CheckIfSet(ATTR_SERVER_IP_ADDRESS, new_value);
	if (!set)
	{
		std::string log_massage = "Setting for <" + std::string(ATTR_SERVER_IP_ADDRESS) + "> was not set to " + std::string(FILE_NAME);
		LOG_INFO << log_massage.c_str();
		return false;
	}
	return true;
}

bool XMLCreator::SetBlockingSockets(const std::string& new_value)
{
	bool set = CheckIfSet(ATTR_SOCKET_BLOCKING, new_value);
	if (!set)
	{
		std::string log_massage = "Setting for <" + std::string(ATTR_SOCKET_BLOCKING) + "> was not set to " + std::string(FILE_NAME);
		LOG_INFO << log_massage.c_str();
		return false;
	}
	return true;
}

bool XMLCreator::SetSocketTimeOut(const std::string& new_value)
{
	bool set = CheckIfSet(ATTR_SOCKET_TIMEOUT, new_value);
	if (!set)
	{
		std::string log_massage = "Setting for <" + std::string(ATTR_SOCKET_TIMEOUT) + "> was not set to " + std::string(FILE_NAME);
		LOG_INFO << log_massage.c_str();
		return false;
	}
	return true;
}

bool XMLCreator::SetLogFilename(const std::string& new_value)
{
	bool set = CheckIfSet(ATTR_LOG_FILENAME, new_value);
	if (!set)
	{
		std::string log_massage = "Setting for <" + std::string(ATTR_LOG_FILENAME) + "> was not set to " + std::string(FILE_NAME);
		LOG_INFO << log_massage.c_str();
		return false;
	}
	return true;
}

bool XMLCreator::SetLogLevel(const std::string& new_value)
{
	bool set = CheckIfSet(ATTR_LOG_LEVEL, new_value);
	if (!set)
	{
		std::string log_massage = "Setting for <" + std::string(ATTR_LOG_LEVEL) + "> was not set to " + std::string(FILE_NAME);
		LOG_INFO << log_massage.c_str();
		return false;
	}
	return true;
}

bool XMLCreator::SetLogFlush(const std::string& new_value)
{
	bool set = CheckIfSet(ATTR_LOG_FLUSH, new_value);
	if (!set)
	{
		std::string log_massage = "Setting for <" + std::string(ATTR_LOG_FLUSH) + "> was not set to " + std::string(FILE_NAME);
		LOG_INFO << log_massage.c_str();
		return false;
	}
	return true;
}

bool XMLCreator::SetThreadIntervalTime(const std::string& new_value)
{
	bool set = CheckIfSet(ATTR_THREAD_INTERVAL, new_value);
	if (!set)
	{
		std::string log_massage = "Setting for <" + std::string(ATTR_THREAD_INTERVAL) + "> was not set to " + std::string(FILE_NAME);
		LOG_INFO << log_massage.c_str();
		return false;
	}
	return true;
}

bool XMLCreator::SetMaxWorkingThreads(const std::string& new_value)
{
	bool set = CheckIfSet(ATTR_MAX_WORKING_THREADS, new_value);
	if (!set)
	{
		std::string log_massage = "Setting for <" + std::string(ATTR_MAX_WORKING_THREADS) + "> was not set to " + std::string(FILE_NAME);
		LOG_INFO << log_massage.c_str();
		return false;
	}
	return true;
}

bool XMLCreator::Write(const char* filename)
{
	if (m_root == nullptr)
	{
		bool parced = ParseFile();
		if (!parced)
		{
			bool is_tree_created = CreateStandartTree();
			if (!is_tree_created)
			{
				return false;
			}
		}
	}

	std::ofstream file{ filename };
	std::shared_ptr<XMLNode> current_node = m_root;
	Symbols symbol;
	size_t depth = 0;
	WriteTreeToFile(current_node, depth, file, symbol);
	file.close();
	return true;
}