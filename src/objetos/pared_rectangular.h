#pragma once
// ============================================================================
// ParedRectangular — AABB (Axis-Aligned Bounding Box)
// Útil para: límites del nivel, plataformas, ladrillos.
// ============================================================================

#include "obstaculo_estatico.h"

class ParedRectangular : public ObstaculoEstatico {
protected:
    double ancho;
    double alto;

public:
    ParedRectangular(int id, Vector2D pos_inicial, double w, double h)
        : ObstaculoEstatico(id, pos_inicial, TipoForma::AABB), ancho(w), alto(h) {}

    // --- Getters ---
    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }

    // Esquina superior-izquierda (min) y esquina inferior-derecha (max)
    // En coordenadas de pantalla: Y+ apunta hacia abajo.
    Vector2D get_min() const { return posicion; }
    Vector2D get_max() const { return Vector2D(posicion.x + ancho, posicion.y + alto); }
};
