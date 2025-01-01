#pragma once
#include "io/OFS_SerializeHelper.h"
#include "state/OFS_StateHandle.h"

#include <imgui.h>

struct BaseOverlayState
{
    static constexpr auto StateName = "BaseOverlayState";

    ImColor MaxSpeedColor = ImColor(0, 0, 255, 255);
    float MaxSpeedPerSecond = 400.f;
    bool ShowMaxSpeedHighlight = false;
    bool SyncLineEnable = false;
    bool SplineMode = false;

    inline static OFS::StateHandle RegisterStatic() noexcept
    {
        return OFS::AppState<BaseOverlayState>::registerState(StateName, StateName);
    }

    inline static BaseOverlayState& State(OFS::StateHandle stateHandle) noexcept
    {
        return OFS::AppState<BaseOverlayState>(stateHandle).get();
    }
};

//REFL_TYPE(BaseOverlayState)
//    REFL_FIELD(MaxSpeedColor)
//    REFL_FIELD(MaxSpeedPerSecond)
//    REFL_FIELD(ShowMaxSpeedHighlight)
//    REFL_FIELD(SyncLineEnable)
//    REFL_FIELD(SplineMode)
//REFL_END
