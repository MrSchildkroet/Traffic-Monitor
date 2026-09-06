#Requires -RunAsAdministrator

$ErrorActionPreference = 'Stop'

$PROJECT_ROOT = Split-Path -Parent $PSScriptRoot
$EXECUTABLE = Join-Path $PROJECT_ROOT 'build\Debug\traffic_monitor.exe'

& $EXECUTABLE @args