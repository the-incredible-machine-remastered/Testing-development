#pragma once
// ============================================================================
// BolaRebotadora -- Obstaculo circular gigante inspirado en las "Big Balls"
// de Wipeout. Es una esfera estatica, elastica y resbalosa que sirve como
// obstaculo de rebote/equilibrio para desviar bolas y mecanismos.
// ============================================================================

#include "obstaculo_estatico.h"
#include <algorithm>
#include <cmath>

class BolaRebotadora : public ObstaculoEstatico {
protected:
    double radio;
    double altura_soporte;
    double deformacion;           // Compresion visual actual de la esfera
    double velocidad_deformacion;  // Velocidad del resorte visual
    double vibracion_timer;        // Tiempo restante de vibracion
    double vibracion_fase;         // Fase senoidal para temblor lateral
    double impacto_cooldown;       // Evita reactivar vibracion por contacto continuo
    double multiplicador_rebote;   // Potencia extra del rebote contra esta bola

public:
    BolaRebotadora(int id, Vector2D pos_centro, double r = 48.0)
        : ObstaculoEstatico(id, pos_centro, TipoForma::CIRCULO),
          radio(r), altura_soporte(34.0), deformacion(0.0),
          velocidad_deformacion(0.0), vibracion_timer(0.0), vibracion_fase(0.0),
          impacto_cooldown(0.0), multiplicador_rebote(3.25) {
        set_restitucion(0.85); // Muy rebotona, como una esfera inflable
        set_friccion(0.18);    // Superficie resbalosa
    }

    // --- Getters ---
    double get_radio() const { return radio; }
    double get_altura_soporte() const { return altura_soporte; }
    double get_deformacion() const { return deformacion; }
    double get_vibracion_timer() const { return vibracion_timer; }
    double get_multiplicador_rebote() const { return multiplicador_rebote; }
    double get_offset_vibracion() const {
        return std::sin(vibracion_fase) * vibracion_timer * 5.0;
    }

    void set_multiplicador_rebote(double m) {
        multiplicador_rebote = std::max(0.0, m);
    }

    // Activar temblor visual al recibir un impacto de una bola
    void registrar_impacto(double velocidad_impacto) {
        if (impacto_cooldown > 0.0 || velocidad_impacto < 35.0) return;

        double impacto = std::min(22.0, std::max(4.0, velocidad_impacto * 0.025));
        deformacion = std::max(deformacion, impacto);
        velocidad_deformacion -= impacto * 22.0;
        vibracion_timer = std::min(0.35, std::max(vibracion_timer, velocidad_impacto * 0.001));
        impacto_cooldown = 0.12;
    }

    // La bola es estatica en fisica, pero su goma vibra visualmente como resorte amortiguado
    void actualizar_fisica(double dt) override {
        double k = 120.0; // Rigidez de la goma inflable
        double c = 22.0;  // Amortiguamiento para que deje de vibrar
        double fuerza_resorte = -k * deformacion - c * velocidad_deformacion;

        velocidad_deformacion += fuerza_resorte * dt;
        deformacion += velocidad_deformacion * dt;

        if (std::abs(deformacion) < 0.05 && std::abs(velocidad_deformacion) < 0.05) {
            deformacion = 0.0;
            velocidad_deformacion = 0.0;
        }
        if (deformacion < 0.0) deformacion = 0.0;
        if (deformacion > 24.0) deformacion = 24.0;

        if (vibracion_timer > 0.0) {
            vibracion_fase += dt * 44.0;
            vibracion_timer -= dt * 1.8;
            if (vibracion_timer < 0.0) vibracion_timer = 0.0;
        }

        if (impacto_cooldown > 0.0) {
            impacto_cooldown -= dt;
            if (impacto_cooldown < 0.0) impacto_cooldown = 0.0;
        }
    }
};
