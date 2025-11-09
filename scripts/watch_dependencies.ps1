# Continuous monitoring script for dependency integrity
# Detects unauthorized changes to critical files

Write-Host "=== Omega DAW Dependency Monitor ===" -ForegroundColor Cyan
Write-Host "Monitoring critical directories for unauthorized changes..." -ForegroundColor Yellow
Write-Host "Press Ctrl+C to stop monitoring`n" -ForegroundColor Gray

$script_dir = Split-Path -Parent $MyInvocation.MyCommand.Path
$project_root = Split-Path -Parent $script_dir

# Directories to monitor
$watch_paths = @(
    (Join-Path $project_root "external"),
    (Join-Path $project_root "cmake"),
    (Join-Path $project_root "scripts")
)

# Create file system watchers
$watchers = @()
foreach ($path in $watch_paths) {
    if (Test-Path $path) {
        $watcher = New-Object System.IO.FileSystemWatcher
        $watcher.Path = $path
        $watcher.IncludeSubdirectories = $true
        $watcher.EnableRaisingEvents = $true
        
        # Watch for deletions
        $deleteAction = {
            $name = $Event.SourceEventArgs.Name
            $changeType = $Event.SourceEventArgs.ChangeType
            $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
            
            Write-Host "[$timestamp] WARNING: File deleted - $name" -ForegroundColor Red
            
            # Auto-backup on deletion
            Write-Host "Triggering automatic backup..." -ForegroundColor Yellow
            & "$using:script_dir\backup_dependencies.ps1"
        }
        
        Register-ObjectEvent -InputObject $watcher -EventName Deleted -Action $deleteAction | Out-Null
        
        $watchers += $watcher
        Write-Host "Monitoring: $path" -ForegroundColor Green
    }
}

Write-Host "`nMonitoring active. Log file: dependency_monitor.log" -ForegroundColor Cyan

try {
    while ($true) {
        Start-Sleep -Seconds 60
        # Periodic integrity check
        & "$script_dir\verify_integrity.ps1" | Out-File -FilePath (Join-Path $project_root "dependency_monitor.log") -Append
    }
} finally {
    # Cleanup
    foreach ($watcher in $watchers) {
        $watcher.EnableRaisingEvents = $false
        $watcher.Dispose()
    }
    Get-EventSubscriber | Unregister-Event
    Write-Host "`nMonitoring stopped" -ForegroundColor Yellow
}
