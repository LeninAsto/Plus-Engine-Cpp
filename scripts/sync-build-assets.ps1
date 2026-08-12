param(
    [string]$Flags = ""
)

$script = Join-Path $PSScriptRoot "assets\sync-build-assets.ps1"
& $script -Flags $Flags
exit $LASTEXITCODE
