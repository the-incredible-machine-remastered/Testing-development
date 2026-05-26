#pragma once
// ============================================================================
// SeguidorBooster (Futbolista Impulsor) — Objeto interactivo de suelo
// Al detectar una pelota, corre lateralmente y la patea (kick) a 45 grados.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/math_utils.h"
#include "bola.h"
#include <vector>
#include <cmath>

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

public:
    SeguidorBooster(int id, Vector2D pos, double w = 24.0, double h = 48.0)
        : EntidadFisica(id, pos, 15.0, TipoForma::AABB, false), // AABB dinámico con masa = 15.0 (pesado)
          posicion_inicial(pos), radio(30.0), ancho(w), alto(h),
          estado(EstadoSeguidor::ESPERANDO), id_bola_objetivo(-1), rango_deteccion(450.0),
          velocidad_persecucion(260.0), velocidad_retorno(180.0),
          cooldown_timer(0.0), oscilacion_idle(0.0), angulo_pierna(0.0),
          direccion_carrera(1.0), kicker_factor(0.0) {
        
        set_restitucion(0.1); // Poco rebote para mantenerse firme
        set_friccion(0.5);    // Buen agarre en plataformas
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

    // Límites de la caja en base a su centro
    Vector2D get_min() const {
        return Vector2D(posicion.x - ancho / 2.0, posicion.y - alto / 2.0);
    }
    Vector2D get_max() const {
        return Vector2D(posicion.x + ancho / 2.0, posicion.y + alto / 2.0);
    }

    // Actualiza el comportamiento lógico y de estados del futbolista
    void actualizar_comportamiento(const std::vector<EntidadFisica*>& entidades, double dt) {
        // Enfriamiento de patada
        if (cooldown_timer > 0.0) {
            cooldown_timer -= dt;
            if (cooldown_timer < 0.0) cooldown_timer = 0.0;
        }

        // Manejo de animaciones
        switch (estado) {
            case EstadoSeguidor::ESPERANDO:
                oscilacion_idle += dt * 5.0;
                angulo_pierna = 0.0;
                kicker_factor = 0.0;
                break;
            case EstadoSeguidor::PERSIGUIENDO:
                oscilacion_idle += dt * 15.0;
                // Piernas oscilan en tijera
                angulo_pierna = std::sin(oscilacion_idle) * 0.6;
                kicker_factor = 0.0;
                break;
            case EstadoSeguidor::RETRAYENDO:
                oscilacion_idle += dt * 12.0;
                angulo_pierna = std::sin(oscilacion_idle) * 0.4;
                // Decaimiento del chute
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
                    // Buscar la bola más cercana en rango horizontal
                    double dist_min = rango_deteccion;
                    EntidadFisica* target = nullptr;
                    for (auto* e : entidades) {
                        if (e->get_tipo_forma() == TipoForma::CIRCULO) {
                            auto* bola = dynamic_cast<Bola*>(e);
                            if (bola) {
                                // Rango de Y razonable (la bola debe estar en o cerca de su altura)
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

                // Si la pelota desaparece o se sale de rango, trotar de vuelta
                if (!target) {
                    estado = EstadoSeguidor::RETRAYENDO;
                    id_bola_objetivo = -1;
                    break;
                }

                double diff_x = target->get_posicion().x - posicion.x;
                double dist = std::abs(diff_x);
                direccion_carrera = (diff_x > 0.0) ? 1.0 : -1.0;

                // Velocidad horizontal controlada por el script
                velocidad.x = direccion_carrera * velocidad_persecucion;

                // Comprobar contacto
                auto* bola = dynamic_cast<Bola*>(target);
                if (bola) {
                    double dist_real = Vector2D::distancia(posicion, bola->get_posicion());
                    double suma_radios = radio + bola->get_radio();

                    if (dist_real < (suma_radios + 8.0)) {
                        // ¡CHUT! (Patear en la dirección de la carrera, a 45 grados hacia arriba)
                        Vector2D dir_impulso = Vector2D(direccion_carrera, -1.0).normalizar();
                        double fuerza_patada = 680.0;
                        bola->set_velocidad(dir_impulso * fuerza_patada);

                        // Efecto de patada
                        kicker_factor = 1.0; 

                        // Volver a la base
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
                    direccion_carrera = dir_x; // Mira hacia donde regresa
                    velocidad.x = dir_x * velocidad_retorno;
                }
                break;
            }
        }
    }

    void actualizar_fisica(double dt) override {
        // 1. Integrar velocidad vertical (gravedad aplicada por el motor de física)
        if (masa > MathUtils::EPSILON) {
            aceleracion = fuerza_neta / masa;
        } else {
            aceleracion = Vector2D(0.0, 0.0);
        }
        
        velocidad.y += aceleracion.y * dt;
        
        // Limitar velocidad de caída libre para estabilidad
        if (velocidad.y > 600.0) velocidad.y = 600.0;

        // 2. Controlar la velocidad horizontal X según el estado actual
        if (estado == EstadoSeguidor::ESPERANDO) {
            velocidad.x = 0.0;
        } else if (estado == EstadoSeguidor::PERSIGUIENDO) {
            velocidad.x = direccion_carrera * velocidad_persecucion;
        } else if (estado == EstadoSeguidor::RETRAYENDO) {
            velocidad.x = direccion_carrera * velocidad_retorno;
        }

        // 3. Integrar posición en base a velocidades linealizadas
        posicion.x += velocidad.x * dt;
        posicion.y += velocidad.y * dt;
        
        // 4. Reiniciar acumuladores de fuerza y forzar rotación a 0 (siempre de pie)
        fuerza_neta = Vector2D(0.0, 0.0);
        aceleracion = Vector2D(0.0, 0.0);
        velocidad_angular = 0.0;
        angulo = 0.0;
    }
};

// TIM_MENU_SPAWN etiqueta="Futbolista" tab=0 categoria=1
