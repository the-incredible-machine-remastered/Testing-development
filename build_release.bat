@echo off
set Path=C:\msys64\mingw64\bin;%Path%
cd /d "%~dp0"
echo Construyendo...
del /F /Q CMakeCache.txt
rmdir /S /Q CMakeFiles
"C:\Program Files\JetBrains\Clion\CLion 2024.3.5\bin\cmake\win\x64\bin\cmake.exe" -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release .
"C:\Program Files\JetBrains\Clion\CLion 2024.3.5\bin\cmake\win\x64\bin\cmake.exe" --build .
