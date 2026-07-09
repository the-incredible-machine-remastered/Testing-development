#pragma once
// ============================================================================
// LadrilloHorizontal — ladrillo (destructible por dinamita) que SOLO se agranda
// en el eje X (ancho), con límite. El alto queda fijo. Aunque el usuario arrastre
// cualquier handle, set_dimensiones ignora el alto.
// ============================================================================

#include "ladrillo.h"

class LadrilloHorizontal : public Ladrillo {
private:
    // Eje principal (ancho): se agranda hasta ANCHO_MAX. Eje secundario (alto): se puede
    // REDUCIR desde ALTO_MAX (su valor "fijo" original) hasta ALTO_MIN, pero NO aumentar
    // más allá de ALTO_MAX.
    static constexpr double ANCHO_MIN = 40.0;
    static constexpr double ANCHO_MAX = 3000.0;
    static constexpr double ALTO_MIN  = 15.0;
    static constexpr double ALTO_MAX  = 40.0;

public:
    LadrilloHorizontal(int id, Vector2D pos, double w = 120.0, double h = ALTO_MAX)
        : Ladrillo(id, pos, std::max(ANCHO_MIN, std::min(ANCHO_MAX, w)),
                   std::max(ALTO_MIN, std::min(ALTO_MAX, h))) {
        tipo_menu = TipoObjetoMenu::LADRILLO_HORIZONTAL;
    }

    // Ancho libre (40-300). Alto reducible pero tope en ALTO_MAX (no crece más).
    void set_dimensiones(double w, double h) override {
        ancho = std::max(ANCHO_MIN, std::min(ANCHO_MAX, w));
        alto  = std::max(ALTO_MIN,  std::min(ALTO_MAX,  h));
    }

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::LADRILLO_HORIZONTAL; }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent LADRILLO_HORIZONTAL id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " w=" << ancho << " h=" << alto
           << " fijo=" << (es_fijo ? 1 : 0);
        return ss.str();
    }
};

// TIM_MENU_SPAWN etiqueta="Ladrillo H" tab=1 categoria=0
