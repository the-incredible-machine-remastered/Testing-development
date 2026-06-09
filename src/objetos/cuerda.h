#pragma once
// ============================================================================
// Cuerda - herramienta/constraint de construccion que pasa por soportes.
// No colisiona ni se integra como cuerpo fisico suelto.
// ============================================================================

#include "../core/entidad_fisica.h"
#include "balancin.h"
#include "cubeta.h"
#include "soporte_torque.h"
#include <algorithm>
#include <vector>

enum class TipoAnclajeCuerda {
    Cubeta,
    BalancinIzquierdo,
    BalancinDerecho
};

struct AnclajeCuerda {
    int entidad_id;
    TipoAnclajeCuerda tipo;
};

class Cuerda : public EntidadFisica {
private:
    AnclajeCuerda extremo_a;
    AnclajeCuerda extremo_b;
    std::vector<int> soportes_id;
    double longitud_inicial;
    double ultima_tension;
    bool estabilizar_columpio;

    static EntidadFisica* buscar_entidad(const std::vector<EntidadFisica*>& entidades, int id) {
        for (auto* e : entidades) {
            if (e && e->get_id() == id) return e;
        }
        return nullptr;
    }

    static Vector2D posicion_anclaje(EntidadFisica* e, TipoAnclajeCuerda tipo) {
        if (!e) return Vector2D();

        if (tipo == TipoAnclajeCuerda::Cubeta) {
            auto* cubeta = dynamic_cast<Cubeta*>(e);
            return cubeta ? cubeta->get_punto_cuerda() : e->get_posicion();
        }

        auto* balancin = dynamic_cast<Balancin*>(e);
        if (!balancin) return e->get_posicion();
        return tipo == TipoAnclajeCuerda::BalancinIzquierdo
            ? balancin->get_punto_extremo_izquierdo()
            : balancin->get_punto_extremo_derecho();
    }

    static Vector2D velocidad_anclaje(EntidadFisica* e, TipoAnclajeCuerda tipo) {
        if (!e) return Vector2D();

        auto* balancin = dynamic_cast<Balancin*>(e);
        if (balancin && tipo != TipoAnclajeCuerda::Cubeta) {
            Vector2D r = posicion_anclaje(e, tipo) - balancin->get_posicion();
            double omega = balancin->get_velocidad_angular();
            return Vector2D(-omega * r.y, omega * r.x);
        }

        return e->get_velocidad();
    }

    static void aplicar_en_anclaje(EntidadFisica* e, TipoAnclajeCuerda tipo,
                                   const Vector2D& punto, const Vector2D& fuerza) {
        auto* balancin = dynamic_cast<Balancin*>(e);
        if (balancin && tipo != TipoAnclajeCuerda::Cubeta) {
            balancin->aplicar_fuerza_en_punto(punto, fuerza);
            return;
        }

        e->aplicar_fuerza(fuerza);
    }

    static double tension_por_peso(EntidadFisica* e, TipoAnclajeCuerda tipo,
                                   const Vector2D& dir_hacia_soporte,
                                   const Vector2D& gravedad) {
        if (!e || tipo != TipoAnclajeCuerda::Cubeta) return 0.0;

        Vector2D peso = gravedad * e->get_masa();
        return std::max(0.0, -Vector2D::dot(peso, dir_hacia_soporte));
    }

    static void amortiguar_columpio(EntidadFisica* e, TipoAnclajeCuerda tipo,
                                    const Vector2D& dir_hacia_soporte) {
        if (!e || tipo != TipoAnclajeCuerda::Cubeta) return;

        Vector2D vel = e->get_velocidad();
        Vector2D vel_radial = dir_hacia_soporte * Vector2D::dot(vel, dir_hacia_soporte);
        Vector2D vel_lateral = vel - vel_radial;
        double amortiguacion_lateral = 18.0;
        e->aplicar_fuerza(vel_lateral * (-amortiguacion_lateral * e->get_masa()));
    }

public:
    Cuerda(int id, AnclajeCuerda a, const std::vector<int>& soportes, AnclajeCuerda b,
           double longitud_total)
        : EntidadFisica(id, Vector2D(), 0.0, TipoForma::NINGUNA, true),
          extremo_a(a), extremo_b(b), soportes_id(soportes),
          longitud_inicial(longitud_total), ultima_tension(0.0),
          estabilizar_columpio(true) {}

    const AnclajeCuerda& get_extremo_a() const { return extremo_a; }
    const AnclajeCuerda& get_extremo_b() const { return extremo_b; }
    const std::vector<int>& get_soportes_id() const { return soportes_id; }
    double get_longitud_inicial() const { return longitud_inicial; }
    double get_ultima_tension() const { return ultima_tension; }
    bool get_estabilizar_columpio() const { return estabilizar_columpio; }
    void set_estabilizar_columpio(bool activo) { estabilizar_columpio = activo; }

    bool obtener_puntos(const std::vector<EntidadFisica*>& entidades,
                        std::vector<Vector2D>& puntos) const {
        EntidadFisica* ent_a = buscar_entidad(entidades, extremo_a.entidad_id);
        EntidadFisica* ent_b = buscar_entidad(entidades, extremo_b.entidad_id);
        if (!ent_a || !ent_b) return false;

        puntos.clear();
        puntos.push_back(posicion_anclaje(ent_a, extremo_a.tipo));
        for (int soporte_id : soportes_id) {
            auto* soporte = dynamic_cast<SoporteTorque*>(buscar_entidad(entidades, soporte_id));
            if (!soporte) return false;
            puntos.push_back(soporte->get_punto_cuerda());
        }
        puntos.push_back(posicion_anclaje(ent_b, extremo_b.tipo));
        return true;
    }

    void aplicar_tension(const std::vector<EntidadFisica*>& entidades, const Vector2D& gravedad) {
        EntidadFisica* ent_a = buscar_entidad(entidades, extremo_a.entidad_id);
        EntidadFisica* ent_b = buscar_entidad(entidades, extremo_b.entidad_id);
        std::vector<Vector2D> puntos;
        if (!ent_a || !ent_b || !obtener_puntos(entidades, puntos) || puntos.size() < 2) {
            ultima_tension = 0.0;
            return;
        }

        Vector2D pa = puntos.front();
        Vector2D pb = puntos.back();
        Vector2D va = velocidad_anclaje(ent_a, extremo_a.tipo);
        Vector2D vb = velocidad_anclaje(ent_b, extremo_b.tipo);

        Vector2D tramo_a = puntos[1] - pa;
        Vector2D tramo_b = puntos[puntos.size() - 2] - pb;
        double len_a = tramo_a.magnitud();
        double len_b = tramo_b.magnitud();
        double longitud_actual = 0.0;
        for (size_t i = 1; i < puntos.size(); ++i) {
            double tramo_len = Vector2D::distancia(puntos[i - 1], puntos[i]);
            if (tramo_len < MathUtils::EPSILON) {
                ultima_tension = 0.0;
                return;
            }
            longitud_actual += tramo_len;
        }
        double estiramiento = longitud_actual - longitud_inicial;

        if (len_a < MathUtils::EPSILON || len_b < MathUtils::EPSILON || estiramiento < -2.0) {
            ultima_tension = 0.0;
            return;
        }

        Vector2D dir_a = tramo_a / len_a;
        Vector2D dir_b = tramo_b / len_b;
        double velocidad_estira = -Vector2D::dot(va, dir_a) - Vector2D::dot(vb, dir_b);
        double k = 520.0;
        double c = 70.0;
        double tension_peso = std::max(
            tension_por_peso(ent_a, extremo_a.tipo, dir_a, gravedad),
            tension_por_peso(ent_b, extremo_b.tipo, dir_b, gravedad)
        );
        double tension_elastica = k * std::max(0.0, estiramiento)
                                + c * std::max(0.0, velocidad_estira);
        ultima_tension = std::max(0.0, tension_peso + tension_elastica);

        aplicar_en_anclaje(ent_a, extremo_a.tipo, pa, dir_a * ultima_tension);
        aplicar_en_anclaje(ent_b, extremo_b.tipo, pb, dir_b * ultima_tension);

        if (estabilizar_columpio) {
            amortiguar_columpio(ent_a, extremo_a.tipo, dir_a);
            amortiguar_columpio(ent_b, extremo_b.tipo, dir_b);
        }
    }
};

// TIM_MENU_SPAWN id=CUERDA etiqueta="Cuerda" tab=0 categoria=0
