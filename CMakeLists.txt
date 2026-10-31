cmake_minimum_required(VERSION 3.20)
project(ScreenCaptureTracking LANGUAGES CXX)

# C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC specific settings
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive- /Zc:__cplusplus /Zc:inline /Zc:throwingNew")
    # Disable specific warnings that are noisy but not problematic
    add_compile_options(/wd4244 /wd4267 /wd4100 /wd4101 /wd4189 /wd4324 /wd4505 /wd4710 /wd4711 /wd4820 /wd5026 /wd5027 /wd5039 /wd5045)
    # Link libraries
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/Config.cpp
    src/GameCapture.cpp
    src/ColorDetection.cpp
    src/AimLogic.cpp
    src/BackendLoop.cpp
)

# Create executable
add_executable(ScreenCaptureTracking ${SOURCES})

# Link Windows libraries
target_link_libraries(ScreenCaptureTracking
    d3d11
    dxgi
    dxguid
)

# Copy config.ini to build directory on first build
add_custom_command(TARGET ScreenCaptureTracking POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/config.ini
        $<TARGET_FILE_DIR:ScreenCaptureTracking>/config.ini
    COMMENT "Copying config.ini to build directory"
)

# Install target (optional)
install(TARGETS ScreenCaptureTracking DESTINATION bin)