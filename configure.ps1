Push-Location
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64
Pop-Location

$env:VCPKG_ROOT = scoop prefix vcpkg
