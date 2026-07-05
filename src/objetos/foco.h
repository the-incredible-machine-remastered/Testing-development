#pragma once
// ============================================================================
// Foco — bombilla activable. Se "enciende" con el mismo mecanismo que activa a
// la Pistola (tensión de cuerda o golpe brusco del balancín): el motor no conoce
// el tipo concreto, solo usa es_activable_por_tension()/activar_por_tension().
//
// Por ahora el efecto de estar encendido es SOLO visual + estado (sirve como meta/
// indicador y como base para futuros efectos: reventar globos, alimentar la Lupa).
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../sistema/assets_extern.h"
#include <cmath>
#include <sstream>

class Foco : public EntidadFisica {
private:
    double radio;
    bool encendido;
    bool ya_encendido;   // se enciende una sola vez (como la pistola con ya_disparo)

public:
    Foco(int id, Vector2D pos, double r = 18.0)
        : EntidadFisica(id, pos, 0.0, TipoForma::AABB, true),
          radio(r), encendido(false), ya_encendido(false) {
        tipo_menu = TipoObjetoMenu::FOCO;
    }

    double get_radio() const { return radio; }
    bool get_encendido() const { return encendido; }
    bool get_ya_encendido() const { return ya_encendido; }

    // Contrato de activable (idéntico al de la Pistola, pero enciende en vez de disparar).
    bool es_activable_por_tension() const override { return !ya_encendido; }
    void activar_por_tension() override { encender(); }

    void encender() { if (!ya_encendido) { encendido = true; ya_encendido = true; } }
    void apagar() { encendido = false; }
    void resetear() { encendido = false; ya_encendido = false; }

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::FOCO; }

    Vector2D get_min() const override { return posicion - Vector2D(radio, radio); }
    Vector2D get_max() const override { return posicion + Vector2D(radio, radio); }

    bool contiene_punto(const Vector2D& p) const override {
        return (p - posicion).magnitud() <= radio + 8.0;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent FOCO id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " r=" << radio;
        return ss.str();
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float r  = static_cast<float>(radio);

        Texture2D tex = encendido ? tex_foco_prendido : tex_foco_apagado;

        if (tex.id > 0) {
            float s_w = r * 2.2f;
            float s_h = s_w * ((float)tex.height / (float)tex.width);
            Rectangle src = {0.0f, 0.0f, (float)tex.width, (float)tex.height};
            Rectangle dst = {px, py, s_w, s_h};
            Vector2 origin = {s_w / 2.0f, s_h * 0.35f};
            DrawTexturePro(tex, src, dst, origin, 0.0f, WHITE);
        } else {
            Color vidrio = encendido ? Color{255, 235, 130, 255} : Color{200, 205, 210, 255};
            Color borde  = encendido ? Color{210, 170, 40, 255}  : Color{120, 128, 135, 255};

            if (encendido) {
                DrawCircle((int)px, (int)py, r * 2.0f, Color{255, 230, 120, 60});
                DrawCircle((int)px, (int)py, r * 1.4f, Color{255, 235, 150, 90});
            }

            DrawCircle((int)px, (int)py, r, vidrio);
            DrawCircleLines((int)px, (int)py, r, borde);

            Color fil = encendido ? Color{255, 150, 30, 255} : Color{110, 115, 120, 255};
            DrawLineEx({px - r * 0.35f, py + r * 0.1f}, {px, py - r * 0.3f}, 2.0f, fil);
            DrawLineEx({px, py - r * 0.3f}, {px + r * 0.35f, py + r * 0.1f}, 2.0f, fil);

            Color metal = Color{150, 155, 160, 255};
            DrawRectangleRec({px - r * 0.45f, py + r * 0.75f, r * 0.9f, r * 0.55f}, metal);
            DrawRectangleLinesEx({px - r * 0.45f, py + r * 0.75f, r * 0.9f, r * 0.55f}, 1.0f, Color{90, 95, 100, 255});
        }

        if (debug) {
            DrawRectangleLines((int)(px - r), (int)(py - r), (int)(r * 2), (int)(r * 2), GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Foco" tab=0 categoria=0
