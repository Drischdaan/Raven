// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

// []===================================================================[]
// Available platform definitions:
// - RAVEN_PLATFORM_LINUX
// - RAVEN_PLATFORM_WINDOWS
// - RAVEN_PLATFORM_NAME
// - RAVEN_PLATFORM_NAME_RAW
// - RAVEN_PLATFORM_X64
// - RAVEN_PLATFORM_X32
// - RAVEN_POINTER_SIZE
// - RAVEN_PLATFORM_POINTER_ALIGNMENT
// []===================================================================[]

#if defined(__linux) || defined(__linux__)
#	define RAVEN_PLATFORM_LINUX 1
#	define RAVEN_PLATFORM_NAME "Linux"
#	define RAVEN_PLATFORM_NAME_RAW Linux
#	ifdef __x86_64__
#		define RAVEN_PLATFORM_X64 1
#		define RAVEN_PLATFORM_X32 0
#	else
#		define RAVEN_PLATFORM_X64 0
#		define RAVEN_PLATFORM_X32 1
#	endif
#elif defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#	define RAVEN_PLATFORM_WINDOWS 1
#	define RAVEN_PLATFORM_NAME "Windows"
#	define RAVEN_PLATFORM_NAME_RAW Windows
#	ifdef _WIN64
#		define RAVEN_PLATFORM_X64 1
#		define RAVEN_PLATFORM_X32 0
#	else
#		define RAVEN_PLATFORM_X64 0
#		define RAVEN_PLATFORM_X32 1
#	endif
#endif

#ifndef RAVEN_PLATFORM_LINUX
#	define RAVEN_PLATFORM_LINUX 0
#endif

#ifndef RAVEN_PLATFORM_WINDOWS
#	define RAVEN_PLATFORM_WINDOWS 0
#endif

#ifdef __WORDSIZE
#	define RAVEN_POINTER_SIZE (__WORDSIZE / 8)
#elif RAVEN_PLATFORM_X64
#	define RAVEN_POINTER_SIZE 8
#else
#	define RAVEN_POINTER_SIZE 4
#endif

#define RAVEN_PLATFORM_POINTER_ALIGNMENT (RAVEN_POINTER_SIZE * 2)
