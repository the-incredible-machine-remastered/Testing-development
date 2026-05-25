#pragma once
// ============================================================================
// Trampolin — Objeto estático con superficie elástica de alto rebote hacia arriba
// ============================================================================

#include "obstaculo_estatico.h"

class Trampolin : public ObstaculoEstatico {
protected:
    double ancho;
    double alto;
    double fuerza_rebote; // Velocidad hacia arriba a aplicar (ej. 600.0)
    double deformacion;          // Deformación actual de la lona en px
    double velocidad_deformacion; // Velocidad de la oscilación de la lona

public:
    Trampolin(int id, Vector2D pos_inicial, double w = 80.0, double h = 20.0, double fuerza = 500.0)
        : ObstaculoEstatico(id, pos_inicial, TipoForma::AABB), ancho(w), alto(h), fuerza_rebote(fuerza),
          deformacion(0.0), velocidad_deformacion(0.0) {
        set_restitucion(0.9); // Alto rebote normal
        set_friccion(0.2);     // Poca fricción en la lona
    }

    // --- Getters & Setters ---
    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    double get_fuerza_rebote() const { return fuerza_rebote; }
    double get_deformacion() const { return deformacion; }
    void set_deformacion(double d) { deformacion = d; }

    Vector2D get_min() const { return posicion; }
    Vector2D get_max() const { return Vector2D(posicion.x + ancho, posicion.y + alto); }

    // Identificador para casting rápido
    bool es_trampolin() const { return true; }

    // Simular el movimiento amortiguado de la lona en cada paso físico
    void actualizar_fisica(double dt) override {
        // Resorte amortiguado: F = -k*x - c*v
        double k = 600.0; // Rigidez
        double c = 12.0;  // Amortiguamiento
        double fuerza_resorte = -k * deformacion - c * velocidad_deformacion;

        velocidad_deformacion += fuerza_resorte * dt;
        deformacion += velocidad_deformacion * dt;

        // Resetear variables dinámicas para evitar que se desplace físicamente
        velocidad = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        fuerza_neta = Vector2D(0.0, 0.0);
    }
};

// TIM_MENU_SPAWN etiqueta="Trampolin" tab=0 categoria=0
