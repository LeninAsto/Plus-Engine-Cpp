# Building Plus-Engine-Cpp en Windows

Este documento explica qué necesitas para compilar este proyecto en Windows usando Visual Studio 2022 y el compilador de MSVC (`cl.exe`).

Actualmente el código depende de estas librerías externas:

- SDL2
- SDL2_image
- SDL2_mixer
- nlohmann-json

Por ahora el proyecto no necesita SDL_ttf.

## 1. Requisitos

Instala esto primero:

1. Visual Studio 2022
2. Workload `Desktop development with C++`
3. MSVC v143 build tools
4. Windows 10 SDK o Windows 11 SDK
5. Git
6. vcpkg

Componentes recomendados dentro de Visual Studio:

- MSVC v143 - VS 2022 C++ x64/x86 build tools
- Windows 10 SDK o Windows 11 SDK
- C++ CMake tools for Windows

Aunque quieras compilar directamente con `cl.exe`, tener las herramientas de CMake sigue siendo útil para depurar y migrar después sin drama innecesario.

## 2. Por qué el task actual todavía no compila

Las rutas de archivos fuente en `.vscode/tasks.json` ya están alineadas con el árbol actual del proyecto.

Si la compilación falla con errores como estos:

```text
fatal error C1083: cannot open include file: 'SDL2/SDL.h'
fatal error C1083: cannot open include file: 'SDL2/SDL_image.h'
fatal error C1083: cannot open include file: 'SDL2/SDL_mixer.h'
```

eso significa que el compilador todavía no conoce las carpetas de includes ni las carpetas de librerías de las dependencias externas.

## 3. Instalar dependencias con vcpkg

Abre PowerShell y ejecuta esto:

```powershell
cd C:\
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install

.\vcpkg install sdl2:x64-windows
.\vcpkg install sdl2-image:x64-windows
.\vcpkg install sdl2-mixer:x64-windows
.\vcpkg install nlohmann-json:x64-windows
```

Para verificar la instalación:

```powershell
C:\vcpkg\vcpkg list
```

Deberías ver paquetes parecidos a estos:

- `sdl2:x64-windows`
- `sdl2-image:x64-windows`
- `sdl2-mixer:x64-windows`
- `nlohmann-json:x64-windows`

## 4. Qué usa este proyecto exactamente

Estos archivos son la razón de esas dependencias:

- SDL2 para ventana y render: `source/core/Application.h`
- SDL2_image para cargar texturas: `source/graphics/Texture.h`
- SDL2_mixer para audio: `source/audio/MusicPlayer.h`
- nlohmann-json para parsear JSON: `source/data/JsonLoader.h`

## 5. Compilar con `cl.exe` en VS Code

Si tu instalación de vcpkg está en `C:\vcpkg`, agrega las rutas de includes y librerías a `.vscode/tasks.json`.

Los argumentos importantes son estos:

```json
"/IC:\\vcpkg\\installed\\x64-windows\\include",
"/link",
"/LIBPATH:C:\\vcpkg\\installed\\x64-windows\\lib",
"SDL2.lib",
"SDL2_image.lib",
"SDL2_mixer.lib",
"user32.lib",
"gdi32.lib",
"psapi.lib",
"gdiplus.lib",
"winmm.lib"
```

Notas importantes:

1. La ruta `/I...` debe ir antes de `/link`.
2. La ruta `/LIBPATH:...` debe ir después de `/link`.
3. Este proyecto está pensado para `x64`, no para `x86`.
4. Este proyecto usa `int main(...)` normal en `source/main.cpp`, así que no necesita `SDL2main.lib`.

## 6. Estructura sugerida para `tasks.json`

Si quieres un task manual con `cl.exe` que tenga sentido, la sección de argumentos debería seguir este patrón:

```json
"args": [
    "/Zi",
    "/EHsc",
    "/std:c++17",
    "/nologo",
    "/IC:\\vcpkg\\installed\\x64-windows\\include",
    "/Fo${workspaceFolder}\\build\\obj\\",
    "/Fd${workspaceFolder}\\build\\PlusEngine.pdb",

    "${workspaceFolder}\\source\\main.cpp",
    "${workspaceFolder}\\source\\audio\\Conductor.cpp",
    "${workspaceFolder}\\source\\audio\\MusicPlayer.cpp",
    "${workspaceFolder}\\source\\core\\Application.cpp",
    "${workspaceFolder}\\source\\core\\Logger.cpp",
    "${workspaceFolder}\\source\\core\\MusicBeatState.cpp",
    "${workspaceFolder}\\source\\core\\StateManager.cpp",
    "${workspaceFolder}\\source\\core\\SubStateManager.cpp",
    "${workspaceFolder}\\source\\data\\Paths.cpp",
    "${workspaceFolder}\\source\\graphics\\Alphabet.cpp",
    "${workspaceFolder}\\source\\graphics\\AnimatedSprite.cpp",
    "${workspaceFolder}\\source\\graphics\\Sprite.cpp",
    "${workspaceFolder}\\source\\graphics\\Texture.cpp",
    "${workspaceFolder}\\source\\ui\\CreditsState.cpp",
    "${workspaceFolder}\\source\\ui\\MainMenuState.cpp",
    "${workspaceFolder}\\source\\ui\\TitleState.cpp",
    "${workspaceFolder}\\source\\ui\\debug\\DebugOverlay.cpp",
    "${workspaceFolder}\\source\\ui\\transition\\FadeTransition.cpp",

    "/link",
    "/OUT:${workspaceFolder}\\build\\PlusEngine.exe",
    "/PDB:${workspaceFolder}\\build\\PlusEngine.pdb",
    "/ILK:${workspaceFolder}\\build\\PlusEngine.ilk",
    "/LIBPATH:C:\\vcpkg\\installed\\x64-windows\\lib",
    "SDL2.lib",
    "SDL2_image.lib",
    "SDL2_mixer.lib",
    "user32.lib",
    "gdi32.lib",
    "psapi.lib",
    "gdiplus.lib",
    "winmm.lib"
]
```

Si tu carpeta de vcpkg no está en `C:\vcpkg`, cambia esa ruta por la tuya.

## 7. DLLs en tiempo de ejecución

Compilar no basta. El ejecutable también necesita las DLLs correctas al arrancar.

Normalmente estarán aquí:

```text
C:\vcpkg\installed\x64-windows\bin
```

Tienes dos opciones:

1. Copiar las DLLs necesarias junto a `build\PlusEngine.exe`
2. Agregar `C:\vcpkg\installed\x64-windows\bin` al `PATH`

DLLs típicas involucradas:

- `SDL2.dll`
- `SDL2_image.dll`
- `SDL2_mixer.dll`

Dependiendo de cómo vcpkg haya construido SDL_mixer, también podrías necesitar DLLs extra relacionadas con codecs.

## 8. Compilar desde un Developer PowerShell

Para asegurarte de que `cl.exe` exista en el entorno, abre una de estas consolas:

- `x64 Native Tools Command Prompt for VS 2022`
- `Developer PowerShell for VS 2022`

Después compila desde la carpeta del proyecto.

Si `cl.exe` no se reconoce, entonces Visual Studio está instalado pero el entorno de desarrollo no está cargado.

## 9. Errores comunes

### `SDL2/SDL.h` no encontrado

Causa:
Falta la ruta de includes de vcpkg.

Solución:
Agrega:

```text
/IC:\vcpkg\installed\x64-windows\include
```

### `SDL2.lib` o `SDL2_image.lib` no encontrados

Causa:
Falta la ruta de librerías de vcpkg.

Solución:
Agrega:

```text
/LIBPATH:C:\vcpkg\installed\x64-windows\lib
```

### `SDL2main.lib` no se puede abrir

Causa:
La configuración del proyecto intenta enlazar una librería que no está presente en tu instalación actual de vcpkg.

Solución:
Elimina `SDL2main.lib` del task y enlaza solo `SDL2.lib`, `SDL2_image.lib` y `SDL2_mixer.lib`. Este proyecto ya define `main` directamente y no usa `SDL_main`.

### El programa compila pero no arranca

Causa:
Faltan DLLs en tiempo de ejecución.

Solución:
Copia las DLLs desde la carpeta `bin` de vcpkg junto al ejecutable, o agrega esa carpeta al `PATH`.

### `nlohmann/json.hpp` no encontrado

Causa:
Falta la ruta de includes, igual que con SDL.

Solución:
Asegúrate de que la misma ruta de includes de vcpkg esté presente.

## 10. Flujo recomendado en Visual Studio 2022

Si quieres seguir usando `cl.exe`, perfecto. Pero el flujo menos doloroso en Windows es este:

1. Instalar dependencias con vcpkg
2. Usar herramientas x64 de Visual Studio 2022
3. Mantener el task de VS Code para builds rápidos
4. Usar Visual Studio para depurar cuando haga falta

Si el código sigue creciendo, mover este task manual a un proyecto CMake te va a ahorrar mantenimiento y varios golpes contra la mesa.

## 11. Checklist rápido

Antes de esperar que el proyecto compile, confirma todo esto:

- Visual Studio 2022 está instalado
- `Desktop development with C++` está instalado
- MSVC v143 está instalado
- Windows SDK está instalado
- vcpkg está instalado
- `sdl2:x64-windows` está instalado
- `sdl2-image:x64-windows` está instalado
- `sdl2-mixer:x64-windows` está instalado
- `nlohmann-json:x64-windows` está instalado
- `.vscode/tasks.json` incluye `/I...include`
- `.vscode/tasks.json` incluye `/LIBPATH:...lib`
- Las DLLs de SDL están disponibles en runtime

Cuando todo eso esté listo, compilar con `cl.exe` debería dejar de fallar por headers o librerías faltantes.