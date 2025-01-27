#include "OFS_Waveform.h"
#include "gl/OFS_GL.h"
#include "ui/OFS_ScriptTimeline.h"

#include "OFS_Util.h"
#include "OFS_Profiling.h"

#include <scn/scan.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_process.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_properties.h>

#include <span>
#include <vector>
#include <string>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string_view>


namespace
{
	void loadPCM(std::span<std::int16_t> rawAudio, std::vector<float>& samples, float& minSample, float& maxSample)
	{
		constexpr unsigned SamplesPerLine = 300;
		auto const sampleCount = static_cast<unsigned>(rawAudio.size());
		float avgSample = 0.f;

		for (unsigned sampleIdx = 0; sampleIdx < sampleCount; sampleIdx += SamplesPerLine)
		{
			auto const samplesInThisLine = std::min(SamplesPerLine, sampleCount - sampleIdx);

			for (unsigned n = 0; n < samplesInThisLine; ++n)
			{
				avgSample += std::abs(rawAudio[sampleIdx + n]) / 32768.f;
			}

			avgSample /= (float)SamplesPerLine;
			samples.push_back(avgSample);
			minSample = std::min(minSample, avgSample);
			maxSample = std::max(maxSample, avgSample);
			avgSample = 0.f;
		}
	}
}

bool OFS_Waveform::GenerateAndLoadFlac(std::filesystem::path const& ffmpegPath, std::filesystem::path const& videoPath, const std::string& output) noexcept
{
	auto const props = SDL_CreateProperties();

	std::u8string videoU8StringPath  = videoPath.u8string();
	std::string   videoStringPath(videoU8StringPath.begin(), videoU8StringPath.end());

	std::u8string ffmpegU8StringPath = ffmpegPath.u8string();
	std::string   ffmpegStringPath(ffmpegU8StringPath.begin(), ffmpegU8StringPath.end());

	char const* ffmpegArgs[] = {
		ffmpegStringPath.c_str(), "-hide_banner", "-nostats", "-nostdin",
		"-i", videoStringPath.c_str(), "-vn",
		"-ac", "1",
		"-f", "s16le",
		"-c:a", "pcm_s16le",
		"pipe:1",
		nullptr
	};

	SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ARGS_POINTER, ffmpegArgs);
	SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDOUT_NUMBER, SDL_PROCESS_STDIO_APP);
	SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDERR_NUMBER, SDL_PROCESS_STDIO_APP);
	SDL_SetBooleanProperty(props, SDL_PROP_PROCESS_CREATE_BACKGROUND_BOOLEAN, true);

	if (auto const ffmpegProcess = SDL_CreateProcessWithProperties(props); ffmpegProcess)
	{
		generating = true;

		auto const processProps = SDL_GetProcessProperties(ffmpegProcess);
		auto const pipeStdout   = (SDL_IOStream*) SDL_GetPointerProperty(processProps, SDL_PROP_PROCESS_STDOUT_POINTER, nullptr);
		auto const pipeStderr   = (SDL_IOStream*) SDL_GetPointerProperty(processProps, SDL_PROP_PROCESS_STDERR_POINTER, nullptr);

		std::size_t outMessageSize{};
		std::vector<std::int16_t> audioData(48000, 0);
		std::vector<float> audioSamples{};

		float minSample{}, maxSample{};
		std::size_t readSize {};
		for (;;)
		{
			if (auto const read = SDL_ReadIO(pipeStdout, audioData.data(), audioData.size() - readSize); read)
			{
				readSize += read;
				if (readSize == audioData.size())
				{
					loadPCM(audioData, audioSamples, minSample, maxSample);
					readSize = 0;
				}
			}
			else if (SDL_GetIOStatus(pipeStdout) == SDL_IO_STATUS_NOT_READY)
			{
				SDL_DelayNS(100);
			}
			else
				break;
		}

		if (readSize)
		{
			loadPCM(std::span(audioData.data(), readSize), audioSamples, minSample, maxSample);
		}
		auto outLogs = SDL_LoadFile_IO(pipeStderr, &outMessageSize, false);

		SDL_WaitProcess(ffmpegProcess, true, nullptr);
		SDL_DestroyProcess(ffmpegProcess);

		if (std::abs(minSample) > std::abs(maxSample)) 
			maxSample = std::abs(minSample);
		else
			minSample = -maxSample;

		for (auto& sample : audioSamples)
		{
			sample = ((sample - minSample) / (maxSample - minSample)) * 2.f - 1.f;
		}

		if (outMessageSize)
		{
			std::string_view metadata((char const*)outLogs, outMessageSize);
			metadata = metadata.substr(metadata.find("Output #0"));
			metadata = metadata.substr(metadata.find("Stream #0:0(und): Audio: pcm_s16le,"));

			if (auto result = scn::scan<unsigned>(metadata, "Stream #0:0(und): Audio: pcm_s16le, {} Hz"))
			{
				//unsigned samplingRate = result->value();
			}
		}

		SDL_free(outLogs);
	}

	SDL_DestroyProperties(props);
	generating = false;
	return true;
}

void OFS_WaveformLOD::Init() noexcept
{
	glGenTextures(1, &WaveformTex);
	glBindTexture(GL_TEXTURE_2D, WaveformTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	WaveShader = std::make_unique<WaveformShader>();
}

void OFS_WaveformLOD::Update(const OverlayDrawingCtx& ctx) noexcept
{
	OFS_PROFILE(__FUNCTION__);
	const float relStart = ctx.offsetTime / ctx.totalDuration;
	const float relDuration = ctx.visibleTime / ctx.totalDuration;
	
	const auto& samples = data.Samples();
	const float totalSampleCount = samples.size();

	float startIndexF = relStart * totalSampleCount;
	float endIndexF = (relStart* totalSampleCount) + (totalSampleCount * relDuration);

	float visibleSampleCountF = endIndexF - startIndexF;

	const float desiredSamples = ctx.canvasSize.x/3.f;
	const float everyNth = SDL_ceilf(visibleSampleCountF / desiredSamples);

	auto& lineBuf = WaveformLineBuffer;		
	if((int32_t)lastMultiple != (int32_t)(startIndexF / everyNth)) {
		int32_t scrollBy = (startIndexF/everyNth) - lastMultiple;

		if(lastVisibleDuration == ctx.visibleTime
		&& lastCanvasX == ctx.canvasSize.x
		&& scrollBy > 0 && scrollBy < lineBuf.size()) {
			OFS_PROFILE("WaveformScrolling");
			std::memcpy(lineBuf.data(), lineBuf.data() + scrollBy, sizeof(float) * (lineBuf.size() - scrollBy));
			lineBuf.resize(lineBuf.size() - scrollBy);
			
			int addedCount = 0;
			float maxSample;
			for(int32_t i = endIndexF - (everyNth*scrollBy); i <= endIndexF; i += everyNth) {
				maxSample = 0.f;
				for(int32_t j=0; j < everyNth; j += 1) {
					int32_t currentIndex = i + j;
					if(currentIndex >= 0 && currentIndex < totalSampleCount) {
						float s = std::abs(samples[currentIndex]);
						maxSample = Util::Max(maxSample, s);
					}
				}
				lineBuf.emplace_back(maxSample);
				addedCount += 1; 
				if(addedCount == scrollBy) break;
			}
			assert(addedCount == scrollBy);
		} else if(scrollBy != 0) {
			OFS_PROFILE("WaveformUpdate");
			lineBuf.clear();
			float maxSample;
			for(int32_t i = startIndexF; i <= endIndexF; i += everyNth) {
				maxSample = 0.f;
				for(int32_t j=0; j < everyNth; j += 1) {
					int32_t currentIndex = i + j;
					if(currentIndex >= 0 && currentIndex < totalSampleCount) {
						float s = std::abs(samples[currentIndex]);
						maxSample = Util::Max(maxSample, s);
					}
				}
				lineBuf.emplace_back(maxSample);
			}
		}

		
		lastMultiple = SDL_floorf(startIndexF / everyNth);
		lastCanvasX = ctx.canvasSize.x;
		lastVisibleDuration = ctx.visibleTime;
		Upload();
	}

	samplingOffset = (1.f / lineBuf.size()) * ((startIndexF/everyNth) - lastMultiple);

#if 0
	ImGui::Begin("Waveform Debug");
	ImGui::Text("Audio samples: %lld", lineBuf.size());
	ImGui::Text("Expected samples: %f", (endIndexF - startIndexF)/everyNth);
	ImGui::Text("Samples in view: %f", (endIndexF - startIndexF));
	ImGui::Text("Start: %f", startIndexF);
	ImGui::Text("End: %f", endIndexF);
	ImGui::Text("Every nth: %f", everyNth);
	ImGui::Text("Last multiple: %f", lastMultiple);
	ImGui::SliderFloat("Offset", &samplingOffset, 0.f, 1.f/lineBuf.size(), "%f");
	ImGui::End();
#endif
}

void OFS_WaveformLOD::Upload() noexcept
{
	OFS_PROFILE(__FUNCTION__);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, WaveformTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, WaveformLineBuffer.size(), 1, 0, GL_RED, GL_FLOAT, WaveformLineBuffer.data());
}
