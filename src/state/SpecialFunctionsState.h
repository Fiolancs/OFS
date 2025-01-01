#pragma once
#include "state/OFS_StateHandle.h"

// ATTENTION: no reordering
enum class SpecialFunctionType : int32_t
{
	RangeExtender,
	RamerDouglasPeucker, 
	TotalFunctionCount
};

struct SpecialFunctionState
{
    static constexpr auto StateName = "SpecialFunctionState";

    SpecialFunctionType selectedFunction = SpecialFunctionType::RangeExtender;

    static inline SpecialFunctionState& State(OFS::StateHandle stateHandle) noexcept
    {
        return OFS::AppState<SpecialFunctionState>(stateHandle).get();
    }
};

//REFL_TYPE(SpecialFunctionState)
//    REFL_FIELD(selectedFunction, serializeEnum{})
//REFL_END
