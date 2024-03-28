cmake_minimum_required(VERSION 3.10)


project(RemoteCli LANGUAGES CXX)

### Append project cmake script dir ###
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

### Enumerate project files ###
include(enum_cli_hdr)
include(enum_cli_src)
include(enum_crsdk_hdr)

### Define output target ###
set(remotecli "${PROJECT_NAME}")
add_executable(${remotecli}
    ${cli_hdrs}
    ${cli_srcs}
    ${crsdk_hdrs}
)




set_target_properties(${remotecli} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN"
)

## Specify char is signed-char to fix mismatch with Raspbian
target_compile_options(${remotecli}
    PRIVATE
        -fsigned-char
)

target_include_directories(${remotecli}
    PRIVATE
        ${crsdk_hdr_dir} # defined in enum script
)

### Configure external library directories ###
set(ldir ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(cr_ldir ${ldir}/crsdk)

### Link CRSDK library
find_library(camera_remote Cr_Core HINTS ${cr_ldir})
target_link_libraries(${remotecli}
    PRIVATE
        ${camera_remote}
)

### Windows specific configuration ###
# if(WIN32)
#     ## Build with unicode on Windows
#     target_compile_definitions(${remotecli} PRIVATE UNICODE _UNICODE)
# endif(WIN32)

### Linux specific configuration ###
if(UNIX AND NOT APPLE)
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8)
            # Must use std::experimental namespace if older than GCC8
            message("[${PROJECT_NAME}] GCC version less than 8. Using std::experimental namespace.")
            target_compile_definitions(${remotecli} PRIVATE USE_EXPERIMENTAL_FS)
        endif()

        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
            # Must explicitly link separate std::filesystem if older than GCC9
            message("[${PROJECT_NAME}] GCC version less than 9. Explicitly linking separate std::filesystem library.")
            target_link_libraries(${remotecli} PRIVATE stdc++fs)
        endif()
    endif()
endif(UNIX AND NOT APPLE)

# ## Copy required library binaries
# if(WIN32)
#     add_custom_command(TARGET ${remotecli} PRE_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy_directory ${cr_ldir} $<TARGET_FILE_DIR:${remotecli}>
#     )
# endif(WIN32)

if(UNIX AND NOT APPLE)
    add_custom_command(TARGET ${remotecli} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${cr_ldir} $<TARGET_FILE_DIR:${remotecli}>
    )
endif(UNIX AND NOT APPLE)

# if(APPLE)
#     set(ct_fw "Contents/Frameworks")
#     set(adaptor_dir_name "CrAdapter")
#     add_custom_command(TARGET ${remotecli} PRE_BUILD
#         COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${remotecli}>/${ct_fw}/${adaptor_dir_name}
#         COMMAND ${CMAKE_COMMAND} -E copy_directory ${cr_ldir} $<TARGET_FILE_DIR:${remotecli}>
#         COMMAND ${CMAKE_COMMAND} -E copy_directory $<TARGET_FILE_DIR:${remotecli}>/${adaptor_dir_name} $<TARGET_FILE_DIR:${remotecli}>/${ct_fw}/${adaptor_dir_name}
#         COMMAND ${CMAKE_COMMAND} -E rm -rf $<TARGET_FILE_DIR:${remotecli}>/${adaptor_dir_name}
#     )
# endif(APPLE)

add_library(SonySDKLib STATIC
${cli_hdrs}
${cli_srcs}
${crsdk_hdrs})
target_include_directories(SonySDKLib PRIVATE ${crsdk_hdr_dir})
target_link_libraries(SonySDKLib PRIVATE ${camera_remote})
# target_compile_options(SonySDKLib PRIVATE -fsigned-char)

add_compile_options(-fsigned-char)
