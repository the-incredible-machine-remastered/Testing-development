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
#include "../objetos/globo.h"
#include "../objetos/tijera.h"
#include "../objetos/gancho.h"
#include "../objetos/pistola.h"
#include "../objetos/soporte_torque.h"
#include "../objetos/caja_hamster.h"
#include "../objetos/banda.h"
#include "../objetos/caja_sorpresa.h"
#include "../objetos/caminadora.h"
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
    double tiempo_simulacion;    // Tiempo total desde que arrancó la simulación
    bool pausado;
    int pasos_desde_reanudar;    // Cuenta pasos tras quitar pausa para ignorar delta inicial
    bool resetear_tensiones_cuerdas;
    int siguiente_id;
    std::vector<RegistroColision> colisiones_frame;
    std::vector<RegistroEventoEspecial> eventos_especiales_frame;

public:
    MotorFisica(double dt = 1.0 / 120.0, Vector2D grav = Vector2D(0, 500.0))
        : gravedad(grav), dt_fijo(dt), acumulador_tiempo(0.0),
          tiempo_simulacion(0.0), pausado(true),
          pasos_desde_reanudar(0), resetear_tensiones_cuerdas(false),
          siguiente_id(1) {}

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
        tiempo_simulacion = 0.0;
        acumulador_tiempo = 0.0;
        pasos_desde_reanudar = 0;
        resetear_tensiones_cuerdas = false;
    }

    // --- Control de simulación ---

    void set_pausado(bool p) {
        if (pausado && !p) {
            // Al reanudar (Play): en edición el usuario pudo mover objetos, así que
            // recalibramos la longitud de reposo de cada cuerda a su geometría actual
            // (para no arrancar pre-tensada). La línea base de tensión se fija con el
            // sistema resetear_tensiones_cuerdas en los primeros pasos.
            for (auto* e : entidades) {
                auto* c = dynamic_cast<Cuerda*>(e);
                if (c) c->sincronizar_reposo(entidades);
            }
            resetear_tensiones_cuerdas = true; // resetear tras los primeros pasos
        }
        pausado = p;
    }
    bool get_pausado() const { return pausado; }
    void set_gravedad(const Vector2D& g) { gravedad = g; }
    Vector2D get_gravedad() const { return gravedad; }
    const std::vector<EntidadFisica*>& get_entidades() const { return entidades; }
    const std::vector<RegistroColision>& get_colisiones_frame() const { return colisiones_frame; }
    const std::vector<RegistroEventoEspecial>& get_eventos_especiales_frame() const { return eventos_especiales_frame; }

    void dibujar_bandas(bool mostrar_debug) const {
        for (auto* e : entidades) {
            auto* banda = dynamic_cast<Banda*>(e);
            if (banda) banda->dibujar_con_entidades(entidades, mostrar_debug);
        }
    }

    // Forzar una pasada de transmisión de bandas (útil antes del primer frame)
    void aplicar_transmision_bandas() {
        for (auto* e : entidades) {
            auto* banda = dynamic_cast<Banda*>(e);
            if (banda) banda->aplicar_transmision(entidades);
        }
    }

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
            tiempo_simulacion += dt_fijo;
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

        auto* globo = dynamic_cast<Globo*>(e);
        if (globo) {
            pos = globo->get_posicion();
            radio = globo->get_radio();
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

        auto* tijera = dynamic_cast<Tijera*>(e);
        if (tijera) {
            min = tijera->get_min();
            max = tijera->get_max();
            return true;
        }

        auto* hamster = dynamic_cast<CajaHamster*>(e);
        if (hamster) {
            min = hamster->get_min();
            max = hamster->get_max();
            return true;
        }

        auto* caja_s = dynamic_cast<CajaSorpresa*>(e);
        if (caja_s) {
            min = caja_s->get_min();
            max = caja_s->get_max();
            return true;
        }

        auto* conv = dynamic_cast<Caminadora*>(e);
        if (conv) {
            min = conv->get_min();
            max = conv->get_max();
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

        // 1.55 Aplicar conveyors activos y animar
        for (auto* e : entidades) {
            auto* conv = dynamic_cast<Caminadora*>(e);
            if (conv) {
                conv->Caminadora::actualizar_fisica(dt);
                conv->aplicar_conveyor(entidades);
            }
        }

        // 1.6 Transmitir energía de bandas (hámster → ventilador)
        for (auto* e : entidades) {
            auto* banda = dynamic_cast<Banda*>(e);
            if (banda) banda->aplicar_transmision(entidades);
        }

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
        // paso_fisico() solo corre cuando la simulación NO está pausada, así que aquí
        // siempre estamos "corriendo" — la física es idéntica en CREATIVO y NIVEL. La
        // edición (objetos quietos) ocurre estando pausado, cuando esto ni se ejecuta.
        for (auto* e : entidades) {
            e->actualizar_fisica(dt);
        }
 
        // 4. Detectar y resolver colisiones
        detectar_y_resolver_colisiones();

        // 4.5 Tijeras activadas cortan cuerdas que pasan por su zona
        cortar_cuerdas_con_tijeras();

        // 4.6 Pistolas activadas disparan
        disparar_pistolas();
        lanzar_cajas_sorpresa();

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
            if (!e->get_es_estatico() && e->get_masa() > MathUtils::EPSILON) {
                if (dynamic_cast<Balancin*>(e)) continue; // El balancín está pivotado y fijo linealmente
                // F = m * g
                e->aplicar_fuerza(gravedad * e->get_masa());
            }
        }
    }

    void aplicar_tensiones_cuerda() {
        // Resetear tension_anterior tras los primeros 3 pasos al reanudar
        if (resetear_tensiones_cuerdas) {
            pasos_desde_reanudar++;
            if (pasos_desde_reanudar >= 3) {
                for (auto* e : entidades) {
                    auto* c = dynamic_cast<Cuerda*>(e);
                    if (c) c->resetear_tension_anterior();
                }
                resetear_tensiones_cuerdas = false;
                pasos_desde_reanudar = 0;
            }
        }

        // Fuente de verdad única para "el balancín recibió un golpe brusco este frame".
        // Se consume el flag de CADA balancín una sola vez, antes de recorrer cuerdas,
        // para que el resultado no dependa del orden ni se pierda con varias cuerdas.
        std::vector<Balancin*> balancines_golpeados;
        for (auto* e : entidades) {
            auto* bal = dynamic_cast<Balancin*>(e);
            if (bal && bal->consumir_impacto_brusco())
                balancines_golpeados.push_back(bal);
        }
        // Un balancín golpeado activa cualquier activable parado sobre su tabla.
        for (auto* bal : balancines_golpeados) {
            for (auto* e3 : entidades) {
                if (e3->es_activable_por_tension() && bal->contiene_punto(e3->get_posicion()))
                    e3->activar_por_tension();
            }
        }

        for (auto* e : entidades) {
            auto* cuerda = dynamic_cast<Cuerda*>(e);
            if (!cuerda) continue;
            cuerda->aplicar_tension(entidades, gravedad);

            if (resetear_tensiones_cuerdas) continue; // aún estabilizando

            int id_a = cuerda->get_extremo_a().entidad_id;
            int id_b = cuerda->get_extremo_b().entidad_id;
            EntidadFisica* ent_a = nullptr;
            EntidadFisica* ent_b = nullptr;
            for (auto* e2 : entidades) {
                if (e2->get_id() == id_a) ent_a = e2;
                if (e2->get_id() == id_b) ent_b = e2;
            }

            auto fue_golpeado = [&](EntidadFisica* e2) {
                for (auto* bal : balancines_golpeados)
                    if (static_cast<EntidadFisica*>(bal) == e2) return true;
                return false;
            };

            // Un extremo activable (pistola/foco/lupa) se activa si:
            //  - la tensión de la cuerda tuvo un pico brusco (delta >= 60), o
            //  - el OTRO extremo es un balancín que recibió un golpe brusco
            //    (fuente de verdad robusta, ignora la geometría frágil de la tensión).
            double delta = cuerda->get_delta_tension();
            auto activar_extremo = [&](EntidadFisica* e2, EntidadFisica* otro) {
                if (!e2 || !e2->es_activable_por_tension()) return;
                if (delta >= 60.0 || fue_golpeado(otro))
                    e2->activar_por_tension();
            };
            activar_extremo(ent_a, ent_b);
            activar_extremo(ent_b, ent_a);
        }
    }

    // Comprueba si el segmento P1-P2 intersecta el rectángulo [min, max]
    static bool segmento_cruza_aabb(const Vector2D& p1, const Vector2D& p2,
                                    const Vector2D& min, const Vector2D& max) {
        // Primero: si algún extremo está dentro del AABB, ya intersecta
        auto dentro = [&](const Vector2D& p) {
            return p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y;
        };
        if (dentro(p1) || dentro(p2)) return true;

        // Cohen-Sutherland simplificado: clipping del segmento contra el AABB
        // Usamos el test de Liang-Barsky
        double dx = p2.x - p1.x;
        double dy = p2.y - p1.y;
        double t0 = 0.0, t1 = 1.0;

        auto clip = [&](double p, double q) -> bool {
            if (std::abs(p) < MathUtils::EPSILON) return q >= 0.0;
            double r = q / p;
            if (p < 0) { if (r > t1) return false; if (r > t0) t0 = r; }
            else        { if (r < t0) return false; if (r < t1) t1 = r; }
            return true;
        };

        return clip(-dx, p1.x - min.x) && clip(dx, max.x - p1.x)
            && clip(-dy, p1.y - min.y) && clip(dy, max.y - p1.y);
    }

    void cortar_cuerdas_con_tijeras() {
        std::vector<int> ids_a_eliminar;
        std::vector<EntidadFisica*> entidades_nuevas;

        for (auto* e : entidades) {
            auto* tijera = dynamic_cast<Tijera*>(e);
            if (!tijera || !tijera->get_fue_activada() || tijera->get_ya_corto_cuerdas()) continue;

            Vector2D t_min = tijera->get_min() - Vector2D(5, 5);
            Vector2D t_max = tijera->get_max() + Vector2D(5, 5);

            for (auto* e2 : entidades) {
                auto* cuerda = dynamic_cast<Cuerda*>(e2);
                if (!cuerda) continue;

                // puntos: [extremo_a(0), sop0(1), sop1(2), ..., extremo_b(n-1)]
                std::vector<Vector2D> puntos;
                if (!cuerda->obtener_puntos(entidades, puntos)) continue;

                int seg_cortado = -1;
                for (size_t k = 1; k < puntos.size() && seg_cortado < 0; ++k) {
                    if (segmento_cruza_aabb(puntos[k-1], puntos[k], t_min, t_max))
                        seg_cortado = static_cast<int>(k);
                }
                if (seg_cortado < 0) continue;

                ids_a_eliminar.push_back(cuerda->get_id());

                const std::vector<int>& sops = cuerda->get_soportes_id();
                int n = static_cast<int>(puntos.size());
                int num_sops = static_cast<int>(sops.size());

                // sops[i] corresponde a puntos[i+1]
                // seg_cortado: índice del punto FINAL del segmento cortado
                // Soportes lado A: sops[0 .. seg_cortado-2]  (count = seg_cortado-1)
                // Soportes lado B: sops[seg_cortado-1 .. num_sops-1]  (count = num_sops - seg_cortado + 1... pero seg_cortado puede ser n-1)

                int num_sops_a = seg_cortado - 1; // soportes antes del corte
                int num_sops_b = num_sops - num_sops_a; // soportes después del corte

                auto crear_bolita = [&](int id_soporte, Vector2D pos_soporte, double lon_ref) {
                    if (lon_ref < 20.0) lon_ref = 20.0;
                    Bola* bs = new Bola(generar_id(), pos_soporte + Vector2D(0, lon_ref * 0.3), 2.0, 0.3);
                    bs->set_amortiguamiento(0.08);
                    bs->set_restitucion(0.05);
                    entidades_nuevas.push_back(bs);
                    AnclajeCuerda a{ id_soporte, TipoAnclajeCuerda::SoporteFijo };
                    AnclajeCuerda b{ bs->get_id(), TipoAnclajeCuerda::Cubeta };
                    entidades_nuevas.push_back(new Cuerda(generar_id(), a, {}, b, lon_ref));
                };

                // LADO A: extremo_a → sops[0..num_sops_a-3] → sops[num_sops_a-2]
                // sops[num_sops_a-1] = tN se descarta, bolita desde sops[num_sops_a-2] = tN-1
                if (num_sops_a >= 2) {
                    int id_b_a = sops[num_sops_a - 2]; // tN-1
                    std::vector<int> sops_a(sops.begin(), sops.begin() + num_sops_a - 2);
                    double lon_a = 0.0;
                    for (int i = 0; i < seg_cortado - 2; ++i)
                        lon_a += Vector2D::distancia(puntos[i], puntos[i+1]);
                    if (lon_a > MathUtils::EPSILON) {
                        AnclajeCuerda anc_b{ id_b_a, TipoAnclajeCuerda::SoporteFijo };
                        entidades_nuevas.push_back(new Cuerda(
                            generar_id(), cuerda->get_extremo_a(), sops_a, anc_b, lon_a
                        ));
                    }
                    crear_bolita(id_b_a, puntos[seg_cortado-2],
                        Vector2D::distancia(puntos[seg_cortado-2], puntos[seg_cortado-1]));
                }

                // LADO B: sops[num_sops_a+1] → sops[num_sops_a+2..] → extremo_b
                // sops[num_sops_a] = t0 se descarta, bolita desde sops[num_sops_a+1] = t1
                if (num_sops_b >= 2) {
                    int id_a_b = sops[num_sops_a + 1]; // t1
                    std::vector<int> sops_b(sops.begin() + num_sops_a + 2, sops.end());
                    double lon_b = 0.0;
                    for (int i = seg_cortado + 1; i < n - 1; ++i)
                        lon_b += Vector2D::distancia(puntos[i], puntos[i+1]);
                    if (lon_b > MathUtils::EPSILON) {
                        AnclajeCuerda anc_a{ id_a_b, TipoAnclajeCuerda::SoporteFijo };
                        entidades_nuevas.push_back(new Cuerda(
                            generar_id(), anc_a, sops_b, cuerda->get_extremo_b(), lon_b
                        ));
                    }
                    crear_bolita(id_a_b, puntos[seg_cortado + 1],
                        Vector2D::distancia(puntos[seg_cortado + 1], puntos[seg_cortado + 2 < n ? seg_cortado + 2 : n - 1]));
                }
            }

            tijera->set_ya_corto_cuerdas();
            tijera->resetear_activacion();
        }

        for (int id : ids_a_eliminar)
            remover_entidad(id);
        for (auto* nueva : entidades_nuevas)
            agregar_entidad(nueva); // usa entidades_owner para no fugar memoria
    }

    void lanzar_cajas_sorpresa() {
        for (auto* e : entidades) {
            auto* caja = dynamic_cast<CajaSorpresa*>(e);
            if (!caja || !caja->get_activada() || caja->get_ya_lanzo()) continue;
            caja->set_ya_lanzo();

            // Buscar entidades dinámicas encima/sobre la caja y empujarlas
            Vector2D cmin = caja->get_min();
            Vector2D cmax = caja->get_max();
            double zona_x0 = cmin.x - 10.0;
            double zona_x1 = cmax.x + 10.0;
            double zona_y0 = cmin.y - 80.0; // zona de búsqueda encima
            double zona_y1 = cmax.y + 5.0;

            Vector2D impulso = caja->get_velocidad_lanzamiento(); // (280, -520)

            for (auto* otro : entidades) {
                if (otro == e || otro->get_es_estatico()) continue;
                Vector2D pos = otro->get_posicion();
                if (pos.x >= zona_x0 && pos.x <= zona_x1 &&
                    pos.y >= zona_y0 && pos.y <= zona_y1) {
                    otro->set_velocidad(impulso);
                }
            }
        }
    }

    void disparar_pistolas() {
        std::vector<EntidadFisica*> nuevas;
        for (auto* e : entidades) {
            auto* pistola = dynamic_cast<Pistola*>(e);
            if (!pistola || !pistola->get_disparada()) continue;

            Vector2D dir = pistola->get_dir_disparo();
            Vector2D pos = pistola->get_punto_bala();
            Bola* bala = new Bola(generar_id(), pos, 8.0, 0.5);
            bala->set_velocidad(dir * pistola->get_velocidad_bala());  // necesita getter
            bala->set_restitucion(0.3);
            bala->set_amortiguamiento(0.01);
            nuevas.push_back(bala);
            pistola->resetear_disparo();
        }
        for (auto* n : nuevas) entidades.push_back(n);
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

                if (a->get_es_estatico() && b->get_es_estatico()) continue;

                // Detección especial: círculo vs rueda de hámster
                {
                    CajaHamster* hamster = dynamic_cast<CajaHamster*>(a);
                    EntidadFisica* circ_ent = b;
                    if (!hamster) { hamster = dynamic_cast<CajaHamster*>(b); circ_ent = a; }
                    if (hamster && !circ_ent->get_es_estatico()) {
                        Vector2D pos_circ; double radio = 0.0;
                        if (obtener_datos_circulo(circ_ent, pos_circ, radio)) {
                            if (hamster->circulo_toca_rueda(pos_circ, radio)) {
                                InfoColision info_rueda;
                                info_rueda.hay_colision = true;
                                info_rueda.normal = Vector2D(0, -1);
                                info_rueda.profundidad = 1.0;
                                info_rueda.punto_contacto = pos_circ;
                                hamster->on_collision(circ_ent, info_rueda);
                            }
                        }
                    }
                }

                // Detección especial: círculo vs aros de tijera
                {
                    Tijera* tijera = dynamic_cast<Tijera*>(a);
                    EntidadFisica* circ_ent = b;
                    if (!tijera) { tijera = dynamic_cast<Tijera*>(b); circ_ent = a; }
                    if (tijera && !circ_ent->get_es_estatico()) {
                        Vector2D pos_circ; double radio = 0.0;
                        if (obtener_datos_circulo(circ_ent, pos_circ, radio)) {
                            if (tijera->circulo_toca_aros(pos_circ, radio)) {
                                // Notificar colisión con el aro (activa la tijera)
                                InfoColision info_aro;
                                info_aro.hay_colision = true;
                                info_aro.normal = Vector2D(0, -1);
                                info_aro.profundidad = 1.0;
                                info_aro.punto_contacto = pos_circ;
                                tijera->on_collision(circ_ent, info_aro);
                                // No resolver físicamente — la bola pasa a través de los aros
                            }
                        }
                    }
                }

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

        // Tijera cerrada: usar el filo derecho como polígono
        auto* tijera = dynamic_cast<Tijera*>(poly_ent);
        if (tijera && !tijera->get_vertices_filo().empty()) {
            return Colisiones::circulo_vs_poligono(
                pos_circ, radio,
                tijera->get_vertices_filo());
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
