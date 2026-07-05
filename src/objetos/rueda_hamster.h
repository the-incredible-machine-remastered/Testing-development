#pragma once
// ============================================================================
// RuedaHamster - Hamster wheel that starts spinning and generating rotation
// when hit by a moving physics object.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "raylib.h"
#include <cmath>

class RuedaHamster : public EntidadFisica {
private:
    double radio;
    bool hamster_corriendo;
    double hamster_anim_time;
    bool sentido_horario;

public:
    RuedaHamster(int id, Vector2D pos_inicial, double r = 35.0)
        : EntidadFisica(id, pos_inicial, 5.0, TipoForma::CIRCULO, false),
          radio(r), hamster_corriendo(false), hamster_anim_time(0.0), sentido_horario(false) {
        tipo_menu = TipoObjetoMenu::RUEDA_HAMSTER;
        set_inercia(0.5 * masa * radio * radio); // Cylinder/hoop inertia
        set_restitucion(0.2);
        set_friccion(0.6);
        velocidad_angular = 0.0;
        angulo = 0.0;
    }

    double get_radio() const { return radio; }
    bool is_hamster_corriendo() const { return hamster_corriendo; }
    void set_hamster_corriendo(bool c) { hamster_corriendo = c; }
    bool get_sentido_horario() const { return sentido_horario; }
    void set_sentido_horario(bool sh) { sentido_horario = sh; }
    void invertir_sentido() { sentido_horario = !sentido_horario; }
    double get_radio_eje() const override { return 10.0; }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::RUEDA_HAMSTER;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent RUEDA_HAMSTER id=" << get_id() << serializar_base()
           << " r=" << radio << " corr=" << (hamster_corriendo ? 1 : 0)
           << " sh=" << (sentido_horario ? 1 : 0);
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        double dist = (posicion - p).magnitud();
        return dist < radio + 10.0;
    }

    void on_collision(EntidadFisica* otro, const InfoColision& info) override {
        if (otro && !otro->get_es_estatico() && otro->get_masa() > 0.1) {
            hamster_corriendo = true;
        }
    }

    void actualizar_fisica(double dt) override {
        if (hamster_corriendo) {
            // Keep spinning at steady speed (counter-clockwise or clockwise)
            double velocidad_objetivo = sentido_horario ? 7.5 : -7.5;
            velocidad_angular += (velocidad_objetivo - velocidad_angular) * 4.0 * dt;
            hamster_anim_time += dt;
        } else {
            // Decelerate naturally, integrating torque_neto (e.g. if turned by a belt)
            double I = get_inercia();
            double alpha = (I > 0.0001) ? (torque_neto / I) : 0.0;
            velocidad_angular += alpha * dt;
            velocidad_angular *= 0.975;
        }

        angulo += velocidad_angular * dt;

        // Keep pinned to center
        velocidad = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        fuerza_neta = Vector2D(0.0, 0.0);
        torque_neto = 0.0;
    }

    void dibujar(bool debug) const override {
        int px = static_cast<int>(posicion.x);
        int py = static_cast<int>(posicion.y);
        float r = static_cast<float>(radio);

        // 1. Draw Support Stand (base structure)
        DrawLineEx({(float)px, (float)py}, {(float)px - 15.0f, (float)py + r + 8.0f}, 3.0f, Color{80, 85, 95, 255});
        DrawLineEx({(float)px, (float)py}, {(float)px + 15.0f, (float)py + r + 8.0f}, 3.0f, Color{80, 85, 95, 255});
        DrawRectangle(px - 25, py + static_cast<int>(r) + 6, 50, 4, Color{50, 52, 60, 255});

        // 2. Draw Wheel Cage (outer circular rim)
        DrawCircleSector({(float)px, (float)py}, r, 0, 360, 40, Color{110, 115, 128, 60}); // Semi-translucent cage fill
        DrawCircleLines(px, py, r, Color{120, 125, 135, 255});
        DrawCircleLines(px, py, r - 3.0f, Color{90, 95, 105, 255});

        // 3. Draw Spokes rotating with the wheel
        int num_spokes = 8;
        for (int i = 0; i < num_spokes; ++i) {
            double angle_spoke = angulo + (i * 2.0 * MathUtils::TIM_PI / num_spokes);
            float sx = px + r * std::cos(angle_spoke);
            float sy = py + r * std::sin(angle_spoke);
            DrawLineEx({(float)px, (float)py}, {sx, sy}, 1.5f, Color{130, 135, 145, 180});
        }

        // 4. Draw Center Pulley Hub (where belt connects)
        DrawCircle(px, py, 10, Color{45, 48, 52, 255});
        DrawCircleLines(px, py, 10, Color{160, 165, 175, 255});
        DrawCircle(px, py, 3, Color{220, 220, 230, 255});

        // 5. Draw Hamster inside the cage
        // Hamster runs inside the wheel. Its angular position is offset from the center.
        double hamster_angle = 0.0;
        if (hamster_corriendo) {
            // Keep running at a speed relative to animation time, oscillating slightly up and down
            double offset_side = sentido_horario ? 0.6 : -0.6;
            hamster_angle = -MathUtils::TIM_PI / 2.0 + offset_side + 0.3 * std::sin(hamster_anim_time * 12.0);
        } else {
            // Rest at the very bottom
            hamster_angle = MathUtils::TIM_PI / 2.0;
        }

        float h_dist = r - 12.0f; // Distance from center
        float hx = px + h_dist * std::cos(hamster_angle);
        float hy = py + h_dist * std::sin(hamster_angle);

        // Draw Hamster body
        Color hamster_color = hamster_corriendo ? Color{215, 145, 80, 255} : Color{180, 130, 90, 255};
        DrawCircle(static_cast<int>(hx), static_cast<int>(hy), 7.0f, hamster_color);
        
        // Head angle offset depends on direction of movement (sentido_horario)
        double head_offset = sentido_horario ? MathUtils::TIM_PI / 2.0 : -MathUtils::TIM_PI / 2.0;
        double head_angle = hamster_angle + head_offset;
        DrawCircle(static_cast<int>(hx + 4.0f * std::cos(head_angle)),
                   static_cast<int>(hy + 4.0f * std::sin(head_angle)), 4.5f, hamster_color); // Head
        
        // Draw Hamster ears
        DrawCircle(static_cast<int>(hx + 2.0f * std::cos(head_angle) - 3.0f * std::sin(head_angle)),
                   static_cast<int>(hy + 2.0f * std::sin(head_angle) + 3.0f * std::cos(head_angle)),
                   2.0f, Color{235, 180, 180, 255});
        
        // Draw little running legs if active (micro-animation!)
        if (hamster_corriendo) {
            float leg_cycle = std::sin(hamster_anim_time * 24.0);
            float leg_offset = leg_cycle * 3.0f;
            DrawLineEx({hx - 3.0f, hy + 5.0f}, {hx - 3.0f + leg_offset, hy + 9.0f}, 1.5f, hamster_color);
            DrawLineEx({hx + 3.0f, hy + 5.0f}, {hx + 3.0f - leg_offset, hy + 9.0f}, 1.5f, hamster_color);
        } else {
            // Dormido (asleep): draw zZZ particles!
            float sleep_cycle = std::fmod(GetTime(), 3.0f);
            if (sleep_cycle < 1.5f) {
                float z_offset_x = 10.0f + sleep_cycle * 8.0f;
                float z_offset_y = -8.0f - sleep_cycle * 12.0f;
                DrawText("zZ", static_cast<int>(hx + z_offset_x), static_cast<int>(hy + z_offset_y), 9, Color{140, 160, 190, 200});
            }
        }
    }
};

// TIM_MENU_SPAWN id=RUEDA_HAMSTER etiqueta="Rueda Hamster" tab=0 categoria=1
