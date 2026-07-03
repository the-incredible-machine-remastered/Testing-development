#pragma once
// ============================================================================
// Balancin - Palanca giratoria pivotada en su centro.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "../sistema/assets_extern.h"
#include <vector>
#include <cmath>
#include <algorithm>

class Balancin : public EntidadFisica {
protected:
    double largo;
    double espesor;
    double angulo_limite;
    double resistencia_pivote;
    double angulo_inicial;
    bool impacto_brusco;

public:
    Balancin(int id, Vector2D pos_pivot, double length = 200.0, double thickness = 6.0, double m = 8.0)
        : EntidadFisica(id, pos_pivot, m, TipoForma::POLIGONO, false),
          largo(length), espesor(thickness), resistencia_pivote(1200.0), impacto_brusco(false) {

        // Barra + herrajes/base: mas inercia para que una carga colgada incline,
        // pero no haga girar el balancin demasiado facil.
        set_inercia((1.0 / 12.0) * m * largo * largo * 1.8);
        set_restitucion(0.2);
        set_friccion(0.3);

        angulo_inicial = 0.0;
        angulo = 0.0;
        velocidad_angular = 0.0;
        angulo_limite = 15.0 * MathUtils::TIM_PI / 180.0;
        tipo_menu = TipoObjetoMenu::BALANCIN;
    }

    // --- Getters ---
    double get_largo() const { return largo; }
    double get_espesor() const { return espesor; }
    double get_angulo() const { return angulo; }
    double get_angulo_limite() const { return angulo_limite; }
    double get_resistencia_pivote() const { return resistencia_pivote; }
    double get_angulo_inicial() const { return angulo_inicial; }

    // Se consume tras leerse: true una sola vez, el frame en que hubo un giro brusco.
    bool consumir_impacto_brusco() {
        bool v = impacto_brusco;
        impacto_brusco = false;
        return v;
    }

    // Marca que hubo un giro brusco (llamar justo cuando un impulso de colisión
    // cambia la velocidad angular). Permite a otros sistemas (ej. pistolas/focos
    // conectados) reaccionar sin depender de que el giro se traduzca en tensión.
    void marcar_impacto_si_brusco() {
        if (std::abs(velocidad_angular) >= 1.5) {
            impacto_brusco = true;
        }
    }

    void ciclar_inclinacion() {
        if (angulo_inicial == 0.0)       angulo_inicial = angulo_limite;
        else if (angulo_inicial > 0.0)   angulo_inicial = -angulo_limite;
        else                             angulo_inicial = 0.0;
        angulo = angulo_inicial;
        velocidad_angular = 0.0;
    }

    Vector2D get_punto_extremo_izquierdo() const {
        Vector2D dir(std::cos(angulo), std::sin(angulo));
        return posicion - dir * (largo / 2.0 - 5.0);
    }

    Vector2D get_punto_extremo_derecho() const {
        Vector2D dir(std::cos(angulo), std::sin(angulo));
        return posicion + dir * (largo / 2.0 - 5.0);
    }

    void aplicar_fuerza_en_punto(const Vector2D& punto_mundo, const Vector2D& fuerza) {
        Vector2D r = punto_mundo - posicion;
        aplicar_torque(Vector2D::cross(r, fuerza));
    }

    std::vector<Vector2D> get_vertices() const {
        double cos_a = std::cos(angulo);
        double sin_a = std::sin(angulo);
        double hl = largo / 2.0;
        double ht = espesor / 2.0;
        Vector2D dir_x(cos_a, sin_a);
        Vector2D dir_y(-sin_a, cos_a);

        return {
            posicion - dir_x * hl - dir_y * ht,
            posicion + dir_x * hl - dir_y * ht,
            posicion + dir_x * hl + dir_y * ht,
            posicion - dir_x * hl + dir_y * ht
        };
    }

    // El balancin no se desplaza linealmente, solo gira.
    void actualizar_fisica(double dt) override {
        if (inercia > MathUtils::EPSILON) {
            double torque_resistente = -velocidad_angular * resistencia_pivote;
            double umbral_estatico = 400000.0;
            if (std::abs(velocidad_angular) < 0.01 && std::abs(torque_neto) < umbral_estatico) {
                velocidad_angular = 0.0;
            } else {
                velocidad_angular += ((torque_neto + torque_resistente) / inercia) * dt;
            }
        }

        if (velocidad_angular > 4.0) velocidad_angular = 4.0;
        else if (velocidad_angular < -4.0) velocidad_angular = -4.0;

        angulo += velocidad_angular * dt;

        velocidad_angular *= 0.975;

        if (angulo > angulo_limite) {
            angulo = angulo_limite;
            if (velocidad_angular > 0.0) {
                velocidad_angular = -velocidad_angular * 0.15;
            }
        }
        else if (angulo < -angulo_limite) {
            angulo = -angulo_limite;
            if (velocidad_angular < 0.0) {
                velocidad_angular = -velocidad_angular * 0.15;
            }
        }

        velocidad = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        fuerza_neta = Vector2D(0.0, 0.0);
        torque_neto = 0.0;
    }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::BALANCIN;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent BALANCIN id=" << get_id() << serializar_base()
           << " largo=" << largo << " esp=" << espesor << " ang0=" << angulo_inicial;
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        double ang = angulo;
        double half_l = largo / 2.0;
        Vector2D dir(std::cos(ang), std::sin(ang));
        Vector2D A = posicion - dir * half_l;
        Vector2D B = posicion + dir * half_l;
        
        Vector2D v = B - A;
        Vector2D w = p - A;
        double dot = w.x * v.x + w.y * v.y;
        double len_sq = v.x * v.x + v.y * v.y;
        double t = (len_sq > 0.0) ? std::max(0.0, std::min(1.0, dot / len_sq)) : 0.0;
        Vector2D C = A + v * t;
        double dist = (p - C).magnitud();
        return dist < espesor / 2.0 + 15.0;
    }

    void dibujar(bool debug) const override {
        int px = static_cast<int>(posicion.x);
        int py = static_cast<int>(posicion.y);
        int larg = static_cast<int>(largo);
        int esp = static_cast<int>(espesor);
        float rot_deg = static_cast<float>(angulo * 180.0 / MathUtils::TIM_PI);
        double cos_a = std::cos(angulo);
        double sin_a = std::sin(angulo);

        // 1. Dibujar soporte del pivot
        if (tex_balancin_base.id > 0) {
            float base_w = 34.0f;
            float base_h = 44.0f;
            DrawTexturePro(
                tex_balancin_base,
                { 0, 0, (float)tex_balancin_base.width, (float)tex_balancin_base.height },
                { (float)px - base_w / 2.0f, (float)py - 2.0f, base_w, base_h },
                { 0, 0 },
                0.0f,
                WHITE
            );
        } else {
            DrawTriangle(
                {static_cast<float>(px), static_cast<float>(py)},
                {static_cast<float>(px - 16), static_cast<float>(py + 40)},
                {static_cast<float>(px + 16), static_cast<float>(py + 40)},
                DARKGRAY
            );
            DrawTriangleLines(
                {static_cast<float>(px), static_cast<float>(py)},
                {static_cast<float>(px - 16), static_cast<float>(py + 40)},
                {static_cast<float>(px + 16), static_cast<float>(py + 40)},
                GRAY
            );
        }

        // 2. Dibujar la tabla giratoria (plank)
        if (tex_balancin_tabla.id > 0) {
            float board_h = esp * 3.5f;
            Rectangle rec = { static_cast<float>(px), static_cast<float>(py), static_cast<float>(larg), board_h };
            Vector2 origin = { static_cast<float>(larg / 2.0), board_h / 2.0f };
            DrawTexturePro(
                tex_balancin_tabla,
                { 0, 0, (float)tex_balancin_tabla.width, (float)tex_balancin_tabla.height },
                rec,
                origin,
                rot_deg,
                WHITE
            );
        } else {
            Rectangle rec = { static_cast<float>(px), static_cast<float>(py), static_cast<float>(larg), static_cast<float>(esp) };
            Vector2 origin = { static_cast<float>(larg / 2.0), static_cast<float>(esp / 2.0) };
            DrawRectanglePro(rec, origin, rot_deg, Color{190, 110, 50, 255});
            
            // Dibujar contorno ocre rotado usando las 4 esquinas del tablón
            double hl = larg / 2.0;
            double ht = esp / 2.0;
            Vector2D dir_x(cos_a, sin_a);
            Vector2D dir_y(-sin_a, cos_a);
            Vector2D c1 = posicion - dir_x * hl - dir_y * ht;
            Vector2D c2 = posicion + dir_x * hl - dir_y * ht;
            Vector2D c3 = posicion + dir_x * hl + dir_y * ht;
            Vector2D c4 = posicion - dir_x * hl + dir_y * ht;

            Color color_borde = Color{220, 140, 70, 255};
            DrawLineEx({(float)c1.x, (float)c1.y}, {(float)c2.x, (float)c2.y}, 1.5f, color_borde);
            DrawLineEx({(float)c2.x, (float)c2.y}, {(float)c3.x, (float)c3.y}, 1.5f, color_borde);
            DrawLineEx({(float)c3.x, (float)c3.y}, {(float)c4.x, (float)c4.y}, 1.5f, color_borde);
            DrawLineEx({(float)c4.x, (float)c4.y}, {(float)c1.x, (float)c1.y}, 1.5f, color_borde);
        }

        // 3. Asientos rojos en los extremos (siempre dibujados encima para mayor detalle)
        Vector2 seat_l = {
            static_cast<float>(px - (larg / 2.0 - 5.0) * cos_a),
            static_cast<float>(py - (larg / 2.0 - 5.0) * sin_a)
        };
        DrawCircle(static_cast<int>(seat_l.x), static_cast<int>(seat_l.y), 6.0f, RED);
        DrawCircleLines(static_cast<int>(seat_l.x), static_cast<int>(seat_l.y), 6.0f, MAROON);

        Vector2 seat_r = {
            static_cast<float>(px + (larg / 2.0 - 5.0) * cos_a),
            static_cast<float>(py + (larg / 2.0 - 5.0) * sin_a)
        };
        DrawCircle(static_cast<int>(seat_r.x), static_cast<int>(seat_r.y), 6.0f, RED);
        DrawCircleLines(static_cast<int>(seat_r.x), static_cast<int>(seat_r.y), 6.0f, MAROON);

        // 4. Perno central negro/gris
        DrawCircle(px, py, 6, BLACK);
        DrawCircle(px, py, 4, LIGHTGRAY);
    }
};

// TIM_MENU_SPAWN etiqueta="Balancin" tab=0 categoria=0
