#pragma once
// ============================================================================
// BarrilChavo — Objeto interactivo estático (AABB, 60x80 px)
// Cuando una pelota colisiona por arriba, sale "El Chavo" del barril,
// impulsa la pelota con un ángulo de 75 grados hacia arriba (aprox. -75° en Y-down),
// se mantiene afuera por 5 segundos y luego se vuelve a esconder.
// ============================================================================

#include "obstaculo_estatico.h"
#include "../core/math_utils.h"
#include <cmath>

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

    Vector2D get_min() const { return posicion; }
    Vector2D get_max() const { return Vector2D(posicion.x + ancho, posicion.y + alto); }

    // Función para activar el disparo del Chavo
    void disparar_chavo() {
        if (estado == EstadoBarril::ESPERANDO) {
            estado = EstadoBarril::AFUERA;
            timer = 5.0; // Se queda afuera por 5 segundos
        }
    }

    // Actualiza el estado lógico y animaciones
    void actualizar_fisica(double dt) override {
        // Actualizamos según el estado
        if (estado == EstadoBarril::AFUERA) {
            // Incrementar pop_factor rápidamente
            pop_factor += dt * 7.0;
            if (pop_factor > 1.0) pop_factor = 1.0;

            // Cuenta regresiva
            timer -= dt;
            if (timer <= 0.0) {
                timer = 0.0;
                estado = EstadoBarril::ESCONDIENDOSE;
            }
        }
        else if (estado == EstadoBarril::ESCONDIENDOSE) {
            // Decrementar pop_factor lentamente
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
};
