#pragma once
// ============================================================================
// LadrilloVertical — ladrillo (destructible por dinamita) que SOLO se agranda en
// el eje Y (alto), con límite. El ancho queda fijo. Aunque el usuario arrastre
// cualquier handle, set_dimensiones ignora el ancho.
// ============================================================================

#include "ladrillo.h"

class LadrilloVertical : public Ladrillo {
private:
    // Eje principal (alto): se agranda hasta ALTO_MAX. Eje secundario (ancho): se puede
    // REDUCIR desde ANCHO_MAX (su valor "fijo" original) hasta ANCHO_MIN, pero NO aumentar
    // más allá de ANCHO_MAX.
    static constexpr double ANCHO_MIN = 15.0;
    static constexpr double ANCHO_MAX = 40.0;
    static constexpr double ALTO_MIN  = 40.0;
    static constexpr double ALTO_MAX  = 240.0;

public:
    LadrilloVertical(int id, Vector2D pos, double w = ANCHO_MAX, double h = 120.0)
        : Ladrillo(id, pos, std::max(ANCHO_MIN, std::min(ANCHO_MAX, w)),
                   std::max(ALTO_MIN, std::min(ALTO_MAX, h))) {
        tipo_menu = TipoObjetoMenu::LADRILLO_VERTICAL;
    }

    // Alto libre (40-240). Ancho reducible pero tope en ANCHO_MAX (no crece más).
    void set_dimensiones(double w, double h) override {
        ancho = std::max(ANCHO_MIN, std::min(ANCHO_MAX, w));
        alto  = std::max(ALTO_MIN,  std::min(ALTO_MAX,  h));
    }

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::LADRILLO_VERTICAL; }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent LADRILLO_VERTICAL id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " w=" << ancho << " h=" << alto
           << " fijo=" << (es_fijo ? 1 : 0);
        return ss.str();
    }
};

// TIM_MENU_SPAWN etiqueta="Ladrillo V" tab=1 categoria=0
