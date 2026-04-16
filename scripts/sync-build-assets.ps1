param(
    [string]$Flags = ""
)

$workspaceRoot = Split-Path -Parent $PSScriptRoot
$buildRoot = Join-Path $workspaceRoot "build"
$buildAssets = Join-Path $buildRoot "assets"

function Get-EnabledFlags {
    param([string]$ExplicitFlags)

    $rawFlags = $ExplicitFlags
    if ([string]::IsNullOrWhiteSpace($rawFlags)) {
        $rawFlags = $env:PLUS_BUILD_FLAGS
    }
    if ([string]::IsNullOrWhiteSpace($rawFlags)) {
        $rawFlags = "officialBuild,TRANSLATIONS_ALLOWED,VIDEOS_ALLOWED,MODS_ALLOWED"
    }

    $parsed = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
    foreach ($flag in ($rawFlags -split ',')) {
        $trimmed = $flag.Trim()
        if (-not [string]::IsNullOrWhiteSpace($trimmed)) {
            [void]$parsed.Add($trimmed)
        }
    }

    if ($parsed.Contains("officialBuild")) {
        [void]$parsed.Add("TITLE_SCREEN_EASTER_EGG")
        [void]$parsed.Add("BASE_GAME_FILES")
    }

    return $parsed
}

function Reset-BuildTargets {
    $pathsToRemove = @(
        $buildAssets,
        (Join-Path $buildRoot "mods"),
        (Join-Path $buildRoot "sm"),
        (Join-Path $buildRoot "modsList.txt")
    )

    foreach ($path in $pathsToRemove) {
        if (Test-Path $path) {
            Remove-Item $path -Recurse -Force
        }
    }
}

function Copy-DirectoryTree {
    param(
        [string]$Source,
        [string]$Destination
    )

    if (-not (Test-Path $Source)) {
        return
    }

    New-Item -ItemType Directory -Force -Path $Destination | Out-Null
    Copy-Item -Path (Join-Path $Source '*') -Destination $Destination -Recurse -Force
}

function Copy-DirectoryContent {
    param(
        [string]$Source,
        [string]$Destination
    )

    if (-not (Test-Path $Source)) {
        return
    }

    New-Item -ItemType Directory -Force -Path $Destination | Out-Null
    Get-ChildItem -Path $Source -Force | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $Destination -Recurse -Force
    }
}

function Copy-FileIfExists {
    param(
        [string]$Source,
        [string]$Destination
    )

    if (-not (Test-Path $Source)) {
        return
    }

    $parent = Split-Path -Parent $Destination
    if ($parent) {
        New-Item -ItemType Directory -Force -Path $parent | Out-Null
    }
    Copy-Item -Path $Source -Destination $Destination -Force
}

$enabledFlags = Get-EnabledFlags -ExplicitFlags $Flags

Reset-BuildTargets
New-Item -ItemType Directory -Force -Path $buildAssets | Out-Null

Copy-DirectoryTree -Source (Join-Path $workspaceRoot "assets\fonts") -Destination (Join-Path $buildAssets "fonts")
Copy-DirectoryTree -Source (Join-Path $workspaceRoot "assets\shared") -Destination (Join-Path $buildAssets "shared")
Copy-DirectoryTree -Source (Join-Path $workspaceRoot "assets\embed") -Destination (Join-Path $buildAssets "embed")
Copy-DirectoryTree -Source (Join-Path $workspaceRoot "assets\songs") -Destination (Join-Path $buildAssets "songs")
Copy-DirectoryContent -Source (Join-Path $workspaceRoot "assets\week_assets") -Destination $buildAssets

if ($enabledFlags.Contains("TITLE_SCREEN_EASTER_EGG")) {
    Copy-DirectoryContent -Source (Join-Path $workspaceRoot "assets\secrets") -Destination (Join-Path $buildAssets "shared")
}

if ($enabledFlags.Contains("TRANSLATIONS_ALLOWED")) {
    Copy-DirectoryContent -Source (Join-Path $workspaceRoot "assets\translations") -Destination $buildAssets
}

if ($enabledFlags.Contains("BASE_GAME_FILES")) {
    Copy-DirectoryContent -Source (Join-Path $workspaceRoot "assets\base_game") -Destination $buildAssets
}

if ($enabledFlags.Contains("VIDEOS_ALLOWED")) {
    Copy-DirectoryTree -Source (Join-Path $workspaceRoot "assets\videos") -Destination (Join-Path $buildAssets "videos")
}

if ($enabledFlags.Contains("MODS_ALLOWED")) {
    Copy-DirectoryTree -Source (Join-Path $workspaceRoot "example_mods") -Destination (Join-Path $buildRoot "mods")
    Copy-DirectoryTree -Source (Join-Path $workspaceRoot "example_sm") -Destination (Join-Path $buildRoot "sm")
    Copy-FileIfExists -Source (Join-Path $workspaceRoot "list.txt") -Destination (Join-Path $buildRoot "modsList.txt")
}

Copy-FileIfExists -Source (Join-Path $workspaceRoot "art\readme.txt") -Destination (Join-Path $buildRoot "do NOT readme.txt")
Copy-FileIfExists -Source (Join-Path $workspaceRoot "alsoft.txt") -Destination (Join-Path $buildRoot "plugins\alsoft.ini")

$sortedFlags = $enabledFlags | Sort-Object
Write-Host "Assets sincronizados en $buildAssets"
Write-Host ("Flags activos: " + ($sortedFlags -join ", "))