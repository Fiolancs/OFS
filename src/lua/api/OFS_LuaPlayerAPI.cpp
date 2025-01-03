#include "OFS_LuaPlayerAPI.h"
#include "lua/OFS_LuaExtensionAPI.h"
#include "OFS_Util.h"
#include "OpenFunscripter.h"

// QQQ
using lua_Number = double;

OFS_PlayerAPI::~OFS_PlayerAPI() noexcept
{
}

OFS_PlayerAPI::OFS_PlayerAPI(/*sol::state_view& L*/) noexcept
{
    //auto player = L.new_usertype<OFS_PlayerAPI>(OFS_ExtensionAPI::PlayerNamespace);
    //player.set_function("Play", 
    //    sol::overload(
    //        OFS_PlayerAPI::Play,
    //        OFS_PlayerAPI::TogglePlay
    //    )
    //);
    //player["Seek"] = OFS_PlayerAPI::Seek;
    //player["CurrentTime"] = OFS_PlayerAPI::CurrentTime;
    //player["Duration"] = OFS_PlayerAPI::Duration;
    //player["IsPlaying"] = OFS_PlayerAPI::IsPlaying;
    //player["CurrentVideo"] = OFS_PlayerAPI::CurrentVideo;
    //player["FPS"] = OFS_PlayerAPI::FPS;
    //player["Width"] = OFS_PlayerAPI::VideoWidth;
    //player["Height"] = OFS_PlayerAPI::VideoHeight;
    //
    //player["playbackSpeed"] = sol::property(OFS_PlayerAPI::getPlaybackSpeed, OFS_PlayerAPI::setPlaybackSpeed);
}

void OFS_PlayerAPI::TogglePlay() noexcept
{
    auto app = OpenFunscripter::ptr;
    app->player->togglePause();    
}

void OFS_PlayerAPI::Play(bool shouldPlay) noexcept
{
    auto app = OpenFunscripter::ptr;
    app->player->setPause(!shouldPlay);
}

void OFS_PlayerAPI::Seek(lua_Number time) noexcept
{
    auto app = OpenFunscripter::ptr;
    app->player->SetPositionExact(time);
}

lua_Number OFS_PlayerAPI::CurrentTime() noexcept
{
    auto app = OpenFunscripter::ptr;
    return app->player->CurrentTime();
}

lua_Number OFS_PlayerAPI::Duration() noexcept
{
    auto app = OpenFunscripter::ptr;
    return app->player->Duration();
}

bool OFS_PlayerAPI::IsPlaying() noexcept
{
    auto app = OpenFunscripter::ptr;
    return !app->player->isPaused();
}

std::string OFS_PlayerAPI::CurrentVideo() noexcept
{
    auto app = OpenFunscripter::ptr;
    auto path = app->player->videoPath();
    return { path.begin(), path.end() };
}

lua_Number OFS_PlayerAPI::FPS() noexcept
{
    auto app = OpenFunscripter::ptr;
    return app->player->getFPS();
}

void OFS_PlayerAPI::setPlaybackSpeed(lua_Number speed) noexcept
{
    auto app = OpenFunscripter::ptr;
    app->player->setSpeed(speed);
}

lua_Number OFS_PlayerAPI::getPlaybackSpeed() noexcept
{
    auto app = OpenFunscripter::ptr;
    return app->player->CurrentSpeed();
}

lua_Number OFS_PlayerAPI::VideoWidth() noexcept
{
    auto app = OpenFunscripter::ptr;
    return app->player->VideoWidth();
}

lua_Number OFS_PlayerAPI::VideoHeight() noexcept
{
    auto app = OpenFunscripter::ptr;
    return app->player->VideoHeight();
}