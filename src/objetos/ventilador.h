#pragma once
// ============================================================================
// Ventilador -- Objeto estatico que empuja bolas con una corriente de aire
// horizontal. No es una pared: su funcion principal es aplicar una fuerza
// a distancia dentro de una zona rectangular frente a sus aspas.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../sistema/assets_extern.h"
#include <cmath>
#include <algorithm>

// ============================================================================
// Ventilador -- Ahora es un objeto con EJE FICTICIO: se construye dinámico pero
// ancla su traslación cada frame (patrón de Polea). Una Correa conectada a una
// RuedaHamster transmite torque real a este eje; la velocidad angular resultante
// se traduce en la potencia del chorro de aire. Sin Correa conectada, el
// ventilador sopla a su potencia_base de forma autónoma.
// ============================================================================

class Ventilador : public EntidadFisica {
protected:
    double ancho;
    double alto;
    double rango;
    double ancho_corriente;
    double potencia;
    double potencia_base;   // potencia autónoma (sin correa conectada)
    bool controlado_por_banda; // legacy: se conserva para compatibilidad de guardado
    Vector2D direccion;
    double fase_aspas;

    // Velocidad angular de referencia (rad/s) para mapear giro -> potencia máxima.
    // Coincide con la velocidad de crucero de RuedaHamster (~7.5 rad/s).
    static constexpr double OMEGA_REF = 7.5;

public:
    Ventilador(int id, Vector2D pos, double w = 42.0, double h = 54.0)
        : EntidadFisica(id, pos, 1.0, TipoForma::AABB, false),
          ancho(w), alto(h), rango(220.0), ancho_corriente(92.0),
          potencia(0.0), potencia_base(1800.0), controlado_por_banda(false),
          direccion(1.0, 0.0), fase_aspas(0.0) {
        set_inercia(0.5 * masa * 10.0 * 10.0); // eje ficticio radio 10 (= RuedaHamster)
        set_restitucion(0.2);
        set_friccion(0.5);
        tipo_menu = TipoObjetoMenu::VENTILADOR;
        potencia = potencia_base; // arranca soplando de forma autónoma
    }

    // Radio de eje = el de RuedaHamster para que la Correa iguale su omega.
    double get_radio_eje() const override { return 10.0; }

    // --- Getters ---
    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    double get_rango() const { return rango; }
    double get_ancho_corriente() const { return ancho_corriente; }
    double get_potencia() const { return potencia; }
    Vector2D get_direccion() const { return direccion; }
    double get_fase_aspas() const { return fase_aspas; }

    Vector2D get_min() const override { return posicion; }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho, posicion.y + alto); }
    Vector2D get_centro_salida() const {
        if (direccion.x < 0.0) {
            return Vector2D(posicion.x, posicion.y + alto / 2.0);
        } else {
            return Vector2D(posicion.x + ancho, posicion.y + alto / 2.0);
        }
    }

    void invertir_direccion() {
        if (direccion.x > 0.0) {
            direccion = Vector2D(-1.0, 0.0);
        } else {
            direccion = Vector2D(1.0, 0.0);
        }
    }

    bool mira_derecha() const {
        return direccion.x > 0.0;
    }

    void set_potencia(double p) { potencia = p; }
    bool get_controlado_por_banda() const { return controlado_por_banda; }
    void set_controlado_por_banda(bool v) { controlado_por_banda = v; } // legacy
    void set_direccion(const Vector2D& dir) { direccion = dir.normalizar(); }

    void actualizar_fisica(double dt) override {
        // Patrón de Polea: el eje ficticio gira con el torque acumulado por la
        // Correa; la traslación se mantiene anclada (el ventilador no se mueve).
        // El torque de la Correa ya fue acumulado en torque_neto antes de este paso.
        double I = get_inercia();
        double alpha = (I > MathUtils::EPSILON) ? (torque_neto / I) : 0.0;
        velocidad_angular += alpha * dt;

        // Modo autónomo: SOLO si no hay ninguna correa conectada, el eje tiende a
        // su velocidad de crucero para soplar a potencia base. Si hay una correa,
        // el giro (y por tanto la potencia) lo gobierna la rueda: si la rueda está
        // parada, el ventilador queda apagado hasta que ésta empiece a girar.
        if (!conectado_a_correa) {
            velocidad_angular += (OMEGA_REF - velocidad_angular) * 4.0 * dt;
        }
        velocidad_angular *= 0.985; // damping natural

        // Traducir giro -> potencia del chorro
        double factor = std::min(std::abs(velocidad_angular) / OMEGA_REF, 1.0);
        potencia = potencia_base * factor;

        angulo += velocidad_angular * dt;
        fase_aspas += dt * 18.0 * (potencia / 1800.0);

        // Anclar traslación: el ventilador nunca se desplaza
        velocidad = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        fuerza_neta = Vector2D(0.0, 0.0);
        torque_neto = 0.0;
    }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::VENTILADOR;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent VENTILADOR id=" << get_id() << serializar_base()
           << " w=" << ancho << " h=" << alto
           << " der=" << (mira_derecha() ? 1 : 0)
           << " banda=" << (controlado_por_banda ? 1 : 0);
        return ss.str();
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
        float cx = px + w / 2.0f;
        float cy = py + h / 2.0f;
        float fase = static_cast<float>(fase_aspas);

        // 1. Cuerpo del ventilador
        if (tex_ventilador_cuerpo.id > 0) {
            Rectangle src = {0.0f, 0.0f, (float)tex_ventilador_cuerpo.width, (float)tex_ventilador_cuerpo.height};
            Rectangle dst = {px, py, w, h};
            DrawTexturePro(tex_ventilador_cuerpo, src, dst, {0,0}, 0.0f, WHITE);
        } else {
            DrawRectangleRec({px, py, w, h}, Color{70, 84, 96, 255});
            DrawRectangleLinesEx({px, py, w, h}, 1.5f, Color{170, 190, 205, 255});
        }

        // 2. Rejilla frontal y aspas giratorias
        if (tex_ventilador_aspa.id > 0) {
            float aspa_w = h * 0.5f + 20.0f;
            float aspa_h = h * 0.3f - 20.0f;
            for (int i = 0; i < 4; ++i) {
                float ang = fase + i * MathUtils::TIM_PI / 2.0f;
                float ang_deg = ang * 180.0f / MathUtils::TIM_PI;
                Rectangle src = {0.0f, 0.0f, (float)tex_ventilador_aspa.width, (float)tex_ventilador_aspa.height};
                Rectangle dst = {cx - aspa_w/2.0f + 37.0f, cy - aspa_h/2.0f + 6, aspa_w, aspa_h};
                Vector2 origin = {aspa_w/2.0f, aspa_h/2.0f};
                DrawTexturePro(tex_ventilador_aspa, src, dst, origin, ang_deg, WHITE);
            }
            DrawCircle(static_cast<int>(cx), static_cast<int>(cy), 4.0f, Color{210, 230, 240, 255});
        } else {
            DrawCircleLines(static_cast<int>(cx), static_cast<int>(cy), h * 0.32f, Color{190, 210, 220, 255});
            for (int i = 0; i < 4; ++i) {
                float ang = fase + i * MathUtils::TIM_PI / 2.0f;
                Vector2 p1 = {cx, cy};
                Vector2 p2 = {
                    cx + std::cos(ang) * h * 0.26f,
                    cy + std::sin(ang) * h * 0.26f
                };
                DrawLineEx(p1, p2, 3.0f, Color{135, 205, 255, 255});
            }
            DrawCircle(static_cast<int>(cx), static_cast<int>(cy), 4.0f, Color{210, 230, 240, 255});
        }

        // 3. Corriente de aire — solo si está activo
        bool der = mira_derecha();
        if (potencia > 0.0) {
            for (int i = 0; i < 4; ++i) {
                float y = cy - 24.0f + i * 16.0f;
                float offset   = std::sin(fase + i * 0.5f) * 15.0f;
                float longitud = 70.0f + std::sin(fase + i * 0.3f) * 25.0f;
                float opacidad = 90 + std::sin(fase + i * 0.4f) * 50;
                if (der) {
                    DrawLineEx({px + w + 8.0f + offset, y}, {px + w + 8.0f + longitud, y}, 1.5f,
                              Color{120, 200, 255, static_cast<unsigned char>(opacidad)});
                } else {
                    DrawLineEx({px - 8.0f - offset, y}, {px - 8.0f - longitud, y}, 1.5f,
                              Color{120, 200, 255, static_cast<unsigned char>(opacidad)});
                }
            }
        }

        if (debug) {
            DrawRectangleLines(static_cast<int>(px), static_cast<int>(py),
                               static_cast<int>(w), static_cast<int>(h), GREEN);
            if (der) {
                DrawRectangleLines(static_cast<int>(px + w), static_cast<int>(cy - ancho_corriente / 2.0),
                                   static_cast<int>(rango), static_cast<int>(ancho_corriente), GREEN);
            } else {
                DrawRectangleLines(static_cast<int>(px - rango), static_cast<int>(cy - ancho_corriente / 2.0),
                                   static_cast<int>(rango), static_cast<int>(ancho_corriente), GREEN);
            }
        }
    }
};
// TIM_MENU_SPAWN etiqueta="Ventilador" tab=0 categoria=1
