#pragma once
// ============================================================================
// DinamitaDetonador — dinamita conectada por un cable a un DETONADOR (émbolo).
// A diferencia de la Dinamita normal (mecha de ~1s), explota INMEDIATAMENTE
// cuando algo golpea la zona del detonador. Mismo daño/empuje que la dinamita.
// ============================================================================

#include "dinamita.h"
#include <algorithm>

class DinamitaDetonador : public Dinamita {
private:
    // Distancia horizontal (en px) entre los cartuchos y el detonador. Sube este
    // valor para un cable MÁS LARGO. También lo puedes cambiar en vivo con set_dist_detonador.
    double dist_detonador;

public:
    DinamitaDetonador(int id, Vector2D pos, double w = 26.0, double h = 46.0)
        : Dinamita(id, pos, w, h), dist_detonador(120.0) {
        retardo = 0.0; // explota en el acto (siguiente tick tras el golpe)
        tipo_menu = TipoObjetoMenu::DINAMITA_DETONADOR;
    }

    double get_dist_detonador() const { return dist_detonador; }
    void set_dist_detonador(double d) { dist_detonador = std::max(30.0, d); }

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::DINAMITA_DETONADOR; }

    // El detonador (émbolo) está a la derecha de los cartuchos, a dist_detonador del
    // borde derecho, unido por un cable.
    Vector2D get_pos_detonador() const {
        return Vector2D(posicion.x + ancho + dist_detonador, posicion.y + alto * 0.5);
    }

    // Medio-tamaño de la CAJA del detonador (émbolo). El hitbox de colisión es SOLO
    // esta caja: golpear los cartuchos o el cable NO activa; solo el detonador explota.
    static constexpr double DET_HW = 14.0; // medio ancho
    static constexpr double DET_HH = 16.0; // medio alto (incluye el émbolo/mango)

    // Hitbox = SOLO la caja del detonador (centrada en get_pos_detonador).
    Vector2D get_min() const override {
        Vector2D d = get_pos_detonador();
        return Vector2D(d.x - DET_HW, d.y - DET_HH);
    }
    Vector2D get_max() const override {
        Vector2D d = get_pos_detonador();
        return Vector2D(d.x + DET_HW, d.y + DET_HH);
    }
    // get_centro() (heredado de Dinamita) = centro de los cartuchos → la explosión
    // ocurre en la dinamita, no en el detonador. Correcto para el daño/empuje.

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent DINAMITA_DETONADOR id=" << get_id()
           << " x=" << posicion.x << " y=" << posicion.y
           << " w=" << ancho << " h=" << alto
           << " fijo=" << (es_fijo ? 1 : 0);
        return ss.str();
    }

    void dibujar(bool debug) const override {
        if (exploto) return;
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float w  = static_cast<float>(ancho);
        float h  = static_cast<float>(alto);

        // --- Cartuchos de dinamita (rojos) ---
        Color rojo  = encendida ? Color{225, 70, 55, 255} : Color{190, 55, 45, 255};
        Color rojo2 = Color{140, 35, 28, 255};
        float cw = w / 3.0f;
        for (int i = 0; i < 3; ++i) {
            float bx = px + i * cw;
            DrawRectangleRec({bx + 1.0f, py + 6.0f, cw - 2.0f, h - 6.0f}, rojo);
            DrawRectangleLinesEx({bx + 1.0f, py + 6.0f, cw - 2.0f, h - 6.0f}, 1.0f, rojo2);
        }
        DrawRectangleRec({px, py + h * 0.45f, w, 5.0f}, Color{90, 60, 30, 255});

        // --- Cable curvo hacia el detonador ---
        Vector2D det = get_pos_detonador();
        Vector2D salida(px + w, py + h * 0.5f);
        Color cable = Color{40, 40, 45, 255};
        // dos segmentos con una comba
        Vector2D medio((salida.x + det.x) * 0.5, std::max(salida.y, det.y) + 14.0);
        DrawLineBezier({(float)salida.x, (float)salida.y}, {(float)det.x, (float)det.y}, 2.5f, cable);
        (void)medio;

        // --- Detonador (caja + émbolo tipo T) ---
        float dx = (float)det.x, dy = (float)det.y;
        Color caja  = Color{120, 70, 30, 255};
        Color caja2 = Color{80, 45, 18, 255};
        Color metal = Color{150, 155, 160, 255};
        DrawRectangleRec({dx - 12.0f, dy - 8.0f, 24.0f, 20.0f}, caja);
        DrawRectangleLinesEx({dx - 12.0f, dy - 8.0f, 24.0f, 20.0f}, 2.0f, caja2);
        // Émbolo (vástago + mango en T)
        DrawRectangleRec({dx - 2.0f, dy - 22.0f, 4.0f, 16.0f}, metal);
        DrawRectangleRec({dx - 9.0f, dy - 24.0f, 18.0f, 4.0f}, metal);

        // Mecha/chispa (feedback si ya se activó, dura 1 tick pero se ve el flash)
        if (encendida) {
            DrawCircle((int)dx, (int)(dy - 22.0f), 4.0f, Color{255, 200, 60, 235});
        }

        if (debug) {
            Vector2D mn = get_min(), mx = get_max();
            DrawRectangleLines((int)mn.x, (int)mn.y, (int)(mx.x - mn.x), (int)(mx.y - mn.y), GREEN);
            DrawCircleLines((int)(px + w*0.5f), (int)(py + h*0.5f), (float)radio_explosion, RED);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Dina Detonador" tab=1 categoria=0
