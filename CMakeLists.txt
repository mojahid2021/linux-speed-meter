cmake_minimum_required(VERSION 3.10)

project(LinuxSpeedMeter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(APPINDICATOR REQUIRED appindicator3-0.1)

# Include directories
include_directories(${GTK3_INCLUDE_DIRS} ${APPINDICATOR_INCLUDE_DIRS} include)

# Source files
set(SOURCES
    src/main.cpp
    src/tray_icon.cpp
    src/window.cpp
    src/speed_monitor.cpp
    src/helpers.cpp
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link against the required libraries
target_link_libraries(${PROJECT_NAME} ${GTK3_LIBRARIES} ${APPINDICATOR_LIBRARIES})

# Add compile options
target_compile_options(${PROJECT_NAME} PRIVATE ${GTK3_CFLAGS_OTHER})
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unused-result)
target_compile_options(${PROJECT_NAME} PRIVATE ${APPINDICATOR_CFLAGS_OTHER})