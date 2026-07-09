#pragma once
// ============================================================================
// SoporteTorque - polea/soporte fijo para pasar una cuerda TIM.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../sistema/assets_extern.h"
#include <cmath>

class SoporteTorque : public EntidadFisica {
private:
    double radio;

public:
    SoporteTorque(int id, Vector2D pos_inicial, double r = 16.0)
        : EntidadFisica(id, pos_inicial, 0.0, TipoForma::NINGUNA, true),
          radio(r) {
        tipo_menu = TipoObjetoMenu::SOPORTE_TORQUE;
    }

    double get_radio() const { return radio; }
    Vector2D get_punto_cuerda() const { return posicion; }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::SOPORTE;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent SOPORTE id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " r=" << radio
           << " fijo=" << (es_fijo ? 1 : 0)
           << " tipo_menu=" << tipo_objeto_menu_a_string(tipo_menu);
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        double dist = (posicion - p).magnitud();
        return dist < radio + 10.0;
    }

    void dibujar(bool debug) const override {
        Vector2D pos = posicion;
        float r = static_cast<float>(radio);
        DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), r,
                   Color{135, 140, 145, 255});
        DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), r,
                        Color{65, 70, 78, 255});
        DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), 5.0f,
                   Color{35, 38, 42, 255});
    }
};
// TIM_MENU_SPAWN id=SOPORTE_TORQUE etiqueta="Torque" tab=0 categoria=0
