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
#include "../objetos/pared_rectangular.h"
#include "../objetos/plano_inclinado.h"
#include "../objetos/bola.h"
#include "../objetos/trampolin.h"
#include "../objetos/balancin.h"
#include "../objetos/seguidor_booster.h"
#include "colisiones.h"
#include <vector>
#include <algorithm>

class MotorFisica {
private:
    std::vector<EntidadFisica*> entidades;
    Vector2D gravedad;
    double dt_fijo;              // Timestep fijo (ej. 1/120 s)
    double acumulador_tiempo;    // Acumula tiempo real para pasos fijos
    bool pausado;
    int siguiente_id;

public:
    MotorFisica(double dt = 1.0 / 120.0, Vector2D grav = Vector2D(0, 500.0))
        : gravedad(grav), dt_fijo(dt), acumulador_tiempo(0.0),
          pausado(false), siguiente_id(1) {}

    ~MotorFisica() {
        limpiar();
    }

    // --- Gestión de entidades ---

    int generar_id() { return siguiente_id++; }

    void agregar_entidad(EntidadFisica* e) {
        entidades.push_back(e);
    }

    void remover_entidad(int id) {
        entidades.erase(
            std::remove_if(entidades.begin(), entidades.end(),
                [id](EntidadFisica* e) {
                    if (e->get_id() == id) { delete e; return true; }
                    return false;
                }),
            entidades.end()
        );
    }

    void limpiar() {
        for (auto* e : entidades) delete e;
        entidades.clear();
        siguiente_id = 1;
    }

    // --- Control de simulación ---

    void set_pausado(bool p) { pausado = p; }
    bool get_pausado() const { return pausado; }
    void set_gravedad(const Vector2D& g) { gravedad = g; }
    Vector2D get_gravedad() const { return gravedad; }
    const std::vector<EntidadFisica*>& get_entidades() const { return entidades; }

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
    // Un paso completo de simulación
    void paso_fisico(double dt) {
        // 1. Aplicar fuerzas globales
        aplicar_gravedad();
 
        // 2. Actualizar comportamiento del futbolista seguidor
        for (auto* e : entidades) {
            auto* seg = dynamic_cast<SeguidorBooster*>(e);
            if (seg) {
                seg->actualizar_comportamiento(entidades, dt);
            }
        }
 
        // 3. Integrar todas las entidades (cada una usa RK4)
        for (auto* e : entidades) {
            e->actualizar_fisica(dt);
        }
 
        // 4. Detectar y resolver colisiones
        detectar_y_resolver_colisiones();
    }

    void aplicar_gravedad() {
        for (auto* e : entidades) {
            if (!e->get_es_estatico() && e->get_masa() > MathUtils::EPSILON) {
                if (dynamic_cast<Balancin*>(e)) continue; // El balancín está pivotado y fijo linealmente
                // F = m * g
                e->aplicar_fuerza(gravedad * e->get_masa());
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

                // Skip si ambos son estáticos
                if (a->get_es_estatico() && b->get_es_estatico()) continue;

                InfoColision info = detectar_colision(a, b);
                if (info.hay_colision) {
                    Colisiones::resolver_colision(a, b, info);
                    aplicar_efecto_trampolin(a, b, info);
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
            auto* ba = dynamic_cast<Bola*>(a);
            auto* bb = dynamic_cast<Bola*>(b);
            if (ba && bb) {
                return Colisiones::circulo_vs_circulo(
                    ba->get_posicion(), ba->get_radio(),
                    bb->get_posicion(), bb->get_radio());
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
        auto* bola = dynamic_cast<Bola*>(circ_ent);
        if (!bola) return InfoColision{};

        auto* pared = dynamic_cast<ParedRectangular*>(aabb_ent);
        if (pared) {
            return Colisiones::circulo_vs_aabb(
                bola->get_posicion(), bola->get_radio(),
                pared->get_min(), pared->get_max());
        }

        auto* tramp = dynamic_cast<Trampolin*>(aabb_ent);
        if (tramp) {
            return Colisiones::circulo_vs_aabb(
                bola->get_posicion(), bola->get_radio(),
                tramp->get_min(), tramp->get_max());
        }

        auto* seg = dynamic_cast<SeguidorBooster*>(aabb_ent);
        if (seg) {
            return Colisiones::circulo_vs_aabb(
                bola->get_posicion(), bola->get_radio(),
                seg->get_min(), seg->get_max());
        }

        return InfoColision{};
    }

    // Helper para aplicar el rebote elástico hacia arriba del Trampolín
    void aplicar_efecto_trampolin(EntidadFisica* a, EntidadFisica* b, const InfoColision& info) {
        Bola* bola = dynamic_cast<Bola*>(a);
        Trampolin* tramp = dynamic_cast<Trampolin*>(b);
        Vector2D normal_para_bola = info.normal; // Normal de separación para A

        if (!bola || !tramp) {
            bola = dynamic_cast<Bola*>(b);
            tramp = dynamic_cast<Trampolin*>(a);
            normal_para_bola = info.normal * -1.0;
        }

        if (bola && tramp) {
            // Si la colisión es en la lona del trampolín (la parte superior, normal hacia arriba en pantalla Y-)
            if (normal_para_bola.y < -0.4) {
                // Obtener velocidad vertical previa para estimar la fuerza del impacto
                double velocidad_impacto = std::abs(bola->get_velocidad().y);

                // Forzar velocidad vertical hacia arriba
                Vector2D vel = bola->get_velocidad();
                vel.y = -tramp->get_fuerza_rebote();
                bola->set_velocidad(vel);

                // Deformar la lona proporcionalmente a la velocidad del impacto
                double nueva_def = std::max(10.0, velocidad_impacto * 0.04);
                tramp->set_deformacion(std::min(24.0, nueva_def));

                // Aplicar un giro (torque) basado en qué tan lejos del centro cayó para un efecto más interactivo
                double centro_tramp = tramp->get_posicion().x + tramp->get_ancho() / 2.0;
                double offset = bola->get_posicion().x - centro_tramp;
                
                // Agregamos rotación proporcional al descentrado
                double kick_rotacional = offset * 0.4;
                bola->set_velocidad_angular(bola->get_velocidad_angular() + kick_rotacional);
            }
        }
    }

    // Helper: Círculo (circ_ent) vs Polígono (poly_ent)
    InfoColision colision_circulo_poligono(EntidadFisica* circ_ent, EntidadFisica* poly_ent) {
        auto* bola = dynamic_cast<Bola*>(circ_ent);
        if (!bola) return InfoColision{};

        auto* rampa = dynamic_cast<PlanoInclinado*>(poly_ent);
        if (rampa) {
            return Colisiones::circulo_vs_poligono(
                bola->get_posicion(), bola->get_radio(),
                rampa->get_vertices());
        }

        auto* balancin = dynamic_cast<Balancin*>(poly_ent);
        if (balancin) {
            return Colisiones::circulo_vs_balancin(
                bola->get_posicion(), bola->get_radio(),
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
        auto* pared = dynamic_cast<ParedRectangular*>(aabb_ent);
        if (pared) {
            min = pared->get_min();
            max = pared->get_max();
        } else {
            auto* tramp = dynamic_cast<Trampolin*>(aabb_ent);
            if (tramp) {
                min = tramp->get_min();
                max = tramp->get_max();
            } else {
                auto* seg = dynamic_cast<SeguidorBooster*>(aabb_ent);
                if (seg) {
                    min = seg->get_min();
                    max = seg->get_max();
                } else {
                    return InfoColision{};
                }
            }
        }

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
        auto* paredA = dynamic_cast<ParedRectangular*>(aabb_a);
        if (paredA) {
            minA = paredA->get_min();
            maxA = paredA->get_max();
        } else {
            auto* trampA = dynamic_cast<Trampolin*>(aabb_a);
            if (trampA) {
                minA = trampA->get_min();
                maxA = trampA->get_max();
            } else {
                auto* segA = dynamic_cast<SeguidorBooster*>(aabb_a);
                if (segA) {
                    minA = segA->get_min();
                    maxA = segA->get_max();
                } else {
                    return InfoColision{};
                }
            }
        }

        Vector2D minB, maxB;
        auto* paredB = dynamic_cast<ParedRectangular*>(aabb_b);
        if (paredB) {
            minB = paredB->get_min();
            maxB = paredB->get_max();
        } else {
            auto* trampB = dynamic_cast<Trampolin*>(aabb_b);
            if (trampB) {
                minB = trampB->get_min();
                maxB = trampB->get_max();
            } else {
                auto* segB = dynamic_cast<SeguidorBooster*>(aabb_b);
                if (segB) {
                    minB = segB->get_min();
                    maxB = segB->get_max();
                } else {
                    return InfoColision{};
                }
            }
        }

        std::vector<Vector2D> vertsA = {
            minA, Vector2D(maxA.x, minA.y), maxA, Vector2D(minA.x, maxA.y)
        };
        std::vector<Vector2D> vertsB = {
            minB, Vector2D(maxB.x, minB.y), maxB, Vector2D(minB.x, maxB.y)
        };

        return Colisiones::poligono_vs_poligono(vertsA, vertsB);
    }
};
