#
# Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
# Software distributed under the permissive MIT License.
#

include_guard(GLOBAL)

#
# Files
#

set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/Include/Core")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(FILES_BUILD
    "${INCLUDE_DIR}/Build/Build.hpp"
    "${SOURCE_DIR}/Build/BuildInfo.hpp.in"
    "${SOURCE_DIR}/Build/BuildInfo.cmake"
    "${SOURCE_DIR}/Build/Build.cpp"
)

set(FILES_SCRIPT
    "${INCLUDE_DIR}/Script/ScriptState.hpp"
    "${INCLUDE_DIR}/Script/ScriptSystem.hpp"
    "${SOURCE_DIR}/Script/ScriptState.cpp"
    "${SOURCE_DIR}/Script/ScriptBinding.cpp"
    "${SOURCE_DIR}/Script/ScriptSystem.cpp"
)

set(FILES_CONFIG
    "${INCLUDE_DIR}/Config/ConfigTypes.hpp"
    "${INCLUDE_DIR}/Config/ConfigSystem.hpp"
    "${SOURCE_DIR}/Config/ConfigSystem.cpp"
)

set(FILES_SYSTEM
    "${INCLUDE_DIR}/System/SystemInterface.hpp"
    "${INCLUDE_DIR}/System/SystemStorage.hpp"
    "${INCLUDE_DIR}/System/EngineSystem.hpp"
    "${INCLUDE_DIR}/System/EngineMetrics.hpp"
    "${INCLUDE_DIR}/System/FrameRateLimiter.hpp"
    "${SOURCE_DIR}/System/EngineMetrics.cpp"
    "${SOURCE_DIR}/System/FrameRateLimiter.cpp"
)

set(FILES_RESOURCEMANAGER
    "${INCLUDE_DIR}/Resource/ResourcePool.hpp"
    "${INCLUDE_DIR}/Resource/ResourceManager.hpp"
    "${SOURCE_DIR}/Resource/ResourceManager.cpp"
)

set(FILES_CORE
    "${INCLUDE_DIR}/Core.hpp"
)

source_group("Build" FILES ${FILES_BUILD})
source_group("Script" FILES ${FILES_SCRIPT})
source_group("Config" FILES ${FILES_CONFIG})
source_group("System" FILES ${FILES_SYSTEM})
source_group("ResourceManager" FILES ${FILES_RESOURCEMANAGER})
source_group("" FILES ${FILES_CORE})

#
# Library
#

add_library(Core
    ${FILES_BUILD}
    ${FILES_SCRIPT}
    ${FILES_CONFIG}
    ${FILES_SYSTEM}
    ${FILES_RESOURCEMANAGER}
    ${FILES_CORE}
)

set_property(TARGET Core PROPERTY FOLDER "Engine")
target_include_directories(Core PUBLIC
    "${PROJECT_SOURCE_DIR}/Include"
    "${PROJECT_SOURCE_DIR}/Source"
)

#
# Precompiled Header
#

target_sources(Core PRIVATE "${SOURCE_DIR}/Precompiled.hpp")
target_precompile_headers(Core PRIVATE "${SOURCE_DIR}/Precompiled.hpp")
source_group("Generated\\Precompiled" REGULAR_EXPRESSION "cmake_pch")
source_group("" FILES "${SOURCE_DIR}/Precompiled.hpp")

#
# Dependencies
#

add_subdirectory("../Common" "Engine/Common")
target_link_libraries(Core PUBLIC Common)

enable_reflection(Core ${INCLUDE_DIR} ${SOURCE_DIR})

#
# Configuration
#

find_package("Git")

set(BUILD_INFO_COMMAND_ECHO
    ${CMAKE_COMMAND} -E echo "Generating build info header..."
)

set(BUILD_INFO_COMMAND_INVOKE
    ${CMAKE_COMMAND}
    -D GIT_EXECUTABLE="${GIT_EXECUTABLE}"
    -D ENGINE_PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../"
    -D GAME_PROJECT_DIR="${CMAKE_SOURCE_DIR}"
    -D INPUT_FILE="${CMAKE_CURRENT_SOURCE_DIR}/Build/BuildInfo.hpp.in"
    -D OUTPUT_FILE="${CMAKE_CURRENT_BINARY_DIR}/Build/BuildInfo.hpp"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/Build/BuildInfo.cmake"
)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    add_custom_command(TARGET Core PRE_BUILD
        COMMAND ${BUILD_INFO_COMMAND_ECHO}
        COMMAND ${BUILD_INFO_COMMAND_INVOKE}
    )
else()
    add_custom_target(BuildInfo
        COMMAND ${BUILD_INFO_COMMAND_ECHO}
        COMMAND ${BUILD_INFO_COMMAND_INVOKE}
    )

    add_dependencies(Core BuildInfo)
    set_property(TARGET BuildInfo PROPERTY FOLDER "Generated")
endif()

# Add file with generated build version to engine sources.
# Only the output header file is needed for the build process.
# Create empty output file first otherwise CMake will complain.
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/Build/BuildInfo.hpp")
source_group("Generated\\Build" FILES "${CMAKE_CURRENT_BINARY_DIR}/Build/BuildInfo.hpp")
target_sources(Core PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/Build/BuildInfo.hpp")
target_include_directories(Core PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../")

#
# Externals
#

target_include_directories(Core PUBLIC "../../External/glm")

add_subdirectory("../../External/fast_float" "External/fast_float")
target_link_libraries(Core PUBLIC "fast_float")
set_property(TARGET "fast_float" PROPERTY FOLDER "External")

add_subdirectory("../../External/simdjson" "External/simdjson")
target_link_libraries(Core PUBLIC "simdjson")
set_property(TARGET "simdjson" PROPERTY FOLDER "External")

add_subdirectory("../../External/lua" "External/lua")
target_link_libraries(Core PUBLIC "lua_static")
set_property(TARGET "lua_static" PROPERTY PROJECT_LABEL "lua")
set_property(TARGET "lua_static" PROPERTY FOLDER "External")

if(NOT EMSCRIPTEN)
    add_subdirectory("../../External/zlib" "External/zlib" EXCLUDE_FROM_ALL)
    target_include_directories(Core PUBLIC "../../External/zlib")
    target_include_directories(Core PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/External/zlib")
    target_link_libraries(Core PUBLIC "zlibstatic")
    set_property(TARGET "zlibstatic" PROPERTY PROJECT_LABEL "zlib")
    set_property(TARGET "zlibstatic" PROPERTY FOLDER "External")

    set(PNG_BUILD_ZLIB ON)
    add_subdirectory("../../External/libpng" "External/libpng")
    target_include_directories("png_static" PRIVATE "../../External/zlib")
    target_include_directories("png_static" PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/External/zlib")
    target_include_directories(Core PUBLIC "../../External/libpng")
    target_include_directories(Core PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/External/libpng")
    target_link_libraries(Core PUBLIC "png_static")
    set_property(TARGET "png_static" PROPERTY PROJECT_LABEL "libpng")
    set_property(TARGET "png_static" PROPERTY FOLDER "External")
    set_property(TARGET "genfiles" PROPERTY PROJECT_LABEL "libpngconf")
    set_property(TARGET "genfiles" PROPERTY FOLDER "External")

    find_package(OpenGL REQUIRED)
    target_link_libraries(Core ${OPENGL_GL_LIBRARY})

    add_subdirectory("../../External/glfw" "External/glfw")
    target_include_directories(Core PUBLIC "../../External/glfw/include")
    target_link_libraries(Core PUBLIC "glfw")
    set_property(TARGET "glfw" PROPERTY FOLDER "External")
else()
    target_compile_definitions(simdjson PRIVATE "SIMDJSON_NO_PORTABILITY_WARNING")

    target_link_options(Core PUBLIC "SHELL: -s USE_ZLIB=1")
    target_link_options(Core PUBLIC "SHELL: -s USE_LIBPNG=1")
    target_link_options(Core PUBLIC "SHELL: -s USE_GLFW=3")
    target_link_options(Core PUBLIC "SHELL: -s FULL_ES3=1")
endif()

add_subdirectory("../../External/glad/" "External/glad")
target_link_libraries(Core PUBLIC "glad")
set_property(TARGET "glad" PROPERTY FOLDER "External")

add_subdirectory("../../External/imgui" "External/imgui")
target_include_directories("imgui" PUBLIC "../../External/imgui/include")
target_link_libraries(Core PUBLIC "imgui")
set_property(TARGET "imgui" PROPERTY FOLDER "External")
