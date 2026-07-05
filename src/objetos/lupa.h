#pragma once
// ============================================================================
// Lupa — lente VERTICAL que concentra la luz de un Foco en un RAYO HORIZONTAL.
//
// La lupa NO se rota. La dirección del haz depende de dónde esté el foco que la
// activa: si la lupa está a la IZQUIERDA del foco, el haz sale a la izquierda; si
// está a la DERECHA, sale a la derecha (la luz se aleja del foco a través del lente).
// El motor calcula esa dirección al activarla (set_dir_haz) según el foco cercano.
//
// El haz enciende la mecha de un Cañón que cruce; sin efecto sobre otros objetos.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "../sistema/assets_extern.h"
#include <cmath>
#include <sstream>

class Lupa : public EntidadFisica {
private:
    double rango;        // alcance del haz en píxeles
    double radio_lente;  // tamaño visual del lente
    bool activa;
    bool ya_activada;
    double dir_x;        // dirección horizontal del haz: -1 = izquierda, +1 = derecha

public:
    Lupa(int id, Vector2D pos, double /*ang_grados ignorado*/ = 0.0, double alcance = 200.0)
        : EntidadFisica(id, pos, 0.0, TipoForma::AABB, true),
          rango(alcance), radio_lente(18.0),
          activa(false), ya_activada(false), dir_x(-1.0) {
        tipo_menu = TipoObjetoMenu::LUPA;
    }

    double get_rango() const { return rango; }
    double get_radio_lente() const { return radio_lente; }
    bool get_activa() const { return activa; }
    bool get_ya_activada() const { return ya_activada; }
    double get_dir_x() const { return dir_x; }

    void set_rango(double r) { rango = std::max(40.0, r); }

    // La dirección del haz la fija el motor según la posición del foco.
    void set_dir_haz(double dx) { dir_x = (dx < 0.0) ? -1.0 : 1.0; }

    Vector2D get_dir() const { return Vector2D(dir_x, 0.0); }
    Vector2D get_punto_final() const { return posicion + get_dir() * rango; }

    bool es_activable_por_tension() const override { return !ya_activada; }
    void activar_por_tension() override { activar(); }

    void activar() { if (!ya_activada) { activa = true; ya_activada = true; } }
    void desactivar() { activa = false; }
    void resetear() { activa = false; ya_activada = false; }

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::LUPA; }

    Vector2D get_min() const override { return posicion - Vector2D(radio_lente * 0.6, radio_lente); }
    Vector2D get_max() const override { return posicion + Vector2D(radio_lente * 0.6, radio_lente); }

    bool contiene_punto(const Vector2D& p) const override {
        return std::abs(p.x - posicion.x) <= radio_lente * 0.6 + 8.0 &&
               std::abs(p.y - posicion.y) <= radio_lente + 8.0;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent LUPA id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " rango=" << rango;
        return ss.str();
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float r  = static_cast<float>(radio_lente);

        // --- Haz horizontal (solo cuando está activa) ---
        if (activa) {
            Vector2D fin = get_punto_final();
            DrawLineEx({px, py}, {(float)fin.x, (float)fin.y}, 6.0f, Color{255, 230, 120, 90});
            DrawLineEx({px, py}, {(float)fin.x, (float)fin.y}, 2.0f, Color{255, 245, 180, 180});
            DrawCircle((int)fin.x, (int)fin.y, 5.0f, Color{255, 240, 150, 160});
        }

        // --- Lente VERTICAL (elipse alta y delgada) + marco + mango abajo ---
        Color marco  = Color{110, 90, 60, 255};
        Color cristal = activa ? Color{200, 230, 255, 150} : Color{190, 210, 225, 100};
        float rx = r * 0.55f;  // semieje horizontal (delgado)
        float ry = r;          // semieje vertical (alto)
        // Mango hacia abajo
        DrawLineEx({px, py + ry}, {px, py + ry + 14.0f}, 4.0f, marco);
        // Cristal + aro (elipse vertical)
        DrawEllipse((int)px, (int)py, rx, ry, cristal);
        DrawEllipseLines((int)px, (int)py, rx, ry, marco);
        DrawEllipseLines((int)px, (int)py, rx - 2.0f, ry - 2.0f, Color{150, 130, 90, 200});

        if (debug) {
            Vector2D fin = get_punto_final();
            DrawLine((int)px, (int)py, (int)fin.x, (int)fin.y, GREEN);
            Vector2D mn = get_min(), mx = get_max();
            DrawRectangleLines((int)mn.x, (int)mn.y, (int)(mx.x-mn.x), (int)(mx.y-mn.y), GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Lupa" tab=0 categoria=0
