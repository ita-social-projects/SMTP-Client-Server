// This is an independent project of an individual developer. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t threadpool_size)
{
    this->m_threadpool_size = threadpool_size;
    m_is_stop = false;

}

ThreadPool::~ThreadPool()
{
    Join();

    if (!IsStopped())
    {
        Stop();
    }
}

void ThreadPool::Join()
{
    for (size_t i = 0; i < m_threadpool_size; ++i)
    {
        m_thread_pool.at(i).join();
    }
}

void ThreadPool::Stop()
{
    m_is_stop = true;

    for (size_t i = 0; i < m_threadpool_size; ++i)
    {
        m_thread_pool.pop_back();
    }
}

void ThreadPool::DoTask()
{
    while (!m_is_stop && !m_task.empty())
    {
        std::unique_lock<std::mutex> lck(m_mutex);

        while (m_task.empty())
        {
            m_task_not_empty.wait(lck);
        }

        auto t_task = m_task.begin()->first;
        auto s_socket = m_task.begin()->second;

        m_task.erase(m_task.begin());
        lck.unlock();

        t_task(s_socket);
    }
}

void ThreadPool::AddTask(void* in_task, SOCKET socket)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    m_task.emplace((RUNFUC)in_task, socket);

    if (m_task.size() == 1)
    {
        m_task_not_empty.notify_one();
    }

    m_thread_pool.emplace_back(&ThreadPool::DoTask, this);
}

void ThreadPool::set_threadpool_size(size_t threadpool_size)
{
    this->m_threadpool_size = threadpool_size;
}
