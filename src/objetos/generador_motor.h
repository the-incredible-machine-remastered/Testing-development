#pragma once
// ============================================================================
// GeneradorMotor - Generator that produces electricity when rotated.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "raylib.h"
#include <cmath>

class GeneradorMotor : public EntidadFisica {
private:
    double ancho;
    double alto;
    bool generando_electricidad;

public:
    GeneradorMotor(int id, Vector2D pos_inicial, double w = 60.0, double h = 50.0)
        : EntidadFisica(id, pos_inicial, 10.0, TipoForma::AABB, false),
          ancho(w), alto(h), generando_electricidad(false) {
        tipo_menu = TipoObjetoMenu::GENERADOR_MOTOR;
        set_inercia(0.5 * masa * 15.0 * 15.0 * 2.0); // inertia of its spinning rotor
        set_restitucion(0.2);
        set_friccion(0.4);
        velocidad_angular = 0.0;
        angulo = 0.0;
    }

    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    bool is_generando_electricidad() const { return generando_electricidad; }
    double get_radio_eje() const override { return 15.0; }

    Vector2D get_min() const override {
        return posicion - Vector2D(ancho / 2.0, alto / 2.0);
    }

    Vector2D get_max() const override {
        return posicion + Vector2D(ancho / 2.0, alto / 2.0);
    }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::GENERADOR_MOTOR;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent GENERADOR_MOTOR id=" << get_id() << serializar_base()
           << " w=" << ancho << " h=" << alto;
        return ss.str();
    }

    void actualizar_fisica(double dt) override {
        // Integrate angular acceleration from torque: alpha = torque / I
        double I = get_inercia();
        double alpha = (I > 0.0001) ? (torque_neto / I) : 0.0;
        velocidad_angular += alpha * dt;

        angulo += velocidad_angular * dt;
        generando_electricidad = (std::abs(velocidad_angular) > 0.4);
        velocidad_angular *= 0.975; // Rotor friction damping

        // Locked in place
        velocidad = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        fuerza_neta = Vector2D(0.0, 0.0);
        torque_neto = 0.0;
    }

    bool contiene_punto(const Vector2D& p) const override {
        Vector2D min = get_min();
        Vector2D max = get_max();
        return p.x >= min.x - 5 && p.x <= max.x + 5 &&
               p.y >= min.y - 5 && p.y <= max.y + 5;
    }

    void dibujar(bool debug) const override {
        int px = static_cast<int>(posicion.x);
        int py = static_cast<int>(posicion.y);
        int w = static_cast<int>(ancho);
        int h = static_cast<int>(alto);

        // Draw electrical terminals on top
        DrawCircle(px - 14, py - h/2 - 2, 4, Color{220, 50, 50, 255}); // Positive (Red)
        DrawCircle(px + 14, py - h/2 - 2, 4, Color{40, 40, 40, 255});  // Negative (Black)

        // Draw main generator casing (a heavy rectangular motor casing)
        DrawRectangleGradientH(px - w/2, py - h/2, w, h, Color{50, 70, 95, 255}, Color{35, 50, 70, 255});
        DrawRectangleLines(px - w/2, py - h/2, w, h, Color{80, 100, 130, 255});

        // Draw cooling fins on the motor casing (vertical ridges)
        int num_fins = 5;
        int fin_w = 4;
        int spacing = (w - 16) / (num_fins - 1);
        for (int i = 0; i < num_fins; ++i) {
            int fx = px - w/2 + 8 + i * spacing - fin_w/2;
            DrawRectangle(fx, py - h/2 + 4, fin_w, h - 8, Color{25, 35, 50, 180});
        }

        // Draw the front axle pulley (where the belt connects)
        float axle_r = 15.0f;
        // Outer wheel
        DrawCircle(px, py, axle_r, Color{110, 115, 125, 255});
        DrawCircle(px, py, axle_r - 2.0f, Color{80, 84, 92, 255});
        
        // Rotating spoke indicators
        int num_spokes = 3;
        for (int s = 0; s < num_spokes; ++s) {
            double spoke_angle = angulo + (s * 2.0 * MathUtils::TIM_PI / num_spokes);
            float sx = px + (axle_r - 2.0f) * std::cos(spoke_angle);
            float sy = py + (axle_r - 2.0f) * std::sin(spoke_angle);
            DrawLineEx({(float)px, (float)py}, {sx, sy}, 1.5f, Color{160, 165, 175, 255});
        }
        DrawCircle(px, py, 3, Color{30, 32, 35, 255});

        // If generating electricity, draw awesome sparks and lightning flashes
        if (generando_electricidad) {
            // Neon glowing ring around the axle
            DrawCircleLines(px, py, axle_r + 5.0f, Color{0, 220, 255, 110});

            // Spark effects at terminals
            float time = static_cast<float>(GetTime());
            int seed = static_cast<int>(time * 40.0f);
            int ty = py - h/2 - 2;

            if (seed % 3 == 0) {
                // Spark from red terminal
                float sx = px - 14.0f + (seed % 11 - 5);
                float sy = ty - 4.0f - (seed % 7);
                DrawLineEx({px - 14.0f, (float)ty}, {sx, sy}, 1.5f, Color{255, 240, 100, 255});
                DrawCircle(static_cast<int>(sx), static_cast<int>(sy), 1.5f, YELLOW);
            }
            if (seed % 3 == 1) {
                // Spark from black terminal
                float sx = px + 14.0f + (seed % 9 - 4);
                float sy = ty - 4.0f - (seed % 8);
                DrawLineEx({px + 14.0f, (float)ty}, {sx, sy}, 1.5f, Color{100, 230, 255, 255});
                DrawCircle(static_cast<int>(sx), static_cast<int>(sy), 1.5f, SKYBLUE);
            }
        }
    }
};

// TIM_MENU_SPAWN id=GENERADOR_MOTOR etiqueta="Generador" tab=0 categoria=1
