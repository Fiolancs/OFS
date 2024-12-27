#pragma once

#include "event/OFS_Event.h"

#include <SDL3/SDL_events.h>
class OFS_SDL_Event : public OFS_Event<OFS_SDL_Event>
{
    public:
    SDL_Event sdl = {0};
};
