#pragma once
// ============================================================================
// BarrilChavo — Objeto interactivo estático (AABB, 60x80 px)
// Cuando una pelota colisiona por arriba, sale "El Chavo" del barril,
// impulsa la pelota con un ángulo de 75 grados hacia arriba (aprox. -75° en Y-down),
// se mantiene afuera por 5 segundos y luego se vuelve a esconder.
// ============================================================================

#include "obstaculo_estatico.h"
#include "../core/math_utils.h"
#include "../sistema/assets_extern.h"
#include "../fisica/colisiones.h"
#include "bola.h"
#include <cmath>
#include <algorithm>

enum class EstadoBarril {
    ESPERANDO,      // Escondido dentro del barril
    AFUERA,         // "El Chavo" está afuera del barril, contando 5 segundos
    ESCONDIENDOSE   // Retrayéndose de vuelta al barril
};

class BarrilChavo : public ObstaculoEstatico {
protected:
    double ancho;
    double alto;
    EstadoBarril estado;
    double timer;
    double pop_factor; // 0.0 = totalmente escondido, 1.0 = completamente afuera

public:
    BarrilChavo(int id, Vector2D pos, double w = 60.0, double h = 80.0)
        : ObstaculoEstatico(id, pos, TipoForma::AABB),
          ancho(w), alto(h),
          estado(EstadoBarril::ESPERANDO), timer(0.0), pop_factor(0.0) {
        set_restitucion(0.2); // Poca elasticidad para que la pelota reciba el impulso del chavo
        set_friccion(0.4);
    }

    // --- Getters ---
    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    EstadoBarril get_estado() const { return estado; }
    double get_pop_factor() const { return pop_factor; }
    double get_timer() const { return timer; }

    Vector2D get_min() const override { return posicion; }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho, posicion.y + alto); }

    // Función para activar el disparo del Chavo
    void disparar_chavo() {
        if (estado == EstadoBarril::ESPERANDO) {
            estado = EstadoBarril::AFUERA;
            timer = 5.0; // Se queda afuera por 5 segundos
        }
    }

    // Actualiza el estado lógico y animaciones
    void actualizar_fisica(double dt) override {
        if (estado == EstadoBarril::AFUERA) {
            pop_factor += dt * 7.0;
            if (pop_factor > 1.0) pop_factor = 1.0;

            timer -= dt;
            if (timer <= 0.0) {
                timer = 0.0;
                estado = EstadoBarril::ESCONDIENDOSE;
            }
        }
        else if (estado == EstadoBarril::ESCONDIENDOSE) {
            pop_factor -= dt * 2.0;
            if (pop_factor <= 0.0) {
                pop_factor = 0.0;
                estado = EstadoBarril::ESPERANDO;
            }
        }
        else { // ESPERANDO
            pop_factor = 0.0;
        }
    }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::BARRIL;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent BARRIL id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " w=" << ancho << " h=" << alto;
        return ss.str();
    }

    void on_collision(EntidadFisica* otro, const InfoColision& info) override {
        Bola* bola = dynamic_cast<Bola*>(otro);
        if (bola) {
            Vector2D normal_para_bola = info.normal * -1.0;
            if (normal_para_bola.y < -0.4 && bola->get_posicion().y < posicion.y + 10.0) {
                if (estado == EstadoBarril::ESPERANDO) {
                    disparar_chavo();
                    
                    registrar_evento_especial(RegistroEventoEspecial{
                        TipoEventoEspecial::BARRIL_LANZADO,
                        get_id(),
                        bola->get_id()
                    });
                    
                    double angulo_rad = MathUtils::grados_a_radianes(-75.0);
                    Vector2D dir_impulso(std::cos(angulo_rad), std::sin(angulo_rad));
                    
                    double fuerza_lanzamiento = 750.0;
                    bola->set_velocidad(dir_impulso * fuerza_lanzamiento);
                    bola->set_velocidad_angular(3.0);
                }
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
        float w = static_cast<float>(ancho);
        float h = static_cast<float>(alto);
        float pop = static_cast<float>(pop_factor);

        // 1. Dibujar "El Chavo" saliendo si pop_factor > 0
        if (pop > 0.0f) {
            float chavo_w = w * 0.72f;
            float chavo_h = h * 0.8f;
            float chavo_y = py - (chavo_h * 0.8f) * pop + 12.0f;
            float chavo_x = px + (w - chavo_w) / 2.0f;

            if (tex_chavo.id > 0) {
                Rectangle src = {0.0f, 0.0f, (float)tex_chavo.width, (float)tex_chavo.height};
                Rectangle dst = {chavo_x, chavo_y, chavo_w, chavo_h};
                DrawTexturePro(tex_chavo, src, dst, {0,0}, 0.0f, WHITE);
            } else {
                // Fallback geométrico del Chavo
                DrawRectangleRec({chavo_x + 4, chavo_y + 12, chavo_w - 8, chavo_h - 12}, Color{235, 170, 155, 255});
                DrawCircle(static_cast<int>(chavo_x + chavo_w * 0.5f), static_cast<int>(chavo_y + 16), 11.0f, Color{245, 210, 190, 255});
                DrawCircleSector({chavo_x + chavo_w * 0.5f, chavo_y + 14}, 12.0f, 180.0f, 360.0f, 0, Color{80, 140, 90, 255});
            }
        }

        // 2. Dibujar el Barril encima
        if (tex_barril.id > 0) {
            Rectangle src = {0.0f, 0.0f, (float)tex_barril.width, (float)tex_barril.height};
            Rectangle dst = {px - 4.0f, py - 4.0f, w + 8.0f, h + 8.0f};
            DrawTexturePro(tex_barril, src, dst, {0,0}, 0.0f, WHITE);
        } else {
            // Fallback geométrico del Barril
            Color col_barril = Color{150, 95, 55, 255};
            DrawRectangleRounded({px, py, w, h}, 0.22f, 4, col_barril);
            DrawRectangleRoundedLinesEx({px, py, w, h}, 0.22f, 4, 1.8f, Color{90, 50, 25, 255});

            float aro_h = 3.5f;
            DrawRectangleRec({px, py + h * 0.22f, w, aro_h}, Color{128, 134, 139, 255});
            DrawRectangleRec({px, py + h * 0.50f, w, aro_h}, Color{128, 134, 139, 255});
            DrawRectangleRec({px, py + h * 0.78f, w, aro_h}, Color{128, 134, 139, 255});

            DrawEllipse(static_cast<int>(px + w * 0.5f), static_cast<int>(py + 2.0f), w * 0.42f, 5.0f, Color{45, 30, 20, 255});
        }

        if (debug) {
            DrawRectangleLines(static_cast<int>(px), static_cast<int>(py),
                               static_cast<int>(w), static_cast<int>(h), GREEN);
        }
    }
};
// TIM_MENU_SPAWN etiqueta="Barril" tab=0 categoria=1
