# Omega DAW Build Script
# Ensures dependencies are installed before building

param(
    [switch]$Clean,
    [switch]$Release,
    [switch]$SkipDeps
)

$ErrorActionPreference = "Stop"

Write-Host "=== Omega DAW Build System ===" -ForegroundColor Cyan

# Check for dependencies first
if (-not $SkipDeps) {
    Write-Host "`nChecking dependencies..." -ForegroundColor Cyan
    
    if (-not (Test-Path ".deps_installed_1.0.0")) {
        Write-Host "Dependencies not installed. Running setup..." -ForegroundColor Yellow
        & "$PSScriptRoot\setup_dependencies.ps1"
        if ($LASTEXITCODE -ne 0) {
            Write-Host "ERROR: Dependency setup failed!" -ForegroundColor Red
            exit 1
        }
    } else {
        Write-Host "  ✓ Dependencies already installed" -ForegroundColor Green
    }
}

# Add MinGW to PATH
$mingwPath = "C:\msys64\mingw64\bin"
if (Test-Path $mingwPath) {
    $env:PATH = "$mingwPath;$env:PATH"
    Write-Host "  Added MinGW to PATH" -ForegroundColor Gray
}

# Set build type
$buildType = if ($Release) { "Release" } else { "Debug" }

# Clean if requested
if ($Clean -and (Test-Path "build")) {
    Write-Host "`nCleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force build
}

# Configure
Write-Host "`nConfiguring CMake ($buildType)..." -ForegroundColor Cyan

$configArgs = @(
    "-B", "build",
    "-G", "MinGW Makefiles",
    "-DCMAKE_BUILD_TYPE=$buildType"
)

if (Test-Path "cmake_toolchain.cmake") {
    $configArgs += "-DCMAKE_TOOLCHAIN_FILE=cmake_toolchain.cmake"
}

cmake @configArgs

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: CMake configuration failed!" -ForegroundColor Red
    exit 1
}

# Build
Write-Host "`nBuilding..." -ForegroundColor Cyan
cmake --build build --config $buildType -j 8

if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Build failed!" -ForegroundColor Red
    exit 1
}

Write-Host "`n=== Build complete! ===" -ForegroundColor Green
Write-Host "`nExecutable location: .\build\bin\OmegaDAW.exe" -ForegroundColor Cyan
Write-Host "`nTo run: .\build\bin\OmegaDAW.exe" -ForegroundColor White
