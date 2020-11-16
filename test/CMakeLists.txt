CMAKE_MINIMUM_REQUIRED(VERSION 3.8)

#Test için gerekli adımlar

option(COMPILE_TESTS "Compile the tests" OFF)

if(COMPILE_TESTS)

    enable_language(C)
    enable_language(CXX)

    set(CMAKE_INCLUDE_CURRENT_DIR ON)

    # (1) Look for installed version of CppUTest
    if(DEFINED ENV{CPPUTEST_HOME})
        message(STATUS "Using CppUTest home: $ENV{CPPUTEST_HOME}")
        set(CPPUTEST_INCLUDE_DIRS $ENV{CPPUTEST_HOME}/include)
        set(CPPUTEST_LIBRARIES $ENV{CPPUTEST_HOME}/lib)
        set(CPPUTEST_LDFLAGS CppUTest CppUTestExt)
    else()
        find_package(PkgConfig REQUIRED)
        pkg_search_module(CPPUTEST REQUIRED cpputest>=3.8)
        message(STATUS "Found CppUTest version ${CPPUTEST_VERSION}")
    endif()

    # NOT: "__node doesnt name a type" adında bir hataya sebep olduğu için kapatıldı.
    add_definitions(-DCPPUTEST_USE_MEM_LEAK_DETECTION=0)
    add_definitions("-std=c++11")

    # Coverage bilgisi elde etmek için CXX flagları
    # 
    set(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)
    set(OBJECT_OUTPUT_DIR ${CMAKE_BINARY_DIR}/test/CMakeFiles/alsalooptobetested.dir/__/src)

    # (1) set sources to be tested
    set(TARGET_LIBTOBETESTED alsalooptobetested)
    set(libtobetested_sources 
        ../src/loop_dev.cpp)

    add_library(${TARGET_LIBTOBETESTED} ${libtobetested_sources})

    # (2) Our unit tests sources
    set(TEST_APP_NAME ${TARGET_LIBTOBETESTED}_tests)
    set(TEST_SOURCES
        main.cpp
        mocks/mock_alsaloop.cpp
        test_loop_dev.cpp
    )

    # (3) Take care of include directories
    include_directories(${CPPUTEST_INCLUDE_DIRS} ../include 
                                                ${alsalooptobetested_depends_INCLUDE_DIRS})

    link_directories(${CPPUTEST_LIBRARIES})
    link_libraries(${TARGET_LIBTOBETESTED})

    # (4) Build the unit tests objects and link then with the app library
    add_executable(${TEST_APP_NAME} ${TEST_SOURCES})

    target_compile_options(${TEST_APP_NAME} PUBLIC ${alsalooptobetested_depends_CFLAGS})
    target_link_libraries(${TEST_APP_NAME} ${link_libraries} 
                                            ${CPPUTEST_LDFLAGS} 
                                            ${alsalooptobetested_depends_LIBRARIES}
                                            )

    # (5) Run the test once the build is done
    add_custom_command(TARGET ${TEST_APP_NAME} COMMAND ./${TEST_APP_NAME} POST_BUILD)

    ############################################################################
    ################################### LCOV ###################################
    ############################################################################

    add_custom_target(lcov

    COMMAND mkdir -p coverage

    COMMAND mkdir -p coverage/html

    COMMAND ${CMAKE_MAKE_PROGRAM}

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    add_custom_command(TARGET lcov

    COMMAND echo "################# LCOV #################"

    COMMAND lcov -c -d ${COVERAGE_DATA_DIR} -o ${CMAKE_BINARY_DIR}/coverage.info

    COMMAND lcov --remove ${CMAKE_BINARY_DIR}/coverage.info '**/include/**' -o ${CMAKE_BINARY_DIR}/coverage_filtered.info

    COMMAND genhtml -o ${CMAKE_BINARY_DIR}/coverage/html ${CMAKE_BINARY_DIR}/coverage_filtered.info

    COMMAND echo "-- Coverage report has been output to ${CMAKE_BINARY_DIR}/coverage/html"

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    add_dependencies(lcov ${TARGET_LIBTOBETESTED})

    ############################################################################
    ################################### GCOV ###################################
    ############################################################################

    add_custom_target(gcov

    COMMAND mkdir -p coverage

    COMMAND ${CMAKE_MAKE_PROGRAM}

    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

    add_custom_command(TARGET gcov

        COMMAND echo "################# GCOV #################"

        COMMAND gcov -b ${libtobetested_sources} -o ${OBJECT_OUTPUT_DIR} | grep -A 5 ".cpp$" > CoverageSummary.tmp

        COMMAND cat CoverageSummary.tmp

        COMMAND echo "-- Coverage files have been output to ${CMAKE_BINARY_DIR}/coverage"

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/coverage
    )

    add_dependencies(gcov ${TARGET_LIBTOBETESTED})

else()
    message(STATUS "Cross compile environment set")
endif()
