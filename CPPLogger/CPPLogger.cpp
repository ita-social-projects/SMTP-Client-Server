// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pch.h"
#include "framework.h"
#include <string.h>
#include <atomic>
#include "../XMLParserForLogger/XMLParserForLogger.h"
#include "CPPLogger.h"

std::wstring CLIENT_INIT_PARAM				= L"/P7.Sink=FileTxt /P7.Format=\"%ti-%tf-%lv-%fs-%fn-%ms\" /P7.Dir=";
const wchar_t* TRACE_CHANNEL				= L"Trace";
const char* FILE_FORMAT						= ".txt";
const char* TIME_FORMAT						= "%Y%m%d-%H%M%S000";
const char* ENCRYPTED_FILE_NAME				= "encrypted_file_";
const tUINT16 TRACE_ID						= NULL;
const IP7_Trace::hModule I_HMODULE			= NULL;
const short MAX_LOG_LVL						= 5;
const short MIN_LOG_LVL						= 0;
std::atomic<Logger*> Logger::s_instance;
std::mutex Logger::s_mutex;

Logger::Logger()
{
	XMLParserForLogger parser;
	m_file_dir = parser.GetLogFilename();

	std::wstring file_path = std::wstring(m_file_dir.begin(), m_file_dir.end());
	CLIENT_INIT_PARAM += file_path;

	m_log_level = eP7Trace_Level::EP7TRACE_LEVEL_TRACE;

	time_t current_time = time(0);
	struct tm  t_struct;
	localtime_s(&t_struct, &current_time);
	strftime(m_buf, sizeof(m_buf), TIME_FORMAT, &t_struct);

	m_client = P7_Create_Client(CLIENT_INIT_PARAM.c_str());
	m_trace = P7_Create_Trace(m_client, TRACE_CHANNEL);

	set_filter_level(parser.GetLogLevel());
}

Logger::~Logger()
{
	m_trace->Release();
	m_client->Release();

	std::string full_path{ m_file_dir + m_buf + FILE_FORMAT };
	std::ifstream file(full_path, std::ios::binary);
	std::ofstream encrypted_file(m_file_dir + ENCRYPTED_FILE_NAME + m_buf + FILE_FORMAT, std::ios::binary);
	std::vector<unsigned char> vec, enc_vec;

	char ch;
	while (file)
	{
		file.get(ch);
		if (file && ch) vec.push_back(ch);
	}

	vec.erase(vec.begin(), vec.begin() + 2);

	auto vec_size{ m_crypto.Encrypt(vec, enc_vec) };
	enc_vec.resize((size_t)vec_size);

	for (size_t i = 0; i < (size_t)vec_size; i++)
		encrypted_file << enc_vec[i];

	file.close();
	encrypted_file.close();

	remove(full_path.c_str());
}

Logger& Logger::operator()(const std::string& func, const std::string& filename, short level)
{
	if (func.empty() || filename.empty() || level < MIN_LOG_LVL || level > MAX_LOG_LVL) return *this;
	m_func_name = func;
	m_file_name = filename;
	m_log_level = (eP7Trace_Level)level;
	return *this;
}

Logger& Logger::operator<<(const char* log_message)
{
	switch (m_log_level)
	{
	case eP7Trace_Level::EP7TRACE_LEVEL_DEBUG:
		m_trace->Trace(TRACE_ID, eP7Trace_Level::EP7TRACE_LEVEL_DEBUG, NULL, (tUINT16)__LINE__, m_file_name.c_str(), m_func_name.c_str(), L"%hs", log_message);
		return *this;

	case eP7Trace_Level::EP7TRACE_LEVEL_INFO:
		m_trace->Trace(TRACE_ID, eP7Trace_Level::EP7TRACE_LEVEL_INFO, NULL, (tUINT16)__LINE__, m_file_name.c_str(), m_func_name.c_str(), L"%hs", log_message);
		return *this;

	case eP7Trace_Level::EP7TRACE_LEVEL_WARNING:
		m_trace->Trace(TRACE_ID, eP7Trace_Level::EP7TRACE_LEVEL_WARNING, NULL, (tUINT16)__LINE__, m_file_name.c_str(), m_func_name.c_str(), L"%hs", log_message);
		return *this;

	case eP7Trace_Level::EP7TRACE_LEVEL_ERROR:
		m_trace->Trace(TRACE_ID, eP7Trace_Level::EP7TRACE_LEVEL_ERROR, NULL, (tUINT16)__LINE__, m_file_name.c_str(), m_func_name.c_str(), L"%hs", log_message);
		return *this;

	case eP7Trace_Level::EP7TRACE_LEVEL_CRITICAL:
		m_trace->Trace(TRACE_ID, eP7Trace_Level::EP7TRACE_LEVEL_CRITICAL, NULL, (tUINT16)__LINE__, m_file_name.c_str(), m_func_name.c_str(), L"%hs", log_message);
		return *this;

	default:
		m_trace->Trace(TRACE_ID, eP7Trace_Level::EP7TRACE_LEVEL_TRACE, NULL, (tUINT16)__LINE__, m_file_name.c_str(), m_func_name.c_str(), L"%hs", log_message);
		return *this;
	}
}

void Logger::set_filter_level(unsigned int level)
{
	if (level < MIN_LOG_LVL || level > MAX_LOG_LVL) return;
	m_trace->Set_Verbosity(I_HMODULE, (eP7Trace_Level)level);
}

Logger* Logger::GetInstance()
{
	Logger* tmp = s_instance.load(std::memory_order_relaxed);
	std::atomic_thread_fence(std::memory_order_acquire);

	if (!tmp)
	{
		std::lock_guard<std::mutex> lock(s_mutex);
		tmp = s_instance.load(std::memory_order_relaxed);
		if (!tmp)
		{
			tmp = new Logger;
			std::atomic_thread_fence(std::memory_order_release);
			s_instance.store(tmp, std::memory_order_relaxed);
		}
	}
	return tmp;
}