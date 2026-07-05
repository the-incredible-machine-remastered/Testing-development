#pragma once
#include "obstaculo_estatico.h"
#include "../sistema/assets_extern.h"
#include <cmath>
#include <sstream>

// ============================================================================
// CajaSorpresa — Jack-in-the-box activado por banda/hámster.
// Al recibir energía, la tapa se abre y lanza una cabeza de payaso
// como proyectil físico hacia arriba-derecha (parábola).
// ============================================================================

class CajaSorpresa : public ObstaculoEstatico {
private:
    double ancho;
    double alto;

    bool activada;          // recibió señal de activación
    bool ya_lanzo;          // solo lanza una vez
    float tapa_angulo;      // animación de apertura de tapa (0 → -90°)
    float tiempo_abierta;   // temporizador para animación post-lanzamiento

public:
    CajaSorpresa(int id, Vector2D pos, double w = 70.0, double h = 70.0)
        : ObstaculoEstatico(id, pos, TipoForma::AABB),
          ancho(w), alto(h),
          activada(false), ya_lanzo(false),
          tapa_angulo(0.0f), tiempo_abierta(0.0f) {
        set_restitucion(0.2);
        set_friccion(0.6);
        tipo_menu = TipoObjetoMenu::CAJA_SORPRESA;
    }

    double get_ancho() const { return ancho; }
    double get_alto()  const { return alto; }
    bool get_activada()  const { return activada; }
    bool get_ya_lanzo()  const { return ya_lanzo; }
    void set_ya_lanzo()        { ya_lanzo = true; }

    Vector2D get_min() const override { return posicion; }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho, posicion.y + alto); }

    // Punto de donde sale la cabeza (centro-superior de la caja)
    Vector2D get_punto_lanzamiento() const {
        return Vector2D(posicion.x + ancho * 0.5, posicion.y);
    }

    // Velocidad inicial de la cabeza: parábola arriba-derecha
    Vector2D get_velocidad_lanzamiento() const {
        return Vector2D(280.0, -520.0); // vx derecha, vy arriba
    }

    void activar() {
        if (!ya_lanzo) activada = true;
    }

    void actualizar_fisica(double dt) override {
        if (activada && !ya_lanzo) {
            // La tapa se abre rápido
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
            if (tex_caja_tapa.id > 0) {
                Rectangle src = {0.0f, 0.0f, (float)tex_caja_tapa.width, (float)tex_caja_tapa.height};
                Rectangle dst = {px - 3.0f, py, w + 6.0f, h * 0.18f};
                DrawTexturePro(tex_caja_tapa, src, dst, {0,0}, 0.0f, WHITE);
            } else {
                DrawRectangleRec({px - 3.0f, py, w + 6.0f, h * 0.18f}, Color{180, 30, 30, 255});
                DrawRectangleLinesEx({px - 3.0f, py, w + 6.0f, h * 0.18f}, 2.0f, Color{140, 20, 20, 255});
            }
            // Resorte (espiral) visible a través de la ranura
            for (int i = 0; i < 4; i++) {
                float sy = py + h * 0.20f + i * h * 0.08f;
                DrawLineEx({cx - w*0.08f, sy}, {cx + w*0.08f, sy + h*0.04f}, 1.5f, Color{180,180,180,200});
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

            // Cabeza de payaso animada
            if (!ya_lanzo || tiempo_abierta < 0.15f) {
                float spring_ext = std::min(tiempo_abierta / 0.15f, 1.0f);
                float head_y = py - h * 0.25f * spring_ext;
                float hr = w * 0.28f;
                // Resorte
                int pasos = 6;
                for (int i = 0; i < pasos; i++) {
                    float t0 = (float)i / pasos;
                    float t1 = (float)(i+1) / pasos;
                    float sx0 = cx + std::sin(t0 * MathUtils::TIM_PI * 4) * w * 0.1f;
                    float sy0 = py + h*0.18f + (head_y - py - h*0.18f) * t0 + hr;
                    float sx1 = cx + std::sin(t1 * MathUtils::TIM_PI * 4) * w * 0.1f;
                    float sy1 = py + h*0.18f + (head_y - py - h*0.18f) * t1 + hr;
                    DrawLineEx({sx0,sy0},{sx1,sy1}, 2.0f, Color{160,160,160,220});
                }
                // Cara del payaso
                if (tex_payaso.id > 0) {
                    Rectangle src = {0.0f, 0.0f, (float)tex_payaso.width, (float)tex_payaso.height};
                    Rectangle dst = {cx, head_y, hr * 2.0f, hr * 2.0f};
                    Vector2 origin = {hr, hr};
                    DrawTexturePro(tex_payaso, src, dst, origin, 0.0f, WHITE);
                } else {
                    DrawCircle((int)cx, (int)(head_y), hr, Color{255, 220, 170, 255});
                    DrawCircleLines((int)cx, (int)(head_y), hr, Color{180, 130, 60, 255});
                    DrawCircle((int)(cx - hr*0.35f), (int)(head_y - hr*0.2f), hr*0.15f, BLACK);
                    DrawCircle((int)(cx + hr*0.35f), (int)(head_y - hr*0.2f), hr*0.15f, BLACK);
                    DrawCircle((int)cx, (int)(head_y + hr*0.1f), hr*0.2f, Color{220,40,40,255});
                    DrawCircleLines((int)cx, (int)(head_y + hr*0.15f), hr*0.3f, Color{80,40,10,255});
                    DrawRectangleRec({cx - hr*0.4f, head_y - hr*1.35f, hr*0.8f, hr*0.5f}, Color{80,20,120,255});
                    DrawRectangleRec({cx - hr*0.55f, head_y - hr*1.05f, hr*1.1f, hr*0.15f}, Color{80,20,120,255});
                }
            }
        }

        if (debug) {
            DrawRectangleLines((int)px, (int)py, (int)w, (int)h, GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="CajaSorpresa" tab=0 categoria=1
