#pragma once
#include "../core/entidad_fisica.h"

class Gancho : public EntidadFisica {
private:
    double radio;

public:
    Gancho(int id, Vector2D pos, double r = 10.0)
        : EntidadFisica(id, pos, 0.0, TipoForma::NINGUNA, true), radio(r) {
        tipo_menu = TipoObjetoMenu::GANCHO;
    }

    double get_radio() const { return radio; }
    Vector2D get_punto_cuerda() const { return posicion; }

    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::GANCHO;
    }

    bool contiene_punto(const Vector2D& p) const override {
        return (posicion - p).magnitud() < radio + 8.0;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent GANCHO id=" << get_id() << " x=" << posicion.x << " y=" << posicion.y;
        return ss.str();
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float r  = static_cast<float>(radio);
        Color c1 = Color{170, 178, 186, 255}; // gris claro metálico
        Color c2 = Color{100, 108, 116, 255}; // gris oscuro borde
        Color c3 = Color{210, 215, 220, 255}; // brillo

        // Cuerpo cilíndrico (rosca) abajo
        float bw = r * 1.1f;
        float bh = r * 1.2f;
        float bx = px - bw * 0.5f;
        float by = py;
        DrawRectangleRec({bx, by, bw, bh}, c1);
        DrawRectangleLinesEx({bx, by, bw, bh}, 1.5f, c2);
        // líneas de rosca
        for (int i = 1; i <= 3; i++) {
            float ry = by + bh * i / 4.0f;
            DrawLineEx({bx + 1, ry}, {bx + bw - 1, ry}, 1.0f, c2);
        }
        // brillo lateral izquierdo
        DrawLineEx({bx + 2, by + 2}, {bx + 2, by + bh - 2}, 1.5f, c3);

        // Cuello más estrecho entre cuerpo y aro
        float nw = bw * 0.7f;
        float nh = r * 0.35f;
        DrawRectangleRec({px - nw * 0.5f, by - nh, nw, nh}, c1);
        DrawRectangleLinesEx({px - nw * 0.5f, by - nh, nw, nh}, 1.0f, c2);

        // Aro circular arriba
        float ar = r * 0.72f;
        float ay = by - nh - ar;
        DrawRing({px, ay}, ar * 0.55f, ar, 0, 360, 24, c1);
        DrawRing({px, ay}, ar * 0.55f, ar, 0, 360, 24, Color{0,0,0,0}); // reset
        // borde del aro
        DrawCircleLines((int)px, (int)ay, ar, c2);
        DrawCircleLines((int)px, (int)ay, ar * 0.55f, c2);
        // brillo en aro
        DrawCircleLines((int)(px - ar * 0.2f), (int)(ay - ar * 0.2f), ar * 0.2f, c3);

        if (debug) DrawCircleLines((int)px, (int)py, r * 1.5f, GREEN);
    }
};

// TIM_MENU_SPAWN etiqueta="Gancho" tab=0 categoria=0
