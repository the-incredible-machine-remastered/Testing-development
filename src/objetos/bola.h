#pragma once
// ============================================================================
// Bola — Objeto dinámico circular
// La pieza más fundamental de TIM. Rueda, rebota, y activa mecanismos.
// ============================================================================

#include "../core/entidad_fisica.h"

class Bola : public EntidadFisica {
private:
    double radio;
    int color_idx;  // Índice en paleta de colores (para el renderizador)

public:
    Bola(int id, Vector2D pos_inicial, double r, double m)
        : EntidadFisica(id, pos_inicial, m, TipoForma::CIRCULO),
          radio(r), color_idx(0) {

        // Momento de inercia de un disco sólido: I = ½ m r²
        set_inercia(0.5 * m * r * r);
        set_restitucion(0.7);   // Rebote alto
        set_friccion(0.5);      // Fricción media-alta (favorece rolling)
        set_amortiguamiento(0.002);  // Mínimo damping
    }

    // --- Getters ---
    double get_radio() const { return radio; }
    int get_color_idx() const { return color_idx; }

    // --- Setters ---
    void set_color_idx(int idx) { color_idx = idx; }
};
