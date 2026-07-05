@echo off
set Path=C:\msys64\mingw64\bin;%Path%
cd /d "%~dp0"

echo ===================================================
echo Creando distribucion para TIM Remastered
echo ===================================================

:: Detectar CMake
set CMAKE_BIN="C:\Program Files\JetBrains\Clion\CLion 2024.3.5\bin\cmake\win\x64\bin\cmake.exe"
if not exist %CMAKE_BIN% (
    set CMAKE_BIN=cmake
)

echo.
echo [1/4] Compilando en modo Release con el Icono incrustado...
del /F /Q CMakeCache.txt >nul 2>&1
rmdir /S /Q CMakeFiles >nul 2>&1
%CMAKE_BIN% -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release .
if %ERRORLEVEL% neq 0 (
    echo ERROR: Fallo la configuracion de CMake.
    pause
    exit /b %ERRORLEVEL%
)

%CMAKE_BIN% --build .
if %ERRORLEVEL% neq 0 (
    echo ERROR: Fallo la construccion del proyecto.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [2/4] Creando estructura de carpetas de distribucion...
set DIST_DIR=TIM_Remastered_Dist
if exist %DIST_DIR% rmdir /S /Q %DIST_DIR%
mkdir %DIST_DIR%

:: Copiar ejecutable
if exist "TIM_Grafica.exe" (
    copy /Y "TIM_Grafica.exe" "%DIST_DIR%\"
) else (
    echo ERROR: No se encontro el ejecutable compilado.
    pause
    exit /b 1
)

:: Copiar recursos
echo Copiando Assets...
xcopy /E /I /Y "Assets" "%DIST_DIR%\Assets" >nul
echo Copiando Fuentes...
xcopy /E /I /Y "fonts" "%DIST_DIR%\fonts" >nul

:: Crear archivo de inicio rapido (Jugar.bat)
echo Creando lanzador Jugar.bat...
(
echo @echo off
echo cd /d "%%~dp0"
echo start "" "TIM_Grafica.exe"
) > "%DIST_DIR%\Jugar.bat"

echo.
echo [3/4] Comprimiendo a archivo ZIP...
if exist "TIM_Remastered.zip" del /F /Q "TIM_Remastered.zip"
powershell -Command "Compress-Archive -Path '%DIST_DIR%' -DestinationPath 'TIM_Remastered.zip' -Force"
if %ERRORLEVEL% neq 0 (
    echo ERROR: Fallo la compresion a ZIP.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo [4/4] Limpiando archivos temporales...
rmdir /S /Q %DIST_DIR%

echo.
echo ===================================================
echo ¡EXITO! Distribucion creada correctamente:
echo Archivo: TIM_Remastered.zip
echo ===================================================
pause
