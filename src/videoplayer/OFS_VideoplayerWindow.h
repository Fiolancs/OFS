#pragma once
#include "gl/OFS_Shader.h"
#include "event/OFS_SDL_Event.h"

#include "state/OFS_StateManager.h"

#include <imgui.h>

#include <memory>
#include <cstdint>

namespace OFS
{
	class VideoPlayer;
}

class OFS_VideoplayerWindow
{
public:
	~OFS_VideoplayerWindow() noexcept;
	OFS::StateHandle StateHandle() const noexcept { return stateHandle; }
private:
	OFS::VideoPlayer* player = nullptr;
	std::unique_ptr<VrShader> vrShader;
	
	ImGuiID videoImageId;
	ImVec2 videoDrawSize;
	ImVec2 viewportPos;
	ImVec2 windowPos;

	OFS::StateHandle stateHandle = OFS::StateManager::INVALID_ID;

	float baseScaleFactor = 1.f;

	bool videoHovered = false;
	bool dragStarted = false;

	static constexpr float ZoomMulti = 0.05f;

	void mouseScroll(const OFS_SDL_Event* ev) noexcept;
	void drawVrVideo(ImDrawList* draw_list) noexcept;
	void draw2dVideo(ImDrawList* draw_list) noexcept;
	void videoRightClickMenu() noexcept;
public:
	static constexpr const char* WindowId = "###VIDEOPLAYER";
	bool Init(OFS::VideoPlayer* player) noexcept;
	void DrawVideoPlayer(bool* open, bool* drawVideo) noexcept;

	void ResetTranslationAndZoom() noexcept;
};
