cmake_minimum_required(VERSION 3.22)
project(LinCheckROOT VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)
pkg_check_modules(JSON REQUIRED nlohmann_json>=3.0.0)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${GTK4_INCLUDE_DIRS})
include_directories(${JSON_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/adb_abstraction.cpp
    src/device_inspector.cpp
    src/root_analyzer.cpp
    src/bootloader_analyzer.cpp
    src/rom_compatibility.cpp
    src/config_manager.cpp
    src/gui_main.cpp
)

# Create executable
add_executable(lincheckroot ${SOURCES})

# Link libraries
target_link_libraries(lincheckroot
    ${GTK4_LIBRARIES}
    ${JSON_LIBRARIES}
)

# Link directories
link_directories(${GTK4_LIBRARY_DIRS})
link_directories(${JSON_LIBRARY_DIRS})

# Compiler flags from pkg-config
target_compile_options(lincheckroot PRIVATE ${GTK4_CFLAGS_OTHER})
target_compile_options(lincheckroot PRIVATE ${JSON_CFLAGS_OTHER})

# Install data files
install(FILES data/lineage_devices.json DESTINATION share/lincheckroot)
install(FILES data/style.css DESTINATION share/lincheckroot)
