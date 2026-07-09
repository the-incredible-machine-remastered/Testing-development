# TIM — Build para WebAssembly (Emscripten)

Esta carpeta contiene los scripts y archivos necesarios para compilar TIM a WebAssembly
y publicarlo en **GitHub Pages** (o cualquier servidor HTTP estático).

---

## Archivos de esta carpeta

| Archivo | Descripción |
|---|---|
| `build_web.bat` | Script de compilación para **Windows** |
| `build.sh` | Script de compilación para **Linux / macOS / WSL** |
| `index.html` | Página web del juego (plantilla con pantalla de carga) |
| `index.js` | *(generado)* Módulo JavaScript de Emscripten |
| `index.wasm` | *(generado)* Binario WebAssembly |
| `index.data` | *(generado)* Assets empaquetados (texturas, fuentes) |

> Los archivos `index.js`, `index.wasm` e `index.data` se generan al compilar.

---

## Requisitos previos

### 1. Instalar Emscripten SDK

```bash
# Clonar emsdk (solo la primera vez)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Instalar y activar la última versión
./emsdk install latest
./emsdk activate latest

# Activar variables de entorno (hacer esto en cada nueva terminal)
# Linux / macOS / WSL:
source ./emsdk_env.sh
# Windows (cmd):
emsdk_env.bat
# Windows (PowerShell):
.\emsdk_env.ps1
```

### 2. Python 3 en el PATH

Necesario para generar `catalogo_menu.gen.h` automáticamente.

---

## Compilar

### Windows (cmd / PowerShell)

```bat
REM Activar emsdk primero (desde la carpeta de emsdk):
emsdk_env.bat

REM Volver a la raíz del proyecto y compilar:
cd <ruta-del-proyecto>
web\build_web.bat
```

### Linux / macOS / WSL

```bash
# Activar emsdk primero:
source <ruta-a-emsdk>/emsdk_env.sh

# Compilar desde la raíz del proyecto:
bash web/build.sh
```

La **primera ejecución** descarga y compila Raylib 5.5 para WebAssembly (~2-5 minutos).
Las ejecuciones posteriores son mucho más rápidas.

---

## Probar localmente

Abrir `index.html` directamente en el navegador **no funciona** porque los navegadores
bloquean la carga de archivos locales por seguridad (CORS). Necesitas un servidor HTTP:

```bash
# Opción A: Python (recomendado)
python -m http.server 8000 -d web/

# Opción B: Node.js
npx serve web/

# Luego abrir en el navegador:
# http://localhost:8000
```

---

## Publicar en GitHub Pages

1. Haz commit de todos los archivos de `web/` (incluyendo `.js`, `.wasm`, `.data`):
   ```bash
   git add web/
   git commit -m "feat: build web con Emscripten"
   git push
   ```

2. En tu repositorio GitHub → **Settings** → **Pages**:
   - **Source**: `Deploy from a branch`
   - **Branch**: `main` (o la que uses)
   - **Folder**: `/web`

3. Después de unos minutos estará disponible en:
   `https://<usuario>.github.io/<repositorio>/`

---

## Notas sobre la build web vs nativa

| Característica | Build nativa | Build web |
|---|---|---|
| Guardar/cargar partidas `.tim` | ✅ Funcional | ❌ Deshabilitado* |
| Importar niveles con drag & drop | ✅ Funcional | ❌ Deshabilitado |
| Música de menú | ✅ Funcional | ❌ Excluida (WAV de 40 MB) |
| Redimensionar ventana | ✅ Funcional | Canvas de tamaño fijo |
| Multihilo | ✅ (no usado) | Compatible (1 solo hilo) |
| Rendimiento | ✅ Nativo | ~60-80% del nativo |

*En la build web, los botones de guardar/cargar mostrarán el mensaje
*"no disponible en la versión web"*.

------------
Para poder compilar todo esto se realizar de manera sencilla.
Solo tenemos que:
1. Activar el entorno emsdk y usando ese misma terminal ir al paso 2.
2. Ejecutar el script build_web.ps1 en la powershell, dentro de la carpeta docs.
3. El paso 2 generará los archivos web estáticos listos para subirlo. 
