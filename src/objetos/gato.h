#pragma once
// ============================================================================
// Gato — persigue al Raton por el suelo (gravedad + plataformas). Si un ratón
// entra en su rango de visión, corre hacia él; si lo alcanza (contacto), lo ATRAPA
// (el motor elimina al ratón). Es más grande que el ratón, así que no cabe por los
// huecos estrechos por donde el ratón escapa.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "../sistema/assets_extern.h"
#include <vector>
#include <cmath>
#include <algorithm>

class Gato : public EntidadFisica {
protected:
    double ancho;
    double alto;
    double rango_vision;
    double velocidad_caza;
    double velocidad_paseo;
    double dir_x;
    bool cazando;
    int id_raton_objetivo;   // ratón que persigue (-1 = ninguno)
    int id_raton_atrapado;   // ratón a eliminar este frame (-1 = ninguno)
    double patas_fase;
    double lengua_timer;     // animación al atrapar

public:
    Gato(int id, Vector2D pos, double w = 54.0, double h = 58.0)
        : EntidadFisica(id, pos, 6.0, TipoForma::AABB, false),
          ancho(w), alto(h), rango_vision(340.0),
          velocidad_caza(210.0), velocidad_paseo(55.0),
          dir_x(1.0), cazando(false),
          id_raton_objetivo(-1), id_raton_atrapado(-1),
          patas_fase(0.0), lengua_timer(0.0) {
        set_restitucion(0.05);
        set_friccion(0.5);
        set_amortiguamiento(0.02);
        tipo_menu = TipoObjetoMenu::GATO;
    }

    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    double get_rango_vision() const { return rango_vision; }
    double get_dir_x() const { return dir_x; }
    bool get_cazando() const { return cazando; }
    double get_patas_fase() const { return patas_fase; }
    double get_lengua_timer() const { return lengua_timer; }

    Vector2D get_min() const override { return Vector2D(posicion.x - ancho/2.0, posicion.y - alto/2.0); }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho/2.0, posicion.y + alto/2.0); }

    // El motor consulta y limpia esto para eliminar al ratón atrapado.
    int consumir_raton_atrapado() { int r = id_raton_atrapado; id_raton_atrapado = -1; return r; }

    // Persigue al ratón más cercano en rango; si lo toca, lo marca atrapado.
    // Parámetros: info del ratón más cercano (calculada por el motor).
    void actualizar_comportamiento(bool hay_raton, int id_raton, const Vector2D& raton_pos,
                                   double dist_raton, double dt) {
        patas_fase += dt * (cazando ? 22.0 : 8.0);
        if (lengua_timer > 0.0) { lengua_timer -= dt; if (lengua_timer < 0.0) lengua_timer = 0.0; }

        if (hay_raton && dist_raton <= rango_vision) {
            cazando = true;
            id_raton_objetivo = id_raton;
            dir_x = (raton_pos.x > posicion.x) ? 1.0 : -1.0;

            // Contacto: si el ratón está muy cerca, lo atrapa.
            double alcance = ancho * 0.5 + 14.0;
            if (dist_raton <= alcance) {
                id_raton_atrapado = id_raton;
                cazando = false;
                id_raton_objetivo = -1;
                lengua_timer = 0.4;
            }
        } else {
            cazando = false;
            id_raton_objetivo = -1;
        }
    }

    void actualizar_fisica(double dt) override {
        aceleracion = (masa > MathUtils::EPSILON) ? fuerza_neta / masa : Vector2D(0.0, 0.0);
        velocidad.y += aceleracion.y * dt;
        if (velocidad.y > 700.0) velocidad.y = 700.0;

        double vx_obj = cazando ? dir_x * velocidad_caza : 0.0; // quieto si no caza
        velocidad.x += (vx_obj - velocidad.x) * std::min(1.0, dt * 10.0);

        posicion += velocidad * dt;

        fuerza_neta = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        velocidad_angular = 0.0;
        angulo = 0.0;
    }

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::GATO; }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent GATO id=" << get_id() << serializar_base()
           << " w=" << ancho << " h=" << alto;
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - ancho/2 - 8 && p.x <= posicion.x + ancho/2 + 8 &&
               p.y >= posicion.y - alto/2 - 8 && p.y <= posicion.y + alto/2 + 8;
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float w  = static_cast<float>(ancho);
        float h  = static_cast<float>(alto);
        float d  = (dir_x < 0.0) ? -1.0f : 1.0f;

        bool esta_quieto = (std::abs(velocidad.x) < 5.0);

        if (esta_quieto && tex_gato_quieto.id > 0) {
            Rectangle src = {0.0f, 0.0f, (float)tex_gato_quieto.width * d, (float)tex_gato_quieto.height};
            Rectangle dst = {px, py, w, h};
            Vector2 origin = {w / 2.0f, h / 2.0f};
            DrawTexturePro(tex_gato_quieto, src, dst, origin, 0.0f, WHITE);
        } else if (cazando && tex_gato_caminando.id > 0) {
            int frame = ((int)patas_fase) % 8;
            int frame_w = tex_gato_caminando.width / 8;
            int frame_h = tex_gato_caminando.height;
            Rectangle src = {(float)(frame * frame_w), 0.0f, (float)frame_w * d, (float)frame_h};
            Rectangle dst = {px, py, w, h};
            Vector2 origin = {w / 2.0f, h / 2.0f};
            DrawTexturePro(tex_gato_caminando, src, dst, origin, 0.0f, WHITE);

        } else if (!esta_quieto && tex_gato_caminando.id > 0) {
            int frame = ((int)patas_fase) % 8;
            int frame_w = tex_gato_caminando.width / 8;
            int frame_h = tex_gato_caminando.height;
            Rectangle src = {(float)(frame * frame_w), 0.0f, (float)frame_w * d, (float)frame_h};
            Rectangle dst = {px, py, w, h};
            Vector2 origin = {w / 2.0f, h / 2.0f};
            DrawTexturePro(tex_gato_caminando, src, dst, origin, 0.0f, WHITE);
        } else {
            Color cuerpo = Color{235, 235, 240, 255}; // gato blanco
            Color oscuro = Color{170, 170, 180, 255};
            Color rosa   = Color{235, 150, 160, 255};

            DrawEllipse((int)px, (int)(py + h*0.5f), w*0.55f, 4.0f, Color{0,0,0,70});
            float colax = px - d * w*0.45f;
            DrawLineBezier({colax, py + h*0.1f}, {colax - d*w*0.3f, py - h*0.5f}, 5.0f, cuerpo);

            float paso = std::sin((float)patas_fase) * 3.0f;
            for (int i=-1;i<=1;i+=2) {
                float lx = px + i * w*0.28f;
                DrawLineEx({lx, py + h*0.25f}, {lx + paso*i, py + h*0.5f}, 4.0f, cuerpo);
            }

            DrawEllipse((int)px, (int)py, w*0.5f, h*0.42f, cuerpo);

            float hx = px + d * w*0.32f;
            float hy = py - h*0.18f;
            DrawCircle((int)hx, (int)hy, h*0.34f, cuerpo);
            DrawTriangle({hx - d*h*0.28f, hy - h*0.18f}, {hx - d*h*0.05f, hy - h*0.55f}, {hx + d*h*0.02f, hy - h*0.2f}, cuerpo);
            DrawTriangle({hx + d*h*0.28f, hy - h*0.18f}, {hx + d*h*0.18f, hy - h*0.55f}, {hx - d*h*0.02f, hy - h*0.2f}, cuerpo);
            DrawCircle((int)(hx + d*h*0.05f), (int)(hy - h*0.05f), 3.0f, Color{80,150,90,255});
            DrawCircle((int)(hx + d*h*0.22f), (int)(hy - h*0.05f), 3.0f, Color{80,150,90,255});
            DrawCircle((int)(hx + d*h*0.05f), (int)(hy - h*0.05f), 1.2f, BLACK);
            DrawCircle((int)(hx + d*h*0.22f), (int)(hy - h*0.05f), 1.2f, BLACK);
            DrawCircle((int)(hx + d*h*0.15f), (int)(hy + h*0.12f), 2.0f, rosa);
            DrawLineEx({hx + d*h*0.15f, hy+h*0.12f}, {hx + d*h*0.7f, hy}, 0.9f, oscuro);
            DrawLineEx({hx + d*h*0.15f, hy+h*0.12f}, {hx + d*h*0.7f, hy+h*0.25f}, 0.9f, oscuro);
            if (lengua_timer > 0.0)
                DrawCircle((int)(hx + d*h*0.2f), (int)(hy + h*0.2f), 3.0f, rosa);

            DrawEllipseLines((int)px, (int)py, w*0.5f, h*0.42f, oscuro);
        }

        if (debug) {
            DrawRectangleLines((int)(px - w/2), (int)(py - h/2), (int)w, (int)h, GREEN);
            // Campo de visión = FRANJA horizontal (alto = 2*MARGEN_ALTURA=110), no círculo.
            float rx = (float)rango_vision;
            float franja = 110.0f;
            DrawRectangleRec({px - rx, py - franja/2, rx*2, franja}, Color{255,120,0,20});
            DrawRectangleLinesEx({px - rx, py - franja/2, rx*2, franja}, 1.0f, Color{255,120,0,70});
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Gato" tab=0 categoria=1
