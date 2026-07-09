#pragma once
// ============================================================================
// Canon — dispara una Bola cuando se le enciende la MECHA. La mecha se prende
// cuando el haz de una Lupa activa lo alcanza (encender_mecha()); tras ~1.5s de
// animación (una chispa que avanza por la mecha), dispara fuerte.
//
// Apunta a la IZQUIERDA por defecto y es rotable con F (como la pistola).
// La bala es el mismo modelo que Bola (bola.h), con sus características por defecto.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "../sistema/assets_extern.h"
#include "bola.h"
#include <cmath>
#include <sstream>

class Canon : public EntidadFisica {
private:
    double angulo;          // dirección de disparo en radianes (PI = izquierda)
    double velocidad_bala;  // fuerza del disparo
    double radio_bala;

    bool mecha_encendida;   // la mecha está quemándose
    bool disparado;         // pidió generar la bala este frame
    bool ya_disparo;        // solo una vez
    double tiempo_mecha;    // segundos transcurridos desde que se encendió
    double duracion_mecha;  // ~1.5s

public:
    Canon(int id, Vector2D pos, double ang_grados = 180.0)
        : EntidadFisica(id, pos, 0.0, TipoForma::AABB, true),
          angulo(ang_grados * MathUtils::TIM_PI / 180.0),
          velocidad_bala(1000.0), radio_bala(22.0),
          mecha_encendida(false), disparado(false), ya_disparo(false),
          tiempo_mecha(0.0), duracion_mecha(1.5) {
        tipo_menu = TipoObjetoMenu::CANON;
    }

    double get_angulo() const { return angulo; }
    double get_angulo_grados() const { return angulo * 180.0 / MathUtils::TIM_PI; }
    bool get_mecha_encendida() const { return mecha_encendida; }
    bool get_ya_disparo() const { return ya_disparo; }
    bool get_disparado() const { return disparado; }
    double get_tiempo_mecha() const { return tiempo_mecha; }
    double get_duracion_mecha() const { return duracion_mecha; }
    // Progreso de la chispa por la mecha, 0..1 (para la animación).
    double get_progreso_mecha() const {
        return duracion_mecha > 0.0 ? MathUtils::clamp(tiempo_mecha / duracion_mecha, 0.0, 1.0) : 0.0;
    }

    void invertir() {
        angulo += MathUtils::TIM_PI;
        if (angulo > MathUtils::TIM_PI) angulo -= 2.0 * MathUtils::TIM_PI;
    }

    // Inclina el cañon en pasos, hacia ARRIBA (sube la boca) o hacia abajo. Respeta
    // el lado al que apunta (izq/der) y limita la inclinacion a +-60 grados sobre
    // la horizontal para que no se pase de vertical.
    void inclinar(bool hacia_arriba) {
        const double paso = 10.0 * MathUtils::TIM_PI / 180.0;   // 10 grados por pulsacion
        const double lim  = 60.0 * MathUtils::TIM_PI / 180.0;   // tope respecto a horizontal
        bool apunta_der = (std::cos(angulo) >= 0.0);
        // En Y-down, subir la boca = disminuir el angulo (der) o aumentarlo (izq).
        double delta = (hacia_arriba ? -paso : paso) * (apunta_der ? 1.0 : -1.0);
        double nuevo = angulo + delta;
        // Limitar respecto a la horizontal del lado correspondiente.
        if (apunta_der) {
            nuevo = MathUtils::clamp(nuevo, -lim, lim);
        } else {
            // Izquierda: angulo cerca de +-PI. Normalizamos midiendo la desviacion.
            double base = MathUtils::TIM_PI;
            double desv = nuevo - base;
            if (desv < -MathUtils::TIM_PI) desv += 2.0 * MathUtils::TIM_PI;
            desv = MathUtils::clamp(desv, -lim, lim);
            nuevo = base + desv;
        }
        angulo = nuevo;
    }

    Vector2D get_dir_disparo() const { return Vector2D(std::cos(angulo), std::sin(angulo)); }
    // Boca del cañón (de donde sale la bala). Cañón grande: tubo largo.
    Vector2D get_punto_boca() const { return posicion + get_dir_disparo() * 50.0; }

    // Llamado por el haz de la Lupa al alcanzar el cañón: prende la mecha.
    void encender_mecha() {
        if (!ya_disparo && !mecha_encendida) {
            mecha_encendida = true;
            tiempo_mecha = 0.0;
        }
    }

    // Avanza la mecha; al terminar marca el disparo (la bala la genera el motor).
    void actualizar_fisica(double dt) override {
        if (mecha_encendida && !ya_disparo) {
            tiempo_mecha += dt;
            if (tiempo_mecha >= duracion_mecha) {
                mecha_encendida = false;
                disparado = true;
                ya_disparo = true;
            }
        }
    }

    // El motor pregunta esto tras actualizar; si dispara, genera la bala.
    bool tiene_disparo_pendiente() const { return disparado; }
    void resetear_disparo() { disparado = false; }
    Bola* crear_bala(int nuevo_id) {
        // Bala de cañón: bola NEGRA (bolos), grande y PESADA, con gran fuerza.
        Bola* bala = new Bola(nuevo_id, get_punto_boca(), radio_bala, 12.0);
        bala->set_velocidad(get_dir_disparo() * velocidad_bala);
        bala->set_restitucion(0.15);      // apenas rebota (es de hierro)
        bala->set_amortiguamiento(0.005);
        bala->set_tipo_menu(TipoObjetoMenu::BOLA_BOLOS); // negra
        bala->set_color_idx(2);
        bala->set_texture_idx(0);
        resetear_disparo();
        return bala;
    }

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::CANON; }

    Vector2D get_min() const override { return posicion - Vector2D(44, 30); }
    Vector2D get_max() const override { return posicion + Vector2D(44, 30); }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - 48 && p.x <= posicion.x + 48 &&
               p.y >= posicion.y - 34 && p.y <= posicion.y + 34;
    }

    
    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent CANON id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " ang=" << get_angulo_grados();
        return ss.str();
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        bool apunta_izq = (std::cos(angulo) < 0);
        float d = apunta_izq ? -1.0f : 1.0f;

        if (tex_canon_tubo.id > 0 && tex_canon_base.id > 0) {
            // 2. Dibujar el tubo del cañón (rotado y/o espejado según su dirección)
            float w_tubo = 110.0f;
            float h_tubo = w_tubo * ((float)tex_canon_tubo.height / (float)tex_canon_tubo.width);
            float flip_tubo = apunta_izq ? -1.0f : 1.0f;
            // El lado izquierdo se ve bien con origin {w_tubo-30, h/2}. El derecho
            // usaba {0, h/2}, que dejaba el tubo corrido respecto a la base: usamos
            // el mismo offset trasero (30px) para que calce igual en ambos lados.
            Vector2 origin_tubo = apunta_izq ? Vector2{w_tubo - 30.0f, h_tubo / 2.0f}
                                             : Vector2{30.0f, h_tubo / 2.0f};
            float angulo_rot = apunta_izq ? (float)(angulo - MathUtils::TIM_PI) : (float)angulo;
            float rotacion_grados = angulo_rot * (180.0f / (float)MathUtils::TIM_PI);

            Rectangle src_tubo = {0.0f, 0.0f, (float)tex_canon_tubo.width * flip_tubo, (float)tex_canon_tubo.height};
            Rectangle dst_tubo = {px, py, w_tubo, h_tubo};
            DrawTexturePro(tex_canon_tubo, src_tubo, dst_tubo, origin_tubo, rotacion_grados, WHITE);
            
            // 1. Dibujar la base (cureña)
            float w_base = 88.0f;
            float h_base = w_base * ((float)tex_canon_base.height / (float)tex_canon_base.width);
            float flip_base = apunta_izq ? -1.0f : 1.0f;
            Rectangle src_base = {0.0f, 0.0f, (float)tex_canon_base.width * flip_base, (float)tex_canon_base.height};
            Rectangle dst_base = {px, py + 30.0f - h_base / 2.0f, w_base, h_base};
            Vector2 origin_base = {w_base / 2.0f, h_base / 2.0f};
            DrawTexturePro(tex_canon_base, src_base, dst_base, origin_base, 0.0f, WHITE);

            // 3. Dibujar la mecha (soga) y chispa encima
            if (!ya_disparo) {
                Vector2D dir = get_dir_disparo();
                Vector2D atras = posicion - dir * 22.0;
                Vector2D fin_mecha = atras - dir * 20.0 + Vector2D(0, -18);
                
                DrawLineEx({(float)atras.x, (float)atras.y}, {(float)fin_mecha.x, (float)fin_mecha.y}, 4.0f, Color{60, 45, 30, 255});
                if (mecha_encendida) {
                    double t = get_progreso_mecha();
                    Vector2D chispa = fin_mecha + (atras - fin_mecha) * t;
                    float sr = 6.0f + 3.0f * (float)std::sin(tiempo_mecha * 30.0);
                    DrawCircle((int)chispa.x, (int)chispa.y, sr, Color{255, 200, 60, 230});
                    DrawCircle((int)chispa.x, (int)chispa.y, sr * 0.5f, Color{255, 255, 210, 255});
                    DrawLineEx({(float)fin_mecha.x, (float)fin_mecha.y}, {(float)chispa.x, (float)chispa.y}, 4.0f, Color{30, 25, 20, 255});
                }
            }
        } else {
            Color metal  = Color{70, 75, 82, 255};
            Color metal2 = Color{45, 48, 54, 255};
            Color madera = Color{110, 70, 35, 255};

            // Base/cureña (madera) — cañón grande
            DrawRectangleRec({px - 28.0f, py + 10.0f, 56.0f, 16.0f}, madera);
            DrawRectangleLinesEx({px - 28.0f, py + 10.0f, 56.0f, 16.0f}, 2.0f, Color{70, 45, 22, 255});
            DrawCircle((int)(px - 17.0f), (int)(py + 27.0f), 8.0f, metal2);
            DrawCircle((int)(px + 17.0f), (int)(py + 27.0f), 8.0f, metal2);
            DrawCircle((int)(px - 17.0f), (int)(py + 27.0f), 3.0f, metal);
            DrawCircle((int)(px + 17.0f), (int)(py + 27.0f), 3.0f, metal);

            // Tubo del cañón (apuntando en la dirección)
            Vector2D dir = get_dir_disparo();
            Vector2D boca = get_punto_boca();
            DrawLineEx({px, py}, {(float)boca.x, (float)boca.y}, 28.0f, metal);
            DrawLineEx({px, py}, {(float)boca.x, (float)boca.y}, 18.0f, metal2);
            // Aro de la boca
            DrawCircle((int)boca.x, (int)boca.y, 14.0f, metal);
            DrawCircle((int)boca.x, (int)boca.y, 9.0f, Color{20, 20, 24, 255});
            // Culata
            DrawCircle((int)px, (int)py, 17.0f, metal);
            DrawCircle((int)px, (int)py, 6.0f, metal2);

            // --- Mecha (atrás, opuesto a la boca) ---
            Vector2D atras = posicion - dir * 22.0;
            Vector2D fin_mecha = atras - dir * 20.0 + Vector2D(0, -18);
            if (!ya_disparo) {
                DrawLineEx({(float)atras.x, (float)atras.y}, {(float)fin_mecha.x, (float)fin_mecha.y}, 4.0f,
                           Color{60, 45, 30, 255});
                if (mecha_encendida) {
                    // Chispa que avanza desde la punta de la mecha hacia el cañón.
                    double t = get_progreso_mecha();
                    Vector2D chispa = fin_mecha + (atras - fin_mecha) * t;
                    float sr = 6.0f + 3.0f * (float)std::sin(tiempo_mecha * 30.0);
                    DrawCircle((int)chispa.x, (int)chispa.y, sr, Color{255, 200, 60, 230});
                    DrawCircle((int)chispa.x, (int)chispa.y, sr * 0.5f, Color{255, 255, 210, 255});
                    DrawLineEx({(float)fin_mecha.x, (float)fin_mecha.y}, {(float)chispa.x, (float)chispa.y}, 4.0f,
                               Color{30, 25, 20, 255});
                }
            }
        }

        if (debug) {
            DrawRectangleLines((int)(px - 48), (int)(py - 34), 96, 68, GREEN);
            DrawLine((int)px, (int)py, (int)get_punto_boca().x, (int)get_punto_boca().y, GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Canon" tab=0 categoria=0
