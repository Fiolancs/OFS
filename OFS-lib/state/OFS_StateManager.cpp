#include "OFS_StateManager.h"

#include <glaze/glaze.hpp>

#include <chrono>
#include <string>


OFS::StateManager* OFS::StateManager::get(void) noexcept
{
    static StateManager gStateManager{};
    return &gStateManager;
}

void OFS::StateManager::clearStateGroup(std::size_t group) noexcept
{
    FUN_ASSERT(group < stateGroups.size(), "Specified state group out of bounds. Please register something to this group first.");

    std::string json = "{}";
    for (auto& state : stateGroups[group].registeredStates)
    {
        state.deserializeJson(state.value, json);
    }
}

void OFS::StateManager::serializeStateGroup(StateGroup& group, std::string& json)
{
    std::vector<std::pair<std::string_view, glz::raw_json>> partial{};
    partial.reserve(group.registeredStates.size());

    for (auto& state : group.registeredStates)
    {
        auto& [_, partialJson] = partial.emplace_back(state.name, glz::raw_json{});
        state.serializeJson(state.value, partialJson.str);
    }

    if (auto const err = glz::write_json(partial, json); err)
    {
        FUN_ASSERT(false, err.custom_error_message);
        json = "{}";
    }
}

bool OFS::StateManager::deserializeStateGroup(StateGroup& group, std::string& json)
{
    std::map<std::string_view, glz::raw_json_view> partial{};

    if (auto const err = glz::read_json(partial, json); err)
        return false;

    for (auto& state : group.registeredStates)
    {
        if (auto const it = partial.find(state.name); partial.end() != it)
        {
            std::string partialJson{ it->second.str };
            state.deserializeJson(state.value, partialJson);
        }
    }
    return true;
}
