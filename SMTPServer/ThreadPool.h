#pragma once

#include<iostream>

#include <winsock2.h>
#include <ws2tcpip.h>

#include <condition_variable>
#include <mutex>
#include <vector>
#include <thread>
#include <map>

#pragma comment(lib, "Ws2_32.lib")

typedef void (*RUNFUC)(SOCKET);

class ThreadPool
{
public:
    ThreadPool() : m_is_stop(false), m_thread_size(0) {}
    ThreadPool(size_t thread_size);
    ~ThreadPool();

    void AddTask(void* in_task, SOCKET socket);

    void set_thread_size(size_t m_thread_size);

private:
    void Join();
    void Stop();
    void DoTask();

    bool IsStopped() { return m_is_stop; };

private:
    size_t m_thread_size;
    bool m_is_stop;

    std::condition_variable m_task_not_empty;
    std::mutex m_mutex;

    std::vector<std::thread> m_thread_pool;
    std::map<RUNFUC, SOCKET> m_task;
};

