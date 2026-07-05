#pragma once
// ============================================================================
// Polea - A simple rotating wheel/pulley for transmitting rotation.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "raylib.h"
#include <cmath>

class Polea : public EntidadFisica {
private:
    double radio;

public:
    Polea(int id, Vector2D pos_inicial, double r = 18.0)
        : EntidadFisica(id, pos_inicial, 1.0, TipoForma::CIRCULO, false),
          radio(r) {
        tipo_menu = TipoObjetoMenu::NINGUNO;
        set_inercia(0.5 * masa * radio * radio);
        set_restitucion(0.2);
        set_friccion(0.5);
        velocidad_angular = 0.0;
        angulo = 0.0;
    }

    double get_radio() const { return radio; }
    double get_radio_eje() const override { return radio; }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::POLEA;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent POLEA id=" << get_id() << serializar_base()
           << " r=" << radio;
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        double dist = (posicion - p).magnitud();
        return dist < radio + 10.0;
    }

    void actualizar_fisica(double dt) override {
        angulo += velocidad_angular * dt;
        velocidad_angular *= 0.985; // Slow down naturally

        // Keep locked in place
        velocidad = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        fuerza_neta = Vector2D(0.0, 0.0);
        torque_neto = 0.0;
    }

    void dibujar(bool debug) const override {
        int px = static_cast<int>(posicion.x);
        int py = static_cast<int>(posicion.y);
        float r = static_cast<float>(radio);

        // Draw outer metallic rim
        DrawCircle(px, py, r, Color{100, 105, 115, 255});
        DrawCircle(px, py, r - 2.0f, Color{140, 145, 155, 255});
        DrawCircle(px, py, r - 4.0f, Color{85, 90, 100, 255});

        // Draw 3 classic pulley holes that rotate
        int num_holes = 3;
        float hole_dist = r * 0.45f;
        float hole_r = r * 0.22f;
        for (int i = 0; i < num_holes; ++i) {
            double hole_angle = angulo + (i * 2.0 * MathUtils::TIM_PI / num_holes);
            int hx = px + static_cast<int>(hole_dist * std::cos(hole_angle));
            int hy = py + static_cast<int>(hole_dist * std::sin(hole_angle));
            DrawCircle(hx, hy, hole_r, Color{60, 62, 70, 255}); // Dark inner hole cutout
        }

        // Draw central axle
        DrawCircle(px, py, 5.0f, Color{40, 42, 45, 255});
        DrawCircle(px, py, 2.0f, Color{200, 200, 210, 255});
    }
};

// (Disabled from menu) TIM_MENU_DISABLED id=POLEA
