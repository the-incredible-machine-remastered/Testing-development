@echo off
setlocal EnableDelayedExpansion

REM =============================================================================
REM build_web.bat - Compila TIM para WebAssembly con Emscripten
REM
REM REQUISITOS: emcc debe estar activado en la terminal antes de ejecutar.
REM   Si usas PowerShell ejecuta en su lugar: .\build_web.ps1
REM
REM USO (desde la raiz del proyecto):
REM   web\build_web.bat
REM =============================================================================

REM -- Rutas base ---------------------------------------------------------------
set "ROOT=%~dp0.."
set "OUT=%~dp0"
set "RAYLIB_SRC=%ROOT%\raylib_web_src"
set "RAYLIB_LIB=%RAYLIB_SRC%\src\libraylib_web.a"

echo [TIM-WEB] Directorio raiz: %ROOT%
echo [TIM-WEB] Salida:          %OUT%
echo.

REM -- Verificar emcc -----------------------------------------------------------
emcc --version >nul 2>&1
if errorlevel 1 (
    echo [ERROR] emcc no responde. Asegurate de haber activado emsdk:
    echo         Si usas PowerShell: ejecuta .\build_web.ps1 en vez de este .bat
    echo         Si usas cmd:        llama a emsdk_env.bat antes de este script
    exit /b 1
)
for /f "tokens=*" %%v in ('emcc --version 2^>nul ^| findstr /i "emcc"') do echo [TIM-WEB] %%v

REM -- Generar catalogo_menu.gen.h ----------------------------------------------
python --version >nul 2>&1
if not errorlevel 1 (
    echo [TIM-WEB] Generando catalogo_menu.gen.h...
    python "%ROOT%\tools\generar_catalogo_menu.py"
)

REM -- Descargar Raylib 5.5 si no existe ----------------------------------------
if not exist "%RAYLIB_SRC%" (
    echo [TIM-WEB] Descargando Raylib 5.5 fuente...
    powershell -NoProfile -Command ^
        "Invoke-WebRequest -Uri https://github.com/raysan5/raylib/archive/refs/tags/5.5.zip -OutFile '%TEMP%\raylib55.zip'; Expand-Archive '%TEMP%\raylib55.zip' -DestinationPath '%ROOT%'"
    if exist "%ROOT%\raylib-5.5" rename "%ROOT%\raylib-5.5" raylib_web_src
    echo [TIM-WEB] Raylib descargado.
)

REM -- Compilar libraylib para web (solo la primera vez) ------------------------
if not exist "%RAYLIB_LIB%" (
    echo [TIM-WEB] Compilando Raylib para WebAssembly (puede tardar unos minutos)...

    set "RFLAGS=-D_DEFAULT_SOURCE -DPLATFORM_WEB -DGRAPHICS_API_OPENGL_ES2"
    set "RFLAGS=!RFLAGS! -I %RAYLIB_SRC%\src"
    set "RFLAGS=!RFLAGS! -Os"


    emcc -c !RFLAGS! "%RAYLIB_SRC%\src\rcore.c"     -o "%RAYLIB_SRC%\src\rcore.o"
    emcc -c !RFLAGS! "%RAYLIB_SRC%\src\rshapes.c"   -o "%RAYLIB_SRC%\src\rshapes.o"
    emcc -c !RFLAGS! "%RAYLIB_SRC%\src\rtextures.c" -o "%RAYLIB_SRC%\src\rtextures.o"
    emcc -c !RFLAGS! "%RAYLIB_SRC%\src\rtext.c"     -o "%RAYLIB_SRC%\src\rtext.o"
    emcc -c !RFLAGS! "%RAYLIB_SRC%\src\rmodels.c"   -o "%RAYLIB_SRC%\src\rmodels.o"
    emcc -c !RFLAGS! "%RAYLIB_SRC%\src\raudio.c"    -o "%RAYLIB_SRC%\src\raudio.o"
    emcc -c !RFLAGS! "%RAYLIB_SRC%\src\utils.c"     -o "%RAYLIB_SRC%\src\utils.o"

    emar rcs "%RAYLIB_LIB%" ^
        "%RAYLIB_SRC%\src\rcore.o" ^
        "%RAYLIB_SRC%\src\rshapes.o" ^
        "%RAYLIB_SRC%\src\rtextures.o" ^
        "%RAYLIB_SRC%\src\rtext.o" ^
        "%RAYLIB_SRC%\src\rmodels.o" ^
        "%RAYLIB_SRC%\src\raudio.o" ^
        "%RAYLIB_SRC%\src\utils.o"

    echo [TIM-WEB] libraylib_web.a lista.
)

REM -- Compilar el juego --------------------------------------------------------
echo [TIM-WEB] Compilando main.cpp...

emcc "%ROOT%\main.cpp" ^
    -std=c++17 ^
    -O2 ^
    -I "%ROOT%\src" ^
    -I "%RAYLIB_SRC%\src" ^
    "%RAYLIB_LIB%" ^
    -D_DEFAULT_SOURCE ^
    -DPLATFORM_WEB ^
    -DGRAPHICS_API_OPENGL_ES2 ^
    -sUSE_GLFW=3 ^
    -sUSE_WEBGL2=1 ^
    -sASYNCIFY=0 ^
    -sINITIAL_MEMORY=536870912 ^
    -sALLOW_MEMORY_GROWTH=1 ^
    -sENVIRONMENT=web ^
    -sEXPORTED_RUNTIME_METHODS="['cwrap']" ^
    -sEXPORTED_FUNCTIONS="['_main','_tim_resume_audio']" ^
    -sMINIFY_HTML=0 ^
    --preload-file "%ROOT%\Assets@Assets" ^
    --preload-file "%ROOT%\fonts@fonts" ^
    --exclude-file "*.wav" ^
    -o "%OUT%index.js"

if errorlevel 1 (
    echo.
    echo [ERROR] Compilacion fallida. Revisa los mensajes de arriba.
    exit /b 1
)

echo.
echo ============================================================
echo  BUILD EXITOSO   -   Archivos en: %OUT%
echo    index.html     ^<-- pagina web del juego
echo    index.js
echo    index.wasm
echo    index.data     ^<-- assets empaquetados
echo ============================================================
echo.
echo  Para probar localmente:
echo    python -m http.server 8000 --directory "%OUT%"
echo    Abrir: http://localhost:8000
echo.

endlocal