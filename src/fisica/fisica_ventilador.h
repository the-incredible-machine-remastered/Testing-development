#pragma once
// ============================================================================
// FisicaVentilador -- Fuerza de aire aplicada por ventiladores
//
// Mantiene la logica del viento separada del MotorFisica principal.
// Un ventilador empuja bolas dentro de una zona rectangular al frente, con
// atenuacion lineal segun la distancia.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../objetos/bola.h"
#include "../objetos/ventilador.h"
#include <vector>
#include <cmath>

namespace FisicaVentilador {

    inline void aplicar(const std::vector<EntidadFisica*>& entidades) {
        for (auto* origen : entidades) {
            auto* ventilador = dynamic_cast<Ventilador*>(origen);
            if (!ventilador) continue;

            Vector2D salida = ventilador->get_centro_salida();
            Vector2D dir = ventilador->get_direccion();
            Vector2D lateral(-dir.y, dir.x);

            for (auto* objetivo : entidades) {
                auto* bola = dynamic_cast<Bola*>(objetivo);
                if (!bola) continue;

                Vector2D rel = bola->get_posicion() - salida;
                double avance = Vector2D::dot(rel, dir);
                double distancia_lateral = std::abs(Vector2D::dot(rel, lateral));

                if (avance < 0.0 || avance > ventilador->get_rango()) continue;
                if (distancia_lateral > ventilador->get_ancho_corriente() / 2.0 + bola->get_radio()) continue;

                double factor_distancia = (1.0 - avance / ventilador->get_rango()) * (50.0 / (avance + 15.0));
                double factor_centro = 1.0 - distancia_lateral / (ventilador->get_ancho_corriente() / 2.0 + bola->get_radio());
                double fuerza = ventilador->get_potencia() * factor_distancia * factor_centro;

                bola->aplicar_fuerza(dir * fuerza);
            }
        }
    }

}  
