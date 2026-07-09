#pragma once
// ============================================================================
// Ladrillo — bloque sólido que SOLO la Dinamita puede destruir. Hereda de
// ParedRectangular, así se redimensiona con los handles del mouse igual que una
// pared (y con las flechas). Al explotar una dinamita en su radio, el motor lo
// elimina (usa get_destruido/destruir).
// ============================================================================

#include "pared_rectangular.h"
#include "../sistema/assets_extern.h"
#include <cmath>
#include <algorithm>

class Ladrillo : public ParedRectangular {
private:
    bool destruido;

public:
    // Clase BASE de LadrilloVertical/Horizontal. El "Ladrillo" normal ya no se coloca en
    // el menú; solo existen las variantes V y H, que fijan su propio tipo_menu en su
    // constructor. Aquí tipo_menu queda NINGUNO por defecto.
    Ladrillo(int id, Vector2D pos, double w = 60.0, double h = 40.0)
        : ParedRectangular(id, pos, w, h, TipoObjetoMenu::NINGUNO), destruido(false) {
        set_restitucion(0.1);
        set_friccion(0.6);
    }

    bool get_destruido() const { return destruido; }
    void destruir() { destruido = true; }
    Vector2D get_centro() const { return Vector2D(posicion.x + ancho * 0.5, posicion.y + alto * 0.5); }

    // get_ancho/get_alto/get_min/get_max/set_dimensiones se heredan de ParedRectangular.

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::LADRILLO; }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent LADRILLO id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " w=" << ancho << " h=" << alto
           << " fijo=" << (es_fijo ? 1 : 0);
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - 6 && p.x <= posicion.x + ancho + 6 &&
               p.y >= posicion.y - 6 && p.y <= posicion.y + alto + 6;
    }

    void dibujar(bool debug) const override {
        if (destruido) return;
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float w  = static_cast<float>(ancho);
        float h  = static_cast<float>(alto);

        Texture2D tex;
        if (w > h * 1.5f) {
            tex = tex_ladrillos_horizontal;
        } else if (h > w * 1.5f) {
            tex = tex_ladrillos_vertical;
        } else {
            tex = tex_ladrillos_cuadrado;
        }

        if (tex.id > 0) {
            // Mosaico (tiling) con baldosa cuadrada fija: tomamos una porcion
            // cuadrada de la textura y la repetimos, para que el patron de ladrillos
            // se vea uniforme y no se estire al redimensionar. El borde se recorta
            // proporcionalmente (src parcial) para no deformar.
            const float BALDOSA = 48.0f; // tamaño en pantalla de cada baldosa
            // Lado cuadrado de la textura fuente que corresponde a una baldosa.
            float lado_src = static_cast<float>(std::min(tex.width, tex.height));
            for (float oy = 0.0f; oy < h; oy += BALDOSA) {
                float ch = std::min(BALDOSA, h - oy);        // alto en pantalla
                float sh = lado_src * (ch / BALDOSA);        // alto recortado en textura
                for (float ox = 0.0f; ox < w; ox += BALDOSA) {
                    float cw = std::min(BALDOSA, w - ox);    // ancho en pantalla
                    float sw = lado_src * (cw / BALDOSA);    // ancho recortado en textura
                    Rectangle src = {0.0f, 0.0f, sw, sh};
                    Rectangle dst = {px + ox, py + oy, cw, ch};
                    DrawTexturePro(tex, src, dst, {0, 0}, 0.0f, WHITE);
                }
            }
        } else {
            Color rojo   = Color{170, 70, 55, 255};
            Color mortero = Color{205, 195, 180, 255};
            Color borde  = Color{110, 45, 35, 255};

            DrawRectangleRec({px, py, w, h}, mortero);

            // Filas de ladrillos con junta desfasada (tamaño de ladrillo ~fijo)
            float bh = 14.0f;                        // alto de fila objetivo
            int filas = std::max(1, (int)std::round(h / bh));
            bh = h / filas;
            float bw = 30.0f;                        // ancho de ladrillo objetivo
            for (int fila = 0; fila < filas; ++fila) {
                float ry = py + fila * bh;
                float offset = (fila % 2 == 0) ? 0.0f : bw * 0.5f;
                for (float bx = px - offset; bx < px + w; bx += bw) {
                    float x0 = std::max(bx, px);
                    float x1 = std::min(bx + bw - 2.0f, px + w);
                    if (x1 > x0)
                        DrawRectangleRec({x0, ry + 1.0f, x1 - x0, bh - 2.0f}, rojo);
                }
            }
            DrawRectangleLinesEx({px, py, w, h}, 2.0f, borde);
        }

        if (debug) DrawRectangleLines((int)px, (int)py, (int)w, (int)h, GREEN);
    }
};

// (El "Ladrillo" normal ya no está en el menú; solo Ladrillo V y H, en sus propios headers.)
