#pragma once
// ============================================================================
// SoporteTorque - polea/soporte fijo para pasar una cuerda TIM.
// ============================================================================

#include "../core/entidad_fisica.h"

class SoporteTorque : public EntidadFisica {
private:
    double radio;

public:
    SoporteTorque(int id, Vector2D pos_inicial, double r = 16.0)
        : EntidadFisica(id, pos_inicial, 0.0, TipoForma::NINGUNA, true),
          radio(r) {}

    double get_radio() const { return radio; }
    Vector2D get_punto_cuerda() const { return posicion; }
};

// TIM_MENU_SPAWN id=SOPORTE_TORQUE etiqueta="Torque" tab=0 categoria=0
