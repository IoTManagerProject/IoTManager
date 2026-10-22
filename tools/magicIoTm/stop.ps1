# Stop magicIoTm server
$ErrorActionPreference = "SilentlyContinue"

# Get the directory where this script is located
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# [1] Kill python processes running app.py from this folder
Get-CimInstance Win32_Process -Filter "Name='python.exe'" | Where-Object {
    $_.CommandLine -like "*$scriptDir*app.py*"
} | ForEach-Object {
    Stop-Process -Id $_.ProcessId -Force
}

# [2] Kill cmd.exe windows running run.bat
#     Search by command line AND window title as fallback
$found = @()

# Method A: by command line - look for run.bat in the magicIoTm folder
$cmdProcs = Get-CimInstance Win32_Process -Filter "Name='cmd.exe'" -ErrorAction SilentlyContinue
if ($cmdProcs) {
    foreach ($p in $cmdProcs) {
        $cmd = $p.CommandLine
        if ($cmd -and ($cmd -like "*\magicIoTm\run.bat*" -or $cmd -like "*magicIoTm\run*") -and
                         ($cmd -notlike "*stop*") -and ($cmd -notlike "*restart*")) {
            $found += $p.ProcessId
        }
    }
}

# Method B: by window title (fallback)
$procList = Get-Process -ErrorAction SilentlyContinue
if ($procList) {
    foreach ($p in $procList) {
        if ($p.MainWindowTitle -like "*Configurator*" -and $p.Id -notin $found) {
            # Double-check it's not our own stop/restart window
            $cmdInfo = Get-CimInstance Win32_Process -Filter "ProcessId=$($p.Id)" -ErrorAction SilentlyContinue
            if ($cmdInfo -and $cmdInfo.CommandLine -like "*\magicIoTm\run*") {
                $found += $p.Id
            }
        }
    }
}

# Kill all found processes
if ($found) {
    foreach ($id in $found) {
        Stop-Process -Id $id -Force -ErrorAction SilentlyContinue
    }
}

# [4] Kill anything on port 5005
$connections = Get-NetTCPConnection -LocalPort 5005 -State Listen -ErrorAction SilentlyContinue
if ($connections) {
    $connections.OwningProcess | Select-Object -Unique | ForEach-Object {
        Stop-Process -Id $_ -Force
    }
}
