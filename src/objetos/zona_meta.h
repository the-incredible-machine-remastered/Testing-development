#pragma once
#include "core/entidad_fisica.h"
#include "core/math_utils.h"
#include <string>

// TIM_MENU_SPAWN etiqueta="Zona Meta" tab=0 categoria=1

class ZonaMeta : public EntidadFisica {
public:
    double ancho;
    double alto;
    // Color/estilo visual del editor (no se renderiza en simulacion)
    Color color_editor = {0, 255, 100, 80};

    ZonaMeta(int id, Vector2D pos, double w = 80, double h = 80)
        : EntidadFisica(id, pos, 0.0, true), ancho(w), alto(h)
    {
        tipo_forma = TipoForma::NINGUNA; // Sin colision fisica
        es_estatico = true;
    }

    bool contiene_punto(Vector2D punto) const {
        double min_x = posicion.x - ancho / 2.0;
        double max_x = posicion.x + ancho / 2.0;
        double min_y = posicion.y - alto / 2.0;
        double max_y = posicion.y + alto / 2.0;
        return punto.x >= min_x && punto.x <= max_x &&
               punto.y >= min_y && punto.y <= max_y;
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
};
