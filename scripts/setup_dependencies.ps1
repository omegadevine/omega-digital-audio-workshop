# Omega DAW Dependency Setup Script
# This script ensures all dependencies are installed and maintained

param(
    [switch]$Force,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

Write-Host "=== Omega DAW Dependency Manager ===" -ForegroundColor Cyan

# Define dependency versions
$DEPS_VERSION = "1.0.0"
$DEPS_MARKER = ".deps_installed_$DEPS_VERSION"

# Check if dependencies are already installed
if ((Test-Path $DEPS_MARKER) -and -not $Force) {
    Write-Host "Dependencies already installed (version $DEPS_VERSION)" -ForegroundColor Green
    Write-Host "Use -Force to reinstall" -ForegroundColor Yellow
    exit 0
}

# Clean build directories if requested
if ($Clean) {
    Write-Host "Cleaning build directories..." -ForegroundColor Yellow
    $buildDirs = @("build", "build_audio_playback", "build_audio_test", "build_integration", "build_midi_test")
    foreach ($dir in $buildDirs) {
        if (Test-Path $dir) {
            Remove-Item -Recurse -Force $dir
            Write-Host "  Removed $dir" -ForegroundColor Gray
        }
    }
}

# Find MinGW compiler
Write-Host "`nLocating MinGW compiler..." -ForegroundColor Cyan
$mingwPaths = @(
    "C:\msys64\mingw64\bin",
    "C:\mingw64\bin",
    "C:\Program Files\mingw-w64\mingw64\bin"
)

$mingwPath = $null
foreach ($path in $mingwPaths) {
    if (Test-Path "$path\g++.exe") {
        $mingwPath = $path
        break
    }
}

if (-not $mingwPath) {
    Write-Host "ERROR: MinGW not found!" -ForegroundColor Red
    Write-Host "Please install MSYS2 from https://www.msys2.org/" -ForegroundColor Yellow
    exit 1
}

Write-Host "  Found MinGW at: $mingwPath" -ForegroundColor Green

# Add MinGW to PATH for this session
$env:PATH = "$mingwPath;$env:PATH"

# Check for required libraries in MinGW
Write-Host "`nChecking MinGW libraries..." -ForegroundColor Cyan

$requiredLibs = @{
    "SDL2" = "$mingwPath\..\lib\libSDL2.dll.a"
    "SDL2_ttf" = "$mingwPath\..\lib\libSDL2_ttf.dll.a"
    "PortAudio" = "$mingwPath\..\lib\libportaudio.dll.a"
}

$missingLibs = @()
foreach ($lib in $requiredLibs.Keys) {
    $libPath = $requiredLibs[$lib]
    if (Test-Path $libPath) {
        Write-Host "  ✓ $lib found" -ForegroundColor Green
    } else {
        Write-Host "  ✗ $lib missing" -ForegroundColor Red
        $missingLibs += $lib
    }
}

# Install missing libraries via MSYS2
if ($missingLibs.Count -gt 0) {
    Write-Host "`nInstalling missing libraries via MSYS2..." -ForegroundColor Yellow
    
    $msys2Packages = @{
        "SDL2" = "mingw-w64-x86_64-SDL2"
        "SDL2_ttf" = "mingw-w64-x86_64-SDL2_ttf"
        "PortAudio" = "mingw-w64-x86_64-portaudio"
    }
    
    foreach ($lib in $missingLibs) {
        $package = $msys2Packages[$lib]
        Write-Host "  Installing $package..." -ForegroundColor Cyan
        
        & "C:\msys64\usr\bin\bash.exe" -lc "pacman -S --noconfirm $package"
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  ✓ $lib installed successfully" -ForegroundColor Green
        } else {
            Write-Host "  ✗ Failed to install $lib" -ForegroundColor Red
        }
    }
}

# Create CMake toolchain file for consistent builds
Write-Host "`nCreating CMake toolchain file..." -ForegroundColor Cyan

$mingwPathCMake = $mingwPath.Replace('\', '/')
$mingwParent = $mingwPathCMake + "/.."

$toolchainContent = @"
# MinGW Toolchain for Omega DAW
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER "$mingwPathCMake/gcc.exe")
set(CMAKE_CXX_COMPILER "$mingwPathCMake/g++.exe")
set(CMAKE_RC_COMPILER "$mingwPathCMake/windres.exe")

# Explicitly set the make program
find_program(CMAKE_MAKE_PROGRAM NAMES mingw32-make PATHS "$mingwPathCMake" NO_DEFAULT_PATH)
if(NOT CMAKE_MAKE_PROGRAM)
    set(CMAKE_MAKE_PROGRAM "$mingwPathCMake/mingw32-make.exe" CACHE FILEPATH "Make program" FORCE)
endif()

# Set library search paths
set(CMAKE_PREFIX_PATH "$mingwParent")
set(CMAKE_FIND_ROOT_PATH "$mingwParent")

# Adjust search behavior
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Set proper library suffixes
set(CMAKE_FIND_LIBRARY_SUFFIXES .dll.a .a .lib)
"@

$toolchainContent | Out-File -FilePath "cmake_toolchain.cmake" -Encoding UTF8

Write-Host "  Created cmake_toolchain.cmake" -ForegroundColor Green

# Create dependency marker
Write-Host "`nMarking dependencies as installed (version $DEPS_VERSION)..." -ForegroundColor Cyan
"Dependencies installed on $(Get-Date)" | Out-File -FilePath $DEPS_MARKER -Encoding UTF8

Write-Host "`n=== Dependency setup complete! ===" -ForegroundColor Green
Write-Host "`nNext steps:" -ForegroundColor Cyan
Write-Host "  1. Run: .\scripts\build.ps1" -ForegroundColor White
Write-Host "  2. Or manually: cmake -B build -G 'MinGW Makefiles' -DCMAKE_TOOLCHAIN_FILE=cmake_toolchain.cmake" -ForegroundColor White
