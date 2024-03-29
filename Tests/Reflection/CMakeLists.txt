#
# Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
# Software distributed under the permissive MIT License.
#

include_guard(GLOBAL)

#
# Files
#

set(TEST_FILES
    "TestReflection.cpp"
    "TestReflectionHeader.hpp"
    "TestReflectionStatic.cpp"
    "TestReflectionDynamic.cpp"
)

#
# Test
#

add_executable(TestReflection ${TEST_FILES})

#
# Precompiled Header
#

target_sources(TestReflection PRIVATE "Precompiled.hpp")
target_precompile_headers(TestReflection PRIVATE "Precompiled.hpp")
source_group("Generated\\Precompiled" REGULAR_EXPRESSION "cmake_pch")
source_group("" FILES "Precompiled.hpp")

#
# Dependencies
#

add_subdirectory("../../Source/Reflection" "Engine/Reflection")
target_link_libraries(TestReflection PRIVATE Reflection)

enable_reflection(TestReflection ${CMAKE_CURRENT_SOURCE_DIR})

#
# Environment
#

set_target_properties(TestReflection PROPERTIES FOLDER "Tests")

if(NOT EMSCRIPTEN)
    gtest_discover_tests(TestReflection TEST_PREFIX "Reflection.")
endif()

#
# External
#

target_link_libraries(TestReflection PRIVATE gtest_main)
