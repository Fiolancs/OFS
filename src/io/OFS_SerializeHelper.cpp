#define  OFS_SERIALIZATION_HELPER_CPP_INCLUDE
#include "state/states/KeybindingState.h"

#include <glaze/glaze.hpp>



template <>
struct glz::meta<OFS_ActionTrigger>
{
	static constexpr auto value = glz::object(&OFS_ActionTrigger::Mod, &OFS_ActionTrigger::Key, &OFS_ActionTrigger::ShouldRepeat, &OFS_ActionTrigger::MappedActionId);
};

