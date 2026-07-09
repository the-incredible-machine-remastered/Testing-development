#pragma once
#include "../core/entidad_fisica.h"
#include "../sistema/assets_extern.h"
#include <cmath>
#include <sstream>

// ============================================================================
// CajaSorpresa — Jack-in-the-box con EJE FICTICIO.
// Se construye dinámica pero ancla su traslación cada frame (patrón de Polea).
// Una Correa conectada a una RuedaHamster transmite torque real a este eje; al
// superar |velocidad_angular| un umbral, la caja se dispara: la tapa se abre y
// lanza una cabeza de payaso como proyectil físico. Dispara una sola vez.
// ============================================================================

class CajaSorpresa : public EntidadFisica {
private:
    double ancho;
    double alto;

    bool activada;          // recibió señal de activación
    bool ya_lanzo;          // solo lanza una vez
    bool balancin_golpeado = false; // la cabeza ya impulso un balancin (una vez)
    bool cabeza_frenada = false;    // la cabeza choco con algo y se detuvo
    double cabeza_tope_y = 0.0;     // Y donde la cabeza quedo frenada
    float tapa_angulo;      // animación de apertura de tapa (0 → -90°)
    float tiempo_abierta;   // temporizador para animación post-lanzamiento

    // Umbral de disparo. El eje acoplado por la Correa asintota cerca de la
    // velocidad de crucero de RuedaHamster (~7.5 rad/s); se dispara a 5.0 para
    // hacerlo de forma fiable durante el arranque, sin falsos positivos.
    static constexpr double OMEGA_DISPARO = 5.0;

public:
    CajaSorpresa(int id, Vector2D pos, double w = 70.0, double h = 70.0)
        : EntidadFisica(id, pos, 1.0, TipoForma::AABB, false),
          ancho(w), alto(h),
          activada(false), ya_lanzo(false),
          tapa_angulo(0.0f), tiempo_abierta(0.0f) {
        set_inercia(0.5 * masa * 10.0 * 10.0); // eje ficticio radio 10 (= RuedaHamster)
        set_restitucion(0.2);
        set_friccion(0.6);
        tipo_menu = TipoObjetoMenu::CAJA_SORPRESA;
        velocidad_angular = 0.0;
        angulo = 0.0;
    }

    // Radio de eje = el de RuedaHamster para que la Correa iguale su omega.
    double get_radio_eje() const override { return 10.0; }

    // La caja esta anclada (no se traslada): en colisiones actua como cuerpo
    // inmovil, para que las bolas reboten/rueden sobre ella sin catapultarse.
    bool es_inmovil_en_colision() const override { return true; }

    // El eje ficticio gira por la Correa (velocidad_angular alta), pero ese giro
    // NO debe transmitirse a las colisiones: para contacto la caja esta quieta.
    // (Sin esto, la velocidad rotacional se inyecta en la bola y explota numericamente.)
    Vector2D get_velocidad_en_punto(const Vector2D&) const override {
        return Vector2D(0.0, 0.0);
    }

    double get_ancho() const { return ancho; }
    double get_alto()  const { return alto; }
    bool get_activada()  const { return activada; }
    bool get_ya_lanzo()  const { return ya_lanzo; }
    bool get_balancin_golpeado() const { return balancin_golpeado; }
    void set_balancin_golpeado()       { balancin_golpeado = true; }

    // La cabeza choco con algo (p.ej. un balancin) y se detiene a esa altura:
    // deja de subir; luego seguira su fase de bajada normal.
    void frenar_cabeza_en(double y) { cabeza_frenada = true; cabeza_tope_y = y; }
    void set_ya_lanzo()        { ya_lanzo = true; }

    Vector2D get_min() const override { return posicion; }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho, posicion.y + alto); }

    // Punto de donde sale la cabeza (centro-superior de la caja)
    Vector2D get_punto_lanzamiento() const {
        return Vector2D(posicion.x + ancho * 0.5, posicion.y);
    }

    // Altura maxima que asoma la cabeza sobre la caja (con leve desvio a la derecha).
    double get_altura_asomada() const { return alto * 1.15; }
    double get_radio_cabeza() const { return ancho * 0.28; }

    // Centro de la cabeza de payaso mientras esta asomada (sujeta al resorte).
    // Movimiento jack-in-the-box: brinca de golpe hasta el pico y baja rebotando
    // UNA vez hasta quedar asomando poco sobre la caja abierta.
    Vector2D get_centro_cabeza() const {
        double frac_altura;
        if (!ya_lanzo) {
            frac_altura = 0.0;
        } else {
            float t = tiempo_abierta;
            const float t_pico = 0.15f; // sube al pico
            const float t_fin  = 0.55f; // termina de asentarse
            const float reposo = 0.45f; // altura final asomada (fraccion del pico)
            if (t <= t_pico) {
                frac_altura = t / t_pico;                 // 0 -> 1 (sube)
            } else if (t <= t_fin) {
                float u = (t - t_pico) / (t_fin - t_pico); // 0 -> 1 (baja rebotando)
                // De 1.0 baja hasta 'reposo' con un solo rebote suave (coseno).
                frac_altura = reposo + (1.0f - reposo) * (0.5f * (1.0f + std::cos(u * MathUtils::TIM_PI)));
            } else {
                frac_altura = reposo;                      // asentado
            }
        }
        double subida = get_altura_asomada() * frac_altura;
        double desvio = subida * std::tan(10.0 * MathUtils::TIM_PI / 180.0); // ~35 grados derecha
        double cy = posicion.y - subida;
        // Si la cabeza choco con algo, no sube mas alla del punto de choque.
        if (cabeza_frenada && cy < cabeza_tope_y) cy = cabeza_tope_y;
        return Vector2D(posicion.x + ancho * 0.5 + desvio, cy);
    }

    // Velocidad inicial de la cabeza de payaso: sale casi vertical (bien alto) con
    // una leve inclinacion de ~5 grados hacia la derecha.
    Vector2D get_velocidad_lanzamiento() const {
        const double V = 640.0;                 // magnitud (mas alto que antes)
        const double ang = 5.0 * MathUtils::TIM_PI / 180.0; // 5 grados a la derecha
        return Vector2D(V * std::sin(ang), -V * std::cos(ang));
    }

    void activar() {
        if (!ya_lanzo) activada = true;
    }

    void actualizar_fisica(double dt) override {
        // Patrón de Polea: el eje ficticio gira con el torque acumulado por la
        // Correa; la traslación queda anclada (la caja no se traslada).
        double I = get_inercia();
        double alpha = (I > MathUtils::EPSILON) ? (torque_neto / I) : 0.0;
        velocidad_angular += alpha * dt;
        angulo += velocidad_angular * dt;
        velocidad_angular *= 0.985; // damping natural

        // Disparo por umbral de velocidad angular (una sola vez).
        if (!ya_lanzo && std::abs(velocidad_angular) >= OMEGA_DISPARO) {
            activar(); // respeta ya_lanzo internamente
        }

        // Anclar traslación.
        velocidad = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        fuerza_neta = Vector2D(0.0, 0.0);
        torque_neto = 0.0;

        // Animación existente.
        if (activada && !ya_lanzo) {
            tapa_angulo -= static_cast<float>(dt) * 300.0f;
            if (tapa_angulo < -90.0f) tapa_angulo = -90.0f;
        }
        if (ya_lanzo) {
            tiempo_abierta += static_cast<float>(dt);
        }
    }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - 8 && p.x <= posicion.x + ancho + 8 &&
               p.y >= posicion.y - 8 && p.y <= posicion.y + alto + 8;
    }

    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::CAJA_SORPRESA;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent CAJA_SORPRESA id=" << get_id() << serializar_base()
           << " w=" << ancho << " h=" << alto;
        return ss.str();
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float w  = static_cast<float>(ancho);
        float h  = static_cast<float>(alto);
        float cx = px + w * 0.5f;

        bool abierta = ya_lanzo || (activada && tapa_angulo < -45.0f);

        // --- Cuerpo de la caja ---
        if (tex_caja_base.id > 0) {
            Rectangle src = {0.0f, 0.0f, (float)tex_caja_base.width, (float)tex_caja_base.height};
            Rectangle dst = {px, py + h * 0.15f, w, h * 0.85f};
            DrawTexturePro(tex_caja_base, src, dst, {0,0}, 0.0f, WHITE);
        } else {
            Color col_caja   = Color{210, 40, 40, 255};   // rojo
            Color col_borde  = Color{140, 20, 20, 255};
            Color col_deco   = Color{255, 200, 50, 255};  // amarillo decoración
            DrawRectangleRec({px, py + h * 0.15f, w, h * 0.85f}, col_caja);
            DrawRectangleLinesEx({px, py + h * 0.15f, w, h * 0.85f}, 2.5f, col_borde);
            int lunares[][2] = {{(int)(px+w*0.25f),(int)(py+h*0.45f)},
                                {(int)(px+w*0.75f),(int)(py+h*0.45f)},
                                {(int)(px+w*0.5f), (int)(py+h*0.65f)},
                                {(int)(px+w*0.25f),(int)(py+h*0.75f)},
                                {(int)(px+w*0.75f),(int)(py+h*0.75f)}};
            for (auto& l : lunares)
                DrawCircle(l[0], l[1], w * 0.07f, col_deco);
            DrawRectangleRec({cx - 2.0f, py + h*0.25f, 4.0f, h*0.65f}, col_deco);
            DrawRectangleRec({px + 4.0f, py + h*0.55f - 2.0f, w - 8.0f, 4.0f}, col_deco);
        }

        // --- Tapa ---
        if (!abierta) {
            // Caja CERRADA: el mono (caja_tapa) cubre la boca abierta de la base.
            // Se dibuja alto para tapar la parte superior y verse como regalo cerrado.
            if (tex_caja_tapa.id > 0) {
                float mono_h = h * 0.55f;
                Rectangle src = {0.0f, 0.0f, (float)tex_caja_tapa.width, (float)tex_caja_tapa.height};
                Rectangle dst = {px - 4.0f, py - mono_h * 0.30f, w + 8.0f, mono_h};
                DrawTexturePro(tex_caja_tapa, src, dst, {0,0}, 0.0f, WHITE);
            } else {
                DrawRectangleRec({px - 3.0f, py, w + 6.0f, h * 0.28f}, Color{180, 30, 30, 255});
                DrawRectangleLinesEx({px - 3.0f, py, w + 6.0f, h * 0.28f}, 2.0f, Color{140, 20, 20, 255});
            }
        } else {
            // Tapa abierta
            float bisagra_x = px + w + 3.0f;
            float bisagra_y = py + h * 0.15f;
            float tapa_w    = w + 6.0f;
            float tapa_h    = h * 0.18f;

            if (tex_caja_tapa.id > 0) {
                Rectangle src = {0.0f, 0.0f, (float)tex_caja_tapa.width, (float)tex_caja_tapa.height};
                Rectangle dst = {bisagra_x, bisagra_y, tapa_w, tapa_h};
                Vector2 origin = {tapa_w, tapa_h * 0.5f};
                DrawTexturePro(tex_caja_tapa, src, dst, origin, tapa_angulo, WHITE);
            } else {
                float ang = tapa_angulo;
                float rad = ang * MathUtils::TIM_PI / 180.0f;
                float tip_x = bisagra_x + std::cos(MathUtils::TIM_PI + rad) * tapa_w;
                float tip_y = bisagra_y + std::sin(MathUtils::TIM_PI + rad) * tapa_w;
                DrawLineEx({bisagra_x, bisagra_y}, {tip_x, tip_y}, tapa_h, Color{180,30,30,200});
            }

            // Cabeza de payaso SUJETA a la caja por el resorte (jack-in-the-box):
            // brinca hacia arriba y se queda asomada (no se separa).
            {
                Vector2D headc = get_centro_cabeza();
                float head_cx = static_cast<float>(headc.x);
                float head_cy = static_cast<float>(headc.y);
                float hr = static_cast<float>(get_radio_cabeza());

                // Resorte: desde la boca de la caja hasta la base de la cabeza.
                float base_x = cx;
                float base_y = py + h * 0.10f;
                int pasos = 7;
                for (int i = 0; i < pasos; i++) {
                    float t0 = (float)i / pasos;
                    float t1 = (float)(i + 1) / pasos;
                    float sx0 = base_x + (head_cx - base_x) * t0 + std::sin(t0 * MathUtils::TIM_PI * 4) * w * 0.12f;
                    float sy0 = base_y + (head_cy + hr - base_y) * t0;
                    float sx1 = base_x + (head_cx - base_x) * t1 + std::sin(t1 * MathUtils::TIM_PI * 4) * w * 0.12f;
                    float sy1 = base_y + (head_cy + hr - base_y) * t1;
                    DrawLineEx({sx0, sy0}, {sx1, sy1}, 2.0f, Color{160, 160, 160, 230});
                }

                // Cara del payaso.
                if (tex_payaso.id > 0) {
                    Rectangle src = {0.0f, 0.0f, (float)tex_payaso.width, (float)tex_payaso.height};
                    Rectangle dst = {head_cx, head_cy, hr * 2.0f, hr * 2.0f};
                    Vector2 origin = {hr, hr};
                    DrawTexturePro(tex_payaso, src, dst, origin, 0.0f, WHITE);
                } else {
                    DrawCircle((int)head_cx, (int)head_cy, hr, Color{255, 220, 170, 255});
                    DrawCircleLines((int)head_cx, (int)head_cy, hr, Color{180, 130, 60, 255});
                    DrawCircle((int)(head_cx - hr*0.35f), (int)(head_cy - hr*0.2f), hr*0.15f, BLACK);
                    DrawCircle((int)(head_cx + hr*0.35f), (int)(head_cy - hr*0.2f), hr*0.15f, BLACK);
                    DrawCircle((int)head_cx, (int)(head_cy + hr*0.1f), hr*0.2f, Color{220,40,40,255});
                }
            }
        }

        if (debug) {
            DrawRectangleLines((int)px, (int)py, (int)w, (int)h, GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="CajaSorpresa" tab=0 categoria=1
