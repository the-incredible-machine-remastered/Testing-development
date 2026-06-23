#pragma once
// ============================================================================
// Bola — Objeto dinámico circular
// La pieza más fundamental de TIM. Rueda, rebota, y activa mecanismos.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../sistema/assets_extern.h"
#include <cmath>
#include <algorithm>

class Bola : public EntidadFisica {
private:
    double radio;
    int color_idx;  // Índice en paleta de colores (para el renderizador)
    int texture_idx; 

public:
    Bola(int id, Vector2D pos_inicial, double r, double m)
        : EntidadFisica(id, pos_inicial, m, TipoForma::CIRCULO),
          radio(r), color_idx(0), texture_idx(0) {

        // Momento de inercia de un disco sólido: I = ½ m r²
        set_inercia(0.5 * m * r * r);
        set_restitucion(0.7);   // Rebote alto
        set_friccion(0.5);      // Fricción media-alta (favorece rolling)
        set_amortiguamiento(0.002);  // Mínimo damping
    }

    // --- Getters ---
    double get_radio() const { return radio; }
    int get_color_idx() const { return color_idx; }
    int get_texture_idx() const { return texture_idx; }
    // --- Setters ---
    void set_color_idx(int idx) { color_idx = idx; }
    void set_texture_idx(int idx) { texture_idx = idx; }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::BOLA;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent BOLA id=" << get_id() << serializar_base()
           << " r=" << radio << " m=" << get_masa()
           << " ci=" << color_idx << " ti=" << texture_idx;
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        return (posicion - p).magnitud() < radio + 15.0;
    }

    void dibujar(bool debug) const override {
        float r = static_cast<float>(radio);
        double ang = angulo;

        // Dibujar sprite de la bola con rotación correcta alrededor del centro
        int tex_idx = texture_idx;
        if (tex_idx >= 0 && tex_idx < 3 && tex_bola[tex_idx].id > 0) {
            DrawTexturePro(
                tex_bola[tex_idx],
                {0, 0, (float)tex_bola[tex_idx].width, (float)tex_bola[tex_idx].height},
                {static_cast<float>(posicion.x), static_cast<float>(posicion.y), 2.0f * r, 2.0f * r},
                {r, r},
                static_cast<float>(ang * 180.0 / MathUtils::TIM_PI),
                WHITE
            );
        } else {
            // Fallback a círculo si la textura no se cargó
            Color col = PALETA_BOLAS[color_idx];
            DrawCircle(static_cast<int>(posicion.x), static_cast<int>(posicion.y), r, col);
            DrawCircleLines(static_cast<int>(posicion.x), static_cast<int>(posicion.y), r,
                            ColorBrightness(col, -0.3f));
        }

        // Indicador de rotación (dos puntos en lados opuestos)
        float dot_dist = r * 0.55f;
        float dot_r = std::max(r * 0.2f, 2.0f);
        Color dot_col = {100, 100, 100, 150};

        int dx1 = static_cast<int>(posicion.x + std::cos(ang) * dot_dist);
        int dy1 = static_cast<int>(posicion.y + std::sin(ang) * dot_dist);
        DrawCircle(dx1, dy1, dot_r, dot_col);

        int dx2 = static_cast<int>(posicion.x - std::cos(ang) * dot_dist);
        int dy2 = static_cast<int>(posicion.y - std::sin(ang) * dot_dist);
        DrawCircle(dx2, dy2, dot_r, dot_col);

        // Debug extra: línea de velocidad angular
        if (debug) {
            Vector2D dir(std::cos(ang) * r, std::sin(ang) * r);
            DrawLine(static_cast<int>(posicion.x), static_cast<int>(posicion.y),
                     static_cast<int>(posicion.x + dir.x),
                     static_cast<int>(posicion.y + dir.y), WHITE);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Bola" tab=0 categoria=0
