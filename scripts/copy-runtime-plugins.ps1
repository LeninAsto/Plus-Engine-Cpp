$script = Join-Path $PSScriptRoot "runtime\copy-runtime-plugins.ps1"
& $script
exit $LASTEXITCODE
