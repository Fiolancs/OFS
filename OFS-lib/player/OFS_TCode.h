#pragma once
#include "c_serial.h"
#include <cstdint>

#include "OFS_TCodeProducer.h"
#include "OFS_Util.h"
#include "FunscriptAction.h"
#include "OFS_im3d.h"


class TCodePlayer {
	std::string loadPath;
public:
	int current_port = 0;
	int port_count = 0;
	const char** port_list = nullptr;

	int status = -1;

	int32_t tickrate = 60;
	int32_t delay = 0;


	bool openPort(const char* name) noexcept;
	c_serial_port_t* port = nullptr;
	c_serial_control_lines_t lines;

	TCodeChannels tcode;
	TCodeProducer prod;

	TCodePlayer();
	~TCodePlayer();
	
	void loadSettings(const std::string& path) noexcept;
	void save() noexcept;

	void DrawWindow(bool* open) noexcept;

	void play(float currentTimeMs, 
		std::weak_ptr<Funscript>&& L0, // everything except L0 is optional
		std::weak_ptr<Funscript>&& R0 = std::weak_ptr<Funscript>(),
		std::weak_ptr<Funscript>&& R1 = std::weak_ptr<Funscript>(),
		std::weak_ptr<Funscript>&& R2 = std::weak_ptr<Funscript>()
		/* TODO: add more channels*/
	) noexcept;
	void stop() noexcept;
	void sync(float currentTimeMs, float speed) noexcept;

	inline bool IgnoreChannel(TChannel c) const noexcept {
		switch (c) {
			// handled channels
			case TChannel::L0:
			case TChannel::R0:
			case TChannel::R1:
			case TChannel::R2:
			{
				return false;
			}

			// ignored channels
			case TChannel::L2:
			case TChannel::L1:
			case TChannel::V0:
			case TChannel::V1:
			case TChannel::V2:
			{
				return true;
			}
		}

		return true;
	}

	template <class Archive>
	inline void reflect(Archive& ar) {
		OFS_REFLECT(tcode, ar);
		OFS_REFLECT(tickrate, ar);
		OFS_REFLECT(delay, ar);
		OFS_REFLECT_NAMED("easingMode", TCodeChannel::EasingMode, ar);
	}
};