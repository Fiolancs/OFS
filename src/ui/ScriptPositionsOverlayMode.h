#pragma once
#include "ui/GradientBar.h"
#include "ui/OFS_ScriptTimeline.h"
#include "Funscript/Funscript.h"
#include "Funscript/FunscriptAction.h"
#include "state/states/BaseOverlayState.h"

#include <imgui.h>
#include <imgui_internal.h>

#include <array>
#include <vector>
#include <memory>
#include <cstdint>


struct OverlayDrawingCtx
{
	const std::vector<std::shared_ptr<Funscript>>* scripts;
	
	std::int32_t drawingScriptIdx;
	inline auto& DrawingScript() const noexcept { return (*scripts)[drawingScriptIdx]; }

	std::int32_t hoveredScriptIdx;
	inline auto& HoveredScript() const noexcept { return (*scripts)[hoveredScriptIdx]; }

	std::int32_t activeScriptIdx;
	inline auto& ActiveScript() const noexcept { return (*scripts)[activeScriptIdx]; }

	std::int32_t drawnScriptCount;

	std::int32_t actionFromIdx;
	std::int32_t actionToIdx;

	std::int32_t selectionFromIdx;
	std::int32_t selectionToIdx;

	ImDrawList* drawList;

	ImVec2 canvasPos;
	ImVec2 canvasSize;
	float visibleTime;
	float offsetTime;
	float totalDuration;
};

class BaseOverlay
{
protected:
	class ScriptTimeline* timeline;
	static OFS::StateHandle StateHandle;

	static void drawActionLinesSpline(const OverlayDrawingCtx& ctx, const BaseOverlayState& state) noexcept;
	static void drawActionLinesLinear(const OverlayDrawingCtx& ctx, const BaseOverlayState& state) noexcept;

public:
	inline static BaseOverlayState& State() noexcept
	{
		return BaseOverlayState::State(StateHandle);
	}

	struct ColoredLine {
		ImVec2 p1;
		ImVec2 p2;
		std::uint32_t color;
	};
	static std::vector<ColoredLine> ColoredLines;
	static float PointSize;
	
	static bool ShowLines;
	static bool ShowPoints;

	BaseOverlay(class ScriptTimeline* timeline) noexcept;
	virtual ~BaseOverlay() noexcept {}
	virtual void DrawSettings() noexcept;

	virtual void update() noexcept;
	virtual void DrawScriptPositionContent(const OverlayDrawingCtx& ctx) noexcept {}

	virtual void nextFrame(float realFrameTime) noexcept {}
	virtual void previousFrame(float realFrameTime) noexcept {}
	virtual float steppingIntervalForward(float realFrameTime, float fromTime) noexcept = 0;
	virtual float steppingIntervalBackward(float realFrameTime, float fromTime) noexcept = 0;
	virtual float logicalFrameTime(float realFrameTime) noexcept;

	static void DrawActionLines(const OverlayDrawingCtx& ctx) noexcept;
	static void DrawActionPoints(const OverlayDrawingCtx& ctx) noexcept;
	static void DrawSecondsLabel(const OverlayDrawingCtx& ctx) noexcept;
	static void DrawHeightLines(const OverlayDrawingCtx& ctx) noexcept;
	static void DrawScriptLabel(const OverlayDrawingCtx& ctx) noexcept;

	static ImVec2 GetPointForAction(const OverlayDrawingCtx& ctx, FunscriptAction action) noexcept;
};

class EmptyOverlay : public BaseOverlay
{
public:
	using BaseOverlay::BaseOverlay;

	virtual void DrawScriptPositionContent(const OverlayDrawingCtx& ctx) noexcept override;
	virtual float steppingIntervalForward(float realFrameTime, float fromTime) noexcept override;
	virtual float steppingIntervalBackward(float realFrameTime, float fromTime) noexcept override;
};


