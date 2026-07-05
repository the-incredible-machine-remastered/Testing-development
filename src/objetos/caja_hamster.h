#pragma once
#include "obstaculo_estatico.h"
#include <cmath>
#include <vector>

// ============================================================================
// CajaHamster — Rueda giratoria activada por colisión.
// Al recibir un golpe, el hámster empieza a correr y la rueda gira.
// Los ventiladores conectados por Banda se activan/desactivan con ella.
// ============================================================================

class CajaHamster : public ObstaculoEstatico {
private:
    double ancho;
    double alto;
    bool activo;
    double fase_rueda;       // ángulo actual de la rueda (radianes)
    double velocidad_rueda;  // rad/s actual
    double velocidad_max;    // rad/s máxima
    double aceleracion;      // rad/s² al arrancar
    std::vector<int> ids_ventiladores; // ventiladores conectados por banda

    // Hitbox preciso: solo la rueda circular (centro + radio)
    Vector2D get_centro_rueda() const {
        // La rueda ocupa la mitad derecha de la caja
        return Vector2D(posicion.x + ancho * 0.65, posicion.y + alto * 0.5);
    }
    double get_radio_rueda() const { return alto * 0.42; }

public:
    CajaHamster(int id, Vector2D pos, double w = 90.0, double h = 80.0)
        : ObstaculoEstatico(id, pos, TipoForma::AABB),
          ancho(w), alto(h), activo(false),
          fase_rueda(0.0), velocidad_rueda(0.0),
          velocidad_max(6.0), aceleracion(3.0) {
        set_restitucion(0.2);
        set_friccion(0.5);
    }

    double get_ancho() const { return ancho; }
    double get_alto()  const { return alto; }
    bool   get_activo() const { return activo; }
    double get_fase_rueda() const { return fase_rueda; }
    double get_velocidad_rueda() const { return velocidad_rueda; }
    const std::vector<int>& get_ids_ventiladores() const { return ids_ventiladores; }
    void agregar_ventilador(int id) { ids_ventiladores.push_back(id); }
    void quitar_ventilador(int id) {
        ids_ventiladores.erase(
            std::remove(ids_ventiladores.begin(), ids_ventiladores.end(), id),
            ids_ventiladores.end());
    }

    // AABB completo para colisión física (la caja sólida)
    Vector2D get_min() const override { return posicion; }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho, posicion.y + alto); }

    // Hitbox preciso de la rueda para activación
    bool circulo_toca_rueda(const Vector2D& pos_circ, double radio_circ) const {
        return (pos_circ - get_centro_rueda()).magnitud() <= get_radio_rueda() + radio_circ;
    }

    void activar() { activo = true; }
    void desactivar() {
        activo = false;
        velocidad_rueda = 0.0;
    }

    void actualizar_fisica(double dt) override {
        if (!activo) {
            // Desacelerar si estaba girando
            if (velocidad_rueda > 0.0) {
                velocidad_rueda -= aceleracion * 2.0 * dt;
                if (velocidad_rueda < 0.0) velocidad_rueda = 0.0;
                fase_rueda += velocidad_rueda * dt;
            }
            return;
        }
        // Acelerar hasta velocidad máxima
        velocidad_rueda += aceleracion * dt;
        if (velocidad_rueda > velocidad_max) velocidad_rueda = velocidad_max;
        fase_rueda += velocidad_rueda * dt;
    }

    void on_collision(EntidadFisica* otro, const InfoColision& info) override {
        if (!otro || otro->get_es_estatico()) return;
        activar();
    }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - 10 && p.x <= posicion.x + ancho + 10 &&
               p.y >= posicion.y - 10 && p.y <= posicion.y + alto + 10;
    }

    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::CAJA_HAMSTER;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent CAJA_HAMSTER id=" << get_id() << serializar_base()
           << " w=" << ancho << " h=" << alto << " vents=";
        for (size_t i = 0; i < ids_ventiladores.size(); ++i) {
            if (i > 0) ss << ",";
            ss << ids_ventiladores[i];
        }
        return ss.str();
    }

    void dibujar(bool debug) const override {
        float px  = static_cast<float>(posicion.x);
        float py  = static_cast<float>(posicion.y);
        float w   = static_cast<float>(ancho);
        float h   = static_cast<float>(alto);

        Vector2D cr = get_centro_rueda();
        float rcx = static_cast<float>(cr.x);
        float rcy = static_cast<float>(cr.y);
        float rr  = static_cast<float>(get_radio_rueda());
        float fase = static_cast<float>(fase_rueda);

        Color col_caja  = activo ? Color{180, 130, 60, 255}  : Color{140, 100, 50, 255};
        Color col_rueda = activo ? Color{210, 170, 80, 255}   : Color{170, 130, 60, 255};
        Color col_rayo  = activo ? Color{230, 200, 120, 255}  : Color{200, 170, 100, 255};
        Color col_hamster = Color{230, 200, 160, 255};

        // --- Caja exterior ---
        DrawRectangleRec({px, py, w * 0.35f, h}, col_caja);
        DrawRectangleLinesEx({px, py, w * 0.35f, h}, 2.0f, Color{80, 50, 20, 255});

        // --- Rueda ---
        DrawCircle(static_cast<int>(rcx), static_cast<int>(rcy), rr, col_rueda);
        DrawCircleLines(static_cast<int>(rcx), static_cast<int>(rcy), rr, Color{80, 50, 20, 255});

        // Rayos de la rueda (giran con fase)
        for (int i = 0; i < 6; i++) {
            float ang = fase + i * (MathUtils::TIM_PI / 3.0f);
            float x2 = rcx + std::cos(ang) * rr * 0.88f;
            float y2 = rcy + std::sin(ang) * rr * 0.88f;
            DrawLineEx({rcx, rcy}, {x2, y2}, 2.0f, col_rayo);
        }

        // Aro interior de la rueda
        DrawCircleLines(static_cast<int>(rcx), static_cast<int>(rcy), rr * 0.25f, Color{80, 50, 20, 200});
        DrawCircle(static_cast<int>(rcx), static_cast<int>(rcy), rr * 0.12f, Color{80, 50, 20, 255});

        // --- Hámster dentro de la rueda ---
        // El hámster se mueve con la rueda — su posición varía con la fase
        float hx = rcx + std::sin(fase) * rr * 0.35f;
        float hy = rcy + std::cos(fase) * rr * 0.25f + rr * 0.15f;
        float hr = rr * 0.22f;
        // Cuerpo
        DrawCircle(static_cast<int>(hx), static_cast<int>(hy), hr, col_hamster);
        // Ojos
        DrawCircle(static_cast<int>(hx + hr * 0.4f), static_cast<int>(hy - hr * 0.3f), hr * 0.18f, BLACK);
        // Orejas
        DrawCircle(static_cast<int>(hx - hr * 0.3f), static_cast<int>(hy - hr * 0.8f), hr * 0.22f, col_hamster);
        DrawCircle(static_cast<int>(hx + hr * 0.3f), static_cast<int>(hy - hr * 0.8f), hr * 0.22f, col_hamster);

        // --- Punto de anclaje de banda (lado izquierdo de la caja) ---
        float anchor_x = px + w * 0.35f;
        float anchor_y = py + h * 0.5f;
        DrawCircle(static_cast<int>(anchor_x), static_cast<int>(anchor_y), 5.0f, Color{255, 180, 0, 255});
        DrawCircleLines(static_cast<int>(anchor_x), static_cast<int>(anchor_y), 5.0f, Color{180, 100, 0, 255});

        if (debug) {
            // AABB completo (gris)
            DrawRectangleLines(static_cast<int>(px), static_cast<int>(py),
                               static_cast<int>(w), static_cast<int>(h), Color{120,120,120,180});
            // Hitbox rueda (verde)
            DrawCircleLines(static_cast<int>(rcx), static_cast<int>(rcy), rr, GREEN);
            // Hitbox caja izquierda (verde)
            DrawRectangleLines(static_cast<int>(px), static_cast<int>(py),
                               static_cast<int>(w * 0.35f), static_cast<int>(h), GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Hamster" tab=0 categoria=1
