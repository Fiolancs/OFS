#pragma once

#include "Funscript.h"
#include "GradientBar.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <vector>
#include <memory>
#include <tuple>

#include "SDL_events.h"

using ActionClickedEventArgs = std::tuple<SDL_Event, FunscriptAction>;

static bool OutputAudioFile(const char* ffmpeg_path, const char* video_path, const char* output_path);

class ScriptPositionsWindow
{
	Funscript* ctx = nullptr;
	ImGradient speedGradient;
	// used for calculating stroke color with speedGradient
	const float max_speed_per_seconds = 530.f; // arbitrarily choosen maximum tuned for coloring
	
	static std::vector<float> audio_waveform_avg;
	static bool ffmpegInProgress;

	ImVec2 canvas_pos;
	ImVec2 canvas_size;
	float offset_ms;
	float frameSizeMs;
	bool IsSelecting = false;
	bool IsMoving = false;
	bool PositionsItemHovered = false;
	float rel_x1 = 0.0f;
	float rel_x2 = 0.0f;

	void mouse_pressed(SDL_Event& ev);
	void mouse_released(SDL_Event& ev);
	void mouse_drag(SDL_Event& ev);
	void mouse_scroll(SDL_Event& ev);

	inline ImVec2 getPointForAction(FunscriptAction action) noexcept {
		float relative_x = (float)(action.at - offset_ms) / frameSizeMs;
		float x = (canvas_size.x) * relative_x;
		float y = (canvas_size.y) * (1 - (action.pos / 100.0));
		x += canvas_pos.x;
		y += canvas_pos.y;
		return ImVec2(x, y);
	}

	inline FunscriptAction getActionForPoint(const ImVec2& point, float frameTime) noexcept {
		ImVec2 localCoord;
		localCoord = point - canvas_pos;
		float relative_x = localCoord.x / canvas_size.x;
		float relative_y = localCoord.y / canvas_size.y;
		float at_ms = offset_ms + (relative_x *frameSizeMs);
		// fix frame alignment
		at_ms =  std::max<float>((int32_t)(at_ms / frameTime) * frameTime, 0.f);
		float pos = Util::Clamp<float>(100.f - (relative_y * 100.f), 0.f, 100.f);
		return FunscriptAction(at_ms, pos);
	}

	void updateSelection(bool clear);

	std::vector<FunscriptAction> ActionPositionWindow;
	std::vector<ImVec2> ActionScreenCoordinates;
	
	void FfmpegAudioProcessingFinished(SDL_Event& ev);

	// ATTENTION: no reordering
	enum RecordingRenderMode : int32_t {
		None,
		All,
		ActiveOnly,
	};

	RecordingRenderMode RecordingMode = RecordingRenderMode::All;
	bool ShowRegularActions = true;
	static bool ShowAudioWaveform;
	static bool ShowLoadedGhosts;
	float ScaleAudio = 1.f;
	int32_t startSelectionMs = -1;
	static float WindowSizeSeconds;
public:
	static constexpr const char* PositionsId = "Positions";
	ScriptPositionsWindow(Funscript* script);
	~ScriptPositionsWindow();

	static constexpr float MAX_WINDOW_SIZE = 60.f; // this limit is arbitrary and not enforced
	static constexpr float MIN_WINDOW_SIZE = 1.f; // this limit is also arbitrary and not enforced

	inline void ClearAudioWaveform() noexcept { audio_waveform_avg.clear(); }
	inline void setStartSelection(int32_t ms) noexcept { startSelectionMs = ms; }
	inline int32_t selectionStart() const noexcept { return startSelectionMs; }
	void ShowScriptPositions(bool* open, float currentPositionMs);
};