#pragma once
#include "OFS_VideoplayerEvents.h"

#include <memory>
#include <string>
#include <cstdint>
#include <filesystem>

namespace OFS
{
    struct VideoPlayerConfig
    {
        std::uint32_t width  = 0; // set to force width. if only one is set, will respect video aspect ratio
        std::uint32_t height = 0; // set to force height. if only one is set, will respect video aspect ratio
        bool allowUserConfig   : 1 = false;
        bool tryHardwareDecode : 1 = false; 
        bool lowQuality        : 1 = false; 
    };

    class VideoPlayer
    {
    public:
        explicit VideoPlayer(VideoPlayerConfig const&);
        ~VideoPlayer(void) noexcept;

        bool init(void) noexcept;
        void shutdown(void) noexcept;
        void update(void) noexcept;

        void openVideo(std::filesystem::path const& path) noexcept;
        void closeVideo(void) noexcept;
        void notifySwap(void) noexcept;

        void setVolume(float volume) noexcept;

        void setMute (bool) noexcept;
        void setPause(bool) noexcept;
        void setSpeed(float) noexcept;
        void addSpeed(float) noexcept;

        void togglePause(void) noexcept { setPause(!isPaused()); }

        float getFPS(void) const noexcept;
        float getVolume(void) const noexcept;
        std::uint32_t getTexture(void) const noexcept;

        bool isValid (void) const noexcept;
        bool isMuted (void) const noexcept;
        bool isPaused(void) const noexcept;
        bool isVideoLoaded(void) const noexcept;

        std::u8string videoPath(void) const noexcept;

        explicit operator bool(void) const noexcept { return isValid(); }

        inline static constexpr float PLAYBACK_SPEED_MIN = .05f;
        inline static constexpr float PLAYBACK_SPEED_MAX = 10.f;
        

        // QQQ
        // The following is copy pasted from the old implementation
        // because we don't want to deal with them yet

        // All seeking functions must update logicalPosition
        void SetPositionExact(float timeSeconds, bool pausesVideo = false) noexcept;
        void SetPositionPercent(float percentPos, bool pausesVideo = false) noexcept;
        void SeekRelative(float timeSeconds) noexcept;
        void SeekFrames(int32_t offset) noexcept;

        void CycleSubtitles() noexcept;
        void SaveFrameToImage(const std::string& directory) noexcept;

        inline void SyncWithPlayerTime() noexcept { SetPositionExact(CurrentPlayerTime()); }

        std::uint16_t VideoWidth() const noexcept;
        std::uint16_t VideoHeight() const noexcept;
        float FrameTime() const noexcept;
        float CurrentSpeed() const noexcept;
        double Duration() const noexcept;
        void NextFrame() noexcept;
        void PreviousFrame() noexcept;

        // Uses the logical position which may be different from CurrentPlayerPosition()
        float CurrentPercentPosition() const noexcept;
        // Also uses the logical position
        double CurrentTime() const noexcept;

        // The "actual" position reported by the player
        double CurrentPlayerPosition() const noexcept;
        double CurrentPlayerTime() const noexcept { return CurrentPlayerPosition() * Duration(); }


    private:
        struct PImpl;
        std::unique_ptr<PImpl> pImpl;
    };
}
