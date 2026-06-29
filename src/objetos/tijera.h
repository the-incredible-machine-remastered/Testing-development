#pragma once
#include "obstaculo_estatico.h"

class Tijera : public ObstaculoEstatico {
private:
    double ancho;
    double alto;
    bool fue_activada;
    bool permanentemente_activada;
    bool ya_corto_cuerdas;

public:
    Tijera(int id, Vector2D pos, double w = 70.0, double h = 30.0)
        : ObstaculoEstatico(id, pos, TipoForma::AABB),
          ancho(w), alto(h), fue_activada(false), permanentemente_activada(false), ya_corto_cuerdas(false) {
        set_restitucion(0.15);
        set_friccion(0.6);
    }

    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    bool get_fue_activada() const { return fue_activada; }
    void resetear_activacion() { fue_activada = false; }

    Vector2D get_min() const override { return posicion; }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho, posicion.y + alto); }

    bool get_permanentemente_activada() const { return permanentemente_activada; }
    bool get_ya_corto_cuerdas() const { return ya_corto_cuerdas; }
    void set_ya_corto_cuerdas() { ya_corto_cuerdas = true; }

    void on_collision(EntidadFisica* otro, const InfoColision& info) override {
        if (!otro || otro->get_es_estatico() || permanentemente_activada) return;
        fue_activada = true;
        permanentemente_activada = true;
    }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - 8 && p.x <= posicion.x + ancho + 8 && p.y >= posicion.y - 8 && p.y <= posicion.y + alto + 8;
    }

    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::TIJERA;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent TIJERA id=" << get_id() << serializar_base() << " w=" << ancho << " h=" << alto;
        return ss.str();
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float w  = static_cast<float>(ancho);
        float h  = static_cast<float>(alto);
        float cx = px + w * 0.5f;
        float cy = py + h * 0.5f;
        Color col  = fue_activada ? Color{255, 80, 80, 255} : Color{175, 182, 195, 255};
        Color col2 = fue_activada ? Color{200, 30, 30, 255} : Color{130, 138, 150, 255};
        if (fue_activada) DrawRectangleRec({px - 5, py - 5, w + 10, h + 10}, Color{255, 100, 100, 80});
        float hx = w * 0.44f;
        float hy = h * 0.40f;
        DrawLineEx({cx - hx, cy - hy}, {cx + hx, cy + hy}, 2.8f, col);
        DrawLineEx({cx - hx, cy + hy}, {cx + hx, cy - hy}, 2.8f, col);
        float r_aro = h * 0.27f;
        Color col_aro = fue_activada ? Color{255, 80, 80, 255} : Color{200, 40, 40, 255};
        DrawCircle(static_cast<int>(cx - hx), static_cast<int>(cy - hy), r_aro, col_aro);
        DrawCircleLines(static_cast<int>(cx - hx), static_cast<int>(cy - hy), r_aro, col2);
        DrawCircle(static_cast<int>(cx - hx), static_cast<int>(cy + hy), r_aro, col_aro);
        DrawCircleLines(static_cast<int>(cx - hx), static_cast<int>(cy + hy), r_aro, col2);
        DrawCircle(static_cast<int>(cx), static_cast<int>(cy), 3.5f, col2);
        DrawCircleLines(static_cast<int>(cx), static_cast<int>(cy), 3.5f, col);
        if (fue_activada) DrawText("CORTANDO", static_cast<int>(px + 5), static_cast<int>(py - 15), 12, Color{255, 100, 100, 255});
        if (debug) DrawRectangleLines(static_cast<int>(px), static_cast<int>(py), static_cast<int>(w), static_cast<int>(h), GREEN);
    }
};

// TIM_MENU_SPAWN etiqueta="Tijera" tab=0 categoria=0
