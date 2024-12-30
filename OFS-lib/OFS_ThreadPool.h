#pragma once

#include <chrono>
#include <memory>
#include <future>
#include <string_view>

namespace OFS
{
    class ThreadPool
    {
    public:
        // Gets the global thread pool 
        static ThreadPool& get(void) noexcept;

        template <typename Task, typename ... Args>
        auto queueTask(Task&& task, Args&& ...);
        void detachTask(std::function<void(void)>);

        // MUST periodically call this
        // \returns 
        //  true if wait finished, false if wait expired
        bool waitDetachedTasks(std::chrono::nanoseconds);

        // Create a local private thread pool
        explicit ThreadPool(unsigned poolSize = std::thread::hardware_concurrency(), std::string_view name = {});
        ~ThreadPool(void) noexcept;

    private:
        std::shared_ptr<void> pImpl;
    };
}

template <typename Task, typename ...Args>
inline auto OFS::ThreadPool::queueTask(Task&& task, Args&& ... args)
{
    std::packaged_task t{ [t = std::forward<Task>(task), ...args = std::forward<Args>(args)] { return t(std::forward<Args>(args)...); } };
    auto future = t.get_future();

    detachTask([f = std::move(t)] { f(); });

    return future;
}
