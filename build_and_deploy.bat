@echo off
echo ========================================
echo    Disk Serial Spoofer Build Script
echo ========================================

REM Check if WDK is available
if not exist "C:\Program Files (x86)\Windows Kits\10\bin\x64\signtool.exe" (
    echo ERROR: Windows Driver Kit not found!
    echo Please install WDK 10 or set the correct path.
    pause
    exit /b 1
)

REM Set build environment
set DRIVER_NAME=disk_spoofer
set BUILD_DIR=x64\Release

echo.
echo [1/4] Cleaning previous build...
if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
if exist "*.sys" del "*.sys"

echo.
echo [2/4] Building driver...

REM Build using MSBuild if available
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe" disk_spoofer.vcxproj /p:Configuration=Release /p:Platform=x64
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe" disk_spoofer.vcxproj /p:Configuration=Release /p:Platform=x64
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" (
    "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" disk_spoofer.vcxproj /p:Configuration=Release /p:Platform=x64
) else (
    echo ERROR: MSBuild not found! Please install Visual Studio 2019 or later.
    pause
    exit /b 1
)

if not exist "%BUILD_DIR%\%DRIVER_NAME%.sys" (
    echo ERROR: Build failed! Driver not found.
    pause
    exit /b 1
)

echo.
echo [3/4] Copying driver to current directory...
copy "%BUILD_DIR%\%DRIVER_NAME%.sys" "%DRIVER_NAME%.sys"

echo.
echo [4/4] Deploying with kdmapper...
if not exist "kdmapper.exe" (
    echo ERROR: kdmapper.exe not found in current directory!
    echo Please ensure kdmapper.exe is in the same folder as this script.
    pause
    exit /b 1
)

echo.
echo WARNING: This will load a kernel driver that modifies disk serial numbers.
echo Make sure you understand the implications and have proper testing environment.
echo.
set /p CONFIRM="Do you want to continue? (y/N): "
if /i not "%CONFIRM%"=="y" (
    echo Operation cancelled.
    pause
    exit /b 0
)

echo.
echo Loading driver with kdmapper...
kdmapper.exe %DRIVER_NAME%.sys

if %ERRORLEVEL% equ 0 (
    echo.
    echo ========================================
    echo Driver loaded successfully!
    echo The disk serial number should now be spoofed.
    echo Use 'wmic diskdrive get serialnumber' to verify.
    echo ========================================
) else (
    echo.
    echo ========================================
    echo ERROR: Failed to load driver!
    echo Check if:
    echo - You are running as Administrator
    echo - Test signing is enabled (bcdedit /set testsigning on)
    echo - Driver signature enforcement is disabled
    echo - No antivirus is blocking the operation
    echo ========================================
)

echo.
pause