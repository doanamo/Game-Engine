#
# Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
# Software distributed under the permissive MIT License.
#

#
# External
#

set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory("../External/gtest" "External/gtest")
set_property(TARGET "gtest" PROPERTY FOLDER "External")
set_property(TARGET "gtest_main" PROPERTY FOLDER "External")
include(GoogleTest)

#
# Tests
#

project(Tests)
enable_testing()
add_subdirectory(Common)
add_subdirectory(Reflection)
add_subdirectory(Game)
