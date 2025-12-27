cmake_minimum_required(VERSION 3.15)
project(na-daesin-click VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Unicode support
add_definitions(-DUNICODE -D_UNICODE)

# Source files
set(SOURCES
    src/main.cpp
    src/window.cpp
    src/clicker.cpp
    src/hotkey.cpp
)

# Header files
set(HEADERS
    src/window.h
    src/clicker.h
    src/hotkey.h
    res/resource.h
)

# Windows executable (no console)
add_executable(${PROJECT_NAME} WIN32
    ${SOURCES}
    ${HEADERS}
    res/resource.rc
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    user32
    gdi32
    comctl32
    shell32
    winmm
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/res
)

# Compiler-specific settings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
    # Manifest for DPI awareness
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_DPI_AWARE "PerMonitor"
    )
else()
    # MinGW settings
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
    target_link_options(${PROJECT_NAME} PRIVATE -mwindows -static)
endif()

# Output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
