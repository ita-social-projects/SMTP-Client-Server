#pragma once

#include<iostream>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <condition_variable>
#include <mutex>
#include <vector>
#include <thread>
#include <map>
#include <chrono>
#include <memory>
#include <functional>

#pragma comment(lib, "Ws2_32.lib")

typedef void (*RUNFUC)(SOCKET);

class ThreadPool
{
public:
    ThreadPool() : m_is_stop(false), m_threadpool_size(0) {}
    ThreadPool(size_t m_threadpool_size);
    ~ThreadPool();

    void AddTask(void* in_task, SOCKET socket);

    void set_size(size_t threadpool_size);

private:
    void Join();
    void Stop();
    void DoTask();

    bool IsStopped() { return m_is_stop; };

private:
    size_t m_threadpool_size;
    bool m_is_stop;

    std::vector<std::thread> m_thread_pool;
    std::map<RUNFUC, SOCKET> m_task;
    std::condition_variable m_task_not_empty;
    std::mutex m_mutex;

};

