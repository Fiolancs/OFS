#pragma once
#include "OFS_Reflection.h"
#include "event/OFS_Event.h"
#include "Funscript/Funscript.h"
#include "state/OFS_StateHandle.h"

#include <cstdint>


class MetadataChanged : public OFS_Event<MetadataChanged>
{
    public:
    MetadataChanged() noexcept {}
};

class OFS_FunscriptMetadataEditor
{
public:
    inline OFS::StateHandle StateHandle() const noexcept { return stateHandle; }

    OFS_FunscriptMetadataEditor() noexcept;
    bool ShowMetadataEditor(bool* open, Funscript::Metadata& metadata) noexcept;

private:
    OFS::StateHandle stateHandle = OFS::StateManager::INVALID_ID;
};
