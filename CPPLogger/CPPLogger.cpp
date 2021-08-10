// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pch.h"
#include "framework.h"
#include <string.h>
#include <atomic>
#include "CPPLogger.h"
#include "../XMLParserForLogger/XMLParserForLogger.h"

std::wstring CLIENT_INIT_PARAM				= L"/P7.Sink=FileTxt /P7.Format=\"%ti-%tf-%lv-%fs-%fn-%ms\" /P7.Dir=";
const wchar_t* TRACE_CHANNEL				= L"Trace";
const tUINT16 TRACE_ID						= NULL;
const IP7_Trace::hModule I_HMODULE			= NULL;
std::atomic<Logger*> Logger::s_instance;
std::mutex Logger::s_mutex;

Logger::Logger()
{
	XMLParserForLogger parser;
	m_file_dir = parser.GetLogFilename();

	std::wstring file_path = std::wstring(m_file_dir.begin(), m_file_dir.end());
	CLIENT_INIT_PARAM += file_path;

	m_log_level = eP7Trace_Level::EP7TRACE_LEVEL_TRACE;

	time_t     now = time(0);
	struct tm  tstruct;
	localtime_s(&tstruct, &now);
	strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S000", &tstruct);

	m_client = P7_Create_Client(CLIENT_INIT_PARAM.c_str());
	m_trace = P7_Create_Trace(m_client, TRACE_CHANNEL);

	set_filter_level(parser.GetLogLevel());
}

Logger::~Logger()
{
	m_trace->Release();
	m_client->Release();

	std::string full_path{ m_file_dir + buf + ".txt" };
	std::ifstream file(full_path);
	std::ofstream encrypted_file(m_file_dir + "encrypted_file.txt", std::ios::app);
	std::string str, str2;
	std::shared_ptr<unsigned char[]> encrypted_line, decrypted_line;

	while (!file)
	{
		if (str.size() == 1 && str.at(0) == '\0')
			continue;
		std::getline(file, str, '\n');
		unsigned int len = m_crypto.Encrypt((unsigned char*)str.c_str(), str.size(), encrypted_line);

		m_crypto.Decrypt(encrypted_line.get(), len, decrypted_line);
		unsigned char* ptr = decrypted_line.get();
		str2 = (char*)ptr;
		encrypted_file << str2;
	}

	file.close();
	encrypted_file.close();
	
	remove(full_path.c_str());
}

Logger& Logger::operator()(const std::string& func, const std::string& filename, short level)
{
	if (func.empty() || filename.empty() || level < 0 || level > 5) return *this;
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
	if (level < 0 || level > 5) return;
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