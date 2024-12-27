#pragma once

#include "tracy/Tracy.hpp"

#if OFS_PROFILE_ENABLED == 1
#define OFS_PROFILE(name)    ZoneScopedN(name)
#define OFS_BEGINPROFILING() 
#define OFS_ENDPROFILING()   FrameMark
#else
#define OFS_PROFILE(name)
#define OFS_BEGINPROFILING()
#define OFS_ENDPROFILING()
#endif

