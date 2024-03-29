@echo off
setlocal

rem Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
rem Software distributed under the permissive MIT License.

set inputDir=%1
set outputDir=%2
set arch=%3

if [%inputDir%] == [] set inputDir="../."
if [%outputDir%] == [] set outputDir="CMake-VS2022-x64"
if [%arch%] == [] set arch=x64

echo ---- Preparing build directory...
mkdir %outputDir% >nul 2>&1
cd %outputDir%

echo ---- Generating CMake project...
set archArg=-A %arch%

cmake -G "Visual Studio 17" %archArg% "../%inputDir%"

if %ERRORLEVEL% NEQ 0 (
    pause
    exit
)

echo ---- Opening project solution...
for /f %%f in ('dir /b "*.sln"') do (
    set solution=%%f
    goto found
)
:found

start "" "%solution%"
