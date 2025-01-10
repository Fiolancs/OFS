#define  OFS_SERIALIZATION_HELPER_CPP_INCLUDE
#include "OFS_SerializeHelper.h"
#include "OFS_VectorSet.h"

#include "state/states/BaseOverlayState.h"
#include "state/states/ChapterState.h"
#include "state/states/KeybindingState.h"
#include "state/states/VideoplayerWindowState.h"
#include "state/states/WaveformState.h"

#include "state/SimulatorState.h"
#include "state/OpenFunscripterState.h"
#include "state/ProjectState.h"

#include <glaze/glaze.hpp>
#include <imgui.h>

#include <vector>
#include <variant>
#include <utility>


// std::filesystem::path to always read and write u8strings
template <>
struct glz::detail::from<glz::JSON, std::filesystem::path>
{
	template <auto Opts>
	static void op(std::filesystem::path& value, auto&& ... args)
	{
		std::string str{};
		read<JSON>::op<Opts>(str, args...);
		value = OFS::util::pathFromU8String(str);
	}
};
template <>
struct glz::detail::to<glz::JSON, std::filesystem::path>
{
	template <auto Opts>
	static void op(std::filesystem::path& value, auto&& ... args)
	{
		auto u8 = value.u8string();
		auto str = std::string(u8.begin(), u8.end());
		write<JSON>::op<Opts>(str, args...);
	}
};


template<>
struct glz::meta<ImVec2>
{
	static constexpr auto value = glz::object(&ImVec2::x, &ImVec2::y);
};

template<>
struct glz::meta<ImVec4>
{
	static constexpr auto value = glz::object(&ImVec4::x, &ImVec4::y, &ImVec4::z, &ImVec4::w);
};

template<>
struct glz::meta<ImColor>
{
	static constexpr auto value = &ImColor::Value;
};

template <typename T>
struct glz::meta<vector_set<T>>
{
	static constexpr auto value = [](auto& self) -> auto& { return static_cast<std::vector<T>&>(self); };
};

template <>
struct glz::meta<OFS_ActionTrigger>
{
	static constexpr auto value = glz::object(&OFS_ActionTrigger::Mod, &OFS_ActionTrigger::Key, &OFS_ActionTrigger::ShouldRepeat, &OFS_ActionTrigger::MappedActionId);
};

template <>
struct glz::meta<::ProjectState>
{
	static constexpr auto value = glz::object( 
		&ProjectState::metadata
		, &ProjectState::relativeMediaPath
		, &ProjectState::activeTimer
		, &ProjectState::lastPlayerPosition
		, &ProjectState::activeScriptIdx
		, &ProjectState::nudgeMetadata
	);
};

template <>
struct glz::meta<WaveformState>
{
	static constexpr auto value = glz::object(
		&WaveformState::Filename
		, &WaveformState::UncompressedSize
	);
};

template <>
void OFS::deserializeState<::ProjectState>(std::any& state, std::string& json)
{
	if (nullptr == std::any_cast<::ProjectState>(std::addressof(state))) [[unlikely]]
	{
		FUN_ASSERT(false, "State serialization failed. deserialize function state type mismatch.");
		state.emplace<::ProjectState>();
		return;
	}

	auto& value = state.emplace<::ProjectState>();
	std::map<std::string_view, glz::raw_json_view> proxy{};
	if (auto const err = glz::read_json(proxy, json); !err)
	{
		if (auto stateJson = proxy.find("State"); stateJson != proxy.end())
		{
			// backwards compatiblity with older versions where the json looks like this 
			// "binaryFunscriptData":{"bytes":[],"subtype":null}

			std::variant<std::vector<std::uint8_t>, std::map<std::string_view, glz::raw_json_view>> binaryData;
			if (auto const err = glz::read_jmespath<"binaryFunscriptData">(binaryData, stateJson->second.str); err) [[unlikely]]
			{
				FUN_ASSERT(false, "State deserialization failed.");
				return;
			}

			if (auto const err = glz::read<glz::opts{ .error_on_unknown_keys = false }>(value, stateJson->second.str); err) [[unlikely]]
			{
				FUN_ASSERT(false, "State deserialization failed.");
				return;
			}

			std::visit([&value] (auto& variant) 
				{
					if constexpr (std::is_same_v<std::vector<std::uint8_t>, std::remove_cvref_t<decltype(variant)>>)
						value.binaryFunscriptData = std::move(variant);
					else
						auto _ = glz::read_json(value.binaryFunscriptData, variant["bytes"].str);
				}, 
				binaryData);
		}
	}
}

template <>
void OFS::deserializeState<WaveformState>(std::any& state, std::string& json)
{
	if (nullptr == std::any_cast<WaveformState>(std::addressof(state))) [[unlikely]]
	{
		FUN_ASSERT(false, "State serialization failed. deserialize function state type mismatch.");
		state.emplace<::WaveformState>();
		return;
	}

	auto& value = state.emplace<WaveformState>();
	std::map<std::string_view, glz::raw_json_view> proxy{};
	if (auto const err = glz::read_json(proxy, json); !err)
	{
		if (auto stateJson = proxy.find("State"); stateJson != proxy.end())
		{
			// backwards compatiblity with older versions where the json looks like this 
			// "BinSamples":{"bytes":[],"subtype":null}

			std::variant<std::vector<std::uint8_t>, std::map<std::string_view, glz::raw_json_view>> binaryData;
			if (auto const err = glz::read_jmespath<"BinSamples">(binaryData, stateJson->second.str); err) [[unlikely]]
			{
				FUN_ASSERT(false, "State deserialization failed.");
				return;
			}

			if (auto const err = glz::read<glz::opts{ .error_on_unknown_keys = false }>(value, stateJson->second.str); err) [[unlikely]]
			{
				FUN_ASSERT(false, "State deserialization failed.");
				return;
			}

			std::visit([&value] (auto& variant) 
				{
					if constexpr (std::is_same_v<std::vector<std::uint8_t>, std::remove_cvref_t<decltype(variant)>>)
						value.BinSamples = std::move(variant);
					else
						auto _ = glz::read_json(value.BinSamples, variant["bytes"].str);
				}, 
				binaryData);
		}
	}
}
