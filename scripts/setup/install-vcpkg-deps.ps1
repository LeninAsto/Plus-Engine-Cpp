$ErrorActionPreference = "Stop"

$vcpkgRoot = if ($env:VCPKG_ROOT) { $env:VCPKG_ROOT } else { "C:\vcpkg" }

if (-not (Test-Path $vcpkgRoot)) {
    git clone --depth 1 https://github.com/Microsoft/vcpkg.git $vcpkgRoot
}

if (-not (Test-Path (Join-Path $vcpkgRoot "vcpkg.exe"))) {
    & (Join-Path $vcpkgRoot "bootstrap-vcpkg.bat")
}

& (Join-Path $vcpkgRoot "vcpkg.exe") install `
    sdl2:x64-windows `
    sdl2-image:x64-windows `
    sdl2-mixer:x64-windows `
    sdl2-ttf:x64-windows `
    nlohmann-json:x64-windows
