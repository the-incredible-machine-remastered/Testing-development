#pragma once
// ============================================================================
// Raton — criatura DIMINUTA y sólida que huye del Gato. Corre por el suelo
// (gravedad + plataformas). Por ser tan pequeño, cabe por huecos estrechos entre
// ladrillos donde el Gato (más grande) no entra: ese es su "escape". Al ser sólido
// puede empujar/pisar objetos ligeros mientras corre.
//
// Comportamiento (inspirado en seguidor_booster.h): si un Gato entra en su rango
// de visión, el ratón corre en dirección CONTRARIA. Fuera de rango, deambula lento.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "../sistema/assets_extern.h"
#include <vector>
#include <cmath>
#include <algorithm>

class Raton : public EntidadFisica {
protected:
    double ancho;
    double alto;
    double rango_vision;      // detecta al gato dentro de este radio
    double velocidad_huida;   // corriendo asustado
    double velocidad_paseo;   // deambulando tranquilo
    double dir_x;             // +1 derecha, -1 izquierda (hacia dónde mira/corre)
    bool asustado;
    double cambio_paseo_timer; // para alternar dirección al deambular
    double patas_fase;        // animación de patitas

public:
    Raton(int id, Vector2D pos, double w = 22.0, double h = 12.0)
        : EntidadFisica(id, pos, 1.2, TipoForma::AABB, false),
          ancho(w), alto(h), rango_vision(260.0),
          velocidad_huida(230.0), velocidad_paseo(45.0),
          dir_x(-1.0), asustado(false), cambio_paseo_timer(0.0), patas_fase(0.0) {
        set_restitucion(0.05);
        set_friccion(0.5);
        set_amortiguamiento(0.02);
        tipo_menu = TipoObjetoMenu::RATON;
    }

    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    double get_rango_vision() const { return rango_vision; }
    double get_dir_x() const { return dir_x; }
    bool get_asustado() const { return asustado; }
    double get_patas_fase() const { return patas_fase; }

    // AABB centrado en la posición.
    Vector2D get_min() const override { return Vector2D(posicion.x - ancho/2.0, posicion.y - alto/2.0); }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho/2.0, posicion.y + alto/2.0); }

    // Decide dirección según el gato más cercano en rango. Llamado por el motor.
    // gato_x/gato_y: posición del gato más cercano; hay_gato: si se encontró uno.
    void actualizar_comportamiento(bool hay_gato, const Vector2D& gato_pos, double dist_gato, double dt) {
        patas_fase += dt * (asustado ? 26.0 : 9.0);

        if (hay_gato && dist_gato <= rango_vision) {
            asustado = true;
            // Huye: dirección OPUESTA al gato en X.
            dir_x = (gato_pos.x > posicion.x) ? -1.0 : 1.0;
        } else {
            asustado = false;
            // Deambula: cambia de rumbo de vez en cuando.
            cambio_paseo_timer -= dt;
            if (cambio_paseo_timer <= 0.0) {
                cambio_paseo_timer = 1.5 + (std::sin(posicion.x * 0.13 + patas_fase) * 0.5 + 0.5) * 2.0;
                dir_x = (dir_x >= 0.0) ? -1.0 : 1.0;
            }
        }
    }

    void actualizar_fisica(double dt) override {
        // Gravedad + velocidad horizontal objetivo (huida o paseo).
        aceleracion = (masa > MathUtils::EPSILON) ? fuerza_neta / masa : Vector2D(0.0, 0.0);
        velocidad.y += aceleracion.y * dt;
        if (velocidad.y > 700.0) velocidad.y = 700.0;

        double vx_obj = dir_x * (asustado ? velocidad_huida : velocidad_paseo);
        // Suavizar hacia la velocidad objetivo para que no sea robótico.
        velocidad.x += (vx_obj - velocidad.x) * std::min(1.0, dt * 12.0);

        posicion += velocidad * dt;

        fuerza_neta = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        velocidad_angular = 0.0;
        angulo = 0.0;
    }

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::RATON; }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent RATON id=" << get_id() << serializar_base()
           << " w=" << ancho << " h=" << alto;
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - ancho/2 - 8 && p.x <= posicion.x + ancho/2 + 8 &&
               p.y >= posicion.y - alto/2 - 8 && p.y <= posicion.y + alto/2 + 8;
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float w  = static_cast<float>(ancho);
        float h  = static_cast<float>(alto);
        float d  = (dir_x < 0.0) ? -1.0f : 1.0f; // hacia dónde mira

        Color cuerpo = Color{150, 150, 158, 255};
        Color oscuro = Color{95, 95, 105, 255};
        Color rosa   = Color{235, 150, 160, 255};

        // Sombra
        DrawEllipse((int)px, (int)(py + h*0.5f), w*0.5f, 2.5f, Color{0,0,0,60});

        // Patitas (animación simple)
        float paso = std::sin((float)patas_fase) * 2.0f;
        DrawLineEx({px - w*0.2f, py + h*0.35f}, {px - w*0.2f + paso, py + h*0.55f}, 1.5f, oscuro);
        DrawLineEx({px + w*0.2f, py + h*0.35f}, {px + w*0.2f - paso, py + h*0.55f}, 1.5f, oscuro);

        // Cola (lado opuesto a la mirada), curvada
        float colax = px - d * w*0.5f;
        DrawLineBezier({colax, py}, {colax - d*w*0.5f, py - h*0.6f}, 1.8f, rosa);

        // Cuerpo ovalado
        DrawEllipse((int)px, (int)py, w*0.5f, h*0.5f, cuerpo);
        // Cabeza (hacia la dirección)
        float hx = px + d * w*0.42f;
        DrawCircle((int)hx, (int)py, h*0.42f, cuerpo);
        // Orejas
        DrawCircle((int)(hx - d*h*0.15f), (int)(py - h*0.4f), h*0.32f, cuerpo);
        DrawCircle((int)(hx - d*h*0.15f), (int)(py - h*0.4f), h*0.18f, rosa);
        // Ojo
        DrawCircle((int)(hx + d*h*0.15f), (int)(py - h*0.05f), 1.6f, BLACK);
        // Nariz
        DrawCircle((int)(hx + d*h*0.42f), (int)py, 1.5f, rosa);
        // Bigotes
        DrawLineEx({hx + d*h*0.4f, py}, {hx + d*h*0.9f, py - h*0.15f}, 0.8f, oscuro);
        DrawLineEx({hx + d*h*0.4f, py}, {hx + d*h*0.9f, py + h*0.15f}, 0.8f, oscuro);

        if (debug) {
            DrawRectangleLines((int)(px - w/2), (int)(py - h/2), (int)w, (int)h, GREEN);
            DrawCircleLines((int)px, (int)py, (float)rango_vision, Color{0,255,0,50});
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Raton" tab=0 categoria=1
