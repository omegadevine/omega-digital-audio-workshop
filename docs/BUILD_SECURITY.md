# Security Measures for Build System

## Overview

This document outlines the security measures implemented to protect the Omega DAW build system from interference, whether accidental or malicious.

## Protection Mechanisms

### 1. Persistent Dependency Storage

**Problem**: Dependencies being deleted and needing reinstallation on every build.

**Solution**: Dependencies are installed in the MSYS2 MinGW system directory (`C:\msys64\mingw64\`), which is:
- Outside the project directory
- Protected by system-level permissions
- Shared across all MSYS2 projects
- Not cleaned by build scripts

**Benefits**:
- Survives project directory cleaning
- Cannot be accidentally deleted by build scripts
- Reduces attack surface (no downloads during normal builds)

### 2. Version Tracking

**File**: `.deps_installed_X.X.X` (version marker)

**Purpose**:
- Tracks that dependencies are installed
- Prevents redundant installations
- Detects when dependencies need updating

**Protection**:
- Scripts check for this file before any build
- If missing, dependencies are verified/reinstalled
- Version number allows for controlled updates

### 3. Build Script Validation

**Setup Script** (`setup_dependencies.ps1`):
- Verifies compiler existence before proceeding
- Checks each library individually
- Only installs missing components
- Creates consistent CMake toolchain file
- Reports any failures clearly

**Build Script** (`build.ps1`):
- Always checks dependencies first (unless `-SkipDeps` used)
- Adds MinGW to PATH for consistent environment
- Validates configuration before building
- Reports errors immediately

### 4. CMake Toolchain Locking

**File**: `cmake_toolchain.cmake`

**Purpose**:
- Locks compiler paths (prevents wrong compiler usage)
- Sets consistent library search paths
- Ensures reproducible builds

**Protection**:
- Generated automatically by setup script
- Hard-coded paths prevent PATH manipulation attacks
- Explicit make program prevents tool confusion

### 5. Gitignore Protection

**Protected Items**:
```gitignore
.deps_installed_*    # Dependency markers
cmake_toolchain.cmake # Toolchain configuration (!)
```

**Note**: `cmake_toolchain.cmake` is actually tracked by git to share configuration across machines, but the generated version is machine-specific.

## Attack Vectors Addressed

### A) Accidental Deletion
- **Threat**: User runs cleanup script that removes dependencies
- **Mitigation**: Dependencies stored outside project directory

### B) Build Script Interference
- **Threat**: Scripts delete and forget to reinstall dependencies
- **Mitigation**: Version marker prevents reinstallation unless `-Force` used

### C) Path Manipulation
- **Threat**: Malicious PATH modification causes wrong tools to run
- **Mitigation**: Toolchain file hard-codes full paths to tools

### D) Library Version Conflicts
- **Threat**: Mixing different compiler/library versions causes subtle bugs
- **Mitigation**: All builds use same MinGW installation and libraries

### E) Missing Dependencies
- **Threat**: Build proceeds with missing dependencies, fails mysteriously
- **Mitigation**: Dependency check runs before every build

## Security Best Practices

### For Developers

1. **Never delete `.deps_installed_*` files** unless you want to reinstall
2. **Don't modify `cmake_toolchain.cmake` manually** - regenerate with setup script
3. **Use build scripts instead of direct CMake** - they include safety checks
4. **If build fails, check dependency status first**

### For CI/CD

1. Cache the `.deps_installed_*` marker between builds
2. Store MinGW installation in a persistent location
3. Only run `setup_dependencies.ps1 -Force` on new runners
4. Use `build.ps1 -SkipDeps` only if dependencies are definitely installed

### Recovery Procedures

#### Complete Reset (Nuclear Option)
```powershell
# Remove all build artifacts and markers
.\scripts\setup_dependencies.ps1 -Clean -Force

# Rebuild everything from scratch
.\scripts\build.ps1 -Clean
```

#### Dependency Verification
```powershell
# Check if dependencies are properly installed
.\scripts\setup_dependencies.ps1 -Force

# This will reinstall any missing dependencies
```

#### Quick Rebuild
```powershell
# Just rebuild without touching dependencies
.\scripts\build.ps1 -Clean
```

## Monitoring for Issues

### Signs of Dependency Problems

1. **Error**: "The procedure entry point ... could not be located"
   - **Cause**: Mixed library versions
   - **Fix**: `setup_dependencies.ps1 -Force -Clean`

2. **Error**: "CMake could not find ..."
   - **Cause**: Missing libraries
   - **Fix**: `setup_dependencies.ps1 -Force`

3. **Error**: "Compiler not able to compile test program"
   - **Cause**: Wrong compiler or missing make program
   - **Fix**: Check MSYS2 installation, reinstall if needed

### Logging

Build scripts provide detailed output:
- ✓ Green checkmarks indicate success
- ✗ Red X marks indicate failures
- Yellow warnings indicate potential issues

Save build logs for debugging:
```powershell
.\scripts\build.ps1 2>&1 | Tee-Object -FilePath build.log
```

## Future Enhancements

Potential security improvements:

1. **Cryptographic Verification**
   - Hash checking for downloaded dependencies
   - Signature verification for critical libraries

2. **Sandboxed Builds**
   - Container-based builds for isolation
   - Virtual environment per build

3. **Dependency Lock File**
   - Pin exact library versions
   - Detect unexpected version changes

4. **Build Reproducibility**
   - Deterministic builds from same source
   - Binary verification across machines

5. **Automated Security Scanning**
   - Dependency vulnerability scanning
   - Static analysis of build scripts

## Contact

For security concerns or to report vulnerabilities:
- Open an issue on GitHub (for non-sensitive matters)
- See [SECURITY.md](../SECURITY.md) for reporting sensitive issues
