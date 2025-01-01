#pragma once
#include "event/OFS_Event.h"
#include "state/OFS_StateManager.h"

#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_atomic.h>

#include <vector>
#include <memory>
#include <vector>
#include <atomic>
#include <cstdint>

struct EventSerializationContext
{
    SDL_Condition* processCond = nullptr;
    
    std::atomic<bool> shouldExit = false;
    std::atomic<bool> hasExited = false;

    SDL_SpinLock eventLock = {0};
    std::vector<EventPointer> events;

    EventSerializationContext() noexcept
    {
        processCond = SDL_CreateCondition();
    }

    template<typename T, typename... Args>
    inline void Push(Args&&... args) noexcept
    {
        SDL_LockSpinlock(&eventLock);
        events.emplace_back(std::move(std::make_shared<T>(std::forward<Args>(args)...)));
        SDL_UnlockSpinlock(&eventLock);
    }

    inline bool EventsEmpty() noexcept
    {
        SDL_LockSpinlock(&eventLock);
        bool empty = events.empty();
        SDL_UnlockSpinlock(&eventLock);
        return empty;
    }

    inline void StartProcessing() noexcept
    {
        SDL_SignalCondition(processCond);
    }

    inline void Shutdown() noexcept
    {
        shouldExit = true;
        StartProcessing();
        while(!hasExited) { 
            SDL_Delay(1); 
        }
        SDL_DestroyCondition(processCond);
    }
};

class OFS_WebsocketApi
{
    private:
    void* ctx = nullptr;
    OFS::StateHandle stateHandle = OFS::StateManager::INVALID_ID;
    std::vector<uint32_t> scriptUpdateCooldown;
    std::unique_ptr<EventSerializationContext> eventSerializationCtx;

    public:
    OFS_WebsocketApi() noexcept;
    OFS_WebsocketApi(const OFS_WebsocketApi&) = delete;
    OFS_WebsocketApi(OFS_WebsocketApi&&) = delete;

    bool Init() noexcept;

    bool StartServer() noexcept;
    void StopServer() noexcept;

    void Update() noexcept;
    void ShowWindow(bool* open) noexcept;
    void Shutdown() noexcept;

    int ClientsConnected() const noexcept;
};