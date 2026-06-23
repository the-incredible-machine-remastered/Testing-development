#pragma once
// ============================================================================
// BolaRebotadora -- Obstaculo circular gigante inspirado en las "Big Balls"
// de Wipeout. Es una esfera estatica, elastica y resbalosa que sirve como
// obstaculo de rebote/equilibrio para desviar bolas y mecanismos.
// ============================================================================

#include "obstaculo_estatico.h"
#include "../sistema/assets_extern.h"
#include "../fisica/colisiones.h"
#include "bola.h"
#include <algorithm>
#include <cmath>

class BolaRebotadora : public ObstaculoEstatico {
protected:
    double radio;
    double altura_soporte;
    double deformacion;           // Compresion visual actual de la esfera
    double velocidad_deformacion;  // Velocidad del resorte visual
    double vibracion_timer;        // Tiempo restante de vibracion
    double vibracion_fase;         // Fase senoidal para temblor lateral
    double impacto_cooldown;       // Evita reactivar vibracion por contacto continuo
    double multiplicador_rebote;   // Potencia extra del rebote contra esta bola

public:
    BolaRebotadora(int id, Vector2D pos_centro, double r = 48.0)
        : ObstaculoEstatico(id, pos_centro, TipoForma::CIRCULO),
          radio(r), altura_soporte(34.0), deformacion(0.0),
          velocidad_deformacion(0.0), vibracion_timer(0.0), vibracion_fase(0.0),
          impacto_cooldown(0.0), multiplicador_rebote(3.25) {
        set_restitucion(0.85); // Muy rebotona, como una esfera inflable
        set_friccion(0.18);    // Superficie resbalosa
    }

    // --- Getters ---
    double get_radio() const { return radio; }
    double get_altura_soporte() const { return altura_soporte; }
    double get_deformacion() const { return deformacion; }
    double get_vibracion_timer() const { return vibracion_timer; }
    double get_multiplicador_rebote() const { return multiplicador_rebote; }
    double get_offset_vibracion() const {
        return std::sin(vibracion_fase) * vibracion_timer * 5.0;
    }

    void set_multiplicador_rebote(double m) {
        multiplicador_rebote = std::max(0.0, m);
    }

    // Activar temblor visual al recibir un impacto de una bola
    void registrar_impacto(double velocidad_impacto) {
        if (impacto_cooldown > 0.0 || velocidad_impacto < 35.0) return;

        double impacto = std::min(22.0, std::max(4.0, velocidad_impacto * 0.025));
        deformacion = std::max(deformacion, impacto);
        velocidad_deformacion -= impacto * 22.0;
        vibracion_timer = std::min(0.35, std::max(vibracion_timer, velocidad_impacto * 0.001));
        impacto_cooldown = 0.12;
    }

    // La bola es estatica en fisica, pero su goma vibra visualmente como resorte amortiguado
    void actualizar_fisica(double dt) override {
        double k = 120.0; // Rigidez de la goma inflable
        double c = 22.0;  // Amortiguamiento para que deje de vibrar
        double fuerza_resorte = -k * deformacion - c * velocidad_deformacion;

        velocidad_deformacion += fuerza_resorte * dt;
        deformacion += velocidad_deformacion * dt;

        if (std::abs(deformacion) < 0.05 && std::abs(velocidad_deformacion) < 0.05) {
            deformacion = 0.0;
            velocidad_deformacion = 0.0;
        }
        if (deformacion < 0.0) deformacion = 0.0;
        if (deformacion > 24.0) deformacion = 24.0;

        if (vibracion_timer > 0.0) {
            vibracion_fase += dt * 44.0;
            vibracion_timer -= dt * 1.8;
            if (vibracion_timer < 0.0) vibracion_timer = 0.0;
        }

        if (impacto_cooldown > 0.0) {
            impacto_cooldown -= dt;
            if (impacto_cooldown < 0.0) impacto_cooldown = 0.0;
        }
    }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::BOLA_REBOTADORA;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent BOLA_REBOTADORA id=" << get_id() << serializar_base()
           << " r=" << radio;
        return ss.str();
    }

    void on_collision(EntidadFisica* otro, const InfoColision& info) override {
        Bola* bola = dynamic_cast<Bola*>(otro);
        if (bola) {
            Vector2D normal_bola = info.normal * -1.0;
            double velocidad_normal = Vector2D::dot(bola->get_velocidad(), normal_bola);
            double velocidad_impacto = std::abs(velocidad_normal);
            registrar_impacto(velocidad_impacto);

            if (velocidad_normal > 0.0 && get_multiplicador_rebote() > 1.0) {
                Vector2D vel = bola->get_velocidad();
                double impulso_extra = velocidad_normal * (get_multiplicador_rebote() - 1.0);
                bola->set_velocidad(vel + normal_bola * impulso_extra);
            }
        }
    }

    bool contiene_punto(const Vector2D& p) const override {
        return (posicion - p).magnitud() < radio + 15.0;
    }

    void dibujar(bool debug) const override {
        float r = static_cast<float>(radio);
        float def = static_cast<float>(deformacion);
        float wobble_x = static_cast<float>(get_offset_vibracion());
        float escala_x = 1.0f + def / (r * 5.0f);
        float escala_y = 1.0f - def / (r * 4.0f);
        float draw_x = static_cast<float>(posicion.x) + wobble_x;
        float draw_y = static_cast<float>(posicion.y) + def * 0.25f;
        float base_top = static_cast<float>(posicion.y + r * 0.72f);
        float base_w = r * 1.35f;
        float base_h = r * 0.58f;

        // Dibujar soporte
        if (tex_robote_soporte.id > 0) {
            float scale_support = base_w / tex_robote_soporte.width;
            DrawTextureEx(tex_robote_soporte, 
                         {draw_x - base_w / 2.0f, base_top-25}, 
                         0.0f, scale_support, WHITE);
        } else {
            // Fallback geométrico
            Color col_base = Color{218, 48, 42, 255};
            DrawRectangleRec({draw_x - base_w / 2.0f, base_top, base_w, base_h}, col_base);
        }

        // Dibujar pelota
        if (tex_robote_pelota.id > 0) {
            float scale_ball = (r * 2.0f) / tex_robote_pelota.width;
            DrawTexturePro(
                tex_robote_pelota,
                {0.0f, 0.0f, (float)tex_robote_pelota.width, (float)tex_robote_pelota.height},
                {draw_x, draw_y, r * 2.0f * escala_x, r * 2.0f * escala_y},
                {r * escala_x, r * escala_y},
                0.0f,
                WHITE
            );
        } else {
            // Fallback geométrico
            float rx = r * escala_x;
            float ry = r * escala_y;
            Color col_base = Color{218, 48, 42, 255};
            DrawEllipse(static_cast<int>(draw_x), static_cast<int>(draw_y), rx, ry, col_base);
        }

        if (debug) {
            DrawCircleLines(static_cast<int>(posicion.x), static_cast<int>(posicion.y), r, GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Bola Reb." tab=0 categoria=0
