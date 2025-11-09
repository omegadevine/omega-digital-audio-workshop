# Dependency Management Guide

## Overview

This project uses a persistent dependency management system to avoid redundant installations and build issues.

## How It Works

### 1. Dependency Tracking
- Dependencies are installed once and tracked with a version marker file (`.deps_installed_X.X.X`)
- The marker file prevents reinstallation unless explicitly requested
- All builds reuse the same installed dependencies

### 2. Build Scripts

#### Setup Dependencies
```powershell
.\scripts\setup_dependencies.ps1
```

Options:
- `-Force`: Reinstall dependencies even if already installed
- `-Clean`: Remove all build directories before setup

#### Build Project
```powershell
.\scripts\build.ps1
```

Options:
- `-Clean`: Clean build directory before building
- `-Release`: Build in Release mode (default is Debug)
- `-SkipDeps`: Skip dependency check (not recommended)

### 3. CMake Toolchain

The setup script creates `cmake_toolchain.cmake` which:
- Locks in compiler paths (MinGW)
- Sets consistent library search paths
- Ensures reproducible builds

## Security Measures

### Against Accidental Deletion
- Dependencies are installed in MinGW directory (outside project)
- Build scripts check for dependencies before proceeding
- Version tracking prevents silent dependency changes

### Against Malicious Interference
- All scripts validate dependency presence before building
- CMake toolchain file locks compiler and library paths
- Build failures are caught and reported clearly

## Troubleshooting

### Dependencies Keep Getting "Reinstalled"
- Check if `.deps_installed_*` file exists in project root
- If missing, dependencies will be checked/reinstalled
- Don't delete this file unless you want to reinstall

### Build Errors About Missing Libraries
1. Run: `.\scripts\setup_dependencies.ps1 -Force`
2. This will verify and reinstall all dependencies

### "Procedure entry point" Errors
- This usually means mixing different compiler/library versions
- Solution: `.\scripts\setup_dependencies.ps1 -Force -Clean`
- Then: `.\scripts\build.ps1 -Clean`

## Best Practices

1. **First Time Setup**
   ```powershell
   .\scripts\setup_dependencies.ps1
   .\scripts\build.ps1
   ```

2. **Normal Development**
   ```powershell
   .\scripts\build.ps1
   ```
   Dependencies are automatically checked but not reinstalled.

3. **After Pulling Changes**
   ```powershell
   .\scripts\build.ps1 -Clean
   ```
   Clean build with existing dependencies.

4. **Complete Reset**
   ```powershell
   .\scripts\setup_dependencies.ps1 -Force -Clean
   .\scripts\build.ps1
   ```
   Nuclear option: reinstall everything and rebuild from scratch.

## Technical Details

### Dependency Locations
- MinGW: `C:\msys64\mingw64\`
- Libraries: `C:\msys64\mingw64\lib\`
- Headers: `C:\msys64\mingw64\include\`

### Required Packages (MSYS2)
- `mingw-w64-x86_64-SDL2`
- `mingw-w64-x86_64-SDL2_ttf`
- `mingw-w64-x86_64-portaudio`
- `mingw-w64-x86_64-cmake`
- `mingw-w64-x86_64-gcc`

### Why This Approach?

**Problem**: CMake and build systems were reinstalling dependencies on every build, causing:
- Wasted time downloading/installing
- Version mismatches between builds
- Linking errors from mixed library versions
- Lost work from cleaned dependencies

**Solution**: Install dependencies once in system location, track with version markers, reuse across all builds.

## Future Improvements

Potential enhancements:
- Dependency version pinning
- Automatic MSYS2 installation
- Package lock file
- Binary dependency caching
- CI/CD integration
