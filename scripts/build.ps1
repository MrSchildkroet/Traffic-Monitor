$ErrorActionPreference = 'Stop'

$PROJECT_ROOT = Split-Path -Parent $PSScriptRoot
$BUILD_DIR = Join-Path $PROJECT_ROOT 'build'

if (-not (Test-Path $BUILD_DIR)) { 
    cmake.exe -S $PROJECT_ROOT -B $BUILD_DIR 
}

cmake.exe --build $BUILD_DIR --config Debug