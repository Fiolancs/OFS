#pragma once
#include "state/OFS_StateHandle.h"

#include <string>

struct WebsocketApiState
{
    static constexpr auto StateName = "WebsocketApi";
    std::string port = "8080";
    bool serverActive = false;

    static inline WebsocketApiState& State(OFS::StateHandle stateHandle) noexcept
    {
        return OFS::AppState<WebsocketApiState>(stateHandle).get();
    }
};

//REFL_TYPE(WebsocketApiState)
//    REFL_FIELD(port)
//    REFL_FIELD(serverActive)
//REFL_END