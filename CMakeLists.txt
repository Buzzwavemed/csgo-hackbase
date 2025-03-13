cmake_minimum_required(VERSION 3.10)
project(CSGOCheat VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Define source files
set(SOURCES
    src/main.cpp
    src/core/memory.cpp
    src/core/pattern_scan.cpp
    src/sdk/entity.cpp
    src/features/aimbot.cpp
    src/features/triggerbot.cpp
    src/features/bunnyhop.cpp
    src/gui/menu.cpp
)

# Define include directories
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

# Windows-specific settings
if(WIN32)
    # Windows SDK version
    set(CMAKE_SYSTEM_VERSION 10.0)
    
    # Set target as a console application for better visibility during testing
    add_executable(${PROJECT_NAME} ${SOURCES})
    
    # Define compiler flags
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        UNICODE
        _UNICODE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
    
    # Link Windows libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        kernel32
        user32
        gdi32
        winspool
        comdlg32
        advapi32
        shell32
        ole32
        oleaut32
        uuid
        odbc32
        odbccp32
        winmm
    )
    
    # We're using a console-based menu instead of DirectX for simplicity
    # No need for DirectX dependencies
    
    # Enable debug console in Debug builds
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(${PROJECT_NAME} PRIVATE _DEBUG)
    endif()
endif()

# Additional compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE 
        /W3
        /wd4100  # Unreferenced formal parameter
        /wd4996  # Deprecated functions
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Install configuration
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# Output executable to bin directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Enable organization of files in IDE
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCES})

# Print configuration summary
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
