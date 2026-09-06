$ErrorActionPreference = 'Stop'

if (-not (Test-Path .\build)) { cmake.exe -B .\build }
cmake --build build --config Debug