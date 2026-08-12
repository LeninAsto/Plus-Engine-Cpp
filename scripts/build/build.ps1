param(
    [string]$Configuration = "Debug",
    [string]$Flags = ""
)

$ErrorActionPreference = "Stop"

$workspaceRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)

& (Join-Path $PSScriptRoot "build-core.ps1") -Configuration $Configuration
& (Join-Path $workspaceRoot "scripts\assets\sync-build-assets.ps1") -Flags $Flags
& (Join-Path $workspaceRoot "scripts\runtime\copy-runtime-plugins.ps1")

Write-Host "Build listo en $(Join-Path $workspaceRoot 'build\PlusEngine.exe')"
