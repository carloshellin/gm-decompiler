@echo off

REM Change your executable name here
set EXECUTABLE_NAME=gm-decompiler.exe

REM Set your sources here (relative paths!)
REM Example with two source folders:
REM set SOURCES=src\*.c src\submodule\*.c
set SOURCES=src\main.c

REM Checks if cl is available and skips to the argument loop if it is
REM (Prevents calling vcvarsall every time you run this script)
WHERE cl >nul 2>nul
IF %ERRORLEVEL% == 0 goto READ_ARGS

REM Activate the msvc build environment if cl isn't available yet
IF EXIST "%ProgramFiles(x86)%\\Microsoft Visual Studio\\2022\\BuildTools\\VC\Auxiliary\Build\vcvars64.bat" (
    echo Setting up the msvc build environment, this could take some time but the next builds should be faster
    call "%ProgramFiles(x86)%\\Microsoft Visual Studio\\2022\\BuildTools\\VC\Auxiliary\Build\vcvars64.bat" > NUL 2>&1
) ELSE (
    echo "Couldn't find vcvars64.bat, please call windows.bat from https://github.com/carloshellin/cup"
    exit /B
)

:READ_ARGS
REM For the ! variable notation
setlocal EnableDelayedExpansion
REM For shifting, which the command line argument parsing needs
setlocal EnableExtensions

:ARG_LOOP
set ARG=%1
if "!ARG!" == "" ( goto BUILD )
IF NOT "x!ARG!" == "x!ARG:run=!" (
  set RUN=1
)
IF NOT "x!ARG!" == "x!ARG:check=!" (
  set CHECK=1
)
IF NOT "x!ARG!" == "x!ARG:release=!" (
  set BUILD_RELEASE=1
)
IF NOT "%1" == "" (
  shift /1
  goto ARG_LOOP
)

:BUILD
REM Directories
set "ROOT_DIR=%CD%"
set "SOURCES=!ROOT_DIR!\!SOURCES!"

REM Flags
set VERBOSITY_FLAG=/nologo
set COMPILATION_FLAGS=/std:c11 /Od /Z7 /utf-8
set WARNING_FLAGS=/W3 /sdl
set OUTPUT_FLAG=/Fe: "!EXECUTABLE_NAME!"
set LINK_FLAGS=/MTd /link kernel32.lib
set SUBSYSTEM_FLAGS=/DEBUG
set OUTPUT_DIR=target\debug\
REM Release changes to flags
IF DEFINED BUILD_RELEASE (
  set COMPILATION_FLAGS=/std:c11 /O2 /GL /favor:blend /utf-8
  set SUBSYSTEM_FLAGS=
  set LINK_FLAGS=/MT /link kernel32.lib
  set OUTPUT_DIR=target\release\
)

REM Move to the build directory
IF NOT EXIST !OUTPUT_DIR! mkdir !OUTPUT_DIR!
cd !OUTPUT_DIR!

IF DEFINED CHECK (
    cl.exe !VERBOSITY_FLAG! !COMPILATION_FLAGS! !WARNING_FLAGS! /c !SOURCES! || exit /B
) ELSE (
    cl.exe !VERBOSITY_FLAG! !COMPILATION_FLAGS! !WARNING_FLAGS! !OUTPUT_FLAG! !SOURCES! !SUBSYSTEM_FLAGS! !LINK_FLAGS! || exit /B
)
del *.obj

REM Finally, run the produced executable
IF DEFINED RUN (
    !EXECUTABLE_NAME!
)

REM Back to development directory
cd !ROOT_DIR!