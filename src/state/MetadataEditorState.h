#pragma once
#include "Funscript/Funscript.h"
#include "state/OFS_StateHandle.h"

struct FunscriptMetadataState
{
    static constexpr auto StateName = "FunscriptMetadata";

    Funscript::Metadata defaultMetadata;

    static inline FunscriptMetadataState& State(OFS::StateHandle stateHandle) noexcept
    {
        return OFS::AppState<FunscriptMetadataState>(stateHandle).get();
    }
};

//REFL_TYPE(FunscriptMetadataState)
//    REFL_FIELD(defaultMetadata)
//REFL_END