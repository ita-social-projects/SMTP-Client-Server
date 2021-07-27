// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "pch.h"
#include "framework.h"
#include <string.h>
#include "CPPLogger.h"
#include "../XMLParserForLogger/XMLParserForLogger.h"

std::wstring CLIENT_INIT_PARAM		= L"/P7.Sink=FileTxt /P7.Format=\"%ti-%tf-%lv-%fs-%fn-%ms\" /P7.Dir=";
const wchar_t* TRACE_CHANNEL		= L"Trace";
const tUINT16 TRACE_ID				= NULL;
const IP7_Trace::hModule I_HMODULE	= NULL;
Logger* Logger::s_instance;

Logger::Logger()
{
	XMLParserForLogger parser;

	std::wstring file_path = std::wstring(parser.GetLogFilename().begin(), parser.GetLogFilename().end());
	CLIENT_INIT_PARAM += file_path;

	m_log_level = eP7Trace_Level::EP7TRACE_LEVEL_TRACE;
	m_client = P7_Create_Client(CLIENT_INIT_PARAM.c_str());
	m_trace = P7_Create_Trace(m_client, TRACE_CHANNEL);

	set_filter_level(parser.GetLogLevel());
}

Logger::~Logger()
{
	m_trace->Release();
	m_client->Release();
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
	if (!s_instance)
	{
		s_instance = new Logger;
	}
	return s_instance;
}