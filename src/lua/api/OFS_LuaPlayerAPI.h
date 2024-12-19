#pragma once
#include "lua/OFS_Lua.h"

#include <string>

class OFS_PlayerAPI
{
    using lua_Number = double;
    using lua_Integer = int;
private:
    static void Play(bool shouldPlay) noexcept;
    static void TogglePlay() noexcept;
    
    static void Seek(lua_Number time) noexcept;
    static lua_Number CurrentTime() noexcept;
    static lua_Number Duration() noexcept;
    static bool IsPlaying() noexcept;
    static std::string CurrentVideo() noexcept;
    static lua_Number FPS() noexcept;

    static void setPlaybackSpeed(lua_Number speed) noexcept;
    static lua_Number getPlaybackSpeed() noexcept;
    static lua_Number VideoWidth() noexcept;
    static lua_Number VideoHeight() noexcept;

public:
    OFS_PlayerAPI(/*sol::state_view& L*/) noexcept;
    ~OFS_PlayerAPI() noexcept;
};
