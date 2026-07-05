#pragma once
// ============================================================================
// CintaTransportadora - Conveyor belt that moves objects on top of it
// when rotated.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "raylib.h"
#include <cmath>

class CintaTransportadora : public EntidadFisica {
private:
    double ancho;
    double alto;
    double radio_eje;

public:
    CintaTransportadora(int id, Vector2D pos_inicial, double w = 240.0, double h = 18.0)
        : EntidadFisica(id, pos_inicial, 8.0, TipoForma::AABB, false),
          ancho(w), alto(h), radio_eje(h / 2.0) {
        tipo_menu = TipoObjetoMenu::CINTA_TRANSPORTADORA;
        set_inercia(0.5 * masa * radio_eje * radio_eje * 1.5);
        set_restitucion(0.1);
        set_friccion(0.8); // High friction to grip balls
        velocidad_angular = 0.0;
        angulo = 0.0;
    }

    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    double get_radio_eje() const override { return radio_eje; }

    Vector2D get_min() const override {
        return posicion - Vector2D(ancho / 2.0, alto / 2.0);
    }

    Vector2D get_max() const override {
        return posicion + Vector2D(ancho / 2.0, alto / 2.0);
    }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::CINTA_TRANSPORTADORA;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent CINTA_TRANSPORTADORA id=" << get_id() << serializar_base()
           << " w=" << ancho << " h=" << alto;
        return ss.str();
    }

    Vector2D get_velocidad_en_punto(const Vector2D& punto_mundo) const override {
        double v_surf = velocidad_angular * radio_eje;
        if (punto_mundo.y < posicion.y) {
            // Top surface (moving forward/right with positive w)
            return velocidad + Vector2D(v_surf, 0.0);
        } else {
            // Bottom surface (moving backward/left)
            return velocidad + Vector2D(-v_surf, 0.0);
        }
    }

    void actualizar_fisica(double dt) override {
        // Integrate angular acceleration from torque: alpha = torque / I
        double I = get_inercia();
        double alpha = (I > 0.0001) ? (torque_neto / I) : 0.0;
        velocidad_angular += alpha * dt;

        angulo += velocidad_angular * dt;
        velocidad_angular *= 0.98; // Slow down naturally

        // Fixed in space
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
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float pw = static_cast<float>(ancho);
        float ph = static_cast<float>(alto);

        // Draw main conveyor body track background
        DrawRectangleRounded(
            { px - pw / 2.0f, py - ph / 2.0f, pw, ph },
            0.5f,
            4,
            Color{60, 64, 72, 255}
        );

        // Draw two rotating end rollers/wheels
        float roller_dist = pw / 2.0f - 10.0f;
        float roller_r = ph / 2.0f - 1.0f;
        float roller_positions[2] = { px - roller_dist, px + roller_dist };

        for (int j = 0; j < 2; ++j) {
            float rx = roller_positions[j];
            // Outer metal circle
            DrawCircle(static_cast<int>(rx), static_cast<int>(py), roller_r, Color{110, 115, 125, 255});
            DrawCircle(static_cast<int>(rx), static_cast<int>(py), roller_r - 1.5f, Color{80, 84, 92, 255});
            
            // Draw rotating spokes
            int num_spokes = 3;
            for (int s = 0; s < num_spokes; ++s) {
                double spoke_angle = angulo + (s * 2.0 * MathUtils::TIM_PI / num_spokes);
                float sx = rx + (roller_r - 2.0f) * std::cos(spoke_angle);
                float sy = py + (roller_r - 2.0f) * std::sin(spoke_angle);
                DrawLineEx({rx, py}, {sx, sy}, 1.2f, Color{150, 155, 165, 255});
            }
            DrawCircle(static_cast<int>(rx), static_cast<int>(py), 1.5f, Color{220, 220, 230, 255});
        }

        // Draw the belt track border
        DrawRectangleRoundedLinesEx(
            { px - pw / 2.0f, py - ph / 2.0f, pw, ph },
            0.5f,
            4,
            1.5f,
            Color{120, 125, 135, 255}
        );

        // Draw moving teeth/ridges on the belt (micro-animation!)
        float top_y = py - ph / 2.0f + 0.5f;
        float bottom_y = py + ph / 2.0f - 2.0f;
        float spacing = 16.0f;
        double offset = std::fmod(angulo * radio_eje, spacing);

        // Draw ridges on top belt surface
        for (float x = px - pw / 2.0f + 10.0f; x < px + pw / 2.0f - 10.0f; x += spacing) {
            float rx = x + static_cast<float>(offset);
            // Wrap around the edges
            if (rx > px + pw / 2.0f - 10.0f) rx -= (pw - 20.0f);
            if (rx < px - pw / 2.0f + 10.0f) rx += (pw - 20.0f);
            DrawRectangle(static_cast<int>(rx), static_cast<int>(top_y), 3, 1, Color{200, 140, 40, 255});
        }

        // Draw ridges on bottom belt surface (moving in reverse)
        for (float x = px - pw / 2.0f + 10.0f; x < px + pw / 2.0f - 10.0f; x += spacing) {
            float rx = x - static_cast<float>(offset);
            if (rx > px + pw / 2.0f - 10.0f) rx -= (pw - 20.0f);
            if (rx < px - pw / 2.0f + 10.0f) rx += (pw - 20.0f);
            DrawRectangle(static_cast<int>(rx), static_cast<int>(bottom_y), 3, 1, Color{200, 140, 40, 255});
        }

        // Draw center pulley hub (where belt connects from other objects)
        DrawCircle(px, py, 7, Color{45, 48, 52, 255});
        DrawCircleLines(px, py, 7, Color{160, 165, 175, 255});
        DrawCircle(px, py, 2.0f, Color{225, 225, 235, 255});
    }
};

// TIM_MENU_SPAWN id=CINTA_TRANSPORTADORA etiqueta="Cinta Transp." tab=1 categoria=0
