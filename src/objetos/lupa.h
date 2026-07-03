#pragma once
// ============================================================================
// Lupa — lente que concentra luz en un RAYO DIRECCIONAL (como en el TIM original,
// donde la lupa redirige/enfoca un haz en una línea, no en un círculo).
//
// Estado ACTUAL (deliberadamente incompleto):
//   - Es ACTIVABLE con el mismo mecanismo que Foco/Pistola (necesita activarse).
//   - Tiene DIRECCIÓN (angulo) y RANGO ajustable (alcance del haz).
//   - El haz NO tiene efecto físico todavía: solo se dibuja el alcance. El efecto
//     real (reventar globo / alimentar cañón / encender) se conecta después.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "../sistema/assets_extern.h"
#include <cmath>
#include <sstream>

class Lupa : public EntidadFisica {
private:
    double angulo;      // dirección del haz (radianes; 0 = derecha)
    double rango;       // alcance del haz en píxeles (ajustable)
    double radio_lente; // tamaño visual del lente
    bool activa;
    bool ya_activada;

public:
    Lupa(int id, Vector2D pos, double ang_grados = 0.0, double alcance = 200.0)
        : EntidadFisica(id, pos, 0.0, TipoForma::AABB, true),
          angulo(ang_grados * MathUtils::TIM_PI / 180.0),
          rango(alcance), radio_lente(16.0),
          activa(false), ya_activada(false) {
        tipo_menu = TipoObjetoMenu::LUPA;
    }

    double get_angulo() const { return angulo; }
    double get_angulo_grados() const { return angulo * 180.0 / MathUtils::TIM_PI; }
    double get_rango() const { return rango; }
    double get_radio_lente() const { return radio_lente; }
    bool get_activa() const { return activa; }
    bool get_ya_activada() const { return ya_activada; }

    void set_rango(double r) { rango = std::max(40.0, r); }
    void ajustar_rango(double delta) { set_rango(rango + delta); }
    void invertir() {
        angulo += MathUtils::TIM_PI;
        if (angulo > MathUtils::TIM_PI) angulo -= 2.0 * MathUtils::TIM_PI;
    }

    Vector2D get_dir() const { return Vector2D(std::cos(angulo), std::sin(angulo)); }
    Vector2D get_punto_final() const { return posicion + get_dir() * rango; }

    bool es_activable_por_tension() const override { return !ya_activada; }
    void activar_por_tension() override { activar(); }

    void activar() { if (!ya_activada) { activa = true; ya_activada = true; } }
    void desactivar() { activa = false; }
    void resetear() { activa = false; ya_activada = false; }

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::LUPA; }

    Vector2D get_min() const override { return posicion - Vector2D(radio_lente, radio_lente); }
    Vector2D get_max() const override { return posicion + Vector2D(radio_lente, radio_lente); }

    bool contiene_punto(const Vector2D& p) const override {
        return (p - posicion).magnitud() <= radio_lente + 8.0;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent LUPA id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " ang=" << get_angulo_grados()
           << " rango=" << rango;
        return ss.str();
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float r  = static_cast<float>(radio_lente);

        if (activa) {
            Vector2D fin = get_punto_final();
            DrawLineEx({px, py}, {(float)fin.x, (float)fin.y}, 6.0f, Color{255, 230, 120, 90});
            DrawLineEx({px, py}, {(float)fin.x, (float)fin.y}, 2.0f, Color{255, 245, 180, 180});
            DrawCircle((int)fin.x, (int)fin.y, 5.0f, Color{255, 240, 150, 160});
        }

        Color marco  = Color{110, 90, 60, 255};
        Color cristal = activa ? Color{200, 230, 255, 140} : Color{190, 210, 225, 90};
        Vector2D atras = posicion - get_dir() * (r + 14.0);
        DrawLineEx({px, py}, {(float)atras.x, (float)atras.y}, 4.0f, marco);
        DrawCircle((int)px, (int)py, r, cristal);
        DrawCircleLines((int)px, (int)py, r, marco);
        DrawCircleLines((int)px, (int)py, r - 2.0f, Color{150, 130, 90, 200});

        if (debug) {
            Vector2D fin = get_punto_final();
            DrawLine((int)px, (int)py, (int)fin.x, (int)fin.y, GREEN);
            DrawRectangleLines((int)(px - r), (int)(py - r), (int)(r * 2), (int)(r * 2), GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Lupa" tab=0 categoria=0
