# Script de PowerShell para crear el ZIP de distribucion
$distDir = "TIM_Remastered_Dist"
if (Test-Path $distDir) {
    Remove-Item -Recurse -Force $distDir
}
New-Item -ItemType Directory -Path $distDir | Out-Null

Write-Host "Copiando ejecutable..."
Copy-Item "TIM_Grafica.exe" "$distDir\"

Write-Host "Copiando Assets..."
Copy-Item -Recurse "Assets" "$distDir\"

Write-Host "Copiando fonts..."
Copy-Item -Recurse "fonts" "$distDir\"

Write-Host "Creando lanzador Jugar.bat..."
@'
@echo off
cd /d "%~dp0"
start "" "TIM_Grafica.exe"
'@ | Out-File -FilePath "$distDir\Jugar.bat" -Encoding ascii

Write-Host "Creando archivo ZIP..."
$zipPath = "TIM_Remastered.zip"
if (Test-Path $zipPath) {
    Remove-Item $zipPath
}
Compress-Archive -Path $distDir -DestinationPath $zipPath -Force

Write-Host "Limpiando archivos temporales..."
Remove-Item -Recurse -Force $distDir

Write-Host "========================================"
Write-Host "ZIP de distribucion creado con exito:"
Write-Host "Archivo: TIM_Remastered.zip"
Write-Host "========================================"
