#pragma once
#include "OFS_Profiling.h"
#include "OFS_StateManager.h"

#include <cstdint>
#include <string_view>

namespace OFS
{
    namespace detail
    {
        template <typename T, std::size_t GROUP_IDX>
        class StateGroupHandle
        {
        public:
            StateGroupHandle(OFS::StateHandle stateID = StateManager::INVALID_ID) noexcept
                : id{ stateID }
            {}

            T& get(void) const noexcept
            {
                OFS_PROFILE(__FUNCTION__);
                auto const stateManager = OFS::StateManager::get();
                return stateManager->getState<T, GROUP_IDX>(id);
            }

            static auto registerState(std::string_view stateName, std::string_view typeName) noexcept
            {
                auto const stateManager = OFS::StateManager::get();
                return stateManager->registerState<T, GROUP_IDX>(stateName, typeName);
            }

            inline static constexpr auto STATE_GROUP = GROUP_IDX;

        private:
            OFS::StateHandle id;
        };

        template <std::size_t GROUP_IDX>
        class StateGroupHandle<void, GROUP_IDX>
        {
        public:
            inline static constexpr auto STATE_GROUP = GROUP_IDX;
        };
    }

    template <typename T>
    using AppState = detail::StateGroupHandle<T, 0>;

    template <typename T>
    using ProjectState = detail::StateGroupHandle<T, 1>;
}
