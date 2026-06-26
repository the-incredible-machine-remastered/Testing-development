#pragma once
#include "../core/entidad_fisica.h"
#include "bola.h"
#include <cmath>
#include <sstream>

class Pistola : public EntidadFisica {
private:
    double angulo;       // dirección de disparo en radianes (0 = derecha)
    bool disparada;
    bool ya_disparo;
    double velocidad_bala;

public:
    Pistola(int id, Vector2D pos, double ang_grados = 0.0)
        : EntidadFisica(id, pos, 0.0, TipoForma::AABB, true),
          angulo(ang_grados * MathUtils::TIM_PI / 180.0),
          disparada(false), ya_disparo(false), velocidad_bala(600.0) {}

    double get_angulo() const { return angulo; }
    double get_angulo_grados() const { return angulo * 180.0 / MathUtils::TIM_PI; }
    bool get_disparada() const { return disparada; }
    bool get_ya_disparo() const { return ya_disparo; }
    void resetear_disparo() { disparada = false; }
    void set_ya_disparo() { ya_disparo = true; }
    void activar_por_tension() { if (!ya_disparo) { disparada = true; ya_disparo = true; } }
    double get_velocidad_bala() const { return velocidad_bala; }

    void invertir() {
        angulo = angulo + MathUtils::TIM_PI;
        if (angulo > MathUtils::TIM_PI) angulo -= 2.0 * MathUtils::TIM_PI;
    }

    Vector2D get_dir_disparo() const {
        return Vector2D(std::cos(angulo), std::sin(angulo));
    }

    // Posición donde sale la bala (punta del cañón)
    Vector2D get_punto_bala() const {
        return posicion + get_dir_disparo() * 28.0;
    }

    void on_collision(EntidadFisica* otro, const InfoColision& info) override {
        if (!otro || ya_disparo) return;
        // Solo dispara si la golpea una bola, beisbol o cubeta — no balancín ni cuerda
        TipoEntidadJuego t = otro->get_tipo_entidad();
        if (t != TipoEntidadJuego::BOLA &&
            t != TipoEntidadJuego::BOLA_REBOTADORA &&
            t != TipoEntidadJuego::BOLA_BEISBOL &&
            t != TipoEntidadJuego::CUBETA) return;
        disparada = true;
        ya_disparo = true;
    }

    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::PISTOLA;
    }

    Vector2D get_min() const override { return posicion - Vector2D(24, 14); }
    Vector2D get_max() const override { return posicion + Vector2D(24, 14); }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - 28 && p.x <= posicion.x + 28 &&
               p.y >= posicion.y - 18 && p.y <= posicion.y + 18;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent PISTOLA id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " ang=" << get_angulo_grados();
        return ss.str();
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);

        bool apunta_izq = (std::cos(angulo) < 0);
        float d = apunta_izq ? -1.0f : 1.0f; // +1 = derecha, -1 = izquierda

        Color cuerpo = ya_disparo ? Color{80, 80, 80, 255} : Color{60, 65, 70, 255};
        Color mango  = Color{100, 60, 30, 255};
        Color metal  = Color{140, 148, 155, 255};

        // Mango inclinado (siempre debajo del cuerpo, hacia atrás)
        DrawRectanglePro(
            {px - 3.0f * d, py + 4.0f, 13.0f, 17.0f},
            {6.5f, 0.0f},
            apunta_izq ? 15.0f : -15.0f,
            mango);

        // Cuerpo principal centrado en px
        DrawRectangleRec({px - 18.0f, py - 8.0f, 36.0f, 14.0f}, cuerpo);

        // Cañón hacia la dirección correcta
        DrawRectangleRec({px + 8.0f * d, py - 5.0f, 18.0f * d, 8.0f}, metal);

        // Gatillo (hacia atrás)
        DrawLineEx({px - 2.0f * d, py + 2.0f}, {px - 7.0f * d, py + 9.0f}, 2.0f, metal);

        // Flash al disparar
        if (disparada) {
            Vector2D punta = get_punto_bala();
            DrawCircle((int)punta.x, (int)punta.y, 8.0f, Color{255, 220, 80, 200});
            DrawCircle((int)punta.x, (int)punta.y, 4.0f, Color{255, 255, 200, 255});
        }

        if (debug) {
            DrawRectangleLines((int)(posicion.x - 28), (int)(posicion.y - 18), 56, 36, GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Pistola" tab=0 categoria=0
