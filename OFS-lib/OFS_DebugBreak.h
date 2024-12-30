#pragma once

#ifndef NDEBUG
#include <version>

#if defined(__cpp_lib_debugging) 
#   include <debugging>
#   define OFS_DEBUGBREAK std::breakpoint_if_debugging()
#elif defined(__has_builtin)
#   if  __has_builtin(__builtin_debugtrap)
#       define OFS_DEBUGBREAK __builtin_debugtrap()
#   elif __has_builtin(__debugbreak)
#       define OFS_DEBUGBREAK __debugbreak()
#   endif
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
#       define OFS_DEBUGBREAK __debugbreak()
#elif defined(__x86_64__)
#       define OFS_DEBUGBREAK { __asm__ __volatile__("int3"); }
#elif __has_include(<signal.h>)
#   include <signal.h>
#   if defined(SIGTRAP)
#       define OFS_DEBUGBREAK raise(SIGTRAP)
#   else
#       // give up
#       define OFS_DEBUGBREAK void(0)
#   endif
#else
#   // give up
#   define OFS_DEBUGBREAK void(0)
#endif

#else
#define OFS_DEBUGBREAK void(0)
#endif
