#pragma once
// ============================================================================
// Dinamita — al ser golpeada por CUALQUIER objeto dinámico, se enciende la mecha
// y tras ~1s EXPLOTA: destruye los Ladrillos en su radio y empuja a los objetos
// dinámicos cercanos. La explosión la procesa el motor (necesita ver a los demás).
// ============================================================================

#include "obstaculo_estatico.h"
#include "../sistema/assets_extern.h"
#include <cmath>
#include <sstream>

class Dinamita : public ObstaculoEstatico {
protected:
    double ancho;
    double alto;
    double radio_explosion;
    double fuerza_empuje;

    bool encendida;      // mecha quemándose
    bool exploto;        // ya explotó (una vez)
    bool pendiente_boom; // pide al motor procesar la explosión este frame
    double tiempo;       // segundos desde que se encendió
    double retardo;      // ~1s (0 = explota inmediato, ver DinamitaDetonador)

public:
    Dinamita(int id, Vector2D pos, double w = 26.0, double h = 46.0)
        : ObstaculoEstatico(id, pos, TipoForma::AABB), ancho(w), alto(h),
          radio_explosion(120.0), fuerza_empuje(650.0),
          encendida(false), exploto(false), pendiente_boom(false),
          tiempo(0.0), retardo(1.0) {
        set_restitucion(0.1);
        set_friccion(0.6);
        tipo_menu = TipoObjetoMenu::DINAMITA;
    }

    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    double get_radio_explosion() const { return radio_explosion; }
    double get_fuerza_empuje() const { return fuerza_empuje; }
    bool get_encendida() const { return encendida; }
    bool get_exploto() const { return exploto; }
    bool get_pendiente_boom() const { return pendiente_boom; }
    void consumir_boom() { pendiente_boom = false; }
    double get_progreso() const { return retardo > 0.0 ? MathUtils::clamp(tiempo / retardo, 0.0, 1.0) : 0.0; }

    Vector2D get_min() const override { return posicion; }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho, posicion.y + alto); }
    Vector2D get_centro() const { return Vector2D(posicion.x + ancho * 0.5, posicion.y + alto * 0.5); }

    // Cualquier golpe de un objeto dinámico enciende la mecha.
    void on_collision(EntidadFisica* otro, const InfoColision& info) override {
        if (!otro || otro->get_es_estatico()) return;
        encender();
    }

    void encender() {
        if (!encendida && !exploto) { encendida = true; tiempo = 0.0; }
    }

    void actualizar_fisica(double dt) override {
        if (encendida && !exploto) {
            tiempo += dt;
            if (tiempo >= retardo) {
                encendida = false;
                exploto = true;
                pendiente_boom = true; // el motor hará el daño/empuje este frame
            }
        }
    }

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::DINAMITA; }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent DINAMITA id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " w=" << ancho << " h=" << alto
           << " fijo=" << (es_fijo ? 1 : 0);
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - 8 && p.x <= posicion.x + ancho + 8 &&
               p.y >= posicion.y - 8 && p.y <= posicion.y + alto + 8;
    }

    void dibujar(bool debug) const override {
        if (exploto) return;
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float w  = static_cast<float>(ancho);
        float h  = static_cast<float>(alto);

        // Cartuchos rojos (3 barras) atados
        Color rojo  = encendida ? Color{225, 70, 55, 255} : Color{190, 55, 45, 255};
        Color rojo2 = Color{140, 35, 28, 255};
        float cw = w / 3.0f;
        for (int i = 0; i < 3; ++i) {
            float bx = px + i * cw;
            DrawRectangleRec({bx + 1.0f, py + 6.0f, cw - 2.0f, h - 6.0f}, rojo);
            DrawRectangleLinesEx({bx + 1.0f, py + 6.0f, cw - 2.0f, h - 6.0f}, 1.0f, rojo2);
        }
        // Cinta central
        DrawRectangleRec({px, py + h * 0.45f, w, 5.0f}, Color{90, 60, 30, 255});

        // Mecha arriba
        Vector2D base_mecha(px + w * 0.5f, py + 6.0f);
        Vector2D punta_mecha(px + w * 0.5f + 6.0f, py - 10.0f);
        DrawLineEx({(float)base_mecha.x, (float)base_mecha.y}, {(float)punta_mecha.x, (float)punta_mecha.y}, 2.5f, Color{70, 55, 35, 255});
        if (encendida) {
            // Chispa que baja por la mecha hacia los cartuchos
            double t = get_progreso();
            Vector2D chispa = punta_mecha + (base_mecha - punta_mecha) * t;
            float sr = 4.0f + 2.0f * (float)std::sin(tiempo * 30.0);
            DrawCircle((int)chispa.x, (int)chispa.y, sr, Color{255, 200, 60, 235});
            DrawCircle((int)chispa.x, (int)chispa.y, sr * 0.5f, Color{255, 255, 210, 255});
        }

        if (debug) {
            DrawRectangleLines((int)px, (int)py, (int)w, (int)h, GREEN);
            DrawCircleLines((int)(px + w*0.5f), (int)(py + h*0.5f), (float)radio_explosion, RED);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Dinamita" tab=1 categoria=0
