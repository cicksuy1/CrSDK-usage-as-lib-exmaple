# Minimum version of CMake required
cmake_minimum_required(VERSION 3.10)

# Project name
set(PROJECT_N CrSDK_HTTPS_Server)
project(${PROJECT_N} VERSION 1.0)

#set CXX
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-fsigned-char)

# Include directories
include_directories(include)

# macro that gets all of the header containing directories. 
MACRO(header_directories return_list includes_base_folder extention )
    FILE(GLOB_RECURSE new_list ${includes_base_folder}/*.${extention})
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

# Source files
file(GLOB_RECURSE SRCS ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Add the SonySDK directory to the build.
# This assumes the SonySDK folder is directly under the main project directory.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/include/SonySDK)

# using said macro.
header_directories(INCLUDES ${PROJECT_SOURCE_DIR}/include/ h)

message("src files:")
foreach(file ${SRCS})
        message(STATUS ${file})
endforeach()

message("include directories:")
foreach(dir ${INCLUDES})
        message(STATUS ${dir})
endforeach()

# Generate executable for your main project
add_executable(${PROJECT_N} ${SRCS})

# Define a variable for the installation prefix
set(CMAKE_INSTALL_PREFIX /usr/local)

### Linux specific configuration ###
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
        # Must use std::experimental namespace if older than GCC8
        message("[${PROJECT_NAME}] GCC version less than 8. Using std::experimental namespace.")
        target_compile_definitions(${PROJECT_N} PRIVATE USE_EXPERIMENTAL_FS)
    endif()

    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
        # Must explicitly link separate std::filesystem if older than GCC9
        message("[${PROJECT_NAME}] GCC version less than 9. Explicitly linking separate std::filesystem library.")
        target_link_libraries(${PROJECT_N} PRIVATE stdc++fs)
    endif()
endif()

# For httplib
message(STATUS "\nSearching for httplib...")
find_package(httplib REQUIRED)
if(httplib_FOUND)
  target_link_libraries(${PROJECT_N} ${httplib_LIBS})
  message(STATUS "httplib found!")
else()
  message(STATUS "httplib not found! Attempting to install...")
  #Setting httplib
  set(EXTERNAL_PROJECT_NAME httplib)
  set(EXTERNAL_PROJECT_URL https://github.com/yhirose/cpp-httplib.git)
  # Define the project and its properties
  ExternalProject_Add(
      httplib
      PREFIX ${DEPS_DIR}/${EXTERNAL_PROJECT_NAME}
      GIT_REPOSITORY ${EXTERNAL_PROJECT_URL}
      GIT_TAG v0.14.3 # Specify the desired release tag MUST BE THIS TAG NO SUPPORT FOR OPENSSL OVER 1.1.1 AFTER THAT!
      BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release -- -j12  # This will build with parallel jobs
      INSTALL_DIR ${CMAKE_INSTALL_PREFIX}  # Install to default CMake install path
  )
  # Add error handling here if httplib is required
endif()

# For SPDLOG_LIBRARY
find_package(spdlog REQUIRED)
if(spdlog_FOUND)
  target_link_libraries(${PROJECT_N} ${SPDLOG_LIBRARY})
  message(STATUS "spdlog found!")
else()
  message(STATUS "spdlog not found, attempting to install...")
  set(EXTERNAL_PROJECT_NAME spdlog)
  set(EXTERNAL_PROJECT_URL https://github.com/gabime/spdlog.git)
  # Define the project and its properties
  ExternalProject_Add(
      spdlog
      PREFIX ${DEPS_DIR}/${EXTERNAL_PROJECT_NAME}
      GIT_REPOSITORY ${EXTERNAL_PROJECT_URL}
      GIT_TAG v1.13.0 # Specify the desired release tag
      BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release -- -j12  # This will build with parallel jobs
      INSTALL_DIR ${CMAKE_INSTALL_PREFIX}  # Install to default CMake install path
  )
endif()

# For fmt
message(STATUS "\nSearching for fmt...")
find_package(fmt REQUIRED)
if(fmt_FOUND)
  target_link_libraries(${PROJECT_N} PUBLIC fmt::fmt)  # Link fmt publicly
  message(STATUS "fmt found!")
else()
  message(STATUS "fmtlib not found, attempting to install...")

  # Run apt-get install command to install fmtlib
  execute_process(
      COMMAND sudo apt-get install -y libfmt-dev
      RESULT_VARIABLE result
  )

  # Check the result of the apt-get install command
  if(result)
      message(FATAL_ERROR "Failed to install fmtlib. Re run with sudo or install it manually and re-run CMake.")
  endif()

  # Re-check for fmtlib after installation
  find_package(fmt REQUIRED)
  if(NOT fmt_FOUND)
      message(FATAL_ERROR "fmtlib is required but could not be found.")
  endif()
  # Add error handling here if fmt is required
endif()

# For Libevent
message(STATUS "\nSearching for Libevent...")
find_package(Libevent REQUIRED)
if(Libevent_FOUND)
    target_link_libraries(${PROJECT_N} PUBLIC ${LIBEVENT_LIBRARIES})
    message(STATUS "Libevent found!")
else()
    message(STATUS "Libevent not found!")
    message(STATUS "Installing Libevent...")
    message(STATUS "Looking for OpenSSL...")
    # Check if libssl-dev is installed
    find_package(OpenSSL REQUIRED)
    if(NOT OpenSSL_FOUND)
        # If libssl-dev is not found, try to install it
        message(STATUS "libssl-dev not found, attempting to install...")

        # Run apt-get install command to install libssl-dev
        execute_process(
            COMMAND sudo apt-get install -y libssl-dev
            RESULT_VARIABLE result
        )

        # Check the result of the apt-get install command
        if(result)
            message(FATAL_ERROR "Failed to install libssl-dev. re-run with sudo or install it manually and re-run CMake.")
        endif()
        # Re-check for libssl-dev after installation
        find_package(OpenSSL REQUIRED)
        if(NOT OpenSSL_FOUND)
            message(FATAL_ERROR "libssl-dev is required but could not be found.")
        endif()
        set(EXTERNAL_PROJECT_NAME libEvent)
        set(EXTERNAL_PROJECT_URL https://github.com/libevent/libevent.git)
        # Define the project and its properties
        ExternalProject_Add(
            LibEvent
            PREFIX ${DEPS_DIR}/${EXTERNAL_PROJECT_NAME}
            GIT_REPOSITORY ${EXTERNAL_PROJECT_URL}
            GIT_TAG release-2.1.12-stable  # Specify the desired release tag
            BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release -- -j12  # This will build with parallel jobs
            INSTALL_DIR ${CMAKE_INSTALL_PREFIX}  # Install to default CMake install path
        )
    endif()
    # Add error handling here if Libevent is required
endif()

add_compile_options(-fsigned-char)
target_link_libraries(${PROJECT_N} PUBLIC SonySDKLib)

target_link_libraries(${PROJECT_N} PUBLIC stdc++fs)

target_link_libraries(${PROJECT_N} PUBLIC -lpthread)

# list(APPEND src_files 
#   ${CMAKE_CURRENT_SOURCE_DIR}/SonySDK_files/libmonitor_protocol_pf.so  # File
#   ${CMAKE_CURRENT_SOURCE_DIR}/SonySDK_files/libmonitor_protocol.so  # File
#   ${CMAKE_CURRENT_SOURCE_DIR}/SonySDK_files/libCr_Core.so  # File
# )
                      
# # Copy CrSDK build artifacts after CrSDK's build step
# add_custom_command(TARGET ${PROJECT_N} POST_BUILD
#   COMMAND ${CMAKE_COMMAND} -E copy
#     ${src_files}  # Source: List of files
#     $<TARGET_FILE_DIR:${PROJECT_N}>  # Destination: Main project's build directory 
# )

# file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/SonySDK_files/CrAdapter DESTINATION ${CMAKE_BINARY_DIR}/)


