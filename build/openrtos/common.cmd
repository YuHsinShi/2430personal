@echo off
set CYGWIN=nodosfilewarning
@if defined COMMON_DEFINED goto end
set COMMON_DEFINED=1
set GCC_COLORS=1

call :set_cfg_platform "%cd%"

pushd ..\..
for /f "delims=" %%a in ('cd') do set CMAKE_SOURCE_DIR=%%a
popd

set CFG_BUILDPLATFORM=%CFG_PLATFORM%
set CMAKE_ROOT=%CMAKE_SOURCE_DIR%\tool
set CFG_TOOLCHAIN_FILE=%CMAKE_SOURCE_DIR%\%CFG_PLATFORM%\toolchain.cmake

set PATH=%CMAKE_ROOT%\bin;D:\Andestech\AndeSight_STD_v320\cygwin\bin;D:\Andestech\AndeSight_STD_v320\toolchains\nds32le-elf-newlib-v5d\bin;C:\ITEGCC\bin;%PATH%
title %CFG_PLATFORM%-%CFG_PROJECT%

:end
goto :eof

:set_cfg_platform
rem ==========================================================================
rem set CFG_PLATFORM=openrtos from the current directory name build\[openrtos]
rem ==========================================================================
set CFG_PLATFORM=%~n1
goto :eof
