#define  OFS_SERIALIZATION_HELPER_CPP_INCLUDE
#include "OFS_SerializeHelper.h"
#include "OFS_VectorSet.h"

#include "state/states/BaseOverlayState.h"
#include "state/states/ChapterState.h"
#include "state/states/KeybindingState.h"
#include "state/states/VideoplayerWindowState.h"

#include "state/SimulatorState.h"

#include <glaze/glaze.hpp>
#include <imgui.h>

#include <vector>


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

