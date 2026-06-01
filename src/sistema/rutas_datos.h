#pragma once
// Resuelve rutas a Assets/ y fonts/ desde cwd o carpeta del ejecutable.

#include "raylib.h"
#include <string>
#include <cstring>

inline std::string g_raiz_datos;

inline std::string ruta_datos(const char* relativo) {
    return g_raiz_datos + relativo;
}

inline bool archivo_datos_existe(const char* relativo) {
    return FileExists(ruta_datos(relativo).c_str());
}

inline void inicializar_raiz_datos() {
    const char* cwd_prefijos[] = { "", "../", "../../", "../../../", "../../../../" };
    for (const char* prefijo : cwd_prefijos) {
        std::string prueba = std::string(prefijo) + "Assets/fondo1.png";
        if (FileExists(prueba.c_str())) {
            g_raiz_datos = prefijo;
            TraceLog(LOG_INFO, "TIM: raiz de datos (cwd) = '%s'", g_raiz_datos.c_str());
            return;
        }
    }

    const char* app = GetApplicationDirectory();
    const char* exe_prefijos[] = { "", "../", "../../", "../../../" };
    for (const char* prefijo : exe_prefijos) {
        std::string prueba = std::string(app) + prefijo + "Assets/fondo1.png";
        if (FileExists(prueba.c_str())) {
            g_raiz_datos = std::string(app) + prefijo;
            TraceLog(LOG_INFO, "TIM: raiz de datos (exe) = '%s'", g_raiz_datos.c_str());
            return;
        }
    }

    g_raiz_datos = "";
    TraceLog(LOG_WARNING,
        "TIM: no se encontro Assets/fondo1.png; rutas relativas sin prefijo");
}

inline Texture2D cargar_textura_datos(const char* relativo) {
    std::string path = ruta_datos(relativo);
    Texture2D tex = LoadTexture(path.c_str());
    if (tex.id == 0) {
        TraceLog(LOG_WARNING, "TIM: textura no cargada: %s", path.c_str());
    }
    return tex;
}

inline Font cargar_fuente_datos(const char* relativo, int fontSize,
                              int* firstCodepoint = nullptr, int codepointCount = 0) {
    std::string path = ruta_datos(relativo);
    if (firstCodepoint && codepointCount > 0) {
        return LoadFontEx(path.c_str(), fontSize, firstCodepoint, codepointCount);
    }
    return LoadFontEx(path.c_str(), fontSize, nullptr, 0);
}
