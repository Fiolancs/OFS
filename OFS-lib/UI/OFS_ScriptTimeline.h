#pragma once
#include "OFS_Waveform.h"
#include "event/OFS_Event.h"
#include "Funscript/Funscript.h"
#include "OFS_ScriptTimelineEvents.h"
#include "ScriptPositionsOverlayMode.h"
#include "videoplayer/OFS_Videoplayer.h"
#include "videoplayer/OFS_VideoplayerEvents.h"

#include <vector>
#include <memory>
#include <string>
#include <cstdint>


class BaseOverlay;

class ScriptTimeline
{
public:
	std::uint32_t overlayStateHandle = 0xFFFF'FFFF;
	float absSel1 = 0.f; // absolute selection start
	float relSel2 = 0.f; // relative selection end

	bool IsSelecting = false;
	bool PositionsItemHovered = false;
	int32_t IsMovingIdx = -1;

	OFS_WaveformLOD Wave;
	static constexpr const char* WindowId = "###POSITIONS";

	static constexpr float MaxVisibleTime = 300.f;
	static constexpr float MinVisibleTime = 1.f;

	void Init();
	inline void ClearAudioWaveform() noexcept { ShowAudioWaveform = false; Wave.data.Clear(); }
	inline void setStartSelection(float time) noexcept { startSelectionTime = time; }
	inline float selectionStart() const noexcept { return startSelectionTime; }
	void ShowScriptPositions(const OFS_Videoplayer* player, BaseOverlay* overlay, const std::vector<std::shared_ptr<Funscript>>& scripts, int activeScriptIdx) noexcept;

	void Update() noexcept;

	void DrawAudioWaveform(const OverlayDrawingCtx& ctx) noexcept;

private:
	void mouseScroll(const OFS_SDL_Event* ev) noexcept;
	void videoLoaded(const class VideoLoadedEvent* ev) noexcept;

	void handleSelectionScrolling(const OverlayDrawingCtx& ctx) noexcept;
	void handleTimelineHover(const OverlayDrawingCtx& ctx) noexcept;
	bool handleTimelineClicks(const OverlayDrawingCtx& ctx) noexcept;

	void updateSelection(const OverlayDrawingCtx& ctx, bool clear) noexcept;
	void FfmpegAudioProcessingFinished(const WaveformProcessingFinishedEvent* ev) noexcept;

	std::string videoPath;
	std::uint32_t visibleTimeUpdate = 0;
	float nextVisisbleTime = 5.f;
	float previousVisibleTime = 5.f;

	float visibleTime = 5.f;
	float startSelectionTime = -1.f;
	
	bool ShowAudioWaveform = false;
	float ScaleAudio = 1.f;
};