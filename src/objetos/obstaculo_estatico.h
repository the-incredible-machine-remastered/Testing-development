#pragma once
// ============================================================================
// ObstaculoEstatico — Clase intermedia para objetos inmóviles
// Masa infinita conceptual (masa=0, estático=true). No integra fuerzas.
// ============================================================================

#include "../core/entidad_fisica.h"

class ObstaculoEstatico : public EntidadFisica {
public:
    ObstaculoEstatico(int id, Vector2D pos_inicial, TipoForma forma = TipoForma::AABB)
        : EntidadFisica(id, pos_inicial, 0.0, forma, true) {
        set_restitucion(0.6);
        set_friccion(0.4);
    }

    // Sobrescritura: un obstáculo estático nunca se mueve, ahorra CPU
    void actualizar_fisica(double dt) override {
        // No-op intencional
    }
};
