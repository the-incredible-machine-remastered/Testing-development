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

        // 2. Integrar todas las entidades (cada una usa RK4)
        for (auto* e : entidades) {
            e->actualizar_fisica(dt);
        }

        // 3. Detectar y resolver colisiones
        detectar_y_resolver_colisiones();
    }

    void aplicar_gravedad() {
        for (auto* e : entidades) {
            if (!e->get_es_estatico() && e->get_masa() > MathUtils::EPSILON) {
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

        return InfoColision{};
    }

    // Helper: Círculo (circ_ent) vs AABB (aabb_ent)
    InfoColision colision_circulo_aabb(EntidadFisica* circ_ent, EntidadFisica* aabb_ent) {
        auto* bola = dynamic_cast<Bola*>(circ_ent);
        auto* pared = dynamic_cast<ParedRectangular*>(aabb_ent);
        if (bola && pared) {
            return Colisiones::circulo_vs_aabb(
                bola->get_posicion(), bola->get_radio(),
                pared->get_min(), pared->get_max());
        }
        return InfoColision{};
    }

    // Helper: Círculo (circ_ent) vs Polígono (poly_ent)
    InfoColision colision_circulo_poligono(EntidadFisica* circ_ent, EntidadFisica* poly_ent) {
        auto* bola = dynamic_cast<Bola*>(circ_ent);
        auto* rampa = dynamic_cast<PlanoInclinado*>(poly_ent);
        if (bola && rampa) {
            return Colisiones::circulo_vs_poligono(
                bola->get_posicion(), bola->get_radio(),
                rampa->get_vertices());
        }
        return InfoColision{};
    }
};
