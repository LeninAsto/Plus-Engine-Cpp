param(
    [string]$Configuration = "Debug"
)

$ErrorActionPreference = "Stop"

$workspaceRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$buildRoot = Join-Path $workspaceRoot "build"
$objRoot = Join-Path $buildRoot "obj"
$vcpkgRoot = if ($env:VCPKG_ROOT) { $env:VCPKG_ROOT } else { "C:\vcpkg" }
$vcvars = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"

if (-not (Test-Path $vcvars)) {
    throw "No se encontro vcvars64.bat en '$vcvars'. Instala Visual Studio Build Tools con MSVC x64."
}

if (-not (Test-Path (Join-Path $vcpkgRoot "installed\x64-windows\include"))) {
    throw "No se encontraron includes de vcpkg en '$vcpkgRoot'. Ejecuta scripts\setup\install-vcpkg-deps.ps1."
}

New-Item -ItemType Directory -Force -Path $objRoot | Out-Null

$sources = @(
    "source\main.cpp",
    "source\backend\Conductor.cpp",
    "source\backend\MusicPlayer.cpp",
    "source\backend\SoundPlayer.cpp",
    "source\backend\VocalsPlayer.cpp",
    "source\backend\Application.cpp",
    "source\backend\Logger.cpp",
    "source\backend\MemoryManager.cpp",
    "source\backend\MusicBeatState.cpp",
    "source\backend\StateManager.cpp",
    "source\backend\SubStateManager.cpp",
    "source\backend\Paths.cpp",
    "source\backend\OpenGLESBackend.cpp",
    "source\backend\RenderText.cpp",
    "source\backend\debug\DebugOverlay.cpp",
    "source\objects\Alphabet.cpp",
    "source\objects\AnimatedSprite.cpp",
    "source\objects\Sprite.cpp",
    "source\objects\Texture.cpp",
    "source\objects\Character.cpp",
    "source\objects\HoldSplash.cpp",
    "source\objects\Note.cpp",
    "source\objects\NoteSplash.cpp",
    "source\objects\SongChart.cpp",
    "source\objects\Stage.cpp",
    "source\objects\StrumNote.cpp",
    "source\states\CreditsState.cpp",
    "source\states\FreeplayState.cpp",
    "source\states\LoadingState.cpp",
    "source\states\MainMenuState.cpp",
    "source\states\PlayState.cpp",
    "source\states\TitleState.cpp",
    "source\substates\FadeTransition.cpp"
)

$compileFlags = @("/EHsc", "/std:c++17", "/nologo")
if ($Configuration -ieq "Release") {
    $compileFlags += @("/O2", "/DNDEBUG")
} else {
    $compileFlags += @("/Zi")
}

$args = @()
$args += $compileFlags
$args += "/I$($vcpkgRoot)\installed\x64-windows\include"
$args += "/Fo$($objRoot)\"
$args += "/Fd$($buildRoot)\PlusEngine.pdb"
$args += $sources
$args += "/link"
$args += "/OUT:$($buildRoot)\PlusEngine.exe"
$args += "/PDB:$($buildRoot)\PlusEngine.pdb"
$args += "/ILK:$($buildRoot)\PlusEngine.ilk"
$args += "/LIBPATH:$($vcpkgRoot)\installed\x64-windows\lib"
$args += @(
    "/DELAYLOAD:SDL2.dll",
    "/DELAYLOAD:SDL2_image.dll",
    "/DELAYLOAD:SDL2_mixer.dll",
    "/DELAYLOAD:SDL2_ttf.dll",
    "SDL2.lib",
    "SDL2_image.lib",
    "SDL2_mixer.lib",
    "SDL2_ttf.lib",
    "delayimp.lib",
    "user32.lib",
    "gdi32.lib",
    "psapi.lib",
    "gdiplus.lib",
    "winmm.lib"
)

$command = 'call "' + $vcvars + '" && cd /d "' + $workspaceRoot + '" && cl.exe ' + ($args -join ' ')
cmd /c $command
if ($LASTEXITCODE -ne 0) {
    throw "MSVC build failed with exit code $LASTEXITCODE."
}
