#!/usr/bin/env bash
# =============================================================================
# build.sh — Compila TIM para WebAssembly con Emscripten (Linux / macOS / WSL)
#
# REQUISITOS:
#   1. emsdk instalado y activado:
#        git clone https://github.com/emscripten-core/emsdk.git
#        cd emsdk && ./emsdk install latest && ./emsdk activate latest
#        source ./emsdk_env.sh
#   2. Python 3 en el PATH
#
# USO (desde la raíz del proyecto):
#   bash web/build.sh
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
OUT="$SCRIPT_DIR"

# ── Verificar emcc ───────────────────────────────────────────────────────────
if ! command -v emcc &>/dev/null; then
    echo "[ERROR] emcc no encontrado. Activa emsdk primero:"
    echo "        source <ruta-a-emsdk>/emsdk_env.sh"
    exit 1
fi
echo "[INFO] Usando: $(emcc --version | head -1)"

# ── Generar catalogo_menu.gen.h ──────────────────────────────────────────────
if command -v python3 &>/dev/null; then
    echo "[INFO] Generando catalogo_menu.gen.h..."
    python3 "$ROOT/tools/generar_catalogo_menu.py"
fi

# ── Descargar Raylib 5.5 si no existe ───────────────────────────────────────
RAYLIB_SRC="$ROOT/raylib_web_src"
if [ ! -d "$RAYLIB_SRC" ]; then
    echo "[INFO] Descargando fuente de Raylib 5.5..."
    curl -L "https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz" -o /tmp/raylib55.tar.gz
    tar -xzf /tmp/raylib55.tar.gz -C "$ROOT"
    mv "$ROOT/raylib-5.5" "$RAYLIB_SRC"
fi

# ── Compilar libraylib para web (solo la primera vez) ───────────────────────
RAYLIB_LIB="$RAYLIB_SRC/src/libraylib_web.a"
if [ ! -f "$RAYLIB_LIB" ]; then
    echo "[INFO] Compilando Raylib para WebAssembly..."
    RAYLIB_FLAGS=(
        -D_DEFAULT_SOURCE
        -DPLATFORM_WEB
        -DGRAPHICS_API_OPENGL_ES2
        -I "$RAYLIB_SRC/src"
        -I "$RAYLIB_SRC/src/external"
        -I "$RAYLIB_SRC/src/external/glfw/include"
        -Os
    )
    SOURCES=(
        "$RAYLIB_SRC/src/rcore.c"
        "$RAYLIB_SRC/src/rshapes.c"
        "$RAYLIB_SRC/src/rtextures.c"
        "$RAYLIB_SRC/src/rtext.c"
        "$RAYLIB_SRC/src/rmodels.c"
        "$RAYLIB_SRC/src/raudio.c"
        "$RAYLIB_SRC/src/utils.c"
    )
    OBJS=()
    for src in "${SOURCES[@]}"; do
        obj="${RAYLIB_SRC}/src/$(basename ${src%.c}).o"
        emcc "${RAYLIB_FLAGS[@]}" -c "$src" -o "$obj"
        OBJS+=("$obj")
    done
    emar rcs "$RAYLIB_LIB" "${OBJS[@]}"
    echo "[INFO] libraylib_web.a lista."
fi

# ── Compilar el juego ────────────────────────────────────────────────────────
echo "[INFO] Compilando TIM..."
emcc "$ROOT/main.cpp" \
    -std=c++17 \
    -O2 \
    -I "$ROOT/src" \
    -I "$RAYLIB_SRC/src" \
    "$RAYLIB_LIB" \
    -D_DEFAULT_SOURCE \
    -DPLATFORM_WEB \
    -DGRAPHICS_API_OPENGL_ES2 \
    -s USE_GLFW=3 \
    -s ASYNCIFY=0 \
    -s TOTAL_MEMORY=268435456 \
    -s ALLOW_MEMORY_GROWTH=0 \
    -s ENVIRONMENT=web \
    -s EXPORTED_RUNTIME_METHODS="['cwrap']" \
    --preload-file "$ROOT/Assets@Assets" \
    --preload-file "$ROOT/fonts@fonts" \
    --exclude-file "*.wav" \
    -o "$OUT/index.js"

echo ""
echo "============================================================"
echo " BUILD EXITOSO"
echo " Archivos en: $OUT"
echo "   index.html   <- plantilla personalizada"
echo "   index.js"
echo "   index.wasm"
echo "   index.data   <- assets empaquetados"
echo "============================================================"
echo ""
echo " Para probar localmente:"
echo "   python3 -m http.server 8000 -d web/"
echo "   Abrir: http://localhost:8000"
