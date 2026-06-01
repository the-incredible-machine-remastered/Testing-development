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

    void set_siguiente_id(int id) {
        siguiente_id = std::max(1, id);
    }

    int get_siguiente_id() const { return siguiente_id; }

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

                // Skip si ambos son estáticos
                if (a->get_es_estatico() && b->get_es_estatico()) continue;

                InfoColision info = detectar_colision(a, b);
                if (info.hay_colision) {
                    Colisiones::resolver_colision(a, b, info);
                    aplicar_efecto_trampolin(a, b, info);
                    aplicar_efecto_barril(a, b, info);
                    aplicar_efecto_bola_rebotadora(a, b, info);
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

    // Helper para aplicar el impulso de lanzamiento de 75 grados del Barril Chavo
    void aplicar_efecto_barril(EntidadFisica* a, EntidadFisica* b, const InfoColision& info) {
        Bola* bola = dynamic_cast<Bola*>(a);
        BarrilChavo* barril = dynamic_cast<BarrilChavo*>(b);
        Vector2D normal_para_bola = info.normal; // Normal de separación para A

        if (!bola || !barril) {
            bola = dynamic_cast<Bola*>(b);
            barril = dynamic_cast<BarrilChavo*>(a);
            normal_para_bola = info.normal * -1.0;
        }

        if (bola && barril) {
            // Si la colisión es en la parte superior del barril (normal hacia arriba en pantalla Y-)
            // Y el centro de la bola está por encima (su Y está cerca o por encima de la parte superior del barril)
            if (normal_para_bola.y < -0.4 && bola->get_posicion().y < barril->get_posicion().y + 10.0) {
                // Solo disparamos si estaba esperando
                if (barril->get_estado() == EstadoBarril::ESPERANDO) {
                    barril->disparar_chavo();
                    
                    // Lanzar la bola a 75 grados hacia arriba
                    // En coordenadas de pantalla Y es hacia abajo, por lo que "arriba" es -Y
                    // Así que el ángulo de 75 grados hacia arriba-derecha es:
                    // cos(75°) * X_dir + sin(-75°) * Y_dir = (0.258819, -0.965926)
                    double angulo_rad = MathUtils::grados_a_radianes(-75.0);
                    Vector2D dir_impulso(std::cos(angulo_rad), std::sin(angulo_rad));
                    
                    double fuerza_lanzamiento = 750.0;
                    bola->set_velocidad(dir_impulso * fuerza_lanzamiento);

                    // Pequeño giro a la bola para dinamismo
                    bola->set_velocidad_angular(3.0);
                }
            }
        }
    }

    // Helper para activar la vibracion elastica de la BolaRebotadora al recibir impactos
    void aplicar_efecto_bola_rebotadora(EntidadFisica* a, EntidadFisica* b, const InfoColision& info) {
        Bola* bola = dynamic_cast<Bola*>(a);
        BolaRebotadora* rebotadora = dynamic_cast<BolaRebotadora*>(b);

        if (!bola || !rebotadora) {
            bola = dynamic_cast<Bola*>(b);
            rebotadora = dynamic_cast<BolaRebotadora*>(a);
        }

        if (bola && rebotadora) {
            Vector2D normal_bola = (bola == a) ? info.normal : info.normal * -1.0;
            double velocidad_normal = Vector2D::dot(bola->get_velocidad(), normal_bola);
            double velocidad_impacto = std::abs(velocidad_normal);
            rebotadora->registrar_impacto(velocidad_impacto);

            if (velocidad_normal > 0.0 && rebotadora->get_multiplicador_rebote() > 1.0) {
                Vector2D vel = bola->get_velocidad();
                double impulso_extra = velocidad_normal * (rebotadora->get_multiplicador_rebote() - 1.0);
                bola->set_velocidad(vel + normal_bola * impulso_extra);
            }
        }
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
