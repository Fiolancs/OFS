#include "OpenFunscripter.h"
#include "state/OpenFunscripterState.h"
#include "state/OFS_LibState.h"

#include <SDL3/SDL_main.h>

int main(int argc, char* argv[])
{
    OpenFunscripterState::RegisterAll();

    OpenFunscripter app;

    if (app.Init(argc, argv))
    {
        int code = app.Run();
        app.Shutdown();
        return code;
    }

    return -1;
}
