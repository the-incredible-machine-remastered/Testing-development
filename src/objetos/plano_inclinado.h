#pragma once
// ============================================================================
// PlanoInclinado — Rampa triangular (polígono convexo de 3 vértices)
// Útil para: toboganes, desvíos de proyectiles, rampas.
//
// Coordenadas de pantalla (Raylib): Origen top-left, Y+ hacia abajo.
//
//   invertido=false (pendiente \):    invertido=true (pendiente /):
//
//     (x,y) *                                        * (x+b, y)
//           |\                                      /|
//           | \                                    / |
//           |  \                                  /  |
//   (x,y+h) *---* (x+b, y+h)     (x, y+h) *---* (x+b, y+h)
//
// ============================================================================

#include "obstaculo_estatico.h"
#include <vector>

class PlanoInclinado : public ObstaculoEstatico {
protected:
    double base_ancho;
    double altura_alto;
    double angulo_inclinacion;
    std::vector<Vector2D> vertices;
    bool es_invertido;

public:
    PlanoInclinado(int id, Vector2D pos_inicial, double b, double h, bool invertido = false)
        : ObstaculoEstatico(id, pos_inicial, TipoForma::POLIGONO),
          base_ancho(b), altura_alto(h), es_invertido(invertido) {

        set_restitucion(0.3);
        set_friccion(0.5);

        // Vértices en orden antihorario (para pantalla con Y-down).
        // Esto asegura que las normales de arista apunten hacia afuera.
        recalcular_vertices();

        // Ángulo exacto de la pendiente (sin aproximaciones)
        angulo_inclinacion = std::atan2(h, b);
    }

    // Recalcular vértices del triángulo a partir de la posición actual.
    // Necesario tras mover la rampa con arrastre del mouse.
    void recalcular_vertices() {
        vertices.clear();
        if (!es_invertido) {
            // Pendiente \ : de arriba-izquierda a abajo-derecha
            vertices.push_back(Vector2D(posicion.x, posicion.y));                           // Top-Left
            vertices.push_back(Vector2D(posicion.x, posicion.y + altura_alto));             // Bottom-Left
            vertices.push_back(Vector2D(posicion.x + base_ancho, posicion.y + altura_alto));// Bottom-Right
        } else {
            // Pendiente / : de abajo-izquierda a arriba-derecha
            vertices.push_back(Vector2D(posicion.x, posicion.y + altura_alto));             // Bottom-Left
            vertices.push_back(Vector2D(posicion.x + base_ancho, posicion.y + altura_alto));// Bottom-Right
            vertices.push_back(Vector2D(posicion.x + base_ancho, posicion.y));              // Top-Right
        }
    }

    // --- Getters ---
    double get_angulo_inclinacion() const { return angulo_inclinacion; }
    const std::vector<Vector2D>& get_vertices() const { return vertices; }
    double get_base() const { return base_ancho; }
    double get_altura() const { return altura_alto; }
    bool get_invertido() const { return es_invertido; }

    void invertir() {
        es_invertido = !es_invertido;
        recalcular_vertices();
    }
};

// TIM_MENU_SPAWN id=RAMPA etiqueta="Rampa" tab=0 categoria=0
