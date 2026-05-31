#pragma once
// ============================================================================
// Cubeta - recipiente dinamico simple para colgar de cuerdas.
// ============================================================================

#include "../core/entidad_fisica.h"

class Cubeta : public EntidadFisica {
private:
    double ancho;
    double alto;

public:
    Cubeta(int id, Vector2D pos_inicial, double w = 58.0, double h = 52.0, double m = 25.0)
        : EntidadFisica(id, pos_inicial, m, TipoForma::AABB, false),
          ancho(w), alto(h) {
        set_restitucion(0.15);
        set_friccion(0.55);
        set_amortiguamiento(0.01);
        set_inercia((1.0 / 12.0) * m * (w * w + h * h));
    }

    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    Vector2D get_min() const { return posicion; }
    Vector2D get_max() const { return Vector2D(posicion.x + ancho, posicion.y + alto); }
    Vector2D get_punto_cuerda() const { return Vector2D(posicion.x + ancho * 0.5, posicion.y + 4.0); }
};

// TIM_MENU_SPAWN etiqueta="Cubeta" tab=0 categoria=0
