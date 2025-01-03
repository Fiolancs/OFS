#pragma once
#include "event/OFS_Event.h"

#include <string>
#include <cstdint>

enum class VideoplayerType : uint8_t
{
	Main,
	Preview
};

class VideoLoadedEvent : public OFS_Event<VideoLoadedEvent>
{
	public:
	std::u8string videoPath;
	VideoplayerType playerType;

	VideoLoadedEvent(const char* path, VideoplayerType type) noexcept
		: videoPath(path, path+strlen(path)), playerType(type) {}
	VideoLoadedEvent(const std::string& path, VideoplayerType type) noexcept
		: videoPath(path.begin(), path.end()), playerType(type) {}
	VideoLoadedEvent(const std::u8string& path, VideoplayerType type) noexcept
		: videoPath(path), playerType(type) {
	}
};

class PlayPauseChangeEvent : public OFS_Event<PlayPauseChangeEvent>
{
	public:
	bool paused = false;
	VideoplayerType playerType;
	PlayPauseChangeEvent(bool pause, VideoplayerType type) noexcept
		: paused(pause), playerType(type) {}
};

class TimeChangeEvent : public OFS_Event<TimeChangeEvent>
{
	public:
	float time;
	VideoplayerType playerType;
	TimeChangeEvent(float time, VideoplayerType type) noexcept
		: playerType(type), time(time) {} 
};

class DurationChangeEvent : public OFS_Event<DurationChangeEvent>
{
	public:
	float duration;
	VideoplayerType playerType;
	DurationChangeEvent(float duration, VideoplayerType type) noexcept
		: playerType(type), duration(duration) {}
};

class PlaybackSpeedChangeEvent : public OFS_Event<PlaybackSpeedChangeEvent>
{
	public:
	float playbackSpeed;
	VideoplayerType playerType;
	PlaybackSpeedChangeEvent(float speed, VideoplayerType type) noexcept
		: playerType(type), playbackSpeed(speed) {}

};