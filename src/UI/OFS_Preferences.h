#pragma once
#include "OFS_ScriptSimulator.h"
#include "state/PreferenceState.h"
#include "ui/OFS_ScriptPositionsOverlays.h"

#include "OFS_Util.h"
#include "OFS_Reflection.h"
#include "state/OFS_StateManager.h"

#include <imgui.h>

#include <vector>
#include <string>
#include <sstream>


class OFS_Preferences
{
private:
	OFS::StateHandle prefStateHandle = OFS::StateManager::INVALID_ID;
	std::vector<std::string> translationFiles;
public:
	inline OFS::StateHandle StateHandle() const noexcept { return prefStateHandle; }
public:
	bool ShowWindow = false;
	OFS_Preferences() noexcept;
	bool ShowPreferenceWindow() noexcept;
	void SetTheme(OFS_Theme theme) noexcept;
};
