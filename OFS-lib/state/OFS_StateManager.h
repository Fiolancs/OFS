#pragma once
#include "OFS_Util.h"
#include "OFS_TypedID.h"
#include "io/OFS_Serialization.h"

#include <glaze/glaze.hpp>

#include <any>
#include <map>
#include <string>
#include <vector>
#include <cstdint>
#include <string_view>

namespace OFS
{
    class StateManager;
    using StateHandle = TypedID<StateManager>;

    template <typename T>
    void serializeState(std::any& state, std::string& json)
    {
        auto *value = std::any_cast<T>(std::addressof(state));
        if (nullptr == value) [[unlikely]]
        {
            FUN_ASSERT(false, "State serialization failed. Serialize function state type mismatch.");
            return;
        }

        if (auto const err = glz::write_json(*value, json); err)
            [[unlikely]]
            FUN_ASSERT(false, err.custom_error_message);
    }

    template <typename T>
    void deserializeState(std::any& state, std::string& json)
    {
        if (nullptr == std::any_cast<T>(std::addressof(state))) [[unlikely]]
        {
            FUN_ASSERT(false, "State serialization failed. deserialize function state type mismatch.");
            state.emplace<T>();
            return;
        }

        if (auto const err = glz::read<glz::opts{ .error_on_unknown_keys = false }>(state.emplace<T>(), json); err)
            [[unlikely]] FUN_ASSERT(false, "State serialization failed.");
    }

    struct StateMeta
    {
        std::any value;
        std::string_view name;

        using SerializationFn = void(*)(std::any&, std::string& json);
        SerializationFn serializeJson;
        SerializationFn deserializeJson;
    };

    class StateManager
    {
    public:
        inline static constexpr StateHandle INVALID_ID = { std::uint32_t(-1) };

        static StateManager* get(void) noexcept;

        template <typename T, std::size_t STATE_GROUP = 0>
        StateHandle registerState(std::string_view stateName, std::string_view typeName);

        template <typename T, std::size_t STATE_GROUP = 0>
        T& getState(StateHandle handle) noexcept;

        template <std::size_t STATE_GROUP>
        std::string serializeStateGroup(void);
        template <std::size_t STATE_GROUP>
        bool deserializeStateGroup(std::string json);

        //void serialize(void);  // TODO: we dont have to support this yet 
        //void deserialize(void);  // TODO: we dont have to support this yet 

        void clearStateGroup(std::size_t group) noexcept;

    private:
        using StateHandleMap = std::map<std::string_view, StateHandle>; // StateName -> Pair(TypeName, Handle)

        struct StateGroup
        {
            StateHandleMap handleMap;
            std::vector<StateMeta> registeredStates;
        };

        void serializeStateGroup  (StateGroup&, std::string& json);
        bool deserializeStateGroup(StateGroup&, std::string& json);

        StateManager(void) = default;

        std::vector<StateGroup> stateGroups;
    };
}


template<typename T, std::size_t STATE_GROUP>
inline OFS::StateHandle OFS::StateManager::registerState(std::string_view stateName, std::string_view /*typeName*/)
{
    if (stateGroups.size() <= STATE_GROUP)
        stateGroups.resize(STATE_GROUP + 1);

    auto& group = stateGroups[STATE_GROUP];
    if (auto const it = group.handleMap.find(stateName); group.handleMap.end() == it)
    {
        auto ret = group.handleMap.emplace_hint(it, stateName, StateHandle(group.registeredStates.size()));
        group.registeredStates.emplace_back(T{}, stateName, serializeState<T>, deserializeState<T>);

        return ret->second;
    }
    else
    {
        FUN_ASSERT(group.registeredStates[it->second.value].name == stateName, "State name hash conflict with already registed state.");
        LOGF_DEBUG("Attempt to register state \"{:s}\" when state \"{:s}\" already exists", 
            stateName, group.registeredStates[it->second.value].name);

        return it->second;
    }
}

template <typename T, std::size_t STATE_GROUP>
inline T& OFS::StateManager::getState(StateHandle handle) noexcept
{
    FUN_ASSERT(STATE_GROUP < stateGroups.size(), "Specified state group out of bounds. Please register something to this group first.");
    FUN_ASSERT(handle.value < stateGroups[STATE_GROUP].registeredStates.size(), "Invalid handle");
    return std::any_cast<T&>(stateGroups[STATE_GROUP].registeredStates[handle.value].value);
}

template <std::size_t STATE_GROUP>
inline std::string OFS::StateManager::serializeStateGroup(void)
{
    if (STATE_GROUP < stateGroups.size())
    {
        std::string outJson{};
        serializeStateGroup(stateGroups[STATE_GROUP], outJson);
        return outJson;
    }
    else
    {
        FUN_ASSERT(false, "Specified state group out of bounds. Please register something to this group first.");
        return "";
    }
}

template <std::size_t STATE_GROUP>
inline bool OFS::StateManager::deserializeStateGroup(std::string json)
{
    if (STATE_GROUP < stateGroups.size())
    {
        FUN_ASSERT(!json.empty(), "JSON is empty.");
        return deserializeStateGroup(stateGroups[STATE_GROUP], json);
    }
    else
    {
        FUN_ASSERT(false, "Specified state group out of bounds. Please register something to this group first.");
    }
    return false;
}
