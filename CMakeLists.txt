cmake_minimum_required(VERSION 3.10)

project(LinuxSpeedMeter VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Platform detection
if(WIN32)
    set(PLATFORM_WINDOWS TRUE)
    message(STATUS "Building for Windows")
elseif(APPLE)
    set(PLATFORM_MACOS TRUE)
    message(STATUS "Building for macOS")
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
    message(STATUS "Building for Linux")
endif()

# Option to build Windows executable
option(BUILD_WINDOWS_EXE "Build Windows executable using cross-compilation" OFF)

if(BUILD_WINDOWS_EXE)
    # Windows cross-compilation setup (let toolchain file handle system/compiler settings)
    # Find libcurl
    find_package(CURL REQUIRED)
    
    # Windows-specific sources (console application)
    set(SOURCES
        src/main_windows.cpp
        src/speed_monitor.cpp
        src/helpers.cpp
        src/data_manager.cpp
        src/speed_test.cpp
        src/download_test.cpp
        src/upload_test.cpp
        src/ping_test.cpp
    )

    # Create Windows executable
    add_executable(${PROJECT_NAME} ${SOURCES})
    
    include_directories(${CURL_INCLUDE_DIRS} include)

    # Windows-specific libraries and flags
    target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES} -static-libgcc -static-libstdc++ -lpthread)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mconsole")

else()
    # Linux build (original GTK version)
    find_package(PkgConfig REQUIRED)
    find_package(CURL REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_check_modules(APPINDICATOR REQUIRED appindicator3-0.1)

    include_directories(${GTK3_INCLUDE_DIRS} ${APPINDICATOR_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} include)

    set(SOURCES
        src/main.cpp
        src/tray_icon.cpp
        src/window.cpp
        src/speed_monitor.cpp
        src/helpers.cpp
        src/data_manager.cpp
        src/speed_test.cpp
        src/download_test.cpp
        src/upload_test.cpp
        src/ping_test.cpp
        src/speed_test_widget.cpp
    )

    add_executable(${PROJECT_NAME} ${SOURCES})

    target_link_libraries(${PROJECT_NAME} ${GTK3_LIBRARIES} ${APPINDICATOR_LIBRARIES} ${CURL_LIBRARIES} pthread)

    target_compile_options(${PROJECT_NAME} PRIVATE ${GTK3_CFLAGS_OTHER})
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unused-result)
    target_compile_options(${PROJECT_NAME} PRIVATE ${APPINDICATOR_CFLAGS_OTHER})
    target_compile_options(${PROJECT_NAME} PRIVATE -Wno-deprecated-declarations)
    
    # Add optimization flags
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${PROJECT_NAME} PRIVATE -O3 -march=native)
    endif()

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()