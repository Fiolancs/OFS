#pragma once
#include "OFS_StateManager.h"
#include "UI/OFS_Profiling.h"

#include <cstdint>
#include <string_view>

template <typename T>
class OFS_AppState
{
public:
    OFS_AppState(std::uint32_t stateID = OFS_StateManager::INVALID_ID) noexcept
        : id(stateID)
    {}

    T& Get(void) noexcept
    {
        OFS_PROFILE(__FUNCTION__);
        auto mgr = OFS_StateManager::Get();
        return mgr->template GetApp<T>(id);
    }

    static std::uint32_t Register(std::string_view stateName, std::string_view typeName) noexcept
    {
        auto mgr = OFS_StateManager::Get();
        return mgr->RegisterApp<T>(stateName, typeName);
    }

private:
    std::uint32_t id;
};

template <typename T>
class OFS_ProjectState
{
public:
    static constexpr std::uint32_t INVALID_ID = 0xFFFF'FFFF;

    inline OFS_ProjectState(std::uint32_t stateID = OFS_StateManager::INVALID_ID) noexcept
        : id(stateID)
    {}

    T& Get(void) noexcept
    {
        OFS_PROFILE(__FUNCTION__);
        auto mgr = OFS_StateManager::Get();
        return mgr->template GetProject<T>(id);
    }

    static std::uint32_t Register(std::string_view stateName, std::string_view typeName) noexcept
    {
        auto mgr = OFS_StateManager::Get();
        return mgr->RegisterProject<T>(stateName, typeName);
    }

private:
    std::uint32_t id;
};
