#include "OFS_Videopreview.h"

#include "OFS_Profiling.h"

#include <cstdint>

VideoPreview::VideoPreview(bool hwAccel, std::uint32_t heightOverride) noexcept
{
	player = std::make_unique<OFS::VideoPlayer>(OFS::VideoPlayerConfig{ .height = heightOverride, .tryHardwareDecode = hwAccel, .highQuality = false });
	player->init(/*hwAccel*/);
}

VideoPreview::~VideoPreview() noexcept
{
}

void VideoPreview::Init() noexcept
{
	player->setVolume(0.f);
}

void VideoPreview::Update(float delta) noexcept
{
	OFS_PROFILE(__FUNCTION__);
	player->update(/*delta*/);
}

void VideoPreview::SetPosition(float pos) noexcept
{
	OFS_PROFILE(__FUNCTION__);
	player->SetPositionPercent(pos);
}

void VideoPreview::PreviewVideo(const std::string& path, float pos) noexcept
{
	OFS_PROFILE(__FUNCTION__);
	auto existing = player->videoPath();
	if (std::filesystem::path(path).u8string() != player->videoPath())
	{
		player->openVideo(path);
		player->setVolume(0.f);
	}
}

void VideoPreview::Play() noexcept
{
	OFS_PROFILE(__FUNCTION__);
	player->setPause(false);
}

void VideoPreview::Pause() noexcept
{
	OFS_PROFILE(__FUNCTION__);
	player->setPause(true);
}

void VideoPreview::CloseVideo() noexcept
{
	player->closeVideo();
}
