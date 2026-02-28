cmake_minimum_required(VERSION 3.9)

check_include_files("valgrind/valgrind.h" MTHREAD_USE_VALGRIND)
check_include_file_cxx("cxxabi.h" MTHREAD_HAVE_CXX_ABI)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mthread_config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/mthread_config.h)


add_library(libmthread SHARED mthread_thread.cpp mthread_vp.cpp mthread_mutex.cpp mthread_semaphore.cpp mthread_condition.cpp)

target_include_directories(
  libmthread PRIVATE . ${CMAKE_CURRENT_SOURCE_DIR}/../include
                     ${CMAKE_CURRENT_BINARY_DIR})
