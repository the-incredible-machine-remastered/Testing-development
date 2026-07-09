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
    double angulo_rotacion;     // Ángulo acumulado del círculo giratorio
    double velocidad_rotacion;  // Velocidad de giro actual (grados/segundo)
    double retroceso_t;         // temporizador del efecto de retroceso (segundos)

public:
    Pistola(int id, Vector2D pos, double ang_grados = 0.0)
        : EntidadFisica(id, pos, 0.0, TipoForma::AABB, true),
          angulo(ang_grados * MathUtils::TIM_PI / 180.0),
          disparada(false), ya_disparo(false), velocidad_bala(1300.0),
          angulo_rotacion(0.0), velocidad_rotacion(0.0), retroceso_t(0.0) {
        tipo_menu = TipoObjetoMenu::PISTOLA;
    }

    double get_angulo() const { return angulo; }
    double get_angulo_grados() const { return angulo * 180.0 / MathUtils::TIM_PI; }
    bool get_disparada() const { return disparada; }
    bool get_ya_disparo() const { return ya_disparo; }
    void resetear_disparo() { disparada = false; }
    void set_ya_disparo() { ya_disparo = true; }
    bool es_activable_por_tension() const override { return !ya_disparo; }
    void activar_por_tension() override {
        if (!ya_disparo) {
            disparada = true;
            ya_disparo = true;
            velocidad_rotacion = 1200.0; // Inicia el giro rápido al disparar
            retroceso_t = 0.18;          // efecto de retroceso (patada del arma)
        }
    }
    double get_velocidad_bala() const { return velocidad_bala; }

    void invertir() {
        angulo = angulo + MathUtils::TIM_PI;
        if (angulo > MathUtils::TIM_PI) angulo -= 2.0 * MathUtils::TIM_PI;
    }

    Vector2D get_dir_disparo() const {
        return Vector2D(std::cos(angulo), std::sin(angulo));
    }

    // Posición donde sale la bala (punta del cañón)
    // NOTA: Puedes modificar el multiplicador (ej. 28.0) para ajustar la distancia longitudinal
    // de salida de la bala, o sumarle un offset vertical si es necesario.
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
        velocidad_rotacion = 1200.0; // Inicia el giro rápido al disparar
        retroceso_t = 0.18;          // efecto de retroceso (patada del arma)
    }

    // Offset de retroceso: la pistola se echa hacia ATRÁS (opuesto al disparo) y
    // vuelve. Devuelve el desplazamiento a aplicar al dibujo.
    Vector2D get_offset_retroceso() const {
        if (retroceso_t <= 0.0) return Vector2D(0.0, 0.0);
        const double dur = 0.18;
        double f = retroceso_t / dur;            // 1 -> 0
        double magnitud = 10.0 * std::sin(f * MathUtils::TIM_PI); // sube y baja: patada+regreso
        return get_dir_disparo() * (-magnitud);  // hacia atras
    }

    void actualizar_fisica(double dt) override {
        EntidadFisica::actualizar_fisica(dt);
        if (velocidad_rotacion > 0.0) {
            angulo_rotacion += velocidad_rotacion * dt;
            // Desaceleración progresiva (frena a un ritmo de 450 grados/s cada segundo)
            velocidad_rotacion -= 450.0 * dt;
            if (velocidad_rotacion < 0.0) {
                velocidad_rotacion = 0.0;
            }
        }
        if (retroceso_t > 0.0) {
            retroceso_t -= dt;
            if (retroceso_t < 0.0) retroceso_t = 0.0;
        }
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
        ss << "ent PISTOLA id=" << get_id() << serializar_base()
           << " deg=" << get_angulo_grados();
        return ss.str();
    }

    void dibujar(bool debug) const override {
        Vector2D off = get_offset_retroceso();
        float px = static_cast<float>(posicion.x + off.x);
        float py = static_cast<float>(posicion.y + off.y);

        bool apunta_izq = (std::cos(angulo) < 0);
        float d = apunta_izq ? -1.0f : 1.0f; // +1 = derecha, -1 = izquierda

        if (tex_pistola.id > 0) {
            float w = 56.0f * 1.4f;
            float h = 36.0f * 1.4f;
            float flip = apunta_izq ? 1.0f : -1.0f;
            
            // Ya no se oscurece la pistola al activarse (usamos WHITE)
            Color tint_color = WHITE;

            Rectangle src = {0.0f, 0.0f, (float)tex_pistola.width * flip, (float)tex_pistola.height};
            Rectangle dst = {px, py, w, h};
            Vector2 origin = {w / 2.0f, h / 2.0f};
            DrawTexturePro(tex_pistola, src, dst, origin, 0.0f, tint_color);

            // Dibujar el círculo giratorio (escala con la pistola *1.4)
            if (tex_pistola_gira.id > 0) {
                float circle_w = 20.0f * 1.4f;
                float circle_h = 20.0f * 1.4f;

                // Rotación acumulada durante la simulación física (frena progresivamente)
                float rotation = (float)angulo_rotacion;

                Rectangle c_src = {0.0f, 0.0f, (float)tex_pistola_gira.width, (float)tex_pistola_gira.height};
                // El círculo está colocado con un ligero offset del centro para quedar alineado en el tambor/centro del arma
                Rectangle c_dst = {px - 4.0f * 1.4f * d, py - 7.0f * 1.4f, circle_w, circle_h};
                Vector2 c_origin = {circle_w / 2.0f, circle_h / 2.0f};
                
                DrawTexturePro(tex_pistola_gira, c_src, c_dst, c_origin, rotation, WHITE);
            }
        } else {
            Color cuerpo = Color{60, 65, 70, 255};
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
        }

        // Flash al disparar (se muestra sobre el sprite)
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
