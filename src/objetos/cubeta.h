#pragma once
// ============================================================================
// Cubeta - recipiente dinamico simple para colgar de cuerdas.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../sistema/assets_extern.h"

class Cubeta : public EntidadFisica {
private:
    double ancho;
    double alto;

public:
    Cubeta(int id, Vector2D pos_inicial, double w = 58.0, double h = 52.0, double m = 25.0)
        : EntidadFisica(id, pos_inicial, m, TipoForma::AABB, false),
          ancho(w), alto(h) {
        set_restitucion(0.15);
        set_friccion(0.55);
        set_amortiguamiento(0.01);
        set_inercia((1.0 / 12.0) * m * (w * w + h * h));
        tipo_menu = TipoObjetoMenu::CUBETA;
    }

    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    Vector2D get_min() const override { return posicion; }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho, posicion.y + alto); }
    Vector2D get_punto_cuerda() const { return Vector2D(posicion.x + ancho * 0.5, posicion.y + 4.0); }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::CUBETA;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent CUBETA id=" << get_id() << serializar_base()
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

        DrawRectangleRec({px + 5.0f, py + 12.0f, pw - 10.0f, ph - 12.0f},
                         Color{112, 140, 155, 255});
        DrawRectangleLinesEx({px + 5.0f, py + 12.0f, pw - 10.0f, ph - 12.0f},
                             2.0f, Color{45, 55, 65, 255});
        DrawLineEx({px + 9.0f, py + 14.0f}, {px + pw * 0.5f, py + 4.0f},
                   2.5f, Color{215, 225, 230, 255});
        DrawLineEx({px + pw - 9.0f, py + 14.0f}, {px + pw * 0.5f, py + 4.0f},
                   2.5f, Color{215, 225, 230, 255});
        DrawCircle(static_cast<int>(px + pw * 0.5f), static_cast<int>(py + 4.0f),
                   5.0f, Color{35, 40, 45, 255});

        if (debug) {
            DrawRectangleLines(static_cast<int>(px), static_cast<int>(py),
                               static_cast<int>(pw), static_cast<int>(ph), GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Cubeta" tab=0 categoria=0
