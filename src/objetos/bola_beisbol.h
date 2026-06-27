#pragma once
#include "bola.h"

class BolaBeisbol : public Bola {
public:
    BolaBeisbol(int id, Vector2D pos, double r = 12.0, double m = 0.8)
        : Bola(id, pos, r, m) {
        set_restitucion(0.75);
        set_friccion(0.05);
        set_amortiguamiento(0.008);
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent BOLA_BEISBOL id=" << get_id() << serializar_base() << " r=" << get_radio();
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        return (p - posicion).magnitud() <= get_radio() + 8.0;
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float r  = static_cast<float>(get_radio());
        DrawCircle(static_cast<int>(px), static_cast<int>(py), r, Color{220, 220, 220, 220});
        DrawCircleLines(static_cast<int>(px), static_cast<int>(py), r, Color{50, 50, 50, 220});
        float angle1 = 0.4f;
        float angle2 = 3.14f - 0.4f;
        DrawLineEx({px + r * cosf(angle1), py + r * sinf(angle1)}, {px + r * cosf(angle2), py + r * sinf(angle2)}, 1.8f, Color{200, 30, 30, 180});
        DrawCircle(static_cast<int>(px - r * 0.35f), static_cast<int>(py - r * 0.35f), r * 0.25f, Color{255, 255, 255, 100});
        if (debug) DrawCircleLines(static_cast<int>(px), static_cast<int>(py), r, GREEN);
    }
};

// TIM_MENU_SPAWN etiqueta="Beisbol" tab=0 categoria=0
