#pragma once
#include "OFS_VectorSet.h"
#include "state/OFS_StateManager.h"

#include <imgui.h>
#include <glaze/glaze.hpp>

#include <any>
#include <string>


// QQQ 
// for now just dump it all here while we collect what we need to serialize...
// then we'll move all that we can to the .cpp to improve compile times

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


#ifdef OFS_SERIALIZATION_HELPER_CPP_INCLUDE
#define OFS_SERIALIZATION_EXTERN
#else
#define OFS_SERIALIZATION_EXTERN extern
#endif

struct OFS_ActionTrigger;
struct OFS_KeybindingState;

OFS_SERIALIZATION_EXTERN template void OFS::serializeState<OFS_ActionTrigger>(std::any& state, std::string& json);
OFS_SERIALIZATION_EXTERN template void OFS::deserializeState<OFS_ActionTrigger>(std::any& state, std::string& json);

OFS_SERIALIZATION_EXTERN template void OFS::serializeState<OFS_KeybindingState>(std::any& state, std::string& json);
OFS_SERIALIZATION_EXTERN template void OFS::deserializeState<OFS_KeybindingState>(std::any& state, std::string& json);

#undef OFS_SERIALIZATION_EXTERN
