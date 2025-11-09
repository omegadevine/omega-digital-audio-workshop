# Backup script for dependencies and build artifacts
# Creates timestamped backups to prevent data loss

Write-Host "=== Omega DAW Dependency Backup ===" -ForegroundColor Cyan

$script_dir = Split-Path -Parent $MyInvocation.MyCommand.Path
$project_root = Split-Path -Parent $script_dir
$backup_root = Join-Path $project_root "backups"

# Create backup directory with timestamp
$timestamp = Get-Date -Format "yyyy-MM-dd_HH-mm-ss"
$backup_dir = Join-Path $backup_root $timestamp

Write-Host "Creating backup at: $backup_dir" -ForegroundColor Yellow
New-Item -ItemType Directory -Path $backup_dir -Force | Out-Null

# Backup external dependencies
Write-Host "Backing up dependencies..." -ForegroundColor Yellow
$external_dir = Join-Path $project_root "external"
if (Test-Path $external_dir) {
    $backup_external = Join-Path $backup_dir "external"
    Copy-Item -Path $external_dir -Destination $backup_external -Recurse -Force
    Write-Host "[OK] External dependencies backed up" -ForegroundColor Green
}

# Backup build artifacts (executable and DLLs only)
Write-Host "Backing up build artifacts..." -ForegroundColor Yellow
$build_bin = Join-Path $project_root "build\bin"
if (Test-Path $build_bin) {
    $backup_bin = Join-Path $backup_dir "bin"
    New-Item -ItemType Directory -Path $backup_bin -Force | Out-Null
    
    # Copy only executables and DLLs
    Get-ChildItem -Path $build_bin -Include @("*.exe", "*.dll") -Recurse | ForEach-Object {
        Copy-Item $_.FullName -Destination $backup_bin -Force
    }
    Write-Host "[OK] Build artifacts backed up" -ForegroundColor Green
}

# Backup CMake configuration
Write-Host "Backing up CMake configuration..." -ForegroundColor Yellow
$cmake_dir = Join-Path $project_root "cmake"
if (Test-Path $cmake_dir) {
    $backup_cmake = Join-Path $backup_dir "cmake"
    Copy-Item -Path $cmake_dir -Destination $backup_cmake -Recurse -Force
    Write-Host "[OK] CMake configuration backed up" -ForegroundColor Green
}

# Create manifest file
$manifest_path = Join-Path $backup_dir "manifest.txt"
$manifest_content = @"
Omega DAW Backup Manifest
Created: $(Get-Date)
==================================

Backup Contents:
- External dependencies (SDL2, PortAudio, RtMidi)
- Build artifacts (executables and DLLs)
- CMake configuration

To restore from this backup:
1. Copy external/ to project root
2. Copy bin/ contents to build/bin/
3. Rebuild project if needed

"@
$manifest_content | Out-File -FilePath $manifest_path -Encoding UTF8

# Cleanup old backups (keep last 10)
Write-Host "Cleaning up old backups..." -ForegroundColor Yellow
$all_backups = Get-ChildItem -Path $backup_root -Directory | Sort-Object Name -Descending
if ($all_backups.Count -gt 10) {
    $old_backups = $all_backups | Select-Object -Skip 10
    foreach ($old_backup in $old_backups) {
        Remove-Item -Path $old_backup.FullName -Recurse -Force
        Write-Host "Removed old backup: $($old_backup.Name)" -ForegroundColor Gray
    }
}

Write-Host "`n=== Backup Complete ===" -ForegroundColor Green
Write-Host "Backup location: $backup_dir" -ForegroundColor Cyan
Write-Host "Total size: $([math]::Round((Get-ChildItem -Path $backup_dir -Recurse | Measure-Object -Property Length -Sum).Sum / 1MB, 2)) MB" -ForegroundColor Cyan
