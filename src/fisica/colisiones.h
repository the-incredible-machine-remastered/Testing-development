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
#include "../objetos/balancin.h"
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

        // 1. Determinar si el centro del círculo está dentro del polígono
        // y encontrar la distancia de penetración a cada arista.
        bool centro_dentro = true;
        double max_dot = -1e18;
        int arista_cercana_idx = -1;
        std::vector<Vector2D> normales_out;
        normales_out.reserve(n);

        for (int i = 0; i < n; i++) {
            int j = (i + 1) % n;
            Vector2D edge = vertices[j] - vertices[i];
            Vector2D normal_out = edge.perpendicular().normalizar();
            normales_out.push_back(normal_out);

            Vector2D to_circ = pos_circ - vertices[i];
            double dot_val = Vector2D::dot(to_circ, normal_out);

            if (dot_val >= 0.0) {
                centro_dentro = false;
            }
            if (dot_val > max_dot) {
                max_dot = dot_val;
                arista_cercana_idx = i;
            }
        }

        if (centro_dentro) {
            // El centro del círculo está DENTRO del polígono!
            // Empujar hacia la arista más cercana
            info.hay_colision = true;
            info.normal = normales_out[arista_cercana_idx]; // apunta hacia afuera del polígono
            
            // La profundidad es el radio más la distancia de penetración (que es negativa de max_dot)
            info.profundidad = radio - max_dot; 

            // Punto de contacto: proyectar centro del círculo sobre la arista más cercana
            int i = arista_cercana_idx;
            int j = (i + 1) % n;
            info.punto_contacto = punto_mas_cercano_en_segmento(vertices[i], vertices[j], pos_circ);
            return info;
        }

        // 2. Caso normal: centro del círculo fuera del polígono
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
                    mejor_normal = normales_out[i];
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
    // 4. Círculo vs Balancín (Lever / Seesaw OBB)
    //    Transforma el círculo a coordenadas locales del balancín (AABB)
    //    para reutilizar circulo_vs_aabb.
    // ========================================================================
    inline InfoColision circulo_vs_balancin(
        const Vector2D& pos_circ, double radio,
        const Vector2D& pos_pivot, double angulo,
        double largo, double espesor)
    {
        // 1. Pasar posición del círculo a coordenadas locales (origen en el pivot)
        Vector2D local_pos = pos_circ - pos_pivot;
        double cos_ang = std::cos(-angulo);
        double sin_ang = std::sin(-angulo);
        Vector2D local_circ(
            local_pos.x * cos_ang - local_pos.y * sin_ang,
            local_pos.x * sin_ang + local_pos.y * cos_ang
        );

        // 2. Definir AABB local del balancín
        Vector2D aabb_min(-largo / 2.0, -espesor / 2.0);
        Vector2D aabb_max(largo / 2.0, espesor / 2.0);

        // 3. Ejecutar circulo_vs_aabb local
        InfoColision local_info = circulo_vs_aabb(local_circ, radio, aabb_min, aabb_max);

        if (!local_info.hay_colision) return local_info;

        // 4. Transformar los resultados de vuelta a coordenadas del mundo
        InfoColision world_info;
        world_info.hay_colision = true;
        world_info.profundidad = local_info.profundidad;

        // Rotar normal de vuelta al mundo (por +angulo)
        double cos_w = std::cos(angulo);
        double sin_w = std::sin(angulo);
        world_info.normal = Vector2D(
            local_info.normal.x * cos_w - local_info.normal.y * sin_w,
            local_info.normal.x * sin_w + local_info.normal.y * cos_w
        );

        // Rotar y trasladar punto de contacto de vuelta al mundo
        world_info.punto_contacto = Vector2D(
            local_info.punto_contacto.x * cos_w - local_info.punto_contacto.y * sin_w,
            local_info.punto_contacto.x * sin_w + local_info.punto_contacto.y * cos_w
        ) + pos_pivot;

        return world_info;
    }

    // ========================================================================
    // 5. Polígono Convexo vs Polígono Convexo (SAT - Separating Axis Theorem)
    //    Calcula colisión precisa entre dos formas poligonales convexas
    // ========================================================================
    inline InfoColision poligono_vs_poligono(
        const std::vector<Vector2D>& verticesA,
        const std::vector<Vector2D>& verticesB)
    {
        InfoColision info;
        if (verticesA.size() < 3 || verticesB.size() < 3) return info;

        double overlap_min = 1e18;
        Vector2D normal_min;

        // Ejes candidatos: normales a las aristas de ambos polígonos
        std::vector<Vector2D> ejes;

        // Ejes de A
        for (size_t i = 0; i < verticesA.size(); i++) {
            size_t j = (i + 1) % verticesA.size();
            Vector2D arista = verticesA[j] - verticesA[i];
            ejes.push_back(arista.perpendicular().normalizar());
        }

        // Ejes de B
        for (size_t i = 0; i < verticesB.size(); i++) {
            size_t j = (i + 1) % verticesB.size();
            Vector2D arista = verticesB[j] - verticesB[i];
            ejes.push_back(arista.perpendicular().normalizar());
        }

        // Proyectar ambos polígonos en cada eje
        for (const auto& eje : ejes) {
            // Proyectar A
            double minA = 1e18, maxA = -1e18;
            for (const auto& v : verticesA) {
                double proj = Vector2D::dot(v, eje);
                minA = std::min(minA, proj);
                maxA = std::max(maxA, proj);
            }

            // Proyectar B
            double minB = 1e18, maxB = -1e18;
            for (const auto& v : verticesB) {
                double proj = Vector2D::dot(v, eje);
                minB = std::min(minB, proj);
                maxB = std::max(maxB, proj);
            }

            // Si hay alguna separación en cualquier eje, no hay colisión
            if (maxA <= minB || maxB <= minA) {
                return info;
            }

            // Calcular solapamiento
            double overlap = std::min(maxA, maxB) - std::max(minA, minB);
            if (overlap < overlap_min) {
                overlap_min = overlap;
                normal_min = eje;
            }
        }

        info.hay_colision = true;
        info.profundidad = overlap_min;

        // Centroides geométricos para orientar la normal
        Vector2D centroA(0.0, 0.0);
        for (const auto& v : verticesA) centroA += v;
        centroA = centroA / static_cast<double>(verticesA.size());

        Vector2D centroB(0.0, 0.0);
        for (const auto& v : verticesB) centroB += v;
        centroB = centroB / static_cast<double>(verticesB.size());

        // Asegurar que la normal apunte de B hacia A (dirección de separación de A)
        Vector2D dir_B_hacia_A = centroA - centroB;
        if (Vector2D::dot(normal_min, dir_B_hacia_A) < 0.0) {
            normal_min = -normal_min;
        }
        info.normal = normal_min;

        // Encontrar punto de contacto comprobando qué vértices penetran
        auto punto_dentro_poligono = [](const Vector2D& p, const std::vector<Vector2D>& vertices) -> bool {
            if (vertices.size() < 3) return false;
            int n = static_cast<int>(vertices.size());
            bool positivo = false;
            bool negativo = false;
            for (int i = 0; i < n; i++) {
                int j = (i + 1) % n;
                Vector2D arista = vertices[j] - vertices[i];
                Vector2D a_punto = p - vertices[i];
                double cross = Vector2D::cross(arista, a_punto);
                if (cross > MathUtils::EPSILON) positivo = true;
                if (cross < -MathUtils::EPSILON) negativo = true;
                if (positivo && negativo) return false;
            }
            return true;
        };

        Vector2D contacto(0.0, 0.0);
        int contador = 0;

        for (const auto& v : verticesA) {
            if (punto_dentro_poligono(v, verticesB)) {
                contacto += v;
                contador++;
            }
        }

        for (const auto& v : verticesB) {
            if (punto_dentro_poligono(v, verticesA)) {
                contacto += v;
                contador++;
            }
        }

        if (contador > 0) {
            info.punto_contacto = contacto / static_cast<double>(contador);
        } else {
            info.punto_contacto = (centroA + centroB) * 0.5;
        }

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

        // Si es un balancín, su masa lineal es conceptualmente infinita (inv_masa = 0.0)
        // ya que está fijo en su pivote. Conserva sin embargo su inercia rotacional.
        double inv_masa_a = (a->get_es_estatico() || dynamic_cast<Balancin*>(a)) ? 0.0 : 1.0 / a->get_masa();
        double inv_masa_b = (b->get_es_estatico() || dynamic_cast<Balancin*>(b)) ? 0.0 : 1.0 / b->get_masa();
        double inv_masa_total = inv_masa_a + inv_masa_b;

        if (inv_masa_total < MathUtils::EPSILON && !dynamic_cast<Balancin*>(a) && !dynamic_cast<Balancin*>(b)) {
            return; // Ambos completamente estáticos
        }

        // Vectores del centro al punto de contacto
        Vector2D r_a = info.punto_contacto - a->get_posicion();
        Vector2D r_b = info.punto_contacto - b->get_posicion();

        // Inversas de inercia (0 para estáticos, y bloqueado para hubs de transmisión)
        double I_a = a->get_inercia();
        double I_b = b->get_inercia();
        bool a_bloqueado = (a->get_tipo_entidad() == TipoEntidadJuego::RUEDA_HAMSTER ||
                            a->get_tipo_entidad() == TipoEntidadJuego::POLEA ||
                            a->get_tipo_entidad() == TipoEntidadJuego::CINTA_TRANSPORTADORA ||
                            a->get_tipo_entidad() == TipoEntidadJuego::GENERADOR_MOTOR);
        bool b_bloqueado = (b->get_tipo_entidad() == TipoEntidadJuego::RUEDA_HAMSTER ||
                            b->get_tipo_entidad() == TipoEntidadJuego::POLEA ||
                            b->get_tipo_entidad() == TipoEntidadJuego::CINTA_TRANSPORTADORA ||
                            b->get_tipo_entidad() == TipoEntidadJuego::GENERADOR_MOTOR);
        double inv_I_a = (!a->get_es_estatico() && !a_bloqueado && I_a > MathUtils::EPSILON) ? 1.0 / I_a : 0.0;
        double inv_I_b = (!b->get_es_estatico() && !b_bloqueado && I_b > MathUtils::EPSILON) ? 1.0 / I_b : 0.0;

        // ---- 1. Corrección Posicional ----
        const double correccion_pct = 0.8;
        const double slop = 0.5;
        double inv_m_total_corr = inv_masa_total;
        if (inv_m_total_corr < MathUtils::EPSILON) inv_m_total_corr = MathUtils::EPSILON;
        
        double correccion = std::max(info.profundidad - slop, 0.0)
                            * correccion_pct / inv_m_total_corr;
        double correccion_pura = std::max(info.profundidad - slop, 0.0) * correccion_pct;

        if (!a->get_es_estatico()) {
            if (dynamic_cast<Balancin*>(a)) {
                double r_cross_n = Vector2D::cross(r_a, info.normal);
                if (std::abs(r_cross_n) > MathUtils::EPSILON) {
                    double delta_theta = correccion_pura / r_cross_n;
                    delta_theta = MathUtils::clamp(delta_theta, -0.08, 0.08);
                    a->set_angulo(a->get_angulo() + delta_theta);
                }
            } else {
                a->set_posicion(a->get_posicion() + info.normal * (correccion * inv_masa_a));
            }
        }
        if (!b->get_es_estatico()) {
            if (dynamic_cast<Balancin*>(b)) {
                double r_cross_n = Vector2D::cross(r_b, info.normal);
                if (std::abs(r_cross_n) > MathUtils::EPSILON) {
                    double delta_theta = -correccion_pura / r_cross_n;
                    delta_theta = MathUtils::clamp(delta_theta, -0.08, 0.08);
                    b->set_angulo(b->get_angulo() + delta_theta);
                }
            } else {
                b->set_posicion(b->get_posicion() - info.normal * (correccion * inv_masa_b));
            }
        }

        // ---- 2. Impulso Normal ----
        Vector2D v_ca = a->get_es_estatico() ? Vector2D(0,0) : a->get_velocidad_en_punto(info.punto_contacto);
        Vector2D v_cb = b->get_es_estatico() ? Vector2D(0,0) : b->get_velocidad_en_punto(info.punto_contacto);
        Vector2D vel_rel = v_ca - v_cb;
        double vel_normal = Vector2D::dot(vel_rel, info.normal);

        // Si ya se están separando, no aplicar impulso
        if (vel_normal > 0.0) return;

        double e = std::min(a->get_restitucion(), b->get_restitucion());
        
        // Masa efectiva normal (incluye contribución rotacional de inercia)
        // 1/m_eff = 1/m_a + 1/m_b + (r_a × n)²/I_a + (r_b × n)²/I_b
        double rn_a = Vector2D::cross(r_a, info.normal);
        double rn_b = Vector2D::cross(r_b, info.normal);
        double inv_masa_normal = inv_masa_total
                               + rn_a * rn_a * inv_I_a
                               + rn_b * rn_b * inv_I_b;

        if (inv_masa_normal < MathUtils::EPSILON) inv_masa_normal = MathUtils::EPSILON;

        double j = -(1.0 + e) * vel_normal / inv_masa_normal;
        Vector2D impulso = info.normal * j;

        if (!a->get_es_estatico()) {
            a->set_velocidad(a->get_velocidad() + impulso * inv_masa_a);
            if (inv_I_a > 0.0) {
                double delta_omega_a = Vector2D::cross(r_a, impulso) * inv_I_a;
                a->set_velocidad_angular(a->get_velocidad_angular() + delta_omega_a);
            }
        }
        if (!b->get_es_estatico()) {
            b->set_velocidad(b->get_velocidad() - impulso * inv_masa_b);
            if (inv_I_b > 0.0) {
                double delta_omega_b = Vector2D::cross(r_b, impulso) * inv_I_b;
                b->set_velocidad_angular(b->get_velocidad_angular() - delta_omega_b);
            }
        }

        // ---- 3. Impulso Tangencial (Fricción + Rolling) ----
        // Recalcular velocidad relativa en el punto de contacto post-impulso normal
        v_ca = a->get_es_estatico() ? Vector2D(0,0) : a->get_velocidad_en_punto(info.punto_contacto);
        v_cb = b->get_es_estatico() ? Vector2D(0,0) : b->get_velocidad_en_punto(info.punto_contacto);
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
