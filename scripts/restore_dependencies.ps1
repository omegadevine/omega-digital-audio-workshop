# Restore dependencies from backup
# Usage: .\restore_dependencies.ps1 [backup_timestamp]

param(
    [string]$BackupTimestamp = ""
)

Write-Host "=== Omega DAW Dependency Restoration ===" -ForegroundColor Cyan

$script_dir = Split-Path -Parent $MyInvocation.MyCommand.Path
$project_root = Split-Path -Parent $script_dir
$backup_root = Join-Path $project_root "backups"

# Find backup to restore
if ($BackupTimestamp -eq "") {
    # Use most recent backup
    $all_backups = Get-ChildItem -Path $backup_root -Directory | Sort-Object Name -Descending
    if ($all_backups.Count -eq 0) {
        Write-Host "ERROR: No backups found!" -ForegroundColor Red
        exit 1
    }
    $backup_dir = $all_backups[0].FullName
    Write-Host "Using most recent backup: $($all_backups[0].Name)" -ForegroundColor Yellow
} else {
    $backup_dir = Join-Path $backup_root $BackupTimestamp
    if (-not (Test-Path $backup_dir)) {
        Write-Host "ERROR: Backup not found: $BackupTimestamp" -ForegroundColor Red
        exit 1
    }
}

Write-Host "Restoring from: $backup_dir" -ForegroundColor Yellow

# Restore external dependencies
$backup_external = Join-Path $backup_dir "external"
if (Test-Path $backup_external) {
    Write-Host "Restoring external dependencies..." -ForegroundColor Yellow
    $external_dir = Join-Path $project_root "external"
    
    if (Test-Path $external_dir) {
        Write-Host "Removing existing external directory..." -ForegroundColor Gray
        Remove-Item -Path $external_dir -Recurse -Force
    }
    
    Copy-Item -Path $backup_external -Destination $external_dir -Recurse -Force
    Write-Host "[OK] External dependencies restored" -ForegroundColor Green
}

# Restore build artifacts
$backup_bin = Join-Path $backup_dir "bin"
if (Test-Path $backup_bin) {
    Write-Host "Restoring build artifacts..." -ForegroundColor Yellow
    $build_bin = Join-Path $project_root "build\bin"
    
    New-Item -ItemType Directory -Path $build_bin -Force | Out-Null
    
    Get-ChildItem -Path $backup_bin -File | ForEach-Object {
        Copy-Item $_.FullName -Destination $build_bin -Force
    }
    Write-Host "[OK] Build artifacts restored" -ForegroundColor Green
}

# Restore CMake configuration
$backup_cmake = Join-Path $backup_dir "cmake"
if (Test-Path $backup_cmake) {
    Write-Host "Restoring CMake configuration..." -ForegroundColor Yellow
    $cmake_dir = Join-Path $project_root "cmake"
    
    if (Test-Path $cmake_dir) {
        Remove-Item -Path $cmake_dir -Recurse -Force
    }
    
    Copy-Item -Path $backup_cmake -Destination $cmake_dir -Recurse -Force
    Write-Host "[OK] CMake configuration restored" -ForegroundColor Green
}

Write-Host "`n=== Restoration Complete ===" -ForegroundColor Green
Write-Host "Run 'scripts\verify_integrity.ps1' to verify restoration" -ForegroundColor Cyan
