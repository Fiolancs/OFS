#pragma once
#include "state/OFS_StateManager.h"

#include <any>
#include <string>
struct OpenFunscripterState;
struct ProjectState;
struct BaseOverlayState;
struct ChapterState;
struct SimulatorState;
struct SimulatorDefaultConfigState;
struct VideoPlayerWindowState;
struct WaveformState;

struct OFS_ActionTrigger;
struct OFS_KeybindingState;


#ifdef OFS_SERIALIZATION_HELPER_CPP_INCLUDE
#define OFS_SERIALIZATION_EXTERN
#else
#define OFS_SERIALIZATION_EXTERN extern

OFS_SERIALIZATION_EXTERN template void OFS::deserializeState<::ProjectState>(std::any& state, std::string& json);

OFS_SERIALIZATION_EXTERN template void OFS::deserializeState<WaveformState>(std::any& state, std::string& json);
#endif

OFS_SERIALIZATION_EXTERN template void OFS::serializeState<::ProjectState>(std::any& state, std::string& json);

OFS_SERIALIZATION_EXTERN template void OFS::serializeState<WaveformState>(std::any& state, std::string& json);

OFS_SERIALIZATION_EXTERN template void OFS::serializeState<OpenFunscripterState>(std::any& state, std::string& json);
OFS_SERIALIZATION_EXTERN template void OFS::deserializeState<OpenFunscripterState>(std::any& state, std::string& json);

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
