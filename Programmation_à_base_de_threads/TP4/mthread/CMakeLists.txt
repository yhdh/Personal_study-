# cmake version
cmake_minimum_required(VERSION 3.9)
# define project name and language
project(
  libmthread
  VERSION 0.1
  DESCRIPTION "User level thread library")
include(GNUInstallDirs)
include(CTest)
include(CheckIncludeFiles)
include(CheckIncludeFileCXX)
include(CheckCXXCompilerFlag)
include(CheckCompilerFlag)
include(CheckCCompilerFlag)
include(CheckLinkerFlag)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


#check_include_file_cxx("stacktrace" HAVE_CXX_STACKTRACE)
if(HAVE_CXX_STACKTRACE)
  set(CMAKE_CXX_STANDARD 23)
  message("Enable C++ 23 stacktrace")
  add_compile_options()
  add_link_options(-lstdc++_libbacktrace)
endif()

add_compile_options(-Wall -Werror $<$<CONFIG:DEBUG>:-g>)

add_compile_definitions(MTHREAD_BUILD_TYPE="$<CONFIG>")

option(ENABLE_ADDRESS_SANITIZER "Enable Address Sanitizer if possible" OFF)

if(ENABLE_ADDRESS_SANITIZER)
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    macro(add_sanitizer flag)

      if(CMAKE_CXX_COMPILER_ID MATCHES "Clang"
         OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
         OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        message("Looking for -fsanitize=${flag}")
        set(CMAKE_REQUIRED_FLAGS "-Werror -fsanitize=${flag}")
        check_cxx_compiler_flag(-fsanitize=${flag} HAVE_FLAG_SANITIZER_${flag})
        if(HAVE_FLAG_SANITIZER_${flag})
          message("Adding -fsanitize=${flag}")
          add_compile_options(-fno-omit-frame-pointer -fsanitize=${flag})
          add_link_options(-fno-omit-frame-pointer -fsanitize=${flag})
        else()
          message("-fsanitize=${flag} unavailable")
        endif()
      endif()
    endmacro()

    add_sanitizer(address)

  endif()
endif()

if(ENABLE_GCOV)
  #check_cxx_compiler_flag("-fprofile-arcs -ftest-coverage -g"  HAVE_GCOVR)
  #if(HAVE_GCOVR)
    message("Enable GCOV")
    add_compile_options(-fprofile-arcs -ftest-coverage -g)
    add_link_options(--coverage -g)
  #endif()
  add_custom_target(
        gcovr-ci
        COMMAND gcovr -r ../src . --xml-pretty --exclude-unreachable-branches --print-summary -o coverage_report.xml)
  add_custom_target(
        gcovr
        COMMAND gcovr -r ../src . )
endif()

if(ENABLE_GPROF)
    message("Enable GPROF")
    add_compile_options(-pg)
    add_link_options(-pg)
endif()



# define directories
set(SRC ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE ${CMAKE_SOURCE_DIR}/include)
set(TESTS ${CMAKE_SOURCE_DIR}/tests)
set(EXAMPLES ${CMAKE_SOURCE_DIR}/examples)

add_subdirectory(${SRC})
add_subdirectory(${TESTS})
add_subdirectory(${EXAMPLES})

# define target properties
set_target_properties(
  libmthread
  PROPERTIES VERSION ${PROJECT_VERSION}
             SOVERSION 1
             PUBLIC_HEADER include/mthread.h)

# define install dir
install(
  TARGETS libmthread
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


  # additional target to perform clang-format run, requires clang-format

# get all project files
file(GLOB_RECURSE ALL_SOURCE_FILES  ${SRC}/*.cpp ${SRC}/*.h ${TESTS}/*.cpp ${TESTS}/*.h ${INCLUDE}/*.cpp ${INCLUDE}/*.h ${EXAMPLES}/*.cpp ${EXAMPLES}/*.h)
add_custom_target(
        clangformat
        COMMAND clang-format
        -i
        ${ALL_SOURCE_FILES}
)

file(GLOB_RECURSE ALL_SOURCE_FILES  ${SRC}/*.cpp ${SRC}/*.h ${TESTS}/*.cpp ${TESTS}/*.h ${INCLUDE}/*.cpp ${INCLUDE}/*.h ${EXAMPLES}/*.cpp ${EXAMPLES}/*.h)
add_custom_target(
        clangformat-error
        COMMAND clang-format
        --dry-run --Werror
        ${ALL_SOURCE_FILES}
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
file(GLOB_RECURSE ALL_SOURCE_FILES  ${SRC}/*.cpp ${SRC}/*.h ${TESTS}/*.cpp ${TESTS}/*.h ${INCLUDE}/*.cpp ${INCLUDE}/*.h ${EXAMPLES}/*.cpp ${EXAMPLES}/*.h)
add_custom_target(
        clangtidy-error
        COMMAND clang-tidy
        -p ../ -warnings-as-errors=* ${ALL_SOURCE_FILES}
)

