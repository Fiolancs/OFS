#pragma once
#include "state/OFS_StateManager.h"

#include <any>
#include <string>

#ifdef OFS_SERIALIZATION_HELPER_CPP_INCLUDE
#define OFS_SERIALIZATION_EXTERN
#else
#define OFS_SERIALIZATION_EXTERN extern
#endif

struct BaseOverlayState;
struct ChapterState;
struct SimulatorState;
struct SimulatorDefaultConfigState;
struct VideoPlayerWindowState;

struct OFS_ActionTrigger;
struct OFS_KeybindingState;

OFS_SERIALIZATION_EXTERN template void OFS::serializeState<BaseOverlayState>(std::any& state, std::string& json);
OFS_SERIALIZATION_EXTERN template void OFS::deserializeState<BaseOverlayState>(std::any& state, std::string& json);

OFS_SERIALIZATION_EXTERN template void OFS::serializeState<ChapterState>(std::any& state, std::string& json);
OFS_SERIALIZATION_EXTERN template void OFS::deserializeState<ChapterState>(std::any& state, std::string& json);

OFS_SERIALIZATION_EXTERN template void OFS::serializeState<SimulatorState>(std::any& state, std::string& json);
OFS_SERIALIZATION_EXTERN template void OFS::deserializeState<SimulatorState>(std::any& state, std::string& json);

OFS_SERIALIZATION_EXTERN template void OFS::serializeState<SimulatorDefaultConfigState>(std::any& state, std::string& json);
OFS_SERIALIZATION_EXTERN template void OFS::deserializeState<SimulatorDefaultConfigState>(std::any& state, std::string& json);

OFS_SERIALIZATION_EXTERN template void OFS::serializeState<VideoPlayerWindowState>(std::any& state, std::string& json);
OFS_SERIALIZATION_EXTERN template void OFS::deserializeState<VideoPlayerWindowState>(std::any& state, std::string& json);

OFS_SERIALIZATION_EXTERN template void OFS::serializeState<OFS_ActionTrigger>(std::any& state, std::string& json);
OFS_SERIALIZATION_EXTERN template void OFS::deserializeState<OFS_ActionTrigger>(std::any& state, std::string& json);

OFS_SERIALIZATION_EXTERN template void OFS::serializeState<OFS_KeybindingState>(std::any& state, std::string& json);
OFS_SERIALIZATION_EXTERN template void OFS::deserializeState<OFS_KeybindingState>(std::any& state, std::string& json);

#undef OFS_SERIALIZATION_EXTERN
