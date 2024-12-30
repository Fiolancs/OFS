#include "OFS_ThreadPool.h"

//#include "BS_thread_pool.hpp"
#include "../3rdParty/thread-pool/include/BS_thread_pool.hpp"

#include <memory>
#include <string>
#include <utility>

namespace
{
#ifdef NDEBUG
    using PoolImpl_t = BS::light_thread_pool;
#else
    using PoolImpl_t = BS::wdc_thread_pool;
#endif

    PoolImpl_t& getPoolImpl(std::shared_ptr<void>& impl) noexcept
    {
        return *std::static_pointer_cast<PoolImpl_t>(impl);
    }
}

static std::unique_ptr<OFS::ThreadPool> gThreadPool = nullptr;

OFS::ThreadPool& OFS::ThreadPool::get(void) noexcept
{
    static const int _ = [] { 
        gThreadPool = std::make_unique<OFS::ThreadPool>(std::thread::hardware_concurrency(), "OFS gThreadPool"); 
        return 1; 
    }();

    return *gThreadPool;
}

void OFS::ThreadPool::detachTask(std::move_only_function<void(void)> f)
{
    auto& impl = getPoolImpl(pImpl);
    impl.detach_task(std::move(f));
    // Consider making a 0s wait here to clear the detached tasks instead for user simplicity?
    //{
    //	using namespace std::chrono_literals;
    //	impl.wait_for(0ms);
    //}
}

bool OFS::ThreadPool::waitDetachedTasks(std::chrono::nanoseconds ns)
{
    auto& impl = getPoolImpl(pImpl);
    return impl.wait_for(ns);
}

OFS::ThreadPool::ThreadPool(unsigned poolSize, std::string_view name)
    : pImpl{ std::make_shared<PoolImpl_t>(
            poolSize, 
            [name = std::string(name)] { 
                if (!name.empty()) BS::this_thread::set_os_thread_name(name);
            }
    )}
{
}

OFS::ThreadPool::~ThreadPool(void) noexcept
{
    getPoolImpl(pImpl).wait();
}
