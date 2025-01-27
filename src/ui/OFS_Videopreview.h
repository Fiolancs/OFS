#pragma once
#include "videoplayer/OFS_Videoplayer.h"

#include <memory>
#include <cstdint>
#include <filesystem>


class VideoPreview {
private:
	std::unique_ptr<OFS::VideoPlayer> player;
public:
	VideoPreview(bool hwAccel, std::uint32_t heightOverride = 0) noexcept;
	~VideoPreview() noexcept;

	void Init() noexcept;
	void Update(float delta) noexcept;

	void SetPosition(float pos) noexcept;
	void PreviewVideo(const std::filesystem::path& path, float pos) noexcept;
	void Play() noexcept;
	void Pause() noexcept;
	void CloseVideo() noexcept;

	inline uint32_t FrameTex() const noexcept { return player->getTexture(); }
};
