# Minimum version of CMake required
cmake_minimum_required(VERSION 3.5)

# Project name
project(Test)

add_compile_options(-fsigned-char)

# Include directories
include_directories(include)
#set CXX
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the SonySDK directory to the build.
# This assumes the SonySDK folder is directly under the main project directory.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/include/SonySDK)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Generate executable for your main project
add_executable(Test ${SOURCES})

# Define a variable for the installation prefix
set(CMAKE_INSTALL_PREFIX /usr/local)

### Linux specific configuration ###
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
        # Must use std::experimental namespace if older than GCC8
        message("[${PROJECT_NAME}] GCC version less than 8. Using std::experimental namespace.")
        target_compile_definitions(Test PRIVATE USE_EXPERIMENTAL_FS)
    endif()

    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        # Must explicitly link separate std::filesystem if older than GCC9
        message("[${PROJECT_NAME}] GCC version less than 9. Explicitly linking separate std::filesystem library.")
        target_link_libraries(Test PRIVATE stdc++fs)
    endif()
endif()

add_compile_options(-fsigned-char)
target_link_libraries(Test PUBLIC SonySDKLib)

target_link_libraries(Test PUBLIC stdc++fs)


