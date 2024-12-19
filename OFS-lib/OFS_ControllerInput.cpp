#include "OFS_ControllerInput.h"

#include "OFS_Util.h"
#include "UI/OFS_Profiling.h"
#include "event/OFS_EventSystem.h"

#include <SDL3/SDL_gamepad.h>

std::array<int64_t, SDL_GAMEPAD_BUTTON_COUNT> ButtonsHeldDown = { -1 };
std::array<ControllerInput, 4> ControllerInput::Controllers;
int32_t ControllerInput::activeControllers = 0;

void ControllerInput::OpenController(int device) noexcept
{
    gamepad = SDL_OpenGamepad(device);
    SDL_Joystick* j = SDL_GetGamepadJoystick(gamepad);
    instance_id = SDL_GetJoystickID(j);
    isConnected = true;
    LOGF_INFO("Controller \"%s\" connected!", GetName());
    if (SDL_IsJoystickHaptic(j)) {
        haptic = SDL_OpenHapticFromJoystick(j);
        LOGF_DEBUG("Haptic Effects: %d\n", SDL_GetMaxHapticEffects(haptic));
        LOGF_DEBUG("Haptic Query: %x\n", SDL_GetHapticFeatures(haptic));
        if (SDL_HapticRumbleSupported(haptic)) {
            if (SDL_InitHapticRumble(haptic) != 0) {
                LOGF_DEBUG("Haptic Rumble Init: %s\n", SDL_GetError());
                SDL_CloseHaptic(haptic);
                haptic = 0;
            }
        }
        else {
            SDL_CloseHaptic(haptic);
            haptic = 0;
        }
    }
}

void ControllerInput::CloseController() noexcept
{
    if (isConnected) {
        isConnected = false;
        if (haptic) {
            SDL_CloseHaptic(haptic);
            haptic = nullptr;
        }
        SDL_CloseGamepad(gamepad);
        gamepad = nullptr;
    }
}

int ControllerInput::GetControllerIndex(SDL_JoystickID instance) noexcept
{
    for (int i = 0; i < Controllers.size(); i += 1) {
        if (Controllers[i].isConnected && Controllers[i].instance_id == instance) {
            return i;
        }
    }
    return -1;
}

void ControllerInput::ControllerButtonDown(const OFS_SDL_Event* ev) const noexcept
{
    OFS_PROFILE(__FUNCTION__);
    constexpr int32_t RepeatDelayMs = 300;
    auto& gbutton = ev->sdl.gbutton;
    ButtonsHeldDown[gbutton.button] = (int64_t)SDL_GetTicks() + RepeatDelayMs;
}

void ControllerInput::ControllerButtonUp(const OFS_SDL_Event* ev) const noexcept
{
    OFS_PROFILE(__FUNCTION__);
    auto& gbutton = ev->sdl.gbutton;
    ButtonsHeldDown[gbutton.button] = -1;
}

void ControllerInput::ControllerDeviceAdded(const OFS_SDL_Event* ev) noexcept
{
    OFS_PROFILE(__FUNCTION__);
    auto& cdevice = ev->sdl.cdevice;
    if (cdevice.which < Controllers.size()) {
        auto& jc = Controllers[cdevice.which];
        jc.OpenController(cdevice.which);
        activeControllers += 1;
    }
}

void ControllerInput::ControllerDeviceRemoved(const OFS_SDL_Event* ev) noexcept
{
    OFS_PROFILE(__FUNCTION__);
    int cIndex = GetControllerIndex(ev->sdl.cdevice.which);
    if (cIndex < 0) return; // unknown controller?
    auto& jc = Controllers[cIndex];
    jc.CloseController();
    activeControllers -= 1;
}

void ControllerInput::Init() noexcept
{
    SDL_SetJoystickEventsEnabled(true);
    SDL_SetGamepadEventsEnabled(true);

    EV::Queue().appendListener(SDL_EVENT_GAMEPAD_ADDED,
        OFS_SDL_Event::HandleEvent(EVENT_SYSTEM_BIND(this, &ControllerInput::ControllerDeviceAdded)));
    EV::Queue().appendListener(SDL_EVENT_GAMEPAD_REMOVED,
        OFS_SDL_Event::HandleEvent(EVENT_SYSTEM_BIND(this, &ControllerInput::ControllerDeviceRemoved)));
    EV::Queue().appendListener(SDL_EVENT_GAMEPAD_BUTTON_DOWN,
        OFS_SDL_Event::HandleEvent(EVENT_SYSTEM_BIND(this, &ControllerInput::ControllerButtonDown)));
    EV::Queue().appendListener(SDL_EVENT_GAMEPAD_BUTTON_UP,
        OFS_SDL_Event::HandleEvent(EVENT_SYSTEM_BIND(this, &ControllerInput::ControllerButtonUp)));
}

void ControllerInput::Update() noexcept
{
}

void ControllerInput::UpdateControllers() noexcept
{
    for (auto& controller : Controllers) {
        if (controller.Connected()) {
            controller.Update();
        }
    }
}

void ControllerInput::Shutdown() noexcept
{
    for (auto& controller : Controllers) {
        controller.CloseController();
    }
}