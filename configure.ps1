Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 1. Load MSVC environment in current session
& "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

$VCPKG_ROOT = & scoop prefix vcpkg

$env:VCPKG_ROOT = $VCPKG_ROOT
