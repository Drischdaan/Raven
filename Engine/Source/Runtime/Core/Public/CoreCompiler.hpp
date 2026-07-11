// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#pragma region Compiler
#   if defined(__clang__) && !defined(_MSC_VER)
#       define RAVEN_COMPILER "Clang"
#       define RAVEN_COMPILER_CLANG 1
#       define RAVEN_COMPILER_VERSION_MAJOR __clang_major__
#       define RAVEN_COMPILER_VERSION_MINOR __clang_minor__
#       define RAVEN_COMPILER_VERSION_PATCH __clang_patchlevel__
#   elifdef __GNUC__
#       define RAVEN_COMPILER "GCC"
#       define RAVEN_COMPILER_GCC 1
#       define RAVEN_COMPILER_VERSION_MAJOR __GNUC__
#       define RAVEN_COMPILER_VERSION_MINOR __GNUC_MINOR__
#       define RAVEN_COMPILER_VERSION_PATCH __GNUC_PATCHLEVEL__
#   elifdef _MSC_VER
#       define RAVEN_COMPILER "MSVC"
#       define RAVEN_COMPILER_MSVC 1
#       define RAVEN_COMPILER_VERSION_MAJOR _MSC_VER
#       define RAVEN_COMPILER_VERSION_MINOR 0
#       define RAVEN_COMPILER_VERSION_PATCH 0
#   else
#       define RAVEN_COMPILER "UNKNOWN"
#       define RAVEN_COMPILER_UNKNOWN 1
#       define RAVEN_COMPILER_VERSION_MAJOR 0
#       define RAVEN_COMPILER_VERSION_MINOR 0
#       define RAVEN_COMPILER_VERSION_PATCH 0
#   endif

#	ifndef RAVEN_COMPILER_CLANG
#		define RAVEN_COMPILER_CLANG 0
#	endif

#	ifndef RAVEN_COMPILER_GCC
#		define RAVEN_COMPILER_GCC 0
#	endif

#	ifndef RAVEN_COMPILER_MSVC
#		define RAVEN_COMPILER_MSVC 0
#	endif

#	ifndef RAVEN_COMPILER_UNKNOWN
#		define RAVEN_COMPILER_UNKNOWN 0
#	endif

#	if RAVEN_COMPILER_UNKNOWN
#		error "Unknown compiler"
#	endif
#pragma endregion

#pragma region CPP
#	define RAVEN_CPP_VERSION11 201103L
#	define RAVEN_CPP_VERSION14 201402L
#	define RAVEN_CPP_VERSION17 201703L
#	define RAVEN_CPP_VERSION20 202002L
#	define RAVEN_CPP_VERSION23 202302L

#	if RAVEN_COMPILER_MSVC
#		define RAVEN_CPP_VERSION _MSVC_LANG
#	elifdef __cplusplus
#		define RAVEN_CPP_VERSION __cplusplus
#	else
#		define RAVEN_CPP_VERSION 0
#	endif

#	if !RAVEN_CPP_VERSION
#		error "A c++ compatible compiler is required"
#	endif

#   ifndef RAVEN_REQUIRED_CPP_VERSION
#	    define RAVEN_REQUIRED_CPP_VERSION RAVEN_CPP_VERSION23
#   endif

#	ifdef RAVEN_REQUIRED_CPP_VERSION
#		if RAVEN_CPP_VERSION < RAVEN_REQUIRED_CPP_VERSION
#			error "C++ version not supported"
#		endif
#	endif
#pragma endregion // CPP
