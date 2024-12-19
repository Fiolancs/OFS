#pragma once
#include "OFS_VectorSet.h"
#include "state/OFS_StateHandle.h"

#include <imgui.h>

#include <string>
#include <cstdint>



// The lower 3 bytes of Mod are able to be used
enum OFS_ActionTriggerFlags
{
    MouseWheelDirection = 0x1,
};

struct OFS_ActionTrigger
{
    std::int32_t Mod = ImGuiKey_None;
    std::int32_t Key = ImGuiKey_None;
    bool ShouldRepeat = false;
    std::string MappedActionId;

    OFS_ActionTrigger() noexcept {}
    OFS_ActionTrigger(std::int32_t modifier, ImGuiKey key, bool shouldRepeat = false) noexcept
        : Mod(modifier), Key(key), ShouldRepeat(shouldRepeat) {}

    inline ImGuiKey ImKey() const noexcept
    {
        static_assert(sizeof(ImGuiKey) == sizeof(Key));
        return static_cast<ImGuiKey>(Key);
    }

    inline void SetFlag(OFS_ActionTriggerFlags flag, bool value) noexcept
    {
        Mod &= ~(flag);
        if(value) Mod |= flag;
    }

    inline bool operator==(const OFS_ActionTrigger& b) const noexcept 
    {
        return Hash() == b.Hash();
    }

    inline bool operator<(const OFS_ActionTrigger& b) const noexcept
    {
        return Hash() < b.Hash();
    }

    inline std::uint64_t Hash() const noexcept
    {
        static_assert(sizeof(ImGuiKey) == 4);
        return ((std::int64_t)Mod << 4) | Key;
    }
};

//REFL_TYPE(OFS_ActionTrigger)
//    REFL_FIELD(Key)
//    REFL_FIELD(Mod)
//    REFL_FIELD(ShouldRepeat)
//    REFL_FIELD(MappedActionId)
//REFL_END

struct OFS_KeybindingState
{
    static constexpr auto StateName = "KeybindingState";
    vector_set<OFS_ActionTrigger> Triggers;
    bool convertedToImGui = false;

    void ConvertToOFS() noexcept;
    void ConvertToImGui() noexcept;

    inline static OFS_KeybindingState& StateSlow() noexcept
    {
        auto stateHandle = OFS_AppState<OFS_KeybindingState>::Register(StateName);
        return State(stateHandle);
    }

    inline static OFS_KeybindingState& State(uint32_t stateHandle) noexcept
    {
        return OFS_AppState<OFS_KeybindingState>(stateHandle).Get();
    }
};

//REFL_TYPE(OFS_KeybindingState)
//    REFL_FIELD(Triggers)
//REFL_END