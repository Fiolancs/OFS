#include "videoplayer/OFS_VideoPlayer.h"
#include "gl/OFS_GL.h"
#include "OFS_SDLUtil.h"

#include "OFS_Util.h"
#include "io/OFS_FileLogging.h"
#include "event/OFS_EventSystem.h"
#include "videoplayer/OFS_VideoPlayerEvents.h"

#define OFS_MPV_LOADER_MACROS
#include "OFS_MpvLoader.h"

#include <SDL3/SDL_video.h>

#include <atomic>
#include <string>
#include <format>
#include <memory>
#include <cstdint>
#include <utility>
#include <algorithm>
#include <filesystem>
#include <string_view>

namespace
{
    // QQQ
    struct MpvDataCache
    {
        double duration = 1.0;
        double percentPosition = 0.0;
        double currentSpeed = 1.0;
        double fps = 30.0;
        double averageFrameTime = 1.0 / fps;

        double abLoopA = 0;
        double abLoopB = 0;

        std::int64_t totalNumFrames = 0;
        std::int64_t paused = false;
        std::int64_t videoWidth = 0;
        std::int64_t videoHeight = 0;

        float currentVolume = .5f;

        bool videoLoaded = false;
        std::string filePath = "";
    };
    enum MpvPropertyGet : std::uint64_t
    {
        MpvDuration,
        MpvPosition,
        MpvTotalFrames,
        MpvSpeed,
        MpvVideoWidth,
        MpvVideoHeight,
        MpvPauseState,
        MpvFilePath,
        MpvHwDecoder,
        MpvFramesPerSecond,
    };

    struct MpvPlayerCtx
    {
        enum CtxFlags : std::uint8_t
        {
            FLAG_NONE         = 0,
            FORCE_WIDTH  = 1 << 0,
            FORCE_HEIGHT = 1 << 1,
        };

        std::uint32_t framebuffer;
        std::uint32_t frameTexture;

        std::uint32_t fbWidth;
        std::uint32_t fbHeight;

        CtxFlags flags;
        std::atomic_bool hasEvent      = false;
        std::atomic_bool renderRequest = false;

        bool isFlagSet(CtxFlags flagToTest) const noexcept
        {
            return (flags & flagToTest) == flagToTest;
        }
    };
    struct VideoProperties
    {
        std::filesystem::path path;

        std::int64_t frames;
        std::int32_t width;
        std::int32_t height;

        double duration;
        double percentPosition;
        double fps;
        float volume    = 100.f;
        float playSpeed = 1.f;

        bool isMute   = false;
        bool isPause  = false;
        bool isLoaded = false;
    };

    // QQQ
    void notifyVideoLoaded(VideoProperties& ctx) noexcept
    {
        EV::Enqueue<VideoLoadedEvent>(ctx.path.u8string(), VideoplayerType{});
    }
    void notifyPaused(VideoProperties& ctx) noexcept
    {
        EV::Enqueue<PlayPauseChangeEvent>(bool(ctx.isPause), VideoplayerType{});
    }
    void notifyTime(VideoProperties& ctx) noexcept
    {
        EV::Enqueue<TimeChangeEvent>((float)(ctx.duration * ctx.percentPosition), VideoplayerType{});
    }
    void notifyDuration(VideoProperties& ctx) noexcept
    {
        EV::Enqueue<DurationChangeEvent>(ctx.duration, VideoplayerType{});
    }
    void notifyPlaybackSpeed(VideoProperties& ctx) noexcept
    {
        EV::Enqueue<PlaybackSpeedChangeEvent>(ctx.playSpeed, VideoplayerType{});
    }
    
    void showText(mpv_handle* mpv, const char* text) noexcept
    {
        const char* cmd[] = { "show_text", text, NULL };
        mpv_command_async(mpv, 0, cmd);
    }
}

struct OFS::VideoPlayer::PImpl
{
    mpv_handle* mpv;
    mpv_render_context* renderCtx;

    MpvPlayerCtx playerContext;
    VideoProperties videoProperties;

    mpv_handle*  get(void) const noexcept { return mpv; }
    static bool  isMpvError(int errorCode) { return errorCode < 0; }

    static void* getProcAddress(void* fn_ctx, const char* name) { return SDL_GL_GetProcAddress(name); }
    static void  mpvEventCallback(void* self) { ((OFS::VideoPlayer::PImpl*)(self))->playerContext.hasEvent.store(true, std::memory_order_relaxed); }
    static void  mpvRenderCallback(void* self) { ((OFS::VideoPlayer::PImpl*)(self))->playerContext.renderRequest.store(true, std::memory_order_relaxed); }

    void mpvSetPropertyCommand(bool   value, char const* const propertyLiteral) const noexcept;
    void mpvSetPropertyCommand(double value, char const* const propertyLiteral) const noexcept;
    void handleMpvEvent(mpv_event const*);

    void mpvRenderFrame(void) const noexcept;
    void updateRenderTexture(void) noexcept;
};

bool OFS::VideoPlayer::init(void) noexcept
{
    if (pImpl && pImpl->mpv && !pImpl->renderCtx)
    {
        int one = 1;
        char renderApi[] = MPV_RENDER_API_TYPE_OPENGL;
        mpv_opengl_init_params mpvOpenglParams{ .get_proc_address = PImpl::getProcAddress };
        mpv_render_param params[] = {
            {MPV_RENDER_PARAM_API_TYPE, renderApi},
            {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &mpvOpenglParams},
            {MPV_RENDER_PARAM_ADVANCED_CONTROL, &one},
            {}
        };

        if (int error = mpv_render_context_create(&pImpl->renderCtx, pImpl->get(), params); !PImpl::isMpvError(error))
        {
            mpv_observe_property(pImpl->get(), MpvVideoHeight,     "height",                MPV_FORMAT_INT64);
            mpv_observe_property(pImpl->get(), MpvVideoWidth,      "width",                 MPV_FORMAT_INT64);
            mpv_observe_property(pImpl->get(), MpvDuration,        "duration",              MPV_FORMAT_DOUBLE);
            mpv_observe_property(pImpl->get(), MpvPosition,        "percent-pos",           MPV_FORMAT_DOUBLE);
            mpv_observe_property(pImpl->get(), MpvTotalFrames,     "estimated-frame-count", MPV_FORMAT_INT64);
            mpv_observe_property(pImpl->get(), MpvSpeed,           "speed",                 MPV_FORMAT_DOUBLE);
            mpv_observe_property(pImpl->get(), MpvPauseState,      "pause",                 MPV_FORMAT_FLAG);
            mpv_observe_property(pImpl->get(), MpvFilePath,        "path",                  MPV_FORMAT_STRING);
            mpv_observe_property(pImpl->get(), MpvHwDecoder,       "hwdec-current",         MPV_FORMAT_STRING);
            mpv_observe_property(pImpl->get(), MpvFramesPerSecond, "estimated-vf-fps",      MPV_FORMAT_DOUBLE);

            mpv_set_wakeup_callback(pImpl->get(), &PImpl::mpvEventCallback, pImpl.get());
            mpv_render_context_set_update_callback(pImpl->renderCtx, &PImpl::mpvRenderCallback, pImpl.get());
        }
        else
            LOG_ERROR("Failed to initialize mpv render context");
    }

    return isValid();
}

void OFS::VideoPlayer::update(void) noexcept
{
    bool const hasEvent = pImpl->playerContext.hasEvent.exchange(false, std::memory_order_acquire);
    while (hasEvent)
    {
        mpv_event const* ev = mpv_wait_event(pImpl->get(), 0.);
        if (ev->event_id == mpv_event_id::MPV_EVENT_NONE)
            break;

        pImpl->handleMpvEvent(ev);
    }

    while (pImpl->playerContext.renderRequest.exchange(false, std::memory_order_acquire))
    {
        if (std::uint64_t flags = mpv_render_context_update(pImpl->renderCtx); flags & MPV_RENDER_UPDATE_FRAME)
        {
            pImpl->mpvRenderFrame();
        }
    }
}

void OFS::VideoPlayer::openVideo(std::filesystem::path const& path) noexcept
{
    LOGF_INFO("Opening video: \"{:s}\"", path.string());
    closeVideo();

    auto u8Str = path.u8string();
    auto pathStr = std::string(u8Str.begin(), u8Str.end());
    const char* cmd[] = { "loadfile", pathStr.c_str(), nullptr };
    mpv_command_async(pImpl->get(), 0, cmd);

    auto const oldProps = std::exchange(pImpl->videoProperties, {});

    setPause(true);
    setMute(oldProps.isMute);
    setVolume(oldProps.volume);
    setSpeed(oldProps.playSpeed);
}

void OFS::VideoPlayer::closeVideo(void) noexcept
{
    pImpl->videoProperties.isLoaded = false;
    char const* cmd[] = { "stop", nullptr };
    mpv_command_async(pImpl->get(), 0, cmd);
    setPause(true);
}

void OFS::VideoPlayer::notifySwap(void) noexcept
{
    mpv_render_context_report_swap(pImpl->renderCtx);
}

void OFS::VideoPlayer::setVolume(float volume) noexcept
{
    if (auto& options = pImpl->videoProperties; volume != options.volume)
    {
        options.volume = volume;
        pImpl->mpvSetPropertyCommand(options.volume, "volume");
    }
}

void OFS::VideoPlayer::setMute(bool mute) noexcept
{
    if (auto& options = pImpl->videoProperties; mute != options.isMute)
    {
        options.isMute = mute;
        pImpl->mpvSetPropertyCommand(options.isMute, "mute");
    }
}

void OFS::VideoPlayer::setPause(bool pause) noexcept
{
    if (auto& options = pImpl->videoProperties; pause != options.isPause)
    {
        options.isPause = pause;
        pImpl->mpvSetPropertyCommand(options.isPause, "pause");
    }
}

void OFS::VideoPlayer::setSpeed(float speed) noexcept
{
    speed = std::clamp(speed, PLAYBACK_SPEED_MIN, PLAYBACK_SPEED_MAX);
    if (auto& options = pImpl->videoProperties; options.playSpeed != speed)
    {
        options.playSpeed = speed;
        pImpl->mpvSetPropertyCommand(options.playSpeed, "speed");
    }
}

void OFS::VideoPlayer::addSpeed(float offset) noexcept
{
    setSpeed(pImpl->videoProperties.playSpeed + offset);
}

float OFS::VideoPlayer::getFPS(void) const noexcept
{
    return pImpl->videoProperties.fps;
}

float OFS::VideoPlayer::getVolume(void) const noexcept
{
    return pImpl->videoProperties.volume;
}

std::uint32_t OFS::VideoPlayer::getTexture() const noexcept
{
    return pImpl->playerContext.frameTexture;
}

bool OFS::VideoPlayer::isMuted(void) const noexcept
{
    return pImpl->videoProperties.isMute;
}

bool OFS::VideoPlayer::isPaused(void) const noexcept
{
    return pImpl->videoProperties.isPause;
}

bool OFS::VideoPlayer::isVideoLoaded(void) const noexcept
{
    return pImpl->videoProperties.isLoaded;
}

std::u8string OFS::VideoPlayer::videoPath(void) const noexcept
{
    return pImpl->videoProperties.path.u8string();
}


OFS::VideoPlayer::VideoPlayer(VideoPlayerConfig const& cfg)
    : pImpl{ std::make_unique<PImpl>(mpv_create(), nullptr) }
{
    if (pImpl->get() == nullptr)
    {
        LOG_ERROR("Failed to create mpv instance.");
        return;
    }

    pImpl->playerContext.fbWidth  = cfg.width;
    pImpl->playerContext.fbHeight = cfg.height;
    pImpl->playerContext.flags = static_cast<MpvPlayerCtx::CtxFlags>(pImpl->playerContext.flags | (pImpl->playerContext.fbWidth  ? MpvPlayerCtx::FORCE_WIDTH  : MpvPlayerCtx::FLAG_NONE));
    pImpl->playerContext.flags = static_cast<MpvPlayerCtx::CtxFlags>(pImpl->playerContext.flags | (pImpl->playerContext.fbHeight ? MpvPlayerCtx::FORCE_HEIGHT : MpvPlayerCtx::FLAG_NONE));

    // Set profiles first 

    if (int error = mpv_set_property_string(pImpl->get(), "profile", "libmpv"); PImpl::isMpvError(error))
        LOG_WARN("Failed to set mpv: profile=libmpv");

    if (cfg.lowQuality)
    {
        if (int error = mpv_set_property_string(pImpl->get(), "profile", "fast"); PImpl::isMpvError(error))
            LOG_WARN("Failed to set mpv: profile=fast");
    }

    if (cfg.allowUserConfig)
    {
        auto configPath    = OFS::util::preferredPath().u8string();
        auto configPathStr = std::string(configPath.begin(), configPath.end());

        if (int error = mpv_set_property_string(pImpl->get(), "config", "yes"); PImpl::isMpvError(error))
            LOG_WARN("Failed to set mpv: config=yes");
        if (int error = mpv_set_property_string(pImpl->get(), "config-dir", configPathStr.c_str()); PImpl::isMpvError(error))
            LOGF_WARN("Failed to set mpv: config-dir={:s}", configPathStr);
    }

    if (cfg.tryHardwareDecode)
    {
        if (int error = mpv_set_property_string(pImpl->get(), "hwdec", "auto-safe"); PImpl::isMpvError(error))
            LOG_WARN("Failed to set mpv: hwdec=auto-safe");
    }

    if (int error = mpv_set_property_string(pImpl->get(), "loop-file", "inf"); error < 0)
        LOG_WARN("Failed to set mpv: loop-file=inf");

    if (int error = mpv_initialize(pImpl->get()); PImpl::isMpvError(error))
    {
        LOG_ERROR("Failed to initialize mpv instance.");
        shutdown();
        return;
    }


    // Set these *after* init as we don't allow the user to change these 
    // mpv init may load a config that sets these so we override them

    if (int error = mpv_set_property_string(pImpl->get(), "vo", "libmpv"); PImpl::isMpvError(error))
    {
        LOG_ERROR("Failed to set mpv: vo=libmpv");
        shutdown();
        return;
    }
}

OFS::VideoPlayer::~VideoPlayer(void) noexcept
{
    shutdown();
}

void OFS::VideoPlayer::shutdown(void) noexcept
{
    if (pImpl->renderCtx) mpv_render_context_free(pImpl->renderCtx);
    if (pImpl->mpv) mpv_destroy(pImpl->mpv);

    pImpl->renderCtx = nullptr;
    pImpl->mpv = nullptr;
}

bool OFS::VideoPlayer::isValid(void) const noexcept
{
    if (!pImpl || !pImpl->mpv || !pImpl->renderCtx)
        return false;
    return true;
}


void OFS::VideoPlayer::PImpl::mpvSetPropertyCommand(bool value, char const* const property) const noexcept
{
    char const* command[]{
        "set", property, value ? "yes" : "no", nullptr
    };
    mpv_command_async(mpv, 0, command);
}

void OFS::VideoPlayer::PImpl::mpvSetPropertyCommand(double value, char const* const property) const noexcept
{
    char buffer[16]{};
    std::format_to_n(buffer, std::size(buffer), "{:f}", value);
    const char* command[]{ "set", property, buffer, nullptr };
    mpv_command_async(mpv, 0, command);
}

void OFS::VideoPlayer::PImpl::handleMpvEvent(mpv_event const* ev)
{
    switch (ev->event_id)
    {
    case MPV_EVENT_LOG_MESSAGE:
    {
        auto msg = static_cast<mpv_event_log_message const*>(ev->data);
        OFS::FileLogger::get().logToFile(std::format("[{:s}][MPV] ({:s}): ", msg->level, msg->prefix), msg->text, false);
        break;
    }
    case MPV_EVENT_COMMAND_REPLY:
    {
        // attach user_data to command
        // and handle it here when it finishes
        break;
    }
    case MPV_EVENT_FILE_LOADED:
    {
        videoProperties.isLoaded = true;
        break;
    }
    case MPV_EVENT_PROPERTY_CHANGE:
    {
        auto prop = static_cast<mpv_event_property const*>(ev->data);
        if (prop->data == nullptr)
            break;

        switch (ev->reply_userdata)
        {
        case MpvHwDecoder:
            LOGF_INFO("Active hardware decoder: {:s}", *(char**)prop->data);
            break;

        case MpvVideoWidth:
        {
            videoProperties.width = *(std::int64_t*)prop->data;
            if (videoProperties.height > 0)
            {
                updateRenderTexture();
                videoProperties.isLoaded = true;
            }
            break;
        }
        case MpvVideoHeight:
        {
            videoProperties.height = *(std::int64_t*)prop->data;
            if (videoProperties.width > 0)
            {
                updateRenderTexture();
                videoProperties.isLoaded = true;
            }
            break;
        }
        case MpvFramesPerSecond:
            videoProperties.fps = *(double*)prop->data;
            break;

        case MpvDuration:
            videoProperties.duration = *(double*)prop->data;
            // QQQ
            //notifyDuration(ctx);
            break;

        case MpvTotalFrames:
            videoProperties.frames = *(std::int64_t*)prop->data;
            break;

        case MpvPosition:
        {
            auto newPercentPos = (*(double*)prop->data) / 100.0;
            videoProperties.percentPosition = newPercentPos;
            // QQQ
            //ctx->smoothTimer = SDL_GetTicks();
            //if (!videoProperties.isPause) {
            //    *ctx->logicalPosition = newPercentPos;
            //}
            notifyTime(videoProperties);
            break;
        }
        case MpvSpeed:
            videoProperties.playSpeed = *(double*)prop->data;
            notifyPlaybackSpeed(videoProperties);
            break;

        case MpvPauseState:
        {
            bool paused = *(std::int64_t*)prop->data;
            // QQQ
            //if (paused)
            //{
            //    float timeSinceLastUpdate = (SDL_GetTicks() - CTX->smoothTimer) / 1000.f;
            //    float positionOffset = (timeSinceLastUpdate * pImpl->videoProperties.currentSpeed) / pImpl->videoProperties.duration;
            //    *ctx->logicalPosition += positionOffset;
            //}
            //ctx->smoothTimer = SDL_GetTicks();
            videoProperties.isPause = paused;
            notifyPaused(videoProperties);
            break;
        }
        case MpvFilePath:
        {
            auto const data = std::string_view(*((const char**)(prop->data)));
#if _WIN32
            videoProperties.path = OFS::util::pathFromU8String(data);
#else
            videoProperties.path = data;  // on linux this is a native string and doesn't have to be utf8
#endif
            notifyVideoLoaded(videoProperties);
            break;
        }
        default: break;
        }
    }
    default: break;
    }
}

void OFS::VideoPlayer::PImpl::mpvRenderFrame(void) const noexcept
{
    mpv_opengl_fbo fbo{
        .fbo = int(playerContext.framebuffer),
        .w = int(playerContext.fbWidth),
        .h = int(playerContext.fbHeight),
        .internal_format = OFS_InternalTexFormat
    };

    std::uint32_t no = 0, yes = 1;
    mpv_render_param params[] = {
        {MPV_RENDER_PARAM_OPENGL_FBO, &fbo},
        {MPV_RENDER_PARAM_BLOCK_FOR_TARGET_TIME, &no},
        mpv_render_param{}
    };

    mpv_render_context_render(renderCtx, params);
}

void OFS::VideoPlayer::PImpl::updateRenderTexture(void) noexcept
{
    if (videoProperties.width <= 0 || videoProperties.height <= 0)
        return;

    if (!playerContext.isFlagSet(playerContext.FORCE_WIDTH) && !playerContext.isFlagSet(playerContext.FORCE_HEIGHT))
    {
        playerContext.fbWidth  = videoProperties.width;
        playerContext.fbHeight = videoProperties.height;
    }
    else if (playerContext.isFlagSet(playerContext.FORCE_HEIGHT))
    {
        playerContext.fbWidth = std::uint32_t(float(playerContext.fbHeight * videoProperties.width) / videoProperties.height);
    }
    else if (playerContext.isFlagSet(playerContext.FORCE_WIDTH))
    {
        playerContext.fbHeight = std::uint32_t(float(playerContext.fbWidth * videoProperties.height) / videoProperties.width);
    }

    if (!playerContext.framebuffer)
    {
        glGenFramebuffers(1, &playerContext.framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, playerContext.framebuffer);

        glGenTextures(1, &playerContext.frameTexture);
        glBindTexture(GL_TEXTURE_2D, playerContext.frameTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, OFS_InternalTexFormat, playerContext.fbWidth, playerContext.fbHeight, 0, OFS_TexFormat, GL_UNSIGNED_BYTE, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Set "renderedTexture" as our colour attachement #0
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, playerContext.frameTexture, 0);

        // Set the list of draw buffers.
        GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, DrawBuffers);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOG_ERROR("Failed to create framebuffer for video!");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        // update size of render texture based on video resolution
        glBindTexture(GL_TEXTURE_2D, playerContext.frameTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, OFS_InternalTexFormat, playerContext.fbWidth, playerContext.fbHeight, 0, OFS_TexFormat, GL_UNSIGNED_BYTE, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}






void OFS::VideoPlayer::NextFrame() noexcept
{
    if (isPaused())
    {
        // use same method as previousFrame for consistency
        double relSeek = FrameTime() * 1.000001;
        pImpl->videoProperties.percentPosition += (relSeek / pImpl->videoProperties.duration);
        pImpl->videoProperties.percentPosition = Util::Clamp(pImpl->videoProperties.percentPosition, 0.0, 1.0);
        SetPositionPercent(pImpl->videoProperties.percentPosition, false);
    }
}

void OFS::VideoPlayer::PreviousFrame() noexcept
{
    if (isPaused()) {
        // this seeks much faster
        // https://github.com/mpv-player/mpv/issues/4019#issuecomment-358641908
        double relSeek = FrameTime() * 1.000001;
        pImpl->videoProperties.percentPosition -= (relSeek / pImpl->videoProperties.duration);
        pImpl->videoProperties.percentPosition = Util::Clamp(pImpl->videoProperties.percentPosition, 0.0, 1.0);
        SetPositionPercent(pImpl->videoProperties.percentPosition, false);
    }
}

void OFS::VideoPlayer::SetPositionPercent(float percentPosition, bool pausesVideo) noexcept
{
    // QQQ
    //logicalPosition = percentPosition;
    pImpl->videoProperties.percentPosition = percentPosition;
    
    auto str = std::format("{:.08f}", (float)(percentPosition * 100.0f));
    const char* cmd[]{ "seek", str.c_str(), "absolute-percent+exact", NULL };
    if (pausesVideo)
    {
        setPause(true);
    }
    mpv_command_async(pImpl->get(), 0, cmd);
}

void OFS::VideoPlayer::SetPositionExact(float timeSeconds, bool pausesVideo) noexcept
{
    // this updates logicalPosition in SetPositionPercent
    timeSeconds = Util::Clamp<float>(timeSeconds, 0.f, Duration());
    float relPos = ((float)timeSeconds) / Duration();
    SetPositionPercent(relPos, pausesVideo);
}

void OFS::VideoPlayer::SeekRelative(float timeSeconds) noexcept
{
    // this updates logicalPosition in SetPositionPercent
    auto seekTo = CurrentTime() + timeSeconds;
    seekTo = std::max(seekTo, 0.0);
    SetPositionExact(seekTo);
}

void OFS::VideoPlayer::SeekFrames(std::int32_t offset) noexcept
{
    // this updates logicalPosition in SetPositionPercent
    if (isPaused()) {
        float relSeek = (FrameTime() * 1.000001f) * offset;
        pImpl->videoProperties.percentPosition += (relSeek / pImpl->videoProperties.duration);
        pImpl->videoProperties.percentPosition = std::clamp(pImpl->videoProperties.percentPosition, 0.0, 1.0);
        SetPositionPercent(pImpl->videoProperties.percentPosition, false);
    }
}

void OFS::VideoPlayer::CycleSubtitles() noexcept
{
    const char* cmd[]{ "cycle", "sub", NULL};
    mpv_command_async(pImpl->get(), 0, cmd);
}

void OFS::VideoPlayer::SaveFrameToImage(const std::string& directory) noexcept
{
    auto dir = OFS::util::pathFromU8String(directory);
    dir.make_preferred();
    if (!OFS::util::createDirectories(dir))
        return;

    char timeStringBuffer[16];
    auto currentFile = pImpl->videoProperties.path.stem();

    double time = CurrentTime();
    auto const len = OFS::util::formatTime(timeStringBuffer, time, true);
    std::replace(timeStringBuffer, timeStringBuffer + len, ':', '_');
    currentFile += std::string_view(timeStringBuffer, len);
    currentFile += ".png";  // UB without file format extension

#if _WIN32
    std::u8string finalPath = (dir / currentFile).u8string();
    std::string finalPathStr = std::string(finalPath.begin(), finalPath.end());
#else
    std::string finalPathStr = (dir / currentFile).string();  // QQQ assuming this works on linux since mpv doesnt guarantee utf8 
#endif
    const char* cmd[]{ "screenshot-to-file", finalPathStr.c_str(), nullptr };
    mpv_command_async(pImpl->get(), 0, cmd);
}

// ==================== Getter ==================== 

std::uint16_t OFS::VideoPlayer::VideoWidth() const noexcept
{
    return pImpl->videoProperties.width;
}

std::uint16_t OFS::VideoPlayer::VideoHeight() const noexcept
{
    return pImpl->videoProperties.height;
}

float OFS::VideoPlayer::FrameTime() const noexcept
{
    return 1.f / pImpl->videoProperties.fps;
}

float OFS::VideoPlayer::CurrentSpeed() const noexcept
{
    return pImpl->videoProperties.playSpeed;
}

double OFS::VideoPlayer::Duration() const noexcept
{
    return pImpl->videoProperties.duration;
}

float OFS::VideoPlayer::CurrentPercentPosition() const noexcept
{
    return CurrentPlayerPosition();
    //return logicalPosition;
}

double OFS::VideoPlayer::CurrentTime() const noexcept
{
    // QQQ
    return pImpl->videoProperties.duration * pImpl->videoProperties.percentPosition;
    //if(pImpl->videoProperties.paused)
    //{
    //    return logicalPosition * pImpl->videoProperties.duration;
    //}
    //else 
    //{
    //    float timeSinceLastUpdate = (SDL_GetTicks() - CTX->smoothTimer) / 1000.f;
    //    float positionOffset = (timeSinceLastUpdate * pImpl->videoProperties.currentSpeed) / Duration();
    //    return (logicalPosition + positionOffset) * pImpl->videoProperties.duration;
    //}
}

double OFS::VideoPlayer::CurrentPlayerPosition() const noexcept
{
    return pImpl->videoProperties.percentPosition;
}
