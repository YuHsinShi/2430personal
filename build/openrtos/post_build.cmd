@echo off

chcp 437 > nul

set NINJA=0
set CYGWIN=nodosfilewarning
if exist %CFG_PROJECT%/project/%TARGET%/%TARGET% del %CFG_PROJECT%\project\%TARGET%\%TARGET%

set PROJECT_NAME=%CFG_PROJECT%

if "%ALT_CPU%" == "1" (
    call :makeclean alt_cpu
    call :buildProject alt_cpu
)

if "%ARMLITEDEV%" == "1" (
    call :makeclean arm_lite_dev
    call :buildProject arm_lite_dev
)

if "%ARMLITECODEC%" == "1" (
    call :makeclean arm_lite_codec
    call :buildProject arm_lite_codec
)

call :makeclean %PROJECT_NAME%
call :buildProject %PROJECT_NAME%
goto :eof

rem ###########################################################################
rem The function to build project
rem ###########################################################################
:buildProject
SETLOCAL
set LOCAL_PROJECT_NAME=%1
set CFG_PROJECT=%LOCAL_PROJECT_NAME%
if not exist %LOCAL_PROJECT_NAME% mkdir %LOCAL_PROJECT_NAME%
pushd %LOCAL_PROJECT_NAME%

if "%NINJA%" == "0" (
    if exist CMakeCache.txt (
        cmake.exe -G"Unix Makefiles" "%CMAKE_SOURCE_DIR%"
    ) else (
        cmake.exe -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="%CFG_TOOLCHAIN_FILE%" "%CMAKE_SOURCE_DIR%"
    )
    if errorlevel 1 exit /b

    if "%MAKECLEAN%"=="1" (
        echo "Clean build..."
        make clean
    )

    if "%MAKEJOBS%"=="" (
        make -j 1 VERBOSE=%VERBOSE%
    ) else (
        make -j %MAKEJOBS% VERBOSE=%VERBOSE%
    )
) else (
    if "%VERBOSE%" == "1" (
        set VERBOSEOUT="-v"
    ) else (
        set VERBOSEOUT=""
    )

    if exist CMakeCache.txt (
        cmake.exe -G Ninja "%CMAKE_SOURCE_DIR%"
    ) else (
        cmake.exe -G Ninja -DCMAKE_TOOLCHAIN_FILE="%CFG_TOOLCHAIN_FILE%" "%CMAKE_SOURCE_DIR%"
    )
    if errorlevel 1 exit /b

    if "%MAKECLEAN%"=="1" (
        echo "Clean build..."
        ninja -t clean
    )

    if "%MAKEJOBS%"=="" (
        ninja -j 1 %VERBOSEOUT%
    ) else (
        ninja -j %MAKEJOBS% %VERBOSEOUT%
    )
)
popd

ENDLOCAL

:makeclean
if "%MAKECLEAN%"=="1" (
    if exist %1%\CMakeFiles (
        del /f/q/s %1\CMakeFiles
        rd /q/s %1%\CMakeFiles
    )
    if exist %1%\CMakeCache.txt (
        del /f/q/s %1\CMakeCache.txt
    )
)