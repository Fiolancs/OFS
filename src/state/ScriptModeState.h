#pragma once
#include "state/OFS_StateHandle.h"

struct ScriptingModeState
{
	static constexpr auto StateName = "ScriptingMode";

    int32_t actionInsertDelayMs = 0;

    inline static ScriptingModeState& State(OFS::StateHandle stateHandle) noexcept
    {
        return OFS::AppState<ScriptingModeState>(stateHandle).get();
    }
};

//REFL_TYPE(ScriptingModeState)
//    REFL_FIELD(actionInsertDelayMs)
//REFL_END
