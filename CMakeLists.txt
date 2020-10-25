cmake_minimum_required(VERSION 3.0.0)
project(AlsaloopForRealSoundDeviceRedirection VERSION 0.1.0)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

project(AlsaloopForRealSoundDeviceRedirection C CXX)

set(PROJECT_NAME "AlsaloopForRealSoundDeviceRedirection")
set(PROJECT_VERSION "1.0")
set(PROJECT_PRETTY_NAME "alsaloopforrealsounddeviceredirection")
set(PROJECT_DESCRIPTION "Library for redirecting real sound devices by using Alsaloop")
set(PROJECT_INCLUDEDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
set(PROJECT_LIBDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
set(LIBALSALOOPFORREALDEVICEREDIRECTION_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(PROJECT_AUTHOR "Oguzhan Mutlu")
set(PROJECT_AUTHOR_MAIL "oguzhanxmutlu@gmail.com")
set(PROJECT_LICENSE "CLOSED")
set(PROJECT_LANGUAGES "C;CXX")

option(COMPILE_TESTS "Compile the tests" OFF)
if(COMPILE_TESTS)

    add_subdirectory(test)

else()

    # get pkgconfig
    INCLUDE(FindPkgConfig)

    add_library(AlsaloopForRealSoundDeviceRedirection src/alsaloop.cpp
                                                        src/control.cpp
                                                        src/loop_dev.cpp
                                                        src/pcmjob.cpp)

    target_include_directories(AlsaloopForRealSoundDeviceRedirection PUBLIC include)

    pkg_check_modules(LIBALSALOOPFORREALDEVICEREDIRECTION_DEPENDS alsa)
    target_link_libraries(AlsaloopForRealSoundDeviceRedirection -lpthread
                                                                ${LIBALSALOOPFORREALDEVICEREDIRECTION_DEPENDS_LIBRARIES})
  
    link_libraries(-Wl,--as-needed -Wl,--gc-sections -Wl,--no-undefined)
  
    set(CMAKE_BUILD_TYPE Release)
  
    #generate configure file
    configure_file(libalsaloopforrealdeviceredirection.pc.in libalsaloopforrealdeviceredirection.pc @ONLY)
    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/libalsaloopforrealdeviceredirection.pc
      DESTINATION
      ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

endif()