file(GLOB_RECURSE RAVEN_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/*.cpp")

file(GLOB_RECURSE RAVEN_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Public/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Private/*.hpp")

if(NOT RAVEN_SOURCES)
  message(STATUS "Raven: no sources under Private/ yet; skipping the Raven target.")
  return()
endif()

add_library(Raven ${RAVEN_SOURCES} ${RAVEN_HEADERS})
add_library(Raven::Engine ALIAS Raven)

target_include_directories(Raven
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Public"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Private")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${RAVEN_SOURCES} ${RAVEN_HEADERS})

target_link_libraries(Raven
  PRIVATE
    Raven::ProjectOptions
    Raven::ProjectWarnings)
