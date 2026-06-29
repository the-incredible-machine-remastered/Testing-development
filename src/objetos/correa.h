#pragma once
// ============================================================================
// Correa - Belt connection that transmits rotation between two rotating objects
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include <sstream>
#include <vector>
#include <algorithm>

class Correa : public EntidadFisica {
private:
    int entidad_a_id;
    int entidad_b_id;

    static EntidadFisica* buscar_entidad(const std::vector<EntidadFisica*>& entidades, int id) {
        for (auto* e : entidades) {
            if (e && e->get_id() == id) return e;
        }
        return nullptr;
    }

public:
    Correa(int id, int aid, int bid)
        : EntidadFisica(id, Vector2D(), 0.0, TipoForma::NINGUNA, true),
          entidad_a_id(aid), entidad_b_id(bid) {
        tipo_menu = TipoObjetoMenu::CORREA;
    }

    int get_entidad_a_id() const { return entidad_a_id; }
    int get_entidad_b_id() const { return entidad_b_id; }

    static double get_radio_eje(const EntidadFisica* e) {
        return e ? e->get_radio_eje() : 1.0;
    }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::CORREA;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent CORREA id=" << get_id()
           << " aid=" << entidad_a_id << " bid=" << entidad_b_id
           << " fijo=" << (es_fijo ? 1 : 0)
           << " tipo_menu=" << static_cast<int>(tipo_menu);
        return ss.str();
    }

    void aplicar_tension_correa(const std::vector<EntidadFisica*>& entidades) {
        EntidadFisica* a = buscar_entidad(entidades, entidad_a_id);
        EntidadFisica* b = buscar_entidad(entidades, entidad_b_id);
        if (!a || !b) return;

        double R_a = get_radio_eje(a);
        double R_b = get_radio_eje(b);

        double w_a = a->get_velocidad_angular();
        double w_b = b->get_velocidad_angular();

        // Tangential surface velocities of the hubs
        double v_a = w_a * R_a;
        double v_b = w_b * R_b;

        double diff = v_a - v_b;

        // Belt constraint coupling force (acts like an angular spring/damper)
        double k_coupling = 520.0;
        double fuerza = k_coupling * diff;

        // Apply corrective torques to both hubs
        a->aplicar_torque(-fuerza * R_a);
        b->aplicar_torque(fuerza * R_b);
    }
};

// TIM_MENU_SPAWN id=CORREA etiqueta="Correa" tab=1 categoria=0
