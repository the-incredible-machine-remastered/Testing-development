#pragma once
// ============================================================================
// Trampolin — Objeto estático con superficie elástica de alto rebote hacia arriba
// ============================================================================

#include "obstaculo_estatico.h"
#include "../sistema/assets_extern.h"
#include "../fisica/colisiones.h"
#include "bola.h"

class Trampolin : public ObstaculoEstatico {
protected:
    double ancho;
    double alto;
    double fuerza_rebote; // Velocidad hacia arriba a aplicar (ej. 600.0)
    double deformacion;          // Deformación actual de la lona en px
    double velocidad_deformacion; // Velocidad de la oscilación de la lona

public:
    Trampolin(int id, Vector2D pos_inicial, double w = 80.0, double h = 20.0, double fuerza = 500.0)
        : ObstaculoEstatico(id, pos_inicial, TipoForma::AABB), ancho(w), alto(h), fuerza_rebote(fuerza),
          deformacion(0.0), velocidad_deformacion(0.0) {
        set_restitucion(0.9); // Alto rebote normal
        set_friccion(0.2);     // Poca fricción en la lona
    }

    // --- Getters & Setters ---
    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    double get_fuerza_rebote() const { return fuerza_rebote; }
    double get_deformacion() const { return deformacion; }
    void set_deformacion(double d) { deformacion = d; }

    Vector2D get_min() const override { return posicion; }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho, posicion.y + alto); }

    // Identificador para casting rápido
    bool es_trampolin() const { return true; }

    // Simular el movimiento amortiguado de la lona en cada paso físico
    void actualizar_fisica(double dt) override {
        // Resorte amortiguado: F = -k*x - c*v
        double k = 600.0; // Rigidez
        double c = 12.0;  // Amortiguamiento
        double fuerza_resorte = -k * deformacion - c * velocidad_deformacion;

        velocidad_deformacion += fuerza_resorte * dt;
        deformacion += velocidad_deformacion * dt;

        // Resetear variables dinámicas para evitar que se desplace físicamente
        velocidad = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        fuerza_neta = Vector2D(0.0, 0.0);
    }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::TRAMPOLIN;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent TRAMPOLIN id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " w=" << ancho << " h=" << alto;
        return ss.str();
    }

    void on_collision(EntidadFisica* otro, const InfoColision& info) override {
        Bola* bola = dynamic_cast<Bola*>(otro);
        if (bola) {
            Vector2D normal_para_bola = info.normal * -1.0;
            if (normal_para_bola.y < -0.4) {
                double velocidad_impacto = std::abs(bola->get_velocidad().y);
                Vector2D vel = bola->get_velocidad();
                vel.y = -get_fuerza_rebote();
                bola->set_velocidad(vel);

                double nueva_def = std::max(10.0, velocidad_impacto * 0.04);
                set_deformacion(std::min(24.0, nueva_def));

                double centro_tramp = posicion.x + ancho / 2.0;
                double offset = bola->get_posicion().x - centro_tramp;
                double kick_rotacional = offset * 0.4;
                bola->set_velocidad_angular(bola->get_velocidad_angular() + kick_rotacional);
            }
        }
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
        float def = static_cast<float>(deformacion);

        if (tex_trampolin.id > 0) {
            int num_frames = 4;
            float frame_w = 0.0f;
            float frame_h = 0.0f;
            Rectangle source;
            int frame_idx = 0;
            if (def > 12.0f) frame_idx = 3;
            else if (def > 7.0f) frame_idx = 2;
            else if (def > 2.0f) frame_idx = 1;

            if (tex_trampolin.width > tex_trampolin.height) {
                frame_w = (float)tex_trampolin.width / num_frames;
                frame_h = (float)tex_trampolin.height;
                source = { frame_idx * frame_w, 0, frame_w, frame_h };
            } else {
                frame_w = (float)tex_trampolin.width;
                frame_h = (float)tex_trampolin.height / num_frames;
                source = { 0, frame_idx * frame_h, frame_w, frame_h };
            }

            Rectangle dest = { px - 4.0f, py - 4.0f, pw + 8.0f, ph + 8.0f };
            DrawTexturePro(tex_trampolin, source, dest, {0, 0}, 0.0f, WHITE);
        } else {
            // 1. Dibujar patas de soporte de acero cromado (estructura triangular estable)
            DrawLineEx({px + 6, py + ph}, {px + 12, py + 8}, 3.0f, DARKGRAY);
            DrawLineEx({px + 18, py + ph}, {px + 12, py + 8}, 3.0f, DARKGRAY);
            DrawLineEx({px + pw - 6, py + ph}, {px + pw - 12, py + 8}, 3.0f, DARKGRAY);
            DrawLineEx({px + pw - 18, py + ph}, {px + pw - 12, py + 8}, 3.0f, DARKGRAY);

            // Barra inferior metálica horizontal de unión
            DrawLineEx({px + 18, py + ph - 2}, {px + pw - 18, py + ph - 2}, 2.5f, GRAY);

            // 2. Dibujar marcos rígidos laterales en los extremos (donde se anclan los resortes)
            DrawRectangleRec({px, py, 6.0f, 10.0f}, GRAY);
            DrawRectangleLinesEx({px, py, 6.0f, 10.0f}, 1.0f, DARKGRAY);
            DrawRectangleRec({px + pw - 6.0f, py, 6.0f, 10.0f}, GRAY);
            DrawRectangleLinesEx({px + pw - 6.0f, py, 6.0f, 10.0f}, 1.0f, DARKGRAY);

            // 3. Dibujar resortes elásticos dorados estirándose hacia la lona curvada
            int num_resortes = 6;
            for (int i = 0; i < num_resortes; ++i) {
                float lx_start = px + 6.0f;
                float ly_start = py + 3.0f + i * 1.2f;
                
                float t = static_cast<float>(i) / (num_resortes - 1);
                float lx_end = px + 6.0f + (pw / 2.0f - 14.0f) * t;
                float ly_end = py + 4.0f + def * t;
                
                float mx = (lx_start + lx_end) / 2.0f;
                float my = (ly_start + ly_end) / 2.0f;
                DrawLineEx({lx_start, ly_start}, {mx, my - 2}, 1.2f, GOLD);
                DrawLineEx({mx, my - 2}, {lx_end, ly_end}, 1.2f, GOLD);

                float rx_start = px + pw - 6.0f;
                float ry_start = py + 3.0f + i * 1.2f;
                float rx_end = px + pw - 6.0f - (pw / 2.0f - 14.0f) * t;
                float ry_end = py + 4.0f + def * t;
                
                float rmx = (rx_start + rx_end) / 2.0f;
                float rmy = (ry_start + ry_end) / 2.0f;
                DrawLineEx({rx_start, ry_start}, {rmx, rmy - 2}, 1.2f, GOLD);
                DrawLineEx({rmx, rmy - 2}, {rx_end, ry_end}, 1.2f, GOLD);
            }

            // 4. Dibujar la lona elástica curvada (Bezier o catenaria según deformación)
            Color col_lona = Color{48, 50, 58, 255};
            int segmentos = 14;
            Vector2 p_prev = {px + 6.0f + (pw / 2.0f - 14.0f), py + 4.0f + def};
            for (int i = 1; i <= segmentos; ++i) {
                float t = static_cast<float>(i) / segmentos;
                float x = px + 6.0f + (pw / 2.0f - 14.0f) + (pw - 2.0f * (pw / 2.0f - 14.0f) - 12.0f) * t;
                float y_offset = std::sin(t * MathUtils::TIM_PI) * def * 0.9f;
                float y = py + 4.0f + def - y_offset;
                
                Vector2 p_curr = {x, y};
                DrawLineEx(p_prev, p_curr, 4.0f, col_lona);
                p_prev = p_curr;
            }
        }

        if (debug) {
            DrawRectangleLines(static_cast<int>(posicion.x), static_cast<int>(posicion.y),
                               static_cast<int>(ancho), static_cast<int>(alto), GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Trampolin" tab=0 categoria=0
