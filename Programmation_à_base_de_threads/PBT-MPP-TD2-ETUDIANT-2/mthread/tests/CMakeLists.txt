enable_testing()

set(mthread_schedulers scheduler_fifo
                       scheduler_fifo_class)


file(GLOB GEN_TEST_SOURCES test_scheduler_perf_*.cpp)
foreach(test ${GEN_TEST_SOURCES})
  foreach(test_sched ${mthread_schedulers})
    get_filename_component(TName ${test} NAME_WE)
    add_executable(${TName}_${test_sched} ${test})
    target_compile_definitions(
      ${TName}_${test_sched} PRIVATE mthread_init_scheduler_test=mthread_init_${test_sched})
    target_include_directories(${TName}_${test_sched} PRIVATE ../include .)
    target_link_libraries(${TName}_${test_sched} PRIVATE libmthread)
        add_test(NAME ${TName}_${test_sched}
                 COMMAND ${TName}_${test_sched})
        set_tests_properties(${TName}_${test_sched} PROPERTIES LABELS perf_test) 
        set_tests_properties(${TName}_${test_sched} PROPERTIES TIMEOUT 300) 
        if(HAVE_FLAG_SANITIZER_address)
          set_property(
            TEST ${TName}_${test_sched}
            PROPERTY
              ENVIRONMENT
              "LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/../tests/Sanitizer_supp_list.txt"
          )
        endif()
  endforeach()
endforeach()


file(GLOB GEN_TEST_SOURCES test_scheduler_unit_*.cpp)
foreach(test ${GEN_TEST_SOURCES})
  foreach(test_sched ${mthread_schedulers})
    get_filename_component(TName ${test} NAME_WE)
    add_executable(${TName}_${test_sched} ${test})
    target_compile_definitions(
      ${TName}_${test_sched} PRIVATE mthread_init_scheduler_test=mthread_init_${test_sched})
    target_include_directories(${TName}_${test_sched} PRIVATE ../include .)
    target_link_libraries(${TName}_${test_sched} PRIVATE libmthread)
        add_test(NAME ${TName}_${test_sched}
                 COMMAND ${TName}_${test_sched})
        set_tests_properties(${TName}_${test_sched} PROPERTIES LABELS unit_test) 
        set_tests_properties(${TName}_${test_sched} PROPERTIES TIMEOUT 10) 
        if(HAVE_FLAG_SANITIZER_address)
          set_property(
            TEST ${TName}_${test_sched}
            PROPERTY
              ENVIRONMENT
              "LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/../tests/Sanitizer_supp_list.txt"
          )
        endif()
  endforeach()
endforeach()

file(GLOB GEN_TEST_SOURCES test_scheduler_intern_*.cpp)
foreach(test ${GEN_TEST_SOURCES})
    get_filename_component(TName ${test} NAME_WE)
    add_executable(${TName} ${test})
    target_compile_definitions(
      ${TName} PRIVATE)
      target_compile_options( ${TName} PRIVATE -Wno-unused-function)
    target_include_directories(${TName} PRIVATE ../include . ../src ${CMAKE_CURRENT_BINARY_DIR}/../src)
    target_link_libraries(${TName} PRIVATE libmthread)
        add_test(NAME ${TName}
                 COMMAND ${TName})
        set_tests_properties(${TName} PROPERTIES LABELS intern_test) 
        set_tests_properties(${TName} PROPERTIES TIMEOUT 10) 
        if(HAVE_FLAG_SANITIZER_address)
          set_property(
            TEST ${TName}
            PROPERTY
              ENVIRONMENT
              "LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/../tests/Sanitizer_supp_list.txt"
          )
        endif()
  endforeach()

file(GLOB GEN_TEST_SOURCES test_scheduler_gen_*.cpp)
set(nb_vps_list
    1
    2
    3
    4
    5
    6
    7
    8
    9
    10)
set(nb_threads_list
    1
    2
    3
    4
    5
    6
    7
    8
    9
    10
    50)
foreach(test ${GEN_TEST_SOURCES})
  foreach(test_sched ${mthread_schedulers})
    get_filename_component(TName ${test} NAME_WE)
    add_executable(${TName}_${test_sched} ${test})
    target_compile_definitions(
      ${TName}_${test_sched} PRIVATE mthread_init_scheduler_test=mthread_init_${test_sched})
    target_include_directories(${TName}_${test_sched} PRIVATE ../include .)
    target_link_libraries(${TName}_${test_sched} PRIVATE libmthread)
    foreach(nb_vps ${nb_vps_list})
      foreach(nb_threads ${nb_threads_list})
        add_test(NAME ${TName}_${test_sched}_${nb_vps}_${nb_threads}
                 COMMAND ${TName}_${test_sched} ${nb_vps} ${nb_threads})
        set_tests_properties(${TName}_${test_sched}_${nb_vps}_${nb_threads} PROPERTIES LABELS global_test) 
        set_tests_properties(${TName}_${test_sched}_${nb_vps}_${nb_threads} PROPERTIES TIMEOUT 60) 
        if(HAVE_FLAG_SANITIZER_address)
          set_property(
            TEST ${TName}_${test_sched}_${nb_vps}_${nb_threads}
            PROPERTY
              ENVIRONMENT
              "LSAN_OPTIONS=suppressions=${CMAKE_CURRENT_SOURCE_DIR}/../tests/Sanitizer_supp_list.txt"
          )
        endif()

      endforeach()
    endforeach()
  endforeach()
endforeach()

if(HAVE_FLAG_SANITIZER_address)
  message("Disable Valgrind tests with AddressSanitizer")
else()
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message("Disable Valgrind tests with Clang")
  else()
    if(MTHREAD_USE_VALGRIND)
      file(GLOB GEN_TEST_SOURCES test_scheduler_gen_*.cpp)
      set(nb_vps_list 1)
      set(nb_threads_list 5)
      foreach(test ${GEN_TEST_SOURCES})
        foreach(test_sched ${mthread_schedulers})
          get_filename_component(TName ${test} NAME_WE)
          foreach(nb_vps ${nb_vps_list})
            foreach(nb_threads ${nb_threads_list})
              add_test(
                NAME ${TName}_${test_sched}_${nb_vps}_${nb_threads}_valgrind
                COMMAND valgrind ./${TName}_${test_sched} ${nb_vps}
                        ${nb_threads})
              set_tests_properties(${TName}_${test_sched}_${nb_vps}_${nb_threads}_valgrind PROPERTIES LABELS valgrind_test) 
        set_tests_properties(${TName}_${test_sched}_${nb_vps}_${nb_threads}_valgrind PROPERTIES TIMEOUT 60) 
            endforeach()
          endforeach()
        endforeach()
      endforeach()
    endif()
  endif()
endif()
