#pragma once
// ============================================================================
// Trampolin — Objeto estático con superficie elástica de alto rebote hacia arriba
// ============================================================================

#include "obstaculo_estatico.h"

class Trampolin : public ObstaculoEstatico {
protected:
    double ancho;
    double alto;
    double fuerza_rebote; // Velocidad hacia arriba a aplicar (ej. 600.0)

public:
    Trampolin(int id, Vector2D pos_inicial, double w = 80.0, double h = 20.0, double fuerza = 500.0)
        : ObstaculoEstatico(id, pos_inicial, TipoForma::AABB), ancho(w), alto(h), fuerza_rebote(fuerza) {
        set_restitucion(0.9); // Alto rebote normal
        set_friccion(0.2);     // Poca fricción en la lona
    }

    // --- Getters ---
    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    double get_fuerza_rebote() const { return fuerza_rebote; }

    Vector2D get_min() const { return posicion; }
    Vector2D get_max() const { return Vector2D(posicion.x + ancho, posicion.y + alto); }

    // Identificador para casting rápido
    bool es_trampolin() const { return true; }
};
