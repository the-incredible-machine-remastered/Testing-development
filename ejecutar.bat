@echo off
set Path=C:\msys64\mingw64\bin;%Path%
cd /d "%~dp0"
echo Iniciando TIM Prototipo en tu sesion activa...
start "" "build\TIM_Grafica.exe"
