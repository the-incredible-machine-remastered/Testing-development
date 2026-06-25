#pragma once
// ============================================================================
// MotorFisica — El "mundo" de la simulación
//
// Responsabilidades:
//   1. Gestionar la lista de entidades
//   2. Aplicar fuerzas globales (gravedad)
//   3. Integrar todas las entidades (cada una usa RK4 internamente)
//   4. Detectar y resolver colisiones
//   5. Usar timestep fijo con acumulador para estabilidad numérica
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../core/registro_eventos.h"
#include "../objetos/pared_rectangular.h"
#include "../objetos/plano_inclinado.h"
#include "../objetos/bola.h"
#include "../objetos/bola_rebotadora.h"
#include "../objetos/trampolin.h"
#include "../objetos/balancin.h"
#include "../objetos/cubeta.h"
#include "../objetos/cuerda.h"
#include "../objetos/seguidor_booster.h"
#include "../objetos/barril_chavo.h"
#include "../objetos/ventilador.h"
#include "colisiones.h"
#include "fisica_ventilador.h"
#include <vector>
#include <algorithm>
#include <memory>

class MotorFisica {
private:
    std::vector<std::unique_ptr<EntidadFisica>> entidades_owner;
    std::vector<EntidadFisica*> entidades;
    Vector2D gravedad;
    double dt_fijo;              // Timestep fijo (ej. 1/120 s)
    double acumulador_tiempo;    // Acumula tiempo real para pasos fijos
    bool pausado;
    int siguiente_id;
    std::vector<RegistroColision> colisiones_frame;
    std::vector<RegistroEventoEspecial> eventos_especiales_frame;

public:
    MotorFisica(double dt = 1.0 / 120.0, Vector2D grav = Vector2D(0, 500.0))
        : gravedad(grav), dt_fijo(dt), acumulador_tiempo(0.0),
          pausado(true), siguiente_id(1) {}

    ~MotorFisica() {
        limpiar();
    }

    // --- Gestión de entidades ---

    int generar_id() { return siguiente_id++; }

    void set_siguiente_id(int id) {
        siguiente_id = std::max(1, id);
    }

    int get_siguiente_id() const { return siguiente_id; }

    void agregar_entidad(EntidadFisica* e) {
        if (!e) return;
        entidades.push_back(e);
        entidades_owner.push_back(std::unique_ptr<EntidadFisica>(e));
    }

    void agregar_entidad(std::unique_ptr<EntidadFisica> e) {
        if (!e) return;
        entidades.push_back(e.get());
        entidades_owner.push_back(std::move(e));
    }

    void remover_entidad(int id) {
        entidades.erase(
            std::remove_if(entidades.begin(), entidades.end(),
                [id](EntidadFisica* e) { return e->get_id() == id; }),
            entidades.end()
        );
        entidades_owner.erase(
            std::remove_if(entidades_owner.begin(), entidades_owner.end(),
                [id](const std::unique_ptr<EntidadFisica>& e) { return e->get_id() == id; }),
            entidades_owner.end()
        );
    }

    std::unique_ptr<EntidadFisica> transferir_entidad(int id) {
        std::unique_ptr<EntidadFisica> target = nullptr;
        for (auto it = entidades_owner.begin(); it != entidades_owner.end(); ++it) {
            if ((*it) && (*it)->get_id() == id) {
                target = std::move(*it);
                entidades_owner.erase(it);
                break;
            }
        }
        if (target) {
            entidades.erase(
                std::remove(entidades.begin(), entidades.end(), target.get()),
                entidades.end()
            );
        }
        return target;
    }

    void limpiar() {
        entidades.clear();
        entidades_owner.clear();
        siguiente_id = 1;
    }

    // --- Control de simulación ---

    void set_pausado(bool p) { pausado = p; }
    bool get_pausado() const { return pausado; }
    void set_gravedad(const Vector2D& g) { gravedad = g; }
    Vector2D get_gravedad() const { return gravedad; }
    const std::vector<EntidadFisica*>& get_entidades() const { return entidades; }
    const std::vector<RegistroColision>& get_colisiones_frame() const { return colisiones_frame; }
    const std::vector<RegistroEventoEspecial>& get_eventos_especiales_frame() const { return eventos_especiales_frame; }

    // --- Bucle principal ---
    // Recibe el delta time real (variable) y lo subdivide en pasos fijos.
    // Esto evita inestabilidad numérica por variaciones de frame rate.
    void actualizar(double dt_real) {
        if (pausado) return;

        acumulador_tiempo += dt_real;

        // Limitar acumulador para evitar espiral de la muerte
        if (acumulador_tiempo > 0.1) acumulador_tiempo = 0.1;

        while (acumulador_tiempo >= dt_fijo) {
            paso_fisico(dt_fijo);
            acumulador_tiempo -= dt_fijo;
        }
    }

private:
    bool obtener_datos_circulo(EntidadFisica* e, Vector2D& pos, double& radio) {
        auto* bola = dynamic_cast<Bola*>(e);
        if (bola) {
            pos = bola->get_posicion();
            radio = bola->get_radio();
            return true;
        }

        auto* rebotadora = dynamic_cast<BolaRebotadora*>(e);
        if (rebotadora) {
            pos = rebotadora->get_posicion();
            radio = rebotadora->get_radio();
            return true;
        }

        return false;
    }

    // Un paso completo de simulación
    bool obtener_datos_aabb(EntidadFisica* e, Vector2D& min, Vector2D& max) {
        auto* pared = dynamic_cast<ParedRectangular*>(e);
        if (pared) {
            min = pared->get_min();
            max = pared->get_max();
            return true;
        }

        auto* tramp = dynamic_cast<Trampolin*>(e);
        if (tramp) {
            min = tramp->get_min();
            max = tramp->get_max();
            return true;
        }

        auto* seg = dynamic_cast<SeguidorBooster*>(e);
        if (seg) {
            min = seg->get_min();
            max = seg->get_max();
            return true;
        }

        auto* barril = dynamic_cast<BarrilChavo*>(e);
        if (barril) {
            min = barril->get_min();
            max = barril->get_max();
            return true;
        }

        auto* ventilador = dynamic_cast<Ventilador*>(e);
        if (ventilador) {
            min = ventilador->get_min();
            max = ventilador->get_max();
            return true;
        }

        auto* cubeta = dynamic_cast<Cubeta*>(e);
        if (cubeta) {
            min = cubeta->get_min();
            max = cubeta->get_max();
            return true;
        }

        return false;
    }

    void paso_fisico(double dt) {
        colisiones_frame.clear();
        eventos_especiales_frame.clear();

        // 1. Aplicar fuerzas globales
        aplicar_gravedad();

        // 1.5 Aplicar corrientes de aire de ventiladores
        FisicaVentilador::aplicar(entidades);

        // 1.6 Aplicar constraints de cuerdas colocadas como herramienta
        aplicar_tensiones_cuerda();
 
        // 2. Actualizar comportamiento del futbolista seguidor
        for (auto* e : entidades) {
            auto* seg = dynamic_cast<SeguidorBooster*>(e);
            if (seg) {
                seg->actualizar_comportamiento(entidades, dt);
            }
        }
 
        // 3. Integrar todas las entidades (cada una usa RK4)
        for (auto* e : entidades) {
            if (estado_actual == EstadoJuego::JUEGO_CREATIVO && !e->get_es_fijo()) {
                e->set_velocidad(Vector2D(0.0, 0.0));
                e->set_velocidad_angular(0.0);
                continue;
            }
            e->actualizar_fisica(dt);
        }
 
        // 4. Detectar y resolver colisiones
        detectar_y_resolver_colisiones();

        // 5. Recolectar eventos especiales pendientes de todas las entidades
        for (auto* e : entidades) {
            auto& evs = e->get_eventos_pendientes();
            if (!evs.empty()) {
                for (const auto& ev : evs) {
                    eventos_especiales_frame.push_back(ev);
                }
                e->limpiar_eventos_pendientes();
            }
        }
    }

    void aplicar_gravedad() {
        for (auto* e : entidades) {
            if (estado_actual == EstadoJuego::JUEGO_CREATIVO && !e->get_es_fijo()) continue;
            if (!e->get_es_estatico() && e->get_masa() > MathUtils::EPSILON) {
                if (dynamic_cast<Balancin*>(e)) continue; // El balancín está pivotado y fijo linealmente
                // F = m * g
                e->aplicar_fuerza(gravedad * e->get_masa());
            }
        }
    }

    void aplicar_tensiones_cuerda() {
        for (auto* e : entidades) {
            auto* cuerda = dynamic_cast<Cuerda*>(e);
            if (cuerda) {
                cuerda->aplicar_tension(entidades, gravedad);
            }
        }
    }

    // ========================================================================
    // Detección y resolución de colisiones (Broad+Narrow phase)
    // Por ahora: fuerza bruta O(n²). Cuando haya muchos objetos,
    // podemos optimizar con spatial hashing o quadtree.
    // ========================================================================
    void detectar_y_resolver_colisiones() {
        for (size_t i = 0; i < entidades.size(); i++) {
            for (size_t j = i + 1; j < entidades.size(); j++) {
                EntidadFisica* a = entidades[i];
                EntidadFisica* b = entidades[j];

                if (estado_actual == EstadoJuego::JUEGO_CREATIVO && (!a->get_es_fijo() || !b->get_es_fijo())) continue;

                // Skip si ambos son estáticos
                if (a->get_es_estatico() && b->get_es_estatico()) continue;

                InfoColision info = detectar_colision(a, b);
                if (info.hay_colision) {
                    Colisiones::resolver_colision(a, b, info);
                    
                    colisiones_frame.push_back({
                        a->get_id(), b->get_id(),
                        info.punto_contacto, info.normal, info.profundidad
                    });

                    a->on_collision(b, info);
                    Vector2D normal_inv = info.normal * -1.0;
                    b->on_collision(a, InfoColision{info.hay_colision, normal_inv, info.profundidad, info.punto_contacto});
                }
            }
        }
    }

    // Dispatch de colisiones según el tipo de forma de cada entidad
    InfoColision detectar_colision(EntidadFisica* a, EntidadFisica* b) {
        TipoForma fa = a->get_tipo_forma();
        TipoForma fb = b->get_tipo_forma();

        // --- Círculo vs Círculo ---
        if (fa == TipoForma::CIRCULO && fb == TipoForma::CIRCULO) {
            Vector2D pos_a, pos_b;
            double radio_a = 0.0, radio_b = 0.0;
            if (obtener_datos_circulo(a, pos_a, radio_a) &&
                obtener_datos_circulo(b, pos_b, radio_b)) {
                return Colisiones::circulo_vs_circulo(
                    pos_a, radio_a,
                    pos_b, radio_b);
            }
        }

        // --- Círculo vs AABB ---
        if (fa == TipoForma::CIRCULO && fb == TipoForma::AABB) {
            return colision_circulo_aabb(a, b);
        }
        if (fa == TipoForma::AABB && fb == TipoForma::CIRCULO) {
            // Invertir: la función espera (círculo, AABB)
            InfoColision info = colision_circulo_aabb(b, a);
            // Flipear la normal: ahora debe separar A(=AABB) de B(=círculo)
            info.normal = info.normal * (-1.0);
            return info;
        }

        // --- Círculo vs Polígono ---
        if (fa == TipoForma::CIRCULO && fb == TipoForma::POLIGONO) {
            return colision_circulo_poligono(a, b);
        }
        if (fa == TipoForma::POLIGONO && fb == TipoForma::CIRCULO) {
            InfoColision info = colision_circulo_poligono(b, a);
            info.normal = info.normal * (-1.0);
            return info;
        }

        // --- Polígono vs AABB ---
        if (fa == TipoForma::POLIGONO && fb == TipoForma::AABB) {
            return colision_poligono_aabb(a, b);
        }
        if (fa == TipoForma::AABB && fb == TipoForma::POLIGONO) {
            InfoColision info = colision_poligono_aabb(b, a);
            info.normal = info.normal * (-1.0);
            return info;
        }

        // --- Polígono vs Polígono ---
        if (fa == TipoForma::POLIGONO && fb == TipoForma::POLIGONO) {
            return colision_poligono_poligono(a, b);
        }

        // --- AABB vs AABB ---
        if (fa == TipoForma::AABB && fb == TipoForma::AABB) {
            return colision_aabb_aabb(a, b);
        }

        return InfoColision{};
    }

    // Helper: Círculo (circ_ent) vs AABB (aabb_ent)
    InfoColision colision_circulo_aabb(EntidadFisica* circ_ent, EntidadFisica* aabb_ent) {
        Vector2D pos_circ;
        double radio = 0.0;
        if (!obtener_datos_circulo(circ_ent, pos_circ, radio)) return InfoColision{};

        Vector2D min, max;
        if (obtener_datos_aabb(aabb_ent, min, max)) {
            return Colisiones::circulo_vs_aabb(
                pos_circ, radio,
                min, max);
        }

        return InfoColision{};
    }



    // Helper: Círculo (circ_ent) vs Polígono (poly_ent)
    InfoColision colision_circulo_poligono(EntidadFisica* circ_ent, EntidadFisica* poly_ent) {
        Vector2D pos_circ;
        double radio = 0.0;
        if (!obtener_datos_circulo(circ_ent, pos_circ, radio)) return InfoColision{};

        auto* rampa = dynamic_cast<PlanoInclinado*>(poly_ent);
        if (rampa) {
            return Colisiones::circulo_vs_poligono(
                pos_circ, radio,
                rampa->get_vertices());
        }

        auto* balancin = dynamic_cast<Balancin*>(poly_ent);
        if (balancin) {
            return Colisiones::circulo_vs_balancin(
                pos_circ, radio,
                balancin->get_posicion(), balancin->get_angulo(),
                balancin->get_largo(), balancin->get_espesor());
        }

        return InfoColision{};
    }

    // Helper: Polígono (poly_ent) vs AABB (aabb_ent)
    InfoColision colision_poligono_aabb(EntidadFisica* poly_ent, EntidadFisica* aabb_ent) {
        auto* balancin = dynamic_cast<Balancin*>(poly_ent);
        if (!balancin) return InfoColision{};

        Vector2D min, max;
        if (!obtener_datos_aabb(aabb_ent, min, max)) return InfoColision{};

        std::vector<Vector2D> vertices_aabb = {
            min,
            Vector2D(max.x, min.y),
            max,
            Vector2D(min.x, max.y)
        };

        return Colisiones::poligono_vs_poligono(balancin->get_vertices(), vertices_aabb);
    }

    // Helper: Polígono (poly_a) vs Polígono (poly_b)
    InfoColision colision_poligono_poligono(EntidadFisica* poly_a, EntidadFisica* poly_b) {
        std::vector<Vector2D> verts_a;
        auto* bal_a = dynamic_cast<Balancin*>(poly_a);
        if (bal_a) {
            verts_a = bal_a->get_vertices();
        } else {
            auto* rampa_a = dynamic_cast<PlanoInclinado*>(poly_a);
            if (rampa_a) verts_a = rampa_a->get_vertices();
        }

        std::vector<Vector2D> verts_b;
        auto* bal_b = dynamic_cast<Balancin*>(poly_b);
        if (bal_b) {
            verts_b = bal_b->get_vertices();
        } else {
            auto* rampa_b = dynamic_cast<PlanoInclinado*>(poly_b);
            if (rampa_b) verts_b = rampa_b->get_vertices();
        }

        if (verts_a.size() < 3 || verts_b.size() < 3) return InfoColision{};

        return Colisiones::poligono_vs_poligono(verts_a, verts_b);
    }

    // Helper: AABB (aabb_a) vs AABB (aabb_b)
    InfoColision colision_aabb_aabb(EntidadFisica* aabb_a, EntidadFisica* aabb_b) {
        Vector2D minA, maxA;
        if (!obtener_datos_aabb(aabb_a, minA, maxA)) return InfoColision{};

        Vector2D minB, maxB;
        if (!obtener_datos_aabb(aabb_b, minB, maxB)) return InfoColision{};

        std::vector<Vector2D> vertsA = {
            minA, Vector2D(maxA.x, minA.y), maxA, Vector2D(minA.x, maxA.y)
        };
        std::vector<Vector2D> vertsB = {
            minB, Vector2D(maxB.x, minB.y), maxB, Vector2D(minB.x, maxB.y)
        };

        return Colisiones::poligono_vs_poligono(vertsA, vertsB);
    }
};
