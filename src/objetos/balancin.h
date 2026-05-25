#pragma once
// ============================================================================
// Balancin — Palanca giratoria (seesaw) pivotada en su centro
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include <vector>

class Balancin : public EntidadFisica {
protected:
    double largo;
    double espesor;
    double angulo_limite; // Ángulo máximo en radianes (ej. 15 grados)

public:
    Balancin(int id, Vector2D pos_pivot, double length = 200.0, double thickness = 6.0, double m = 0.8)
        : EntidadFisica(id, pos_pivot, m, TipoForma::POLIGONO, false), 
          largo(length), espesor(thickness) {
        
        // Momento de inercia de una barra girando sobre su centro: I = 1/12 * m * L²
        set_inercia((1.0 / 12.0) * m * largo * largo);
        set_restitucion(0.2); // Rebote bajo para transferir energía de forma estable
        set_friccion(0.3);
        
        angulo = 0.0;
        velocidad_angular = 0.0;
        angulo_limite = 15.0 * MathUtils::TIM_PI / 180.0; // 15 grados
    }

    // --- Getters ---
    double get_largo() const { return largo; }
    double get_espesor() const { return espesor; }
    double get_angulo_limite() const { return angulo_limite; }

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

    // El balancín no se desplaza linealmente, solo gira
    void actualizar_fisica(double dt) override {
        // Limitar velocidad angular máxima para prevenir tunelización
        if (velocidad_angular > 10.0) velocidad_angular = 10.0;
        else if (velocidad_angular < -10.0) velocidad_angular = -10.0;

        // Integrar velocidad angular a ángulo
        angulo += velocidad_angular * dt;

        // Amortiguamiento rotacional sutil para detenerse
        velocidad_angular *= 0.985;

        // Limitar ángulo con topes elásticos
        if (angulo > angulo_limite) {
            angulo = angulo_limite;
            if (velocidad_angular > 0.0) {
                // Pequeño rebote contra el tope
                velocidad_angular = -velocidad_angular * 0.15;
            }
        }
        else if (angulo < -angulo_limite) {
            angulo = -angulo_limite;
            if (velocidad_angular < 0.0) {
                velocidad_angular = -velocidad_angular * 0.15;
            }
        }

        // Forzar velocidad lineal a 0 para que no se mueva de su lugar
        velocidad = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        fuerza_neta = Vector2D(0.0, 0.0);
    }
};

// TIM_MENU_SPAWN etiqueta="Balancin" tab=0 categoria=0
