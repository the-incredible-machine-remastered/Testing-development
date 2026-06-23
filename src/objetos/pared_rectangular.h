#pragma once
// ============================================================================
// ParedRectangular — AABB (Axis-Aligned Bounding Box)
// Útil para: límites del nivel, plataformas, ladrillos.
// ============================================================================

#include "obstaculo_estatico.h"
#include "../sistema/assets_extern.h"

class ParedRectangular : public ObstaculoEstatico {
protected:
    double ancho;
    double alto;

public:
    ParedRectangular(int id, Vector2D pos_inicial, double w, double h)
        : ObstaculoEstatico(id, pos_inicial, TipoForma::AABB), ancho(w), alto(h) {}

    // --- Getters ---
    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }

    // --- Setters (útil al redimensionar la ventana) ---
    void set_dimensiones(double w, double h) { ancho = w; alto = h; }

    // Esquina superior-izquierda (min) y esquina inferior-derecha (max)
    // En coordenadas de pantalla: Y+ apunta hacia abajo.
    Vector2D get_min() const override { return posicion; }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho, posicion.y + alto); }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::PARED;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent PARED id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " w=" << ancho << " h=" << alto;
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - 10 && p.x <= posicion.x + ancho + 10 &&
               p.y >= posicion.y - 10 && p.y <= posicion.y + alto + 10;
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float pw = static_cast<float>(ancho);
        float ph = static_cast<float>(alto);

        bool texturizado = false;
        if (!es_borde_nivel(this)) {
            if (pw > ph) {
                if (tex_plata_larga.id > 0) {
                    DrawTexturePro(
                        tex_plata_larga,
                        { 0, 0, (float)tex_plata_larga.width, (float)tex_plata_larga.height },
                        { px, py, pw, ph },
                        { 0, 0 },
                        0.0f,
                        WHITE
                    );
                    texturizado = true;
                }
            } else {
                if (tex_plata_peque.id > 0) {
                    DrawTexturePro(
                        tex_plata_peque,
                        { 0, 0, (float)tex_plata_peque.width, (float)tex_plata_peque.height },
                        { px, py, pw, ph },
                        { 0, 0 },
                        0.0f,
                        WHITE
                    );
                    texturizado = true;
                }
            }
        }

        if (!texturizado) {
            DrawRectangle(static_cast<int>(px), static_cast<int>(py), static_cast<int>(pw), static_cast<int>(ph), COLOR_PARED);
            DrawRectangleLines(static_cast<int>(px), static_cast<int>(py), static_cast<int>(pw), static_cast<int>(ph), COLOR_PARED_BORDE);
        }
    }
};

// TIM_MENU_SPAWN id=PLATAFORMA etiqueta="Plataforma" tab=0 categoria=0 variante=plataforma
// TIM_MENU_SPAWN id=PARED_LARGA etiqueta="Pared" tab=0 categoria=0 variante=pared
// TIM_MENU_SPAWN id=PLATAFORMA_DECOR etiqueta="Ladrillo" tab=1 categoria=0 variante=decor_plataforma

