#pragma once
// ============================================================================
// Banda — Transmite activación de un CajaHamster a un Ventilador.
// Visualmente es una correa dibujada entre los dos objetos.
// No tiene física propia — solo conecta lógicamente dos entidades.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "caja_hamster.h"
#include "ventilador.h"
#include "caja_sorpresa.h"
#include "caminadora.h"
#include <vector>

class Banda : public EntidadFisica {
private:
    int id_origen;   // CajaHamster
    int id_destino;  // Ventilador

    static EntidadFisica* buscar(const std::vector<EntidadFisica*>& ents, int id) {
        for (auto* e : ents) if (e && e->get_id() == id) return e;
        return nullptr;
    }

public:
    Banda(int id, int id_hamster, int id_ventilador)
        : EntidadFisica(id, Vector2D(), 0.0, TipoForma::NINGUNA, true),
          id_origen(id_hamster), id_destino(id_ventilador) {
        tipo_menu = TipoObjetoMenu::BANDA;
    }

    int get_id_origen()  const { return id_origen; }
    int get_id_destino() const { return id_destino; }

    // Propaga el estado activo del hámster al destino cada paso
    void aplicar_transmision(const std::vector<EntidadFisica*>& entidades) {
        auto* hamster = dynamic_cast<CajaHamster*>(buscar(entidades, id_origen));
        if (!hamster) return;

        EntidadFisica* dest = buscar(entidades, id_destino);
        if (!dest) return;

        // Destino: Ventilador
        if (auto* vent = dynamic_cast<Ventilador*>(dest)) {
            if (hamster->get_activo()) {
                double factor = std::min(hamster->get_velocidad_rueda() / 6.0, 1.0);
                vent->set_potencia(1800.0 * factor);
            } else {
                vent->set_potencia(0.0);
            }
            return;
        }

        // Destino: CajaSorpresa
        if (auto* caja = dynamic_cast<CajaSorpresa*>(dest)) {
            if (hamster->get_activo()) caja->activar();
            return;
        }

        // Destino: Caminadora
        if (auto* conv = dynamic_cast<Caminadora*>(dest)) {
            conv->set_activo(hamster->get_activo());
            return;
        }
    }

    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::BANDA;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent BANDA id=" << get_id()
           << " orig=" << id_origen << " dest=" << id_destino
           << " fijo=" << (es_fijo ? 1 : 0)
           << " tipo_menu=" << static_cast<int>(tipo_menu);
        return ss.str();
    }

    void dibujar(bool debug) const override {
        // La banda se dibuja en motor_fisica donde tenemos acceso a las entidades.
        // Aquí solo dibujamos algo si no hay contexto de entidades disponible.
        (void)debug;
    }

    // Dibuja la correa entre hámster y destino (ventilador o caja sorpresa)
    void dibujar_con_entidades(const std::vector<EntidadFisica*>& entidades, bool mostrar_debug) const {
        auto* hamster = dynamic_cast<CajaHamster*>(buscar(entidades, id_origen));
        if (!hamster) return;

        EntidadFisica* dest = buscar(entidades, id_destino);
        if (!dest) return;

        bool activo = hamster->get_activo();

        // Punto de salida: centro-derecho del hámster (donde está el anclaje amarillo)
        Vector2D p_hamster(hamster->get_posicion().x + hamster->get_ancho() * 0.35,
                           hamster->get_posicion().y + hamster->get_alto() * 0.5);

        // Punto de entrada: centro del destino
        Vector2D p_dest;
        auto* vent = dynamic_cast<Ventilador*>(dest);
        auto* caja = dynamic_cast<CajaSorpresa*>(dest);
        auto* conv = dynamic_cast<Caminadora*>(dest);
        if (vent) {
            p_dest = vent->mira_derecha()
                ? Vector2D(vent->get_posicion().x, vent->get_posicion().y + vent->get_alto() * 0.5)
                : Vector2D(vent->get_posicion().x + vent->get_ancho(), vent->get_posicion().y + vent->get_alto() * 0.5);
        } else if (caja) {
            p_dest = Vector2D(caja->get_posicion().x + caja->get_ancho() * 0.5,
                              caja->get_posicion().y + caja->get_alto() * 0.5);
        } else if (conv) {
            p_dest = Vector2D(conv->get_posicion().x + conv->get_ancho() * 0.5,
                              conv->get_posicion().y + conv->get_alto() * 0.5);
        } else {
            return;
        }
        Vector2D p_vent = p_dest; // alias para el resto del código

        Color col_banda = activo ? Color{255, 200, 50, 220} : Color{160, 130, 60, 180};
        float grosor    = activo ? 4.0f : 2.5f;

        // Dos líneas paralelas = correa
        Vector2D dir = (p_vent - p_hamster);
        double mag = dir.magnitud();
        if (mag < 1.0) return;
        Vector2D perp(-dir.y / mag * 3.0, dir.x / mag * 3.0);

        Vector2D a1 = p_hamster + perp;
        Vector2D a2 = p_hamster - perp;
        Vector2D b1 = p_vent + perp;
        Vector2D b2 = p_vent - perp;

        DrawLineEx({(float)a1.x, (float)a1.y}, {(float)b1.x, (float)b1.y}, grosor, col_banda);
        DrawLineEx({(float)a2.x, (float)a2.y}, {(float)b2.x, (float)b2.y}, grosor, col_banda);

        // Flechas de dirección animadas
        if (activo && mag > 40.0) {
            double t = std::fmod(GetTime() * 0.5, 1.0);
            Vector2D mid = p_hamster + dir * t;
            DrawCircle(static_cast<int>(mid.x), static_cast<int>(mid.y), 4.0f, col_banda);
        }

        if (mostrar_debug) {
            DrawLineEx({(float)p_hamster.x, (float)p_hamster.y},
                       {(float)p_vent.x,    (float)p_vent.y}, 1.0f, GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Banda" tab=0 categoria=1
