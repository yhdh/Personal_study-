enable_testing()

file(GLOB GEN_TEST_SOURCES example_*.cpp)
foreach(test ${GEN_TEST_SOURCES})
  get_filename_component(TName ${test} NAME_WE)
  add_executable(${TName} ${test})
  target_include_directories(${TName} PRIVATE ../include)
  target_link_libraries(${TName} PRIVATE libmthread)
  add_test(NAME ${TName} COMMAND ${TName})
  if(HAVE_FLAG_SANITIZER_address)
    set_property(
      TEST ${TName}
      PROPERTY
        ENVIRONMENT
        "LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/../tests/Sanitizer_supp_list.txt"
    )
  endif()
endforeach()
