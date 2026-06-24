include_guard(GLOBAL)

include(CompilerWarnings)
include(Sanitizers)

function(raven_setup_options)
  option(RAVEN_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
  option(RAVEN_ENABLE_SANITIZER_ADDRESS "Enable the address sanitizer" OFF)
  option(RAVEN_ENABLE_SANITIZER_UNDEFINED "Enable the undefined behaviour sanitizer" OFF)
  option(RAVEN_ENABLE_SANITIZER_THREAD "Enable the thread sanitizer" OFF)
  option(RAVEN_ENABLE_SANITIZER_LEAK "Enable the leak sanitizer" OFF)
  option(RAVEN_ENABLE_IPO "Enable interprocedural optimization (LTO)" OFF)
  option(RAVEN_BUILD_TESTS "Build the Raven test suite" OFF)

  set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

  if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR
      "In-source builds are not allowed. Configure into a separate directory, "
      "e.g. `cmake --preset default` (writes to Build/).")
  endif()

  if(RAVEN_ENABLE_IPO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT raven_ipo_supported OUTPUT raven_ipo_output)
    if(raven_ipo_supported)
      set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON PARENT_SCOPE)
    else()
      message(WARNING "RAVEN_ENABLE_IPO is ON but IPO is not supported: ${raven_ipo_output}")
    endif()
  endif()

  add_library(raven_project_options INTERFACE)
  add_library(Raven::ProjectOptions ALIAS raven_project_options)
  target_compile_features(raven_project_options INTERFACE cxx_std_20)

  raven_enable_sanitizers(raven_project_options
    "${RAVEN_ENABLE_SANITIZER_ADDRESS}"
    "${RAVEN_ENABLE_SANITIZER_UNDEFINED}"
    "${RAVEN_ENABLE_SANITIZER_THREAD}"
    "${RAVEN_ENABLE_SANITIZER_LEAK}")

  add_library(raven_project_warnings INTERFACE)
  add_library(Raven::ProjectWarnings ALIAS raven_project_warnings)
  raven_set_project_warnings(raven_project_warnings
    WARNINGS_AS_ERRORS "${RAVEN_WARNINGS_AS_ERRORS}")
endfunction()
