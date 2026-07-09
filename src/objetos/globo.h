#pragma once
#include "../core/entidad_fisica.h"
#include "balancin.h"
#include "../sistema/assets_extern.h"
#include <cmath>

class Globo : public EntidadFisica {
private:
    double radio;
    bool reventado;
    static constexpr double FUERZA_FLOTACION = 4500.0;

public:
    Globo(int id, Vector2D pos, double r = 36.0, double m = 2.0)
        : EntidadFisica(id, pos, m, TipoForma::CIRCULO, false),
          radio(r), reventado(false) {
        set_restitucion(0.02);
        set_friccion(0.1);
        set_amortiguamiento(0.004);
        tipo_menu = TipoObjetoMenu::GLOBO;
    }

    double get_radio() const { return radio; }
    bool esta_reventado() const { return reventado; }
    void set_reventado(bool r) { reventado = r; }
    Vector2D get_punto_cuerda() const { return posicion; }

    Vector2D calcular_aceleracion_en(const Vector2D& pos, const Vector2D& vel) const override {
        if (masa < MathUtils::EPSILON) return Vector2D(0, 0);
        Vector2D fuerza_total = fuerza_neta + Vector2D(0.0, -FUERZA_FLOTACION);
        return fuerza_total / masa;
    }

    void on_collision(EntidadFisica* otro, const InfoColision& info) override {
        if (info.normal.y > 0.3) {
            Vector2D vel = get_velocidad();
            set_velocidad(Vector2D(vel.x * 0.3, 0.0));
        }
    }

    Vector2D get_min() const override {
        return Vector2D(posicion.x - radio, posicion.y - radio);
    }
    Vector2D get_max() const override {
        return Vector2D(posicion.x + radio, posicion.y + radio);
    }

    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::GLOBO;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent GLOBO id=" << get_id() << serializar_base() << " r=" << radio;
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        return (p - posicion).magnitud() <= radio + 8.0;
    }

    void dibujar(bool debug) const override {
        if (reventado) return;
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float r  = static_cast<float>(radio);
        DrawLineEx({px, py + r * 0.9f}, {px + 3.0f, py + r * 1.6f}, 1.8f, Color{80, 55, 30, 200});
        if (tex_globo.id > 0) {
            Rectangle src = {0.0f, 0.0f, (float)tex_globo.width, (float)tex_globo.height};
            Rectangle dst = {px, py, r * 2.0f, r * 2.0f};
            Vector2 origin = {r, r};
            DrawTexturePro(tex_globo, src, dst, origin, 0.0f, WHITE);
        } else {
            DrawCircle(static_cast<int>(px), static_cast<int>(py), r, Color{220, 48, 48, 220});
            DrawCircleLines(static_cast<int>(px), static_cast<int>(py), r, Color{120, 10, 10, 220});
            DrawCircle(static_cast<int>(px - r * 0.30f), static_cast<int>(py - r * 0.30f), r * 0.22f, Color{255, 200, 200, 115});
        }
        if (debug) DrawCircleLines(static_cast<int>(px), static_cast<int>(py), r, GREEN);
    }
};

// TIM_MENU_SPAWN etiqueta="Globo" tab=0 categoria=0
