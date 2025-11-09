# Security Measures for Omega DAW

## Overview
This document describes the security measures implemented to protect the Omega DAW project from unauthorized modifications, dependency corruption, and build artifacts loss.

## Implemented Security Features

### 1. Git Hooks
Located in `.git/hooks/`

#### Pre-commit Hook
- Verifies critical directories exist before committing
- Warns about deletions of dependency files
- Requires confirmation for critical file deletions
- Prevents accidental removal of external libraries

#### Post-merge Hook
- Automatically checks dependency integrity after merging
- Restores missing dependencies automatically
- Ensures build directory structure is intact

### 2. Backup System

#### Automated Backups (`scripts/backup_dependencies.ps1`)
- Creates timestamped backups of:
  - External dependencies (SDL2, PortAudio, RtMidi)
  - Build artifacts (executables and DLLs)
  - CMake configuration
- Maintains rolling window of last 10 backups
- Generates manifest files for each backup

#### Restoration (`scripts/restore_dependencies.ps1`)
- Restores from most recent backup or specific timestamp
- Safely replaces corrupted dependencies
- Verifies restoration completion

Usage:
```powershell
# Restore from most recent backup
.\scripts\restore_dependencies.ps1

# Restore from specific backup
.\scripts\restore_dependencies.ps1 -BackupTimestamp "2025-01-09_12-30-00"
```

### 3. Integrity Verification

#### Verification Script (`scripts/verify_integrity.ps1`)
- Checks all critical paths and files
- Verifies build artifacts exist and are current
- Checks runtime dependencies (DLLs)
- Generates detailed integrity reports

Run verification:
```powershell
.\scripts\verify_integrity.ps1
```

#### Continuous Monitoring (`scripts/watch_dependencies.ps1`)
- Real-time monitoring of critical directories
- Detects unauthorized file deletions
- Triggers automatic backups on suspicious activity
- Periodic integrity checks every 60 seconds

Run monitor (leave running in background):
```powershell
.\scripts\watch_dependencies.ps1
```

### 4. Git Attributes
- Ensures consistent line endings across platforms
- Prevents binary file corruption during version control
- Protects against merge conflicts in generated files

### 5. Enhanced .gitignore
- Excludes backup directory from version control
- Prevents tracking of monitoring logs
- Allows critical DLLs in build/bin to be tracked

## Recommended Workflow

### Daily Development
1. Before starting work:
   ```powershell
   .\scripts\verify_integrity.ps1
   ```

2. Create backup before major changes:
   ```powershell
   .\scripts\backup_dependencies.ps1
   ```

3. If dependencies are corrupted:
   ```powershell
   .\scripts\restore_dependencies.ps1
   ```

### Continuous Protection
Run the monitoring script in a separate terminal:
```powershell
.\scripts\watch_dependencies.ps1
```

### After Git Operations
Git hooks automatically run:
- Pre-commit: Before each commit
- Post-merge: After git pull/merge

## Critical Directories Protected

1. **external/** - Third-party dependencies
   - SDL2
   - PortAudio
   - RtMidi

2. **cmake/** - Build configuration
   - FindSDL2.cmake
   - FindPortAudio.cmake
   - FindRtMidi.cmake

3. **scripts/** - Automation and security scripts
   - setup_dependencies.ps1
   - build.ps1
   - All security scripts

4. **build/bin/** - Runtime artifacts
   - OmegaDAW.exe
   - SDL2.dll
   - libportaudio-2.dll

## What to Do If Dependencies Are Lost

### Quick Recovery
```powershell
# Method 1: Restore from backup
.\scripts\restore_dependencies.ps1

# Method 2: Reinstall from scratch
.\scripts\setup_dependencies.ps1
```

### Manual Recovery
1. Check available backups:
   ```powershell
   Get-ChildItem backups\
   ```

2. Restore specific backup:
   ```powershell
   .\scripts\restore_dependencies.ps1 -BackupTimestamp "YYYY-MM-DD_HH-MM-SS"
   ```

3. Verify restoration:
   ```powershell
   .\scripts\verify_integrity.ps1
   ```

4. Rebuild if needed:
   ```powershell
   .\scripts\build.ps1
   ```

## Security Best Practices

1. **Regular Backups**: Run backup script before major changes
2. **Verify Integrity**: Check integrity after git operations
3. **Monitor Activity**: Keep watch script running during development
4. **Never Manually Delete**: Don't manually delete external/ or cmake/ directories
5. **Review Changes**: Check git status before committing

## Troubleshooting

### "Dependencies missing after git pull"
- Git hooks should auto-restore
- If not, run `.\scripts\restore_dependencies.ps1`

### "Build fails with missing DLL errors"
- Run `.\scripts\verify_integrity.ps1` to diagnose
- Restore from backup or reinstall dependencies

### "Suspicious file deletions detected"
- Check dependency_monitor.log
- Automatic backup should have been triggered
- Run integrity verification
- Restore from backup if needed

## Additional Protection

The system now includes:
- ✅ Git hooks for commit-time verification
- ✅ Automated backup system with rotation
- ✅ Integrity verification scripts
- ✅ Continuous monitoring capability
- ✅ Quick restoration tools
- ✅ Git attributes for file handling
- ✅ Enhanced .gitignore for critical files

## Future Enhancements

Consider adding:
- Cloud backup integration
- Hash-based file verification
- Automated testing after restoration
- Email/notification alerts for monitoring
- Encrypted backup storage
