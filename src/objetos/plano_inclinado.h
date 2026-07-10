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
#include "../sistema/assets_extern.h"
#include <vector>
#include <cmath>
#include <algorithm>

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
        tipo_menu = TipoObjetoMenu::RAMPA;
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

    void set_dimensiones(double b, double h) {
        base_ancho = b;
        altura_alto = h;
        recalcular_vertices();
        angulo_inclinacion = std::atan2(altura_alto, base_ancho);
    }

    void alternar_tamano() {
        // Ciclar entre 3 tamaños predefinidos:
        // Mediano (160x120) -> Grande (240x180) -> Pequeño (80x60) -> Mediano (160x120)
        double b = base_ancho;
        double h = altura_alto;
        
        if (std::abs(b - 160.0) < 1.0 && std::abs(h - 120.0) < 1.0) {
            set_dimensiones(240.0, 180.0);
        } else if (std::abs(b - 240.0) < 1.0 && std::abs(h - 180.0) < 1.0) {
            set_dimensiones(80.0, 60.0);
        } else {
            set_dimensiones(160.0, 120.0);
        }
    }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::RAMPA;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent RAMPA id=" << get_id() << serializar_base()
           << " b=" << base_ancho << " h=" << altura_alto
           << " inv=" << (es_invertido ? 1 : 0);
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        if (vertices.size() < 3) return false;
        double min_x = std::min({vertices[0].x, vertices[1].x, vertices[2].x});
        double max_x = std::max({vertices[0].x, vertices[1].x, vertices[2].x});
        double min_y = std::min({vertices[0].y, vertices[1].y, vertices[2].y});
        double max_y = std::max({vertices[0].y, vertices[1].y, vertices[2].y});
        return p.x >= min_x - 10 && p.x <= max_x + 10 &&
               p.y >= min_y - 10 && p.y <= max_y + 10;
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float pw = static_cast<float>(base_ancho);
        float ph = static_cast<float>(altura_alto);

        bool texturizado = false;
        if (!es_invertido && tex_plata_rampa_der.id > 0) {
            DrawTexturePro(
                tex_plata_rampa_der,
                { 0 , 0, static_cast<float>(tex_plata_rampa_der.width), static_cast<float>(tex_plata_rampa_der.height) },
                { px, py, pw, ph },
                { 0, 0 },
                0.0f,
                WHITE
            );
            texturizado = true;
        } else if (es_invertido && tex_plata_rampa_izq.id > 0) {
            DrawTexturePro(
                tex_plata_rampa_izq,
                { 0, 0, (float)tex_plata_rampa_izq.width, (float)tex_plata_rampa_izq.height },
                { px, py, pw, ph },
                { 0, 0 },
                0.0f,
                WHITE
            );
            texturizado = true;
        }

        if (!texturizado) {
            if (vertices.size() >= 3) {
                Vector2 v1 = {static_cast<float>(vertices[0].x), static_cast<float>(vertices[0].y)};
                Vector2 v2 = {static_cast<float>(vertices[1].x), static_cast<float>(vertices[1].y)};
                Vector2 v3 = {static_cast<float>(vertices[2].x), static_cast<float>(vertices[2].y)};

                DrawTriangle(v1, v2, v3, COLOR_RAMPA);
                DrawTriangleLines(v1, v2, v3, COLOR_RAMPA_BORDE);
            }
        }
    }
};
// TIM_MENU_SPAWN id=RAMPA etiqueta="Rampa" tab=0 categoria=0
