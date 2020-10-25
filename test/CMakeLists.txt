
OPTION(COMPILE_TESTS "Compile the tests" OFF)

IF(COMPILE_TESTS)

    SET(PROJECT_NAME AlsaloopForRealDeviceRedirection)

    CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
    CMAKE_POLICY(SET CMP0015 NEW)

    # CppUTest versiyon kontrolü

    IF(DEFINED ENV{CPPUTEST_HOME})

        MESSAGE(STATUS "CppUTest Home: $ENV{CPPUTEST_HOME}")
        SET(CPPUTEST_INCLUDE_DIRS $ENV{CPPUTEST_HOME}/include)
        SET(CPPUTEST_LIBRARIES $ENV{CPPUTEST_HOME}/lib)
        SET(CPPUTEST_LDFLAGS CppUTest CppUTestExt)
        
    ELSE()

        FIND_PACKAGE(PkgConfig REQUIRED)
        PKG_SEARCH_MODULE(CPPUTEST REQUIRED cpputest>=3.8)
        MESSAGE(STATUS "CppUTest Version: ${CPPUTEST_VERSION}")

    ENDIF()

    ADD_DEFINITIONS(-DCPPUTEST_USE_MEM_LEAK_DETECTION=0)
    ADD_DEFINITIONS("-std=c++11")

    SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    SET(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE ON)

    # Test gereksinimleri

    # pkg_check_modules(
        
    #     ${PROJECT_NAME}_TESTS_DEPENDS 
    # )

    # Proje kaynak dosyaları

    SET(PROJECT_SOURCES 

        ../src/loop_dev.cpp
    )

    # Proje include dizinleri

    SET(PROJECT_INCLUDE_DIRS
    
        ../include 
    )

    # Test kaynak dosyaları

    SET(TEST_SOURCES

        # Main
        main.cpp

        # Tests
        test_loop_dev.cpp

        # Mocks
        mocks/mock_alsaloop.cpp
    )

    # Test include dizinleri

    SET(TEST_INCLUDE_DIRS

        ${CPPUTEST_INCLUDE_DIRS}   
    )

    # Proje servis kütüphanesi. 
    # Test executable'ına link edilecek.

    ADD_LIBRARY(${PROJECT_NAME} ${PROJECT_SOURCES})

    # Include dizinleri

    INCLUDE_DIRECTORIES(${PROJECT_INCLUDE_DIRS} ${TEST_INCLUDE_DIRS})

    # Test executable

    ADD_EXECUTABLE(${PROJECT_NAME}_TESTS ${TEST_SOURCES})

    TARGET_LINK_LIBRARIES(
        
        ${PROJECT_NAME}_TESTS

        # Proje
        ${PROJECT_NAME}

        # CPPUTEST
        ${CPPUTEST_LDFLAGS} 

        # Test gereksinimleri
        ${${PROJECT_NAME}_TESTS_DEPENDS_LIBRARIES}
    )

    # Derleme bitince testleri çalıştır

    ADD_CUSTOM_COMMAND(TARGET ${PROJECT_NAME}_TESTS COMMAND ./${PROJECT_NAME}_TESTS POST_BUILD)

endif()