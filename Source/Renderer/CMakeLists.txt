#
# Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
# Software distributed under the permissive MIT License.
#

include_guard(GLOBAL)

#
# Files
#

set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/Include/Renderer")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set(FILES_RENDERER
    "${INCLUDE_DIR}/GameRenderer.hpp"
    "${SOURCE_DIR}/GameRenderer.cpp"
)

source_group("" FILES ${FILES_RENDERER})

#
# Library
#

add_library(Renderer
    ${FILES_RENDERER}
)

set_property(TARGET Renderer PROPERTY FOLDER "Engine")
target_include_directories(Renderer PUBLIC
    "${PROJECT_SOURCE_DIR}/Include"
    "${PROJECT_SOURCE_DIR}/Source"
)

#
# Precompiled Header
#

target_sources(Renderer PRIVATE "${SOURCE_DIR}/Precompiled.hpp")
target_precompile_headers(Renderer PRIVATE "${SOURCE_DIR}/Precompiled.hpp")
source_group("Generated\\Precompiled" REGULAR_EXPRESSION "cmake_pch")
source_group("" FILES "${SOURCE_DIR}/Precompiled.hpp")

#
# Dependencies
#

add_subdirectory("../Core" "Engine/Core")
target_link_libraries(Renderer PRIVATE Core)

add_subdirectory("../Platform" "Engine/Platform")
target_link_libraries(Renderer PRIVATE Platform)

add_subdirectory("../Graphics" "Engine/Graphics")
target_link_libraries(Renderer PRIVATE Graphics)

add_subdirectory("../Game" "Engine/Game")
target_link_libraries(Renderer PRIVATE Game)

enable_reflection(Renderer ${INCLUDE_DIR} ${SOURCE_DIR})
