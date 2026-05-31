#pragma once
// ============================================================================
// Balancin - Palanca giratoria pivotada en su centro.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include <vector>

class Balancin : public EntidadFisica {
protected:
    double largo;
    double espesor;
    double angulo_limite;
    double resistencia_pivote;

public:
    Balancin(int id, Vector2D pos_pivot, double length = 200.0, double thickness = 6.0, double m = 8.0)
        : EntidadFisica(id, pos_pivot, m, TipoForma::POLIGONO, false),
          largo(length), espesor(thickness), resistencia_pivote(1800.0) {

        // Barra + herrajes/base: mas inercia para que una carga colgada incline,
        // pero no haga girar el balancin demasiado facil.
        set_inercia((1.0 / 12.0) * m * largo * largo * 1.8);
        set_restitucion(0.2);
        set_friccion(0.3);

        angulo = 0.0;
        velocidad_angular = 0.0;
        angulo_limite = 15.0 * MathUtils::TIM_PI / 180.0;
    }

    // --- Getters ---
    double get_largo() const { return largo; }
    double get_espesor() const { return espesor; }
    double get_angulo_limite() const { return angulo_limite; }
    double get_resistencia_pivote() const { return resistencia_pivote; }

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
            velocidad_angular += ((torque_neto + torque_resistente) / inercia) * dt;
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
};

// TIM_MENU_SPAWN etiqueta="Balancin" tab=0 categoria=0
