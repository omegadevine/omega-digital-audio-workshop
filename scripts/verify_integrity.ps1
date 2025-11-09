# Integrity verification script for Omega DAW
# Verifies all critical files and dependencies are present

Write-Host "=== Omega DAW Integrity Verification ===" -ForegroundColor Cyan

$script_dir = Split-Path -Parent $MyInvocation.MyCommand.Path
$project_root = Split-Path -Parent $script_dir

Set-Location $project_root

# Define critical paths
$critical_paths = @(
    "external\SDL2",
    "external\portaudio",
    "external\rtmidi",
    "cmake",
    "scripts",
    "src\audio",
    "src\midi",
    "src\ui",
    "CMakeLists.txt",
    "scripts\setup_dependencies.ps1",
    "scripts\build.ps1"
)

$missing_items = @()
$all_good = $true

Write-Host "`nChecking critical paths..." -ForegroundColor Yellow

foreach ($path in $critical_paths) {
    $full_path = Join-Path $project_root $path
    if (-not (Test-Path $full_path)) {
        Write-Host "[MISSING] $path" -ForegroundColor Red
        $missing_items += $path
        $all_good = $false
    } else {
        Write-Host "[OK] $path" -ForegroundColor Green
    }
}

# Check if build artifacts exist
Write-Host "`nChecking build artifacts..." -ForegroundColor Yellow
$build_exe = Join-Path $project_root "build\bin\OmegaDAW.exe"
if (Test-Path $build_exe) {
    $file_info = Get-Item $build_exe
    Write-Host "[OK] Executable found (Size: $($file_info.Length) bytes, Modified: $($file_info.LastWriteTime))" -ForegroundColor Green
} else {
    Write-Host "[INFO] Executable not built yet" -ForegroundColor Yellow
}

# Check DLL dependencies in build
$required_dlls = @(
    "build\bin\SDL2.dll",
    "build\bin\libportaudio-2.dll"
)

Write-Host "`nChecking runtime dependencies..." -ForegroundColor Yellow
foreach ($dll in $required_dlls) {
    $full_path = Join-Path $project_root $dll
    if (Test-Path $full_path) {
        Write-Host "[OK] $dll" -ForegroundColor Green
    } else {
        Write-Host "[MISSING] $dll" -ForegroundColor Red
        $all_good = $false
    }
}

# Generate integrity report
$report_path = Join-Path $project_root "integrity_report.txt"
$report_content = @"
Omega DAW Integrity Report
Generated: $(Get-Date)
==================================

Missing Items: $($missing_items.Count)
$($missing_items -join "`n")

Status: $(if ($all_good) { "ALL SYSTEMS OK" } else { "ISSUES DETECTED" })
"@

$report_content | Out-File -FilePath $report_path -Encoding UTF8

Write-Host "`n=== Summary ===" -ForegroundColor Cyan
if ($all_good -and $missing_items.Count -eq 0) {
    Write-Host "All critical components verified successfully!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "Found $($missing_items.Count) missing components" -ForegroundColor Red
    Write-Host "Run 'scripts\setup_dependencies.ps1' to restore dependencies" -ForegroundColor Yellow
    exit 1
}
