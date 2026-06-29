# =============================================================================
# build_web.ps1 - Compila TIM para WebAssembly con Emscripten
#
# REQUISITOS: emcc activado en esta terminal (emsdk ya activado).
#
# USO (desde la raiz del proyecto o desde web/):
#   .\web\build_web.ps1
#   -- o desde dentro de web/ --
#   .\build_web.ps1
# =============================================================================

$ErrorActionPreference = "Stop"

# -- Rutas base ----------------------------------------------------------------
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$root      = Resolve-Path "$scriptDir\.."
$out       = $scriptDir
$raylibSrc = "$root\raylib_web_src"
$raylibLib = "$raylibSrc\src\libraylib_web.a"

Write-Host "[TIM-WEB] Directorio raiz : $root"
Write-Host "[TIM-WEB] Salida          : $out"
Write-Host ""

# -- Verificar emcc ------------------------------------------------------------
try {
    $ver = & emcc --version 2>&1 | Select-Object -First 1
    Write-Host "[TIM-WEB] $ver"
} catch {
    Write-Error "[ERROR] emcc no encontrado. Activa emsdk antes de ejecutar este script."
    exit 1
}

# -- Generar catalogo_menu.gen.h -----------------------------------------------
if (Get-Command python -ErrorAction SilentlyContinue) {
    Write-Host "[TIM-WEB] Generando catalogo_menu.gen.h..."
    & python "$root\tools\generar_catalogo_menu.py"
}

# -- Descargar Raylib 5.5 si no existe -----------------------------------------
if (-not (Test-Path $raylibSrc)) {
    Write-Host "[TIM-WEB] Descargando Raylib 5.5 fuente..."
    $zip = "$env:TEMP\raylib55.zip"
    Invoke-WebRequest -Uri "https://github.com/raysan5/raylib/archive/refs/tags/5.5.zip" -OutFile $zip
    Expand-Archive -Path $zip -DestinationPath $root -Force
    Rename-Item "$root\raylib-5.5" "raylib_web_src"
    Write-Host "[TIM-WEB] Raylib descargado."
}

# -- Compilar libraylib para web (solo la primera vez) -------------------------
if (-not (Test-Path $raylibLib)) {
    Write-Host "[TIM-WEB] Compilando Raylib para WebAssembly (puede tardar unos minutos)..."

    # NOTA sobre flags de Raylib para Emscripten:
    # - Solo incluimos "$raylibSrc\src" (NO external/ ni external/glfw/).
    #   Si se incluyera external/, el #include <dirent.h> del else-branch POSIX
    #   resolveria a external/dirent.h (el de Windows) en lugar del dirent.h
    #   del sysroot de Emscripten, causando error con <io.h>.
    # - USE_GLFW=3 en el linker hace que Emscripten provea su propio GLFW.
    # - PLATFORM_WEB hace que Raylib use el backend web (no Windows/Desktop).
    $rflags = @(
        "-D_DEFAULT_SOURCE",
        "-DPLATFORM_WEB",
        "-DGRAPHICS_API_OPENGL_ES2",
        "-I", "$raylibSrc\src",
        "-Os"
    )

    $sources = @("rcore","rshapes","rtextures","rtext","rmodels","raudio","utils")
    foreach ($s in $sources) {
        Write-Host "  Compilando $s.c..."
        & emcc -c @rflags "$raylibSrc\src\$s.c" -o "$raylibSrc\src\$s.o"
        if ($LASTEXITCODE -ne 0) { Write-Error "Fallo compilando $s.c"; exit 1 }
    }

    $objs = $sources | ForEach-Object { "$raylibSrc\src\$_.o" }
    & emar rcs $raylibLib @objs
    if ($LASTEXITCODE -ne 0) { Write-Error "Fallo creando libraylib_web.a"; exit 1 }

    Write-Host "[TIM-WEB] libraylib_web.a lista."
}

# -- Compilar el juego ---------------------------------------------------------
Write-Host "[TIM-WEB] Compilando main.cpp..."

$emccArgs = @(
    "$root\main.cpp",
    "-std=c++17",
    "-O2",
    "-I", "$root\src",
    "-I", "$raylibSrc\src",
    $raylibLib,
    "-D_DEFAULT_SOURCE",
    "-DPLATFORM_WEB",
    "-DGRAPHICS_API_OPENGL_ES2",
    "-s", "USE_GLFW=3",
    "-s", "ASYNCIFY=0",
    "-s", "TOTAL_MEMORY=268435456",
    "-s", "ALLOW_MEMORY_GROWTH=0",
    "-s", "ENVIRONMENT=web",
    "-s", "EXPORTED_RUNTIME_METHODS=['cwrap']",
    "--preload-file", "$root\Assets@Assets",
    "--preload-file", "$root\fonts@fonts",
    "--exclude-file", "*.wav",
    "-o", "$out\index.js"
)

& emcc @emccArgs
if ($LASTEXITCODE -ne 0) {
    Write-Error "[ERROR] Compilacion fallida."
    exit 1
}

Write-Host ""
Write-Host "============================================================"
Write-Host " BUILD EXITOSO   -   Archivos en: $out"
Write-Host "   index.html     <- pagina web del juego"
Write-Host "   index.js"
Write-Host "   index.wasm"
Write-Host "   index.data     <- assets empaquetados"
Write-Host "============================================================"
Write-Host ""
Write-Host " Para probar localmente:"
Write-Host "   python -m http.server 8000 -d web"
Write-Host "   Abrir: http://localhost:8000"