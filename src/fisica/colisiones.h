#pragma once
// ============================================================================
// Colisiones — Detección y resolución de colisiones
//
// Convención de normales:
//   Todas las funciones de detección retornan un InfoColision cuya normal
//   apunta en la DIRECCIÓN DE SEPARACIÓN del primer argumento (A).
//   Es decir, la normal indica "hacia dónde empujar A para sacarlo de B".
//
// Pares implementados:
//   1. Círculo vs Círculo
//   2. Círculo vs AABB
//   3. Círculo vs Polígono convexo (triángulos, rampas)
// ============================================================================

#include "../core/vector2d.h"
#include "../core/math_utils.h"
#include "../core/entidad_fisica.h"
#include <vector>
#include <cmath>
#include <algorithm>

// Resultado de una prueba de colisión
struct InfoColision {
    bool hay_colision = false;
    Vector2D normal;            // Dirección de separación para A (ver convención arriba)
    double profundidad = 0.0;   // Penetración en pixeles
    Vector2D punto_contacto;    // Punto de contacto aproximado
};

namespace Colisiones {

    // ========================================================================
    // 1. Círculo vs Círculo
    // ========================================================================
    inline InfoColision circulo_vs_circulo(
        const Vector2D& pos1, double r1,
        const Vector2D& pos2, double r2)
    {
        InfoColision info;
        Vector2D diff = pos1 - pos2;  // de B hacia A
        double dist_sq = diff.magnitud_cuadrada();
        double sum_r = r1 + r2;

        if (dist_sq >= sum_r * sum_r) return info;

        double dist = std::sqrt(dist_sq);
        info.hay_colision = true;

        if (dist < MathUtils::EPSILON) {
            // Centros superpuestos: dirección arbitraria
            info.normal = Vector2D(0, -1);
            info.profundidad = sum_r;
        } else {
            info.normal = diff / dist;  // de B hacia A (separa A de B)
            info.profundidad = sum_r - dist;
        }

        // Punto de contacto: sobre la superficie de A, en dirección a B
        info.punto_contacto = pos1 - info.normal * r1;
        return info;
    }

    // ========================================================================
    // 2. Círculo vs AABB
    //    Primer argumento = círculo, segundo = AABB (min/max corners)
    // ========================================================================
    inline InfoColision circulo_vs_aabb(
        const Vector2D& pos_circ, double radio,
        const Vector2D& aabb_min, const Vector2D& aabb_max)
    {
        InfoColision info;

        // Encontrar el punto más cercano del AABB al centro del círculo
        double cx = MathUtils::clamp(pos_circ.x, aabb_min.x, aabb_max.x);
        double cy = MathUtils::clamp(pos_circ.y, aabb_min.y, aabb_max.y);
        Vector2D closest(cx, cy);

        Vector2D diff = pos_circ - closest;  // de AABB hacia círculo
        double dist_sq = diff.magnitud_cuadrada();

        if (dist_sq >= radio * radio) return info;

        info.hay_colision = true;
        double dist = std::sqrt(dist_sq);

        if (dist < MathUtils::EPSILON) {
            // Centro del círculo está DENTRO del AABB.
            // Buscamos la arista más cercana para la dirección de salida.
            double dx_left   = pos_circ.x - aabb_min.x;
            double dx_right  = aabb_max.x - pos_circ.x;
            double dy_top    = pos_circ.y - aabb_min.y;
            double dy_bottom = aabb_max.y - pos_circ.y;

            double min_d = dx_left;
            info.normal = Vector2D(-1, 0);

            if (dx_right < min_d)  { min_d = dx_right;  info.normal = Vector2D(1, 0); }
            if (dy_top < min_d)    { min_d = dy_top;     info.normal = Vector2D(0, -1); }
            if (dy_bottom < min_d) { min_d = dy_bottom;  info.normal = Vector2D(0, 1); }

            info.profundidad = radio + min_d;
        } else {
            // Caso normal: centro fuera del AABB
            info.normal = diff / dist;  // de AABB hacia círculo (separa círculo del AABB)
            info.profundidad = radio - dist;
        }

        info.punto_contacto = closest;
        return info;
    }

    // ========================================================================
    // Helper: Punto más cercano en un segmento AB al punto P
    // ========================================================================
    inline Vector2D punto_mas_cercano_en_segmento(
        const Vector2D& A, const Vector2D& B, const Vector2D& P)
    {
        Vector2D AB = B - A;
        double ab_sq = AB.magnitud_cuadrada();
        if (ab_sq < MathUtils::EPSILON) return A;  // Segmento degenerado

        double t = Vector2D::dot(P - A, AB) / ab_sq;
        t = MathUtils::clamp(t, 0.0, 1.0);
        return A + AB * t;
    }

    // ========================================================================
    // 3. Círculo vs Polígono Convexo
    //    Busca la arista más cercana del polígono al centro del círculo.
    //    Funciona para triángulos (rampas), rectángulos, y polígonos arbitrarios.
    // ========================================================================
    inline InfoColision circulo_vs_poligono(
        const Vector2D& pos_circ, double radio,
        const std::vector<Vector2D>& vertices)
    {
        InfoColision info;
        int n = static_cast<int>(vertices.size());
        if (n < 3) return info;

        double min_dist_sq = 1e18;
        Vector2D mejor_punto;
        Vector2D mejor_normal;

        for (int i = 0; i < n; i++) {
            int j = (i + 1) % n;
            Vector2D closest = punto_mas_cercano_en_segmento(
                vertices[i], vertices[j], pos_circ);

            Vector2D diff = pos_circ - closest;  // del polígono hacia el círculo
            double dist_sq = diff.magnitud_cuadrada();

            if (dist_sq < min_dist_sq) {
                min_dist_sq = dist_sq;
                mejor_punto = closest;

                if (diff.magnitud() > MathUtils::EPSILON) {
                    mejor_normal = diff.normalizar();
                } else {
                    // Caso degenerado: usar la normal de la arista
                    Vector2D edge = vertices[j] - vertices[i];
                    mejor_normal = edge.perpendicular().normalizar();
                }
            }
        }

        double dist = std::sqrt(min_dist_sq);
        if (dist >= radio) return info;

        info.hay_colision = true;
        info.profundidad = radio - dist;
        info.normal = mejor_normal;  // apunta del polígono hacia el círculo
        info.punto_contacto = mejor_punto;

        return info;
    }

    // ========================================================================
    // Resolución de Colisión — Método de impulsos con rotación
    //
    // 1. Corrección posicional (evita que los objetos se hundan)
    // 2. Impulso normal (rebote basado en coeficiente de restitución)
    // 3. Impulso tangencial (fricción + torque → rolling)
    //
    // La velocidad relativa se calcula en el PUNTO DE CONTACTO, incluyendo
    // la contribución de la velocidad angular: v_contact = v + ω × r
    // Esto permite que la fricción genere rotación y que las bolas
    // alcancen el estado de "rodar sin deslizar" naturalmente.
    // ========================================================================
    inline void resolver_colision(
        EntidadFisica* a, EntidadFisica* b, const InfoColision& info)
    {
        if (!info.hay_colision) return;

        double inv_masa_a = a->get_es_estatico() ? 0.0 : 1.0 / a->get_masa();
        double inv_masa_b = b->get_es_estatico() ? 0.0 : 1.0 / b->get_masa();
        double inv_masa_total = inv_masa_a + inv_masa_b;

        if (inv_masa_total < MathUtils::EPSILON) return; // Ambos estáticos

        // Vectores del centro al punto de contacto
        Vector2D r_a = info.punto_contacto - a->get_posicion();
        Vector2D r_b = info.punto_contacto - b->get_posicion();

        // Inversas de inercia (0 para estáticos)
        double I_a = a->get_inercia();
        double I_b = b->get_inercia();
        double inv_I_a = (!a->get_es_estatico() && I_a > MathUtils::EPSILON) ? 1.0 / I_a : 0.0;
        double inv_I_b = (!b->get_es_estatico() && I_b > MathUtils::EPSILON) ? 1.0 / I_b : 0.0;

        // ---- 1. Corrección Posicional ----
        const double correccion_pct = 0.8;
        const double slop = 0.5;
        double correccion = std::max(info.profundidad - slop, 0.0)
                            * correccion_pct / inv_masa_total;

        if (!a->get_es_estatico()) {
            a->set_posicion(a->get_posicion() + info.normal * (correccion * inv_masa_a));
        }
        if (!b->get_es_estatico()) {
            b->set_posicion(b->get_posicion() - info.normal * (correccion * inv_masa_b));
        }

        // ---- Helper: velocidad en el punto de contacto (incluye rotación) ----
        // v_contact = v_center + ω × r   (en 2D: ω × r = ω * (-r.y, r.x))
        auto vel_contacto = [](const EntidadFisica* ent, const Vector2D& r) -> Vector2D {
            double w = ent->get_velocidad_angular();
            return ent->get_velocidad() + Vector2D(-w * r.y, w * r.x);
        };

        // ---- 2. Impulso Normal ----
        Vector2D v_ca = a->get_es_estatico() ? Vector2D(0,0) : vel_contacto(a, r_a);
        Vector2D v_cb = b->get_es_estatico() ? Vector2D(0,0) : vel_contacto(b, r_b);
        Vector2D vel_rel = v_ca - v_cb;
        double vel_normal = Vector2D::dot(vel_rel, info.normal);

        // Si ya se están separando, no aplicar impulso
        if (vel_normal > 0.0) return;

        double e = std::min(a->get_restitucion(), b->get_restitucion());
        // Para círculos, r es paralelo a normal → cross(r, n) ≈ 0,
        // así que la masa efectiva normal ≈ inv_masa_total (sin rotación).
        double j = -(1.0 + e) * vel_normal / inv_masa_total;
        Vector2D impulso = info.normal * j;

        if (!a->get_es_estatico()) {
            a->set_velocidad(a->get_velocidad() + impulso * inv_masa_a);
        }
        if (!b->get_es_estatico()) {
            b->set_velocidad(b->get_velocidad() - impulso * inv_masa_b);
        }

        // ---- 3. Impulso Tangencial (Fricción + Rolling) ----
        // Recalcular velocidad relativa en el punto de contacto post-impulso normal
        v_ca = a->get_es_estatico() ? Vector2D(0,0) : vel_contacto(a, r_a);
        v_cb = b->get_es_estatico() ? Vector2D(0,0) : vel_contacto(b, r_b);
        vel_rel = v_ca - v_cb;

        Vector2D tangente = vel_rel - info.normal * Vector2D::dot(vel_rel, info.normal);
        double tang_mag = tangente.magnitud();

        if (tang_mag > MathUtils::EPSILON) {
            tangente = tangente / tang_mag;

            // Masa efectiva tangencial (incluye contribución rotacional)
            // 1/m_eff = 1/m_a + 1/m_b + (r_a × t)²/I_a + (r_b × t)²/I_b
            double rxt_a = Vector2D::cross(r_a, tangente);
            double rxt_b = Vector2D::cross(r_b, tangente);
            double inv_masa_tang = inv_masa_a + inv_masa_b
                                   + rxt_a * rxt_a * inv_I_a
                                   + rxt_b * rxt_b * inv_I_b;

            // Media geométrica de coeficientes de fricción
            double mu = std::sqrt(a->get_friccion() * b->get_friccion());
            double jt = -Vector2D::dot(vel_rel, tangente) / inv_masa_tang;

            // Ley de Coulomb: |fricción| ≤ μ × |impulso normal|
            if (std::abs(jt) > mu * j) {
                jt = MathUtils::signo(jt) * mu * j;
            }

            Vector2D impulso_friccion = tangente * jt;

            // Aplicar impulso lineal de fricción
            if (!a->get_es_estatico()) {
                a->set_velocidad(a->get_velocidad() + impulso_friccion * inv_masa_a);
            }
            if (!b->get_es_estatico()) {
                b->set_velocidad(b->get_velocidad() - impulso_friccion * inv_masa_b);
            }

            // Aplicar impulso angular de fricción (ESTO genera el rolling)
            // torque_impulso = r × J_friccion
            // Δω = torque_impulso / I
            if (inv_I_a > 0.0) {
                double delta_omega_a = Vector2D::cross(r_a, impulso_friccion) * inv_I_a;
                a->set_velocidad_angular(a->get_velocidad_angular() + delta_omega_a);
            }
            if (inv_I_b > 0.0) {
                double delta_omega_b = Vector2D::cross(r_b, impulso_friccion) * inv_I_b;
                b->set_velocidad_angular(b->get_velocidad_angular() - delta_omega_b);
            }
        }
    }

} // namespace Colisiones
