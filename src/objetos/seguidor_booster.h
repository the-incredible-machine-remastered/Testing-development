#pragma once
// ============================================================================
// SeguidorBooster (Futbolista Impulsor) — Objeto interactivo de suelo
// Al detectar una pelota, corre lateralmente y la patea (kick) a 45 grados.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "../core/registro_eventos.h"
#include "../sistema/assets_extern.h"
#include "bola.h"
#include <vector>
#include <cmath>
#include <algorithm>

enum class EstadoSeguidor {
    ESPERANDO,    // Parado en su base, rebotando sutilmente
    PERSIGUIENDO,  // Corriendo lateralmente hacia la pelota
    RETRAYENDO    // Volviendo a la base
};

class SeguidorBooster : public EntidadFisica {
protected:
    Vector2D posicion_inicial;
    double radio; // Radio para chequeos de proximidad de patada
    double ancho; // Ancho físico AABB
    double alto;  // Alto físico AABB
    EstadoSeguidor estado;
    int id_bola_objetivo;
    double rango_deteccion;
    double velocidad_persecucion;
    double velocidad_retorno;
    double cooldown_timer;
    double oscilacion_idle;
    
    // Propiedades de animación
    double angulo_pierna;
    double direccion_carrera; // +1.0 = Derecha, -1.0 = Izquierda
    double kicker_factor;     // Animación de chute (1.0 = pierna arriba)

    // Cabezazo de Messi
    bool cabezazo_activo;
    double cabezazo_timer;
    double cooldown_cabezazo;

    void dibuja_seguidor_geometrico(Vector2D pos, float w, float h, float draw_y, 
                                     EstadoSeguidor est) const {
        float r_sz = w * 0.75f;
        double ang_p = angulo_pierna;
        double dir_c = direccion_carrera;
        double kick_f = kicker_factor;

        // Pierna 1
        float leg1_ang = static_cast<float>(ang_p);
        float lx1 = pos.x - dir_c * w * 0.3f + std::sin(leg1_ang) * r_sz * 0.7f;
        float ly1 = draw_y + h / 2.0f - r_sz * 0.2f + std::cos(leg1_ang) * r_sz * 0.5f;
        DrawLineEx({(float)(pos.x - dir_c * w * 0.2f), (float)(draw_y + h / 2.0f - r_sz * 0.4f)}, {lx1, ly1}, 4.0f, DARKGRAY);
        DrawCircle(lx1, ly1, 3.0f, RED);

        // Pierna 2
        float leg2_ang = static_cast<float>(-ang_p + kick_f * dir_c * 1.5);
        float lx2 = pos.x + dir_c * w * 0.3f + std::sin(leg2_ang) * r_sz * 0.7f;
        float ly2 = draw_y + h / 2.0f - r_sz * 0.2f + std::cos(leg2_ang) * r_sz * 0.5f;
        DrawLineEx({(float)(pos.x + dir_c * w * 0.2f), (float)(draw_y + h / 2.0f - r_sz * 0.4f)}, {lx2, ly2}, 4.0f, DARKGRAY);
        DrawCircle(lx2, ly2, 3.0f, RED);

        // Cuerpo
        int cx = static_cast<int>(pos.x);
        int cy = static_cast<int>(draw_y + h * 0.1f);
        DrawCircle(cx, cy, r_sz * 0.7f, SKYBLUE);
        DrawCircleLines(cx, cy, r_sz * 0.7f, BLUE);
        DrawRectangleRec({(float)(cx - r_sz * 0.3f), (float)(cy - r_sz * 0.6f), (float)(r_sz * 0.15f), (float)(r_sz * 1.2f)}, WHITE);
        DrawRectangleRec({(float)(cx + r_sz * 0.15f), (float)(cy - r_sz * 0.6f), (float)(r_sz * 0.15f), (float)(r_sz * 1.2f)}, WHITE);

        // Cabeza
        int head_y = static_cast<int>(draw_y - h * 0.2f);
        DrawCircle(cx, head_y, r_sz * 0.45f, Color{250, 200, 160, 255});
        DrawCircleLines(cx, head_y, r_sz * 0.45f, DARKGRAY);
        DrawCircleSector({(float)cx, (float)head_y}, r_sz * 0.46f, 180.0f, 360.0f, 0, Color{130, 80, 40, 255});

        int eye_x = cx + static_cast<int>(dir_c * r_sz * 0.25f);
        int eye_y = head_y - static_cast<int>(r_sz * 0.05f);
        DrawCircle(eye_x, eye_y, 2.5f, BLACK);

        // LED
        Color led_color = GREEN;
        if (est == EstadoSeguidor::PERSIGUIENDO) led_color = ORANGE;
        else if (est == EstadoSeguidor::RETRAYENDO) led_color = PURPLE;
        DrawCircle(cx - static_cast<int>(dir_c * r_sz * 0.3f), cy - r_sz * 0.1f, 3.5f, led_color);
    }

public:
    SeguidorBooster(int id, Vector2D pos, double w = 24.0, double h = 48.0)
        : EntidadFisica(id, pos, 15.0, TipoForma::AABB, false),
          posicion_inicial(pos), radio(30.0), ancho(w), alto(h),
          estado(EstadoSeguidor::ESPERANDO), id_bola_objetivo(-1), rango_deteccion(450.0),
          velocidad_persecucion(260.0), velocidad_retorno(180.0),
          cooldown_timer(0.0), oscilacion_idle(0.0), angulo_pierna(0.0),
          direccion_carrera(1.0), kicker_factor(0.0),
          cabezazo_activo(false), cabezazo_timer(0.0), cooldown_cabezazo(0.0) {
        
        set_restitucion(0.1); // Poco rebote para mantenerse firme
        set_friccion(0.5);    // Buen agarre en plataformas
        tipo_menu = TipoObjetoMenu::SEGUIDOR_BOOSTER;
    }

    // --- Getters ---
    double get_radio() const { return radio; }
    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    EstadoSeguidor get_estado() const { return estado; }
    Vector2D get_posicion_inicial() const { return posicion_inicial; }
    double get_angulo_pierna() const { return angulo_pierna; }
    double get_direccion_carrera() const { return direccion_carrera; }
    double get_kicker_factor() const { return kicker_factor; }
    double get_cooldown() const { return cooldown_timer; }
    double get_rango_deteccion() const { return rango_deteccion; }
    bool get_cabezazo_activo() const { return cabezazo_activo; }
    double get_cabezazo_timer() const { return cabezazo_timer; }

    // Límites de la caja en base a su centro
    Vector2D get_min() const override {
        return Vector2D(posicion.x - ancho / 2.0, posicion.y - alto / 2.0);
    }
    Vector2D get_max() const override {
        return Vector2D(posicion.x + ancho / 2.0, posicion.y + alto / 2.0);
    }

    void detectar_cabezazo(const std::vector<EntidadFisica*>& entidades) {
        if (cooldown_cabezazo > 0.0) return;

        for (auto* e : entidades) {
            if (e->get_tipo_forma() == TipoForma::CIRCULO) {
                auto* bola = dynamic_cast<Bola*>(e);
                if (bola) {
                    double bx = bola->get_posicion().x;
                    double by = bola->get_posicion().y;
                    double br = bola->get_radio();

                    double tope_cabeza = posicion.y - alto / 2.0;

                    bool encima_cabeza = (by + br < posicion.y - alto * 0.3);
                    bool contacto_vertical = (by + br >= tope_cabeza - 10.0) && (by - br <= tope_cabeza + 15.0);
                    bool horizontal_ok = (std::abs(bx - posicion.x) < ancho * 0.8);

                    if (encima_cabeza && contacto_vertical && horizontal_ok) {
                        Vector2D dir_impulso = Vector2D(direccion_carrera * 0.7071, -0.7071).normalizar();
                        double fuerza_cabezazo = 350.0;
                        bola->set_velocidad(dir_impulso * fuerza_cabezazo);

                        cabezazo_activo = true;
                        cabezazo_timer = 0.6;
                        cooldown_cabezazo = 1.0;

                        eventos_pendientes.push_back({
                            TipoEventoEspecial::CABEZAZO,
                            this->get_id(),
                            bola->get_id()
                        });

                        velocidad.x = 0.0;
                        estado = EstadoSeguidor::RETRAYENDO;
                        id_bola_objetivo = -1;
                        break; 
                    }
                }
            }
        }
    }

    void actualizar_comportamiento(const std::vector<EntidadFisica*>& entidades, double dt) {
        if (cooldown_timer > 0.0) {
            cooldown_timer -= dt;
            if (cooldown_timer < 0.0) cooldown_timer = 0.0;
        }

        if (cabezazo_timer > 0.0) {
            cabezazo_timer -= dt;
            if (cabezazo_timer <= 0.0) {
                cabezazo_timer = 0.0;
                cabezazo_activo = false;
            }
        }
        if (cooldown_cabezazo > 0.0) {
            cooldown_cabezazo -= dt;
            if (cooldown_cabezazo < 0.0) cooldown_cabezazo = 0.0;
        }

        detectar_cabezazo(entidades);

        switch (estado) {
            case EstadoSeguidor::ESPERANDO:
                oscilacion_idle += dt * 5.0;
                angulo_pierna = 0.0;
                kicker_factor = 0.0;
                break;
            case EstadoSeguidor::PERSIGUIENDO:
                oscilacion_idle += dt * 15.0;
                angulo_pierna = std::sin(oscilacion_idle) * 0.6;
                kicker_factor = 0.0;
                break;
            case EstadoSeguidor::RETRAYENDO:
                oscilacion_idle += dt * 12.0;
                angulo_pierna = std::sin(oscilacion_idle) * 0.4;
                if (kicker_factor > 0.0) {
                    kicker_factor -= dt * 4.0;
                    if (kicker_factor < 0.0) kicker_factor = 0.0;
                }
                break;
        }

        switch (estado) {
            case EstadoSeguidor::ESPERANDO: {
                velocidad.x = 0.0;

                if (cooldown_timer <= 0.0) {
                    double dist_min = rango_deteccion;
                    EntidadFisica* target = nullptr;
                    for (auto* e : entidades) {
                        if (e->get_tipo_forma() == TipoForma::CIRCULO) {
                            auto* bola = dynamic_cast<Bola*>(e);
                            if (bola) {
                                double diff_y = std::abs(posicion.y - bola->get_posicion().y);
                                if (diff_y < 120.0) {
                                    double dist = Vector2D::distancia(posicion, bola->get_posicion());
                                    if (dist < dist_min) {
                                        dist_min = dist;
                                        target = bola;
                                    }
                                }
                            }
                        }
                    }

                    if (target) {
                        id_bola_objetivo = target->get_id();
                        estado = EstadoSeguidor::PERSIGUIENDO;
                        direccion_carrera = (target->get_posicion().x > posicion.x) ? 1.0 : -1.0;
                    }
                }
                break;
            }

            case EstadoSeguidor::PERSIGUIENDO: {
                EntidadFisica* target = nullptr;
                for (auto* e : entidades) {
                    if (e->get_id() == id_bola_objetivo) {
                        target = e;
                        break;
                    }
                }

                if (!target) {
                    estado = EstadoSeguidor::RETRAYENDO;
                    id_bola_objetivo = -1;
                    break;
                }

                double diff_x = target->get_posicion().x - posicion.x;
                direccion_carrera = (diff_x > 0.0) ? 1.0 : -1.0;

                velocidad.x = direccion_carrera * velocidad_persecucion;

                auto* bola = dynamic_cast<Bola*>(target);
                if (bola) {
                    double dist_real = Vector2D::distancia(posicion, bola->get_posicion());
                    double suma_radios = radio + bola->get_radio();

                    if (dist_real < (suma_radios + 8.0)) {
                        Vector2D dir_impulso = Vector2D(direccion_carrera, -1.0).normalizar();
                        double fuerza_patada = 680.0;
                        bola->set_velocidad(dir_impulso * fuerza_patada);

                        kicker_factor = 1.0; 

                        eventos_pendientes.push_back({
                            TipoEventoEspecial::PATADA,
                            this->get_id(),
                            bola->get_id()
                        });

                        velocidad.x = 0.0;
                        estado = EstadoSeguidor::RETRAYENDO;
                        id_bola_objetivo = -1;
                    }
                }
                break;
            }

            case EstadoSeguidor::RETRAYENDO: {
                double diff_x = posicion_inicial.x - posicion.x;
                double dist = std::abs(diff_x);

                if (dist < 4.0) {
                    posicion.x = posicion_inicial.x;
                    velocidad.x = 0.0;
                    cooldown_timer = 1.5;
                    estado = EstadoSeguidor::ESPERANDO;
                } else {
                    double dir_x = (diff_x > 0.0) ? 1.0 : -1.0;
                    direccion_carrera = dir_x;
                    velocidad.x = dir_x * velocidad_retorno;
                }
                break;
            }
        }
    }

    void actualizar_fisica(double dt) override {
        if (masa > MathUtils::EPSILON) {
            aceleracion = fuerza_neta / masa;
        } else {
            aceleracion = Vector2D(0.0, 0.0);
        }
        
        velocidad.y += aceleracion.y * dt;
        
        if (velocidad.y > 600.0) velocidad.y = 600.0;

        if (estado == EstadoSeguidor::ESPERANDO) {
            velocidad.x = 0.0;
        } else if (estado == EstadoSeguidor::PERSIGUIENDO) {
            velocidad.x = direccion_carrera * velocidad_persecucion;
        } else if (estado == EstadoSeguidor::RETRAYENDO) {
            velocidad.x = direccion_carrera * velocidad_retorno;
        }

        posicion.x += velocidad.x * dt;
        posicion.y += velocidad.y * dt;
        
        fuerza_neta = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        velocidad_angular = 0.0;
        angulo = 0.0;
    }

    // --- Métodos polimórficos ---
    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::SEGUIDOR;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent SEGUIDOR id=" << get_id() << serializar_base()
           << " w=" << ancho << " h=" << alto;
        return ss.str();
    }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - ancho/2 - 10 && p.x <= posicion.x + ancho/2 + 10 &&
               p.y >= posicion.y - alto/2 - 10 && p.y <= posicion.y + alto/2 + 10;
    }

    void dibujar(bool debug) const override {
        float w = static_cast<float>(ancho);
        float h = static_cast<float>(alto);
        double dir_carr = direccion_carrera;
        float draw_y = posicion.y - 6.0f; // Subir el sprite 6 píxeles

        // 1. Dibujar sombra sutil en el suelo
        DrawEllipse(static_cast<int>(posicion.x), static_cast<int>(posicion.y + h / 2.0f - 2.0f), w * 0.8f, 3.0f, Color{0, 0, 0, 80});

        // 2. Línea de anclaje
        if (estado != EstadoSeguidor::ESPERANDO) {
            DrawLineEx({(float)posicion_inicial.x, (float)posicion_inicial.y}, {(float)posicion.x, (float)draw_y}, 1.5f, Color{100, 150, 255, 100});
        }

        // 3. Dibujar sprite del personaje
        float sprite_w = w * 1.5f;
        float sprite_h = h * 1.2f;
        Vector2 pos_draw = {static_cast<float>(posicion.x), draw_y};

        if (cabezazo_activo) {
            if (tex_seguidor_cabezazo.id > 0) {
                float flip_dir = (dir_carr < 0.0) ? -1.0f : 1.0f;
                Rectangle source = {0, 0, (float)tex_seguidor_cabezazo.width * flip_dir, (float)tex_seguidor_cabezazo.height};
                Rectangle dest = {pos_draw.x - sprite_w/2, pos_draw.y - sprite_h/2, sprite_w, sprite_h};
                DrawTexturePro(tex_seguidor_cabezazo, source, dest, {0, 0}, 0.0f, WHITE);
            } else {
                dibuja_seguidor_geometrico(posicion, w, h, draw_y, estado);
            }
        } else if (estado == EstadoSeguidor::ESPERANDO) {
            if (tex_seguidor_quieto.id > 0) {
                Rectangle source = {0, 0, (float)tex_seguidor_quieto.width, (float)tex_seguidor_quieto.height};
                Rectangle dest = {pos_draw.x - sprite_w/2, pos_draw.y - sprite_h/2, sprite_w, sprite_h};
                DrawTexturePro(tex_seguidor_quieto, source, dest, {0, 0}, 0.0f, WHITE);
            } else {
                dibuja_seguidor_geometrico(posicion, w, h, draw_y, estado);
            }
        } else if (anim_seguidor_corriendo) {
            if (dir_carr > 0) {
                anim_seguidor_corriendo->dibujar(pos_draw, sprite_w, sprite_h);
            } else {
                anim_seguidor_corriendo->dibujar_volteado(pos_draw, sprite_w, sprite_h);
            }
        } else if (tex_seguidor_corriendo.id > 0) {
            Rectangle source = {0, 0, (float)tex_seguidor_corriendo.width, (float)tex_seguidor_corriendo.height};
            Rectangle dest = {pos_draw.x - sprite_w/2, pos_draw.y - sprite_h/2, sprite_w, sprite_h};
            DrawTexturePro(tex_seguidor_corriendo, source, dest, {0, 0}, 0.0f, WHITE);
        } else {
            dibuja_seguidor_geometrico(posicion, w, h, draw_y, estado);
        }

        if (debug) {
            float rx = static_cast<float>(rango_deteccion);
            DrawCircleLines(static_cast<int>(posicion.x), static_cast<int>(posicion.y), rx, Color{0, 255, 0, 80});
            DrawRectangleRec({(float)(posicion.x - rx), (float)(posicion.y - 120.0f), rx * 2.0f, 240.0f}, Color{0, 255, 0, 15});
            DrawRectangleLinesEx({(float)(posicion.x - rx), (float)(posicion.y - 120.0f), rx * 2.0f, 240.0f}, 1.0f, Color{0, 255, 0, 45});
            DrawRectangleLines(static_cast<int>(posicion.x - w/2), static_cast<int>(posicion.y - h/2), static_cast<int>(w), static_cast<int>(h), GREEN);
        }
    }
};
// TIM_MENU_SPAWN etiqueta="Futbolista" tab=0 categoria=1
