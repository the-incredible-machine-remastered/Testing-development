#pragma once
// ============================================================================
// Ventilador -- Objeto estatico que empuja bolas con una corriente de aire
// horizontal. No es una pared: su funcion principal es aplicar una fuerza
// a distancia dentro de una zona rectangular frente a sus aspas.
// ============================================================================

#include "obstaculo_estatico.h"

class Ventilador : public ObstaculoEstatico {
protected:
    double ancho;
    double alto;
    double rango;
    double ancho_corriente;
    double potencia;
    Vector2D direccion;
    double fase_aspas;

public:
    Ventilador(int id, Vector2D pos, double w = 42.0, double h = 54.0)
        : ObstaculoEstatico(id, pos, TipoForma::AABB),
          ancho(w), alto(h), rango(150.0), ancho_corriente(92.0),
          potencia(950.0), direccion(1.0, 0.0), fase_aspas(0.0) {
        set_restitucion(0.2);
        set_friccion(0.5);
    }

    // --- Getters ---
    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    double get_rango() const { return rango; }
    double get_ancho_corriente() const { return ancho_corriente; }
    double get_potencia() const { return potencia; }
    Vector2D get_direccion() const { return direccion; }
    double get_fase_aspas() const { return fase_aspas; }

    Vector2D get_min() const { return posicion; }
    Vector2D get_max() const { return Vector2D(posicion.x + ancho, posicion.y + alto); }
    Vector2D get_centro_salida() const {
        return Vector2D(posicion.x + ancho, posicion.y + alto / 2.0);
    }

    void set_potencia(double p) { potencia = p; }
    void set_direccion(const Vector2D& dir) { direccion = dir.normalizar(); }

    void actualizar_fisica(double dt) override {
        fase_aspas += dt * 18.0;
    }
};

// TIM_MENU_SPAWN etiqueta="Ventilador" tab=0 categoria=1
