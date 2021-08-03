#pragma once

#include <iostream>
#include <string>
#include <mutex>

#include "..\CPPLogger\include\p7Headers\P7_Client.h"
#include "..\CPPLogger\include\p7Headers\P7_Trace.h"

enum LogLevels
{
	L_TRACE = 0,
	L_DEBUG,
	L_INFO,
	L_WARNING,
	L_ERROR,
	L_CRITICAL,
};

class ILogger
{
public:
	virtual ILogger& operator() (const std::string& func_name, const std::string& file_name, short level) = 0;
	virtual ILogger& operator<< (const char* log_message) = 0;
	virtual void set_filter_level(unsigned int level) = 0;
};

class Logger final : public ILogger
{
public:
	~Logger();

	static Logger* GetInstance();

	Logger& operator()(const std::string& func, const std::string& filename, short level) override;
	Logger& operator<<(const char* log_message) override;

	void set_filter_level(unsigned int level) override;

private:
	Logger();
	Logger(const Logger&);

	static std::atomic<Logger*>	s_instance;
	static std::mutex			s_mutex;
	eP7Trace_Level				m_log_level;
	IP7_Client*					m_client;
	IP7_Trace*					m_trace;
	std::string					m_func_name;
	std::string					m_file_name;
};

#define LOG_TRACE (*LOG)(__FUNCTION__, __FILE__, eP7Trace_Level::EP7TRACE_LEVEL_TRACE)
#define LOG_DEBUG (*LOG)(__FUNCTION__, __FILE__, eP7Trace_Level::EP7TRACE_LEVEL_DEBUG)
#define LOG_INFO (*LOG)(__FUNCTION__, __FILE__, eP7Trace_Level::EP7TRACE_LEVEL_INFO)
#define LOG_WARN (*LOG)(__FUNCTION__, __FILE__, eP7Trace_Level::EP7TRACE_LEVEL_WARNING)
#define LOG_ERROR (*LOG)(__FUNCTION__, __FILE__, eP7Trace_Level::EP7TRACE_LEVEL_ERROR)
#define LOG_FATAL (*LOG)(__FUNCTION__, __FILE__, eP7Trace_Level::EP7TRACE_LEVEL_CRITICAL)