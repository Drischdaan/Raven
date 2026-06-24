include_guard(GLOBAL)

function(raven_set_project_warnings target)
  set(options "")
  set(oneValueArgs WARNINGS_AS_ERRORS)
  set(multiValueArgs "")
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(MSVC_WARNINGS
    /W4            # baseline reasonable warnings
    /permissive-   # standards conformance
    /w14242        # conversion, possible loss of data
    /w14254        # conversion, possible loss of data
    /w14263        # member does not override any base class virtual member
    /w14265        # class has virtual functions but non-virtual destructor
    /w14287        # unsigned/negative constant mismatch
    /we4289        # loop control variable used outside the loop
    /w14296        # expression is always 'boolean_value'
    /w14311        # pointer truncation
    /w14545        # expression before comma evaluates to a missing argument
    /w14546        # function call before comma missing argument list
    /w14547        # operator before comma has no effect
    /w14549        # operator before comma has no effect
    /w14555        # expression has no effect
    /w14619        # pragma warning: unknown warning number
    /w14640        # thread-unsafe static member initialization
    /w14826        # conversion is sign-extended
    /w14905        # wide string literal cast to 'LPSTR'
    /w14906        # string literal cast to 'LPWSTR'
    /w14928        # illegal copy-initialization
  )

  set(CLANG_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wdouble-promotion
    -Wformat=2
    -Wimplicit-fallthrough
  )

  set(GCC_WARNINGS
    ${CLANG_WARNINGS}
    -Wmisleading-indentation
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wuseless-cast
  )

  if(ARG_WARNINGS_AS_ERRORS)
    list(APPEND MSVC_WARNINGS /WX)
    list(APPEND CLANG_WARNINGS -Werror)
    list(APPEND GCC_WARNINGS -Werror)
  endif()

  if(MSVC)
    set(project_warnings ${MSVC_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
    set(project_warnings ${CLANG_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(project_warnings ${GCC_WARNINGS})
  else()
    message(AUTHOR_WARNING
      "raven_set_project_warnings: no warning set for compiler '${CMAKE_CXX_COMPILER_ID}'.")
    set(project_warnings "")
  endif()

  target_compile_options(${target} INTERFACE ${project_warnings})
endfunction()
