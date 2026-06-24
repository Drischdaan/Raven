include_guard(GLOBAL)

find_program(RAVEN_SLANGC_EXECUTABLE
  NAMES slangc
  HINTS
    ENV VULKAN_SDK
    ENV SLANG_SDK
  PATH_SUFFIXES bin
  DOC "Path to the Slang compiler (slangc).")

function(raven_compile_shaders)
  set(options "")
  set(oneValueArgs TARGET OUTPUT_DIR TARGET_FORMAT PROFILE STAGE)
  set(multiValueArgs SHADERS)
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT ARG_TARGET)
    message(FATAL_ERROR "raven_compile_shaders: TARGET is required.")
  endif()
  if(NOT ARG_SHADERS)
    message(FATAL_ERROR "raven_compile_shaders: SHADERS is required.")
  endif()
  if(NOT RAVEN_SLANGC_EXECUTABLE)
    message(FATAL_ERROR
      "raven_compile_shaders: slangc not found. Install the Slang/Vulkan SDK or set "
      "RAVEN_SLANGC_EXECUTABLE to its path.")
  endif()

  if(NOT ARG_OUTPUT_DIR)
    set(ARG_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/Shaders")
  endif()
  if(NOT ARG_TARGET_FORMAT)
    set(ARG_TARGET_FORMAT "spirv")
  endif()

  file(MAKE_DIRECTORY "${ARG_OUTPUT_DIR}")

  set(compiled_outputs "")
  foreach(shader IN LISTS ARG_SHADERS)
    get_filename_component(shader_abs "${shader}" ABSOLUTE)
    get_filename_component(shader_name "${shader}" NAME)
    set(output "${ARG_OUTPUT_DIR}/${shader_name}.spv")

    set(slang_flags -target "${ARG_TARGET_FORMAT}")
    if(ARG_PROFILE)
      list(APPEND slang_flags -profile "${ARG_PROFILE}")
    endif()
    if(ARG_STAGE)
      list(APPEND slang_flags -stage "${ARG_STAGE}")
    endif()

    add_custom_command(
      OUTPUT "${output}"
      COMMAND "${RAVEN_SLANGC_EXECUTABLE}" "${shader_abs}" ${slang_flags} -o "${output}"
      DEPENDS "${shader_abs}"
      COMMENT "Compiling shader ${shader_name} -> ${output}"
      VERBATIM)
    list(APPEND compiled_outputs "${output}")
  endforeach()

  add_custom_target(${ARG_TARGET} DEPENDS ${compiled_outputs})
endfunction()
