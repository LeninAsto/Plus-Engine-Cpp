$ErrorActionPreference = "Stop"

$workspaceRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$destination = Join-Path $workspaceRoot "build\plugins"

$candidateRoots = @()
if ($env:VCPKG_ROOT) {
    $candidateRoots += $env:VCPKG_ROOT
}
$candidateRoots += "C:\vcpkg"

$source = $null
foreach ($root in $candidateRoots) {
    $candidate = Join-Path $root "installed\x64-windows\bin"
    if (Test-Path $candidate) {
        $source = $candidate
        break
    }
}

if (-not $source) {
    $checked = ($candidateRoots | ForEach-Object { Join-Path $_ "installed\x64-windows\bin" }) -join ", "
    throw "No se encontro una carpeta valida de DLLs en: $checked"
}

New-Item -ItemType Directory -Force -Path $destination | Out-Null
Copy-Item (Join-Path $source "*.dll") $destination -Force

Write-Host "Runtime plugins copiados en $destination"
