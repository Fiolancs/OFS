#pragma once
#include "event/OFS_SDL_Event.h"

#include "OFS_Reflection.h"
#include "io/OFS_BinarySerialization.h"
#include "event/OFS_Event.h"
#include "Funscript/Funscript.h"

#include <imgui.h>

#include <memory>

class ScriptSimulator {
private:
	ImVec2 startDragP1;
	ImVec2 startDragP2;
	ImVec2* dragging = nullptr;
	float mouseValue;
	uint32_t stateHandle = 0xFFFF'FFFF;
	bool IsMovingSimulator = false;
	bool EnableVanilla = false;
	bool MouseOnSimulator = false;
public:
	static constexpr const char* WindowId = "###SIMULATOR";

	float positionOverride = -1.f;

	void MouseMovement(const OFS_SDL_Event* ev) noexcept;

	inline float getMouseValue() const { return mouseValue; }

	void Init() noexcept;
	void CenterSimulator() noexcept;
	void ShowSimulator(bool* open, std::shared_ptr<Funscript>& activeScript, float currentTime, bool splineMode) noexcept;
};

