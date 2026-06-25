#pragma once
#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "../sistema/assets_extern.h"
#include <string>
#include <algorithm>

// TIM_MENU_SPAWN etiqueta="Zona Meta" tab=0 categoria=1

class ZonaMeta : public EntidadFisica {
public:
    double ancho;
    double alto;
    Color color_editor = {0, 255, 100, 80};

    ZonaMeta(int id, Vector2D pos, double w = 80, double h = 80)
        : EntidadFisica(id, pos, 0.0, TipoForma::NINGUNA, true), ancho(w), alto(h)
    {
        tipo_forma = TipoForma::NINGUNA; // Sin colision fisica
        es_estatico = true;
        tipo_menu = TipoObjetoMenu::ZONA_META;
    }

    bool contiene_punto(const Vector2D& p) const override {
        double min_x = posicion.x - ancho / 2.0 - 15;
        double max_x = posicion.x + ancho / 2.0 + 15;
        double min_y = posicion.y - alto / 2.0 - 15;
        double max_y = posicion.y + alto / 2.0 + 15;
        return p.x >= min_x && p.x <= max_x &&
               p.y >= min_y && p.y <= max_y;
    }

    bool intersecta_circulo(Vector2D centro, double radio) const {
        double min_x = posicion.x - ancho / 2.0;
        double max_x = posicion.x + ancho / 2.0;
        double min_y = posicion.y - alto / 2.0;
        double max_y = posicion.y + alto / 2.0;
        double cx = MathUtils::clamp(centro.x, min_x, max_x);
        double cy = MathUtils::clamp(centro.y, min_y, max_y);
        double dx = centro.x - cx;
        double dy = centro.y - cy;
        return (dx * dx + dy * dy) <= (radio * radio);
    }

    bool intersecta_aabb(Vector2D pos_otro, double w, double h) const {
        double min_x = posicion.x - ancho / 2.0;
        double max_x = posicion.x + ancho / 2.0;
        double min_y = posicion.y - alto / 2.0;
        double max_y = posicion.y + alto / 2.0;
        double omin_x = pos_otro.x - w / 2.0;
        double omax_x = pos_otro.x + w / 2.0;
        double omin_y = pos_otro.y - h / 2.0;
        double omax_y = pos_otro.y + h / 2.0;
        return min_x < omax_x && max_x > omin_x &&
               min_y < omax_y && max_y > omin_y;
    }

    void actualizar_fisica(double) override {} // No-op

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::ZONA_META;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent ZONA_META id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " w=" << ancho << " h=" << alto
           << " fijo=" << (es_fijo ? 1 : 0)
           << " tipo_menu=" << static_cast<int>(tipo_menu);
        return ss.str();
    }

    void dibujar(bool debug) const override {
        Vector2D min_p = posicion - Vector2D(ancho / 2.0, alto / 2.0);
        DrawRectangle(min_p.x, min_p.y, ancho, alto, color_editor);
        DrawRectangleLines(min_p.x, min_p.y, ancho, alto, GREEN);
    }
};
