#pragma once
// ============================================================================
// Escalon — rampita CHICA (triángulo pequeño). Hereda de PlanoInclinado, así que
// reutiliza toda la física de rampa (colisión POLIGONO, desvío, resize, invertir).
// Solo cambia el tamaño por defecto (pequeño) y su tipo de menú.
// ============================================================================

#include "plano_inclinado.h"

class Escalon : public PlanoInclinado {
public:
    Escalon(int id, Vector2D pos, double b = 50.0, double h = 26.0, bool invertido = false)
        : PlanoInclinado(id, pos, b, h, invertido) {
        tipo_menu = TipoObjetoMenu::ESCALON;
    }

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::ESCALON; }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent ESCALON id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " b=" << base_ancho << " h=" << altura_alto
           << " inv=" << (es_invertido ? 1 : 0)
           << " fijo=" << (es_fijo ? 1 : 0);
        return ss.str();
    }
};

// TIM_MENU_SPAWN etiqueta="Escalon" tab=0 categoria=0
