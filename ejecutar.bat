@echo off
set Path=C:\msys64\mingw64\bin;%Path%
cd /d "%~dp0"
set CMAKE_EXE=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe
if exist "%CMAKE_EXE%" (
    echo Compilando cambios...
    "%CMAKE_EXE%" --build build --config Release
    if errorlevel 1 (
        echo.
        echo No se pudo compilar. Cierra TIM_Grafica.exe si esta abierto y vuelve a intentar.
        pause
        exit /b 1
    )
)
echo Iniciando TIM Prototipo en tu sesion activa...
if exist "build\Release\TIM_Grafica.exe" (
    start "" "build\Release\TIM_Grafica.exe"
) else (
    start "" "build\TIM_Grafica.exe"
)
