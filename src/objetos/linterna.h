#pragma once
// ============================================================================
// Linterna — objeto ESTÁTICO con colisión que se ENCIENDE al ser impactada por
// un objeto dinámico (una bola que la golpea o cae sobre ella). Una vez
// encendida, proyecta un HAZ de luz direccional hacia donde apunta (rotable con
// F, como la pistola/cañón). Funciona como el Foco pero se activa por impacto.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "../sistema/assets_extern.h"
#include <cmath>
#include <sstream>

class Linterna : public EntidadFisica {
private:
    double angulo;       // dirección del haz en radianes (0 = derecha, PI = izquierda)
    double rango;        // alcance del haz
    double ancho_cuerpo; // media anchura del cuerpo (para hitbox)
    double alto_cuerpo;  // media altura
    bool encendida;
    bool ya_encendida;   // una sola vez

public:
    Linterna(int id, Vector2D pos, double ang_grados = 0.0)
        : EntidadFisica(id, pos, 0.0, TipoForma::AABB, true),
          angulo(ang_grados * MathUtils::TIM_PI / 180.0),
          rango(240.0), ancho_cuerpo(34.0), alto_cuerpo(18.0),
          encendida(false), ya_encendida(false) {
        set_restitucion(0.2);
        set_friccion(0.6);
        tipo_menu = TipoObjetoMenu::LINTERNA;
    }

    double get_angulo() const { return angulo; }
    double get_angulo_grados() const { return angulo * 180.0 / MathUtils::TIM_PI; }
    bool get_encendida() const { return encendida; }
    bool get_ya_encendida() const { return ya_encendida; }
    double get_rango() const { return rango; }

    Vector2D get_dir() const { return Vector2D(std::cos(angulo), std::sin(angulo)); }
    // Boca/salida del haz (frente de la linterna) y punto final del rayo.
    Vector2D get_punto_boca() const { return posicion + get_dir() * ancho_cuerpo; }
    Vector2D get_punto_final() const { return get_punto_boca() + get_dir() * rango; }

    void invertir() {
        angulo += MathUtils::TIM_PI;
        if (angulo > MathUtils::TIM_PI) angulo -= 2.0 * MathUtils::TIM_PI;
    }

    // Inclina el haz en pasos (Q sube la boca / E la baja), respetando el lado.
    void inclinar(bool hacia_arriba) {
        const double paso = 10.0 * MathUtils::TIM_PI / 180.0;
        const double lim  = 60.0 * MathUtils::TIM_PI / 180.0;
        bool apunta_der = (std::cos(angulo) >= 0.0);
        double delta = (hacia_arriba ? -paso : paso) * (apunta_der ? 1.0 : -1.0);
        double nuevo = angulo + delta;
        if (apunta_der) {
            nuevo = MathUtils::clamp(nuevo, -lim, lim);
        } else {
            double base = MathUtils::TIM_PI;
            double desv = nuevo - base;
            if (desv < -MathUtils::TIM_PI) desv += 2.0 * MathUtils::TIM_PI;
            desv = MathUtils::clamp(desv, -lim, lim);
            nuevo = base + desv;
        }
        angulo = nuevo;
    }

    void encender() { if (!ya_encendida) { encendida = true; ya_encendida = true; } }
    void apagar() { encendida = false; }
    void resetear() { encendida = false; ya_encendida = false; }

    // Cualquier golpe de un objeto dinámico enciende la linterna (como la dinamita).
    void on_collision(EntidadFisica* otro, const InfoColision& info) override {
        (void)info;
        if (!otro || otro->get_es_estatico()) return;
        encender();
    }

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::LINTERNA; }

    Vector2D get_min() const override { return posicion - Vector2D(ancho_cuerpo, alto_cuerpo); }
    Vector2D get_max() const override { return posicion + Vector2D(ancho_cuerpo, alto_cuerpo); }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - ancho_cuerpo - 6 && p.x <= posicion.x + ancho_cuerpo + 6 &&
               p.y >= posicion.y - alto_cuerpo - 6 && p.y <= posicion.y + alto_cuerpo + 6;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent LINTERNA id=" << get_id() << serializar_base()
           << " deg=" << get_angulo_grados();
        return ss.str();
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        bool apunta_izq = (std::cos(angulo) < 0.0);

        // --- Haz de luz direccional (solo encendida) ---
        if (encendida) {
            Vector2D b = get_punto_boca();
            Vector2D f = get_punto_final();
            // Cono suave: linea gruesa translucida + nucleo brillante + destello final.
            DrawLineEx({(float)b.x, (float)b.y}, {(float)f.x, (float)f.y}, 14.0f, Color{255, 240, 150, 70});
            DrawLineEx({(float)b.x, (float)b.y}, {(float)f.x, (float)f.y}, 6.0f, Color{255, 250, 200, 150});
            DrawCircle((int)f.x, (int)f.y, 7.0f, Color{255, 245, 170, 130});
        }

        Texture2D tex = encendida ? tex_linterna_encendida : tex_linterna_apagada;
        if (tex.id > 0) {
            // La textura apunta a la IZQUIERDA por defecto (foco a la izquierda).
            // Cuando apunta izquierda: sin flip, rotamos por la desviacion desde PI.
            // Cuando apunta derecha: espejamos la textura y rotamos por el angulo.
            float w = ancho_cuerpo * 2.4f;
            float h = w * ((float)tex.height / (float)tex.width);
            float flip = apunta_izq ? 1.0f : -1.0f;
            float ang_rot = apunta_izq ? (float)(angulo - MathUtils::TIM_PI) : (float)angulo;
            float rot_grados = ang_rot * (180.0f / (float)MathUtils::TIM_PI);
            Rectangle src = {0.0f, 0.0f, (float)tex.width * flip, (float)tex.height};
            Rectangle dst = {px, py, w, h};
            Vector2 origin = {w / 2.0f, h / 2.0f};
            DrawTexturePro(tex, src, dst, origin, rot_grados, WHITE);
        } else {
            // Fallback vectorial: cuerpo cilíndrico + cabezal.
            Color cuerpo = Color{70, 75, 82, 255};
            Color cuerpo2 = Color{45, 48, 54, 255};
            Color aro = encendida ? Color{255, 220, 90, 255} : Color{150, 155, 160, 255};
            Vector2D d = get_dir();
            Vector2D b = get_punto_boca();
            DrawLineEx({px, py}, {(float)b.x, (float)b.y}, (float)(alto_cuerpo * 1.4), cuerpo);
            DrawLineEx({px, py}, {(float)b.x, (float)b.y}, (float)(alto_cuerpo * 0.8), cuerpo2);
            DrawCircle((int)b.x, (int)b.y, (float)(alto_cuerpo * 0.9), aro);
            DrawCircle((int)b.x, (int)b.y, (float)(alto_cuerpo * 0.55),
                       encendida ? Color{255, 245, 180, 255} : Color{30, 32, 36, 255});
            (void)d;
        }

        if (debug) {
            Vector2D mn = get_min(), mx = get_max();
            DrawRectangleLines((int)mn.x, (int)mn.y, (int)(mx.x - mn.x), (int)(mx.y - mn.y), GREEN);
            Vector2D f = get_punto_final();
            DrawLine((int)px, (int)py, (int)f.x, (int)f.y, GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Linterna" tab=0 categoria=0
