#pragma once
#include "../core/entidad_fisica.h"
#include "bola.h"
#include "../sistema/assets_extern.h"
#include <cmath>
#include <sstream>
#include <algorithm>

// ============================================================================
// Caminadora — Banda transportadora horizontal con EJE FICTICIO.
// Se construye dinámica pero ancla su traslación cada frame (patrón de Polea /
// CintaTransportadora). Una Correa conectada a una RuedaHamster transmite torque
// real a este eje; la velocidad angular resultante determina la velocidad de la
// cinta y su dirección (según el signo). Aplica fuerza lateral a objetos encima.
// ============================================================================

class Caminadora : public EntidadFisica {
private:
    double ancho;
    double alto;
    bool va_derecha;       // dirección de la correa (derivada del signo de omega)
    bool activo;           // derivado de si el eje supera el umbral de reposo
    double velocidad_cinta; // px/s que empuja los objetos encima (derivada de |omega|)
    double fase_anim;       // para animar las marcas de la cinta
    bool anclada;          // si true, los impactos no la desplazan (queda fija en el aire)

    // Mapeo giro -> velocidad de cinta: a OMEGA_REF rad/s la cinta va a VEL_MAX px/s.
    static constexpr double OMEGA_REF = 7.5;
    static constexpr double VEL_MAX   = 400.0;
    static constexpr double OMEGA_UMBRAL_REPOSO = 0.3; // por debajo, cinta apagada

public:
    Caminadora(int id, Vector2D pos, double w = 150.0, double h = 24.0, bool derecha = true)
        : EntidadFisica(id, pos, 8.0, TipoForma::AABB, false),
          ancho(w), alto(h), va_derecha(derecha),
          activo(false), velocidad_cinta(0.0), fase_anim(0.0), anclada(true) {
        set_inercia(0.5 * masa * 10.0 * 10.0 * 1.5);
        set_restitucion(0.05);
        set_friccion(0.1);
        tipo_menu = TipoObjetoMenu::CAMINADORA;
        velocidad_angular = 0.0;
        angulo = 0.0;
    }

    double get_ancho()  const { return ancho; }
    double get_alto()   const { return alto; }
    bool   get_activo() const { return activo; }
    bool   get_va_derecha() const { return va_derecha; }
    double get_velocidad_cinta() const { return velocidad_cinta; }
    // Radio de eje = el de RuedaHamster para que la Correa iguale su omega.
    double get_radio_eje() const override { return 10.0; }

    void set_activo(bool v) { activo = v; } // legacy: la actividad ahora deriva del eje
    void invertir() { va_derecha = !va_derecha; }

    bool get_anclada() const { return anclada; }
    void set_anclada(bool v) { anclada = v; }
    bool es_inmovil_en_colision() const override { return anclada; }

    // Velocidad de la cinta ajustable (px/s que empuja los objetos al activarse).
    void set_velocidad_cinta(double v) { velocidad_cinta = std::max(0.0, v); }
    void ajustar_velocidad(double delta) { set_velocidad_cinta(velocidad_cinta + delta); }

    // Solo se expande en el eje X (ancho), con LÍMITE máximo. El alto es fijo.
    static constexpr double ANCHO_MIN = 80.0;
    static constexpr double ANCHO_MAX = 400.0;
    void set_dimensiones(double w, double /*h ignorado: alto fijo*/) {
        ancho = std::max(ANCHO_MIN, std::min(ANCHO_MAX, w));
    }

    Vector2D get_min() const override { return posicion; }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho, posicion.y + alto); }

    // Zona superior donde empuja objetos (superficie de la cinta)
    double get_superficie_y() const { return posicion.y; }

    // Velocidad de la superficie en el punto de contacto. IMPORTANTE: sólo
    // componente horizontal (como CintaTransportadora). Si usáramos el giro
    // genérico omega×r (posicion es la esquina, no el centro), el término
    // vertical sería enorme y lanzaría la bola hacia arriba en cada frame.
    Vector2D get_velocidad_en_punto(const Vector2D& punto_mundo) const override {
        (void)punto_mundo;
        double dir = va_derecha ? 1.0 : -1.0;
        return Vector2D(velocidad_cinta * dir, 0.0);
    }

    void actualizar_fisica(double dt) override {
        // Patrón de CintaTransportadora: el eje ficticio gira con el torque
        // acumulado por la Correa; la traslación queda anclada.
        double I = get_inercia();
        double alpha = (I > MathUtils::EPSILON) ? (torque_neto / I) : 0.0;
        velocidad_angular += alpha * dt;
        angulo += velocidad_angular * dt;
        velocidad_angular *= 0.98; // damping natural

        // Derivar estado visible/físico de la cinta desde el eje.
        double omega = velocidad_angular;
        activo = std::abs(omega) > OMEGA_UMBRAL_REPOSO;
        if (activo) {
            va_derecha = (omega < 0.0);
            velocidad_cinta = std::min(std::abs(omega) / OMEGA_REF, 1.0) * VEL_MAX;
            fase_anim += dt * velocidad_cinta * (va_derecha ? 1.0 : -1.0);
        } else {
            velocidad_cinta = 0.0;
        }

        // Anclar traslación: la caminadora nunca se desplaza.
        velocidad = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        fuerza_neta = Vector2D(0.0, 0.0);
        torque_neto = 0.0;
    }

    // Aplica fuerza a entidades dinámicas que están encima
    void aplicar_conveyor(const std::vector<EntidadFisica*>& entidades) const {
        if (!activo) return;
        double fuerza = velocidad_cinta * 8.0; // fuerza lateral proporcional
        double dir = va_derecha ? 1.0 : -1.0;

        double x0 = posicion.x - 5.0;
        double x1 = posicion.x + ancho + 5.0;
        double y_sup = posicion.y;
        double margen_vertical = alto + 30.0; // zona encima de la cinta

        for (auto* e : entidades) {
            if (!e || e->get_es_estatico()) continue;
            Vector2D pos = e->get_posicion();
            // Verificar si está encima de la cinta
            if (pos.x < x0 || pos.x > x1) continue;
            if (pos.y < y_sup - margen_vertical || pos.y > y_sup + alto * 0.5) continue;
            // Solo empujar si la velocidad horizontal ya no supera la cinta
            double vx = e->get_velocidad().x;
            if ((va_derecha && vx < velocidad_cinta) ||
                (!va_derecha && vx > -velocidad_cinta)) {
                e->aplicar_fuerza(Vector2D(fuerza * dir, 0.0));
            }
        }
    }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - 8 && p.x <= posicion.x + ancho + 8 &&
               p.y >= posicion.y - 8 && p.y <= posicion.y + alto + 8;
    }

    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::CAMINADORA;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent CONVEYOR id=" << get_id() << serializar_base()
           << " w=" << ancho << " h=" << alto
           << " der=" << (va_derecha ? 1 : 0)
           << " act=" << (activo ? 1 : 0)
           << " vel=" << velocidad_cinta
           << " ancl=" << (anclada ? 1 : 0);
        return ss.str();
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float w  = static_cast<float>(ancho);
        float h  = static_cast<float>(alto);

        if (tex_caminadora.id > 0) {
            // Sprite: caminadora1.png tiene rueda grande a la izquierda
            // Si va a la derecha: flipeamos horizontalmente (rueda motriz queda a la derecha)
            Rectangle src = {
                0.0f, 0.0f,
                va_derecha ? -(float)tex_caminadora.width : (float)tex_caminadora.width,
                (float)tex_caminadora.height
            };
            Rectangle dst = { px, py, w, h };
            DrawTexturePro(tex_caminadora, src, dst, {0.0f, 0.0f}, 0.0f, WHITE);

            // Animación de cinta: franja semitransparente que se mueve
            if (activo) {
                float paso = w / 6.0f;
                float offset = std::fmod(static_cast<float>(fase_anim) * 0.1f, paso);
                if (!va_derecha) offset = paso - offset;
                float cinta_y = py + h * 0.3f;
                float cinta_h = h * 0.4f;
                BeginScissorMode((int)px, (int)cinta_y, (int)w, (int)cinta_h);
                for (float mx2 = px - paso + offset; mx2 < px + w; mx2 += paso) {
                    DrawRectangleRec({mx2, cinta_y, paso * 0.5f, cinta_h},
                                     Color{255, 255, 255, 25});
                }
                EndScissorMode();

                // Flecha de dirección encima
                float ax = px + w * 0.5f;
                float ay = py + h * 0.5f;
                float aw = w * 0.12f;
                float d  = va_derecha ? 1.0f : -1.0f;
                DrawLineEx({ax - aw * d, ay}, {ax + aw * d, ay}, 2.5f, Color{255, 220, 80, 200});
                DrawLineEx({ax + aw * d, ay}, {ax + aw * d - 6 * d, ay - 5}, 2.0f, Color{255, 220, 80, 200});
                DrawLineEx({ax + aw * d, ay}, {ax + aw * d - 6 * d, ay + 5}, 2.0f, Color{255, 220, 80, 200});
            }
        } else {
            // Fallback si no cargó el sprite
            Color col_base  = activo ? Color{60, 80, 100, 255} : Color{50, 55, 65, 255};
            Color col_borde = Color{30, 35, 45, 255};
            float rueda_r = h * 0.48f;
            DrawRectangleRec({px + rueda_r, py, w - rueda_r * 2, h}, col_base);
            DrawCircle((int)(px + rueda_r), (int)(py + h * 0.5f), rueda_r, col_borde);
            DrawCircle((int)(px + w - rueda_r), (int)(py + h * 0.5f), rueda_r, col_borde);
        }

        if (debug) {
            DrawRectangleLines(static_cast<int>(px), static_cast<int>(py),
                               static_cast<int>(w), static_cast<int>(h), GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Caminadora" tab=0 categoria=1
