@echo off
setlocal

set "manifest=build\install_manifest.txt"

echo This will delete the following files installed by CMake and listed in %manifest%:
echo %manifest%
echo(

if not exist "%manifest%" (
    echo Error: manifest not found.
    exit /b 1
)

echo(
type "%manifest%"
echo(

set /p confirm=Do you want to proceed? (y/N): 

if /i not "%confirm%"=="y" (
    echo Uninstall cancelled.
    exit /b 0
)

echo(

for /f "usebackq delims=" %%F in ("%manifest%") do (
    if exist "%%F" (
        echo Deleting %%F
        del /f /q "%%F"
    ) else (
        echo Missing: %%F
    )
)

echo Uninstall finished.
endlocal