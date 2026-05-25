// ============================================================================
// TIM: Motor de Física — Prototipo Interactivo con Raylib
//
// Controles:
//   [Click Izq]  Crear bola en la posición del mouse
//   [SPACE]      Pausar / Reanudar simulación
//   [D]          Activar / Desactivar modo debug (wireframes, vectores)
//   [R]          Reiniciar escena
//   [+/-]        Aumentar / Disminuir gravedad
//   [ESC]        Salir
// ============================================================================

#include "raylib.h"

#include "core/vector2d.h"
#include "core/math_utils.h"
#include "core/entidad_fisica.h"
#include "objetos/obstaculo_estatico.h"
#include "objetos/pared_rectangular.h"
#include "objetos/plano_inclinado.h"
#include "objetos/bola.h"
#include "objetos/bola_rebotadora.h"
#include "objetos/trampolin.h"
#include "objetos/balancin.h"
#include "objetos/seguidor_booster.h"
#include "objetos/barril_chavo.h"
#include "objetos/ventilador.h"
#include "fisica/colisiones.h"
#include "fisica/motor_fisica.h"

#include <cmath>

// ============================================================================
// Configuración global
// ============================================================================
const int ANCHO = 1024;
const int ALTO  = 768;

// Paleta de colores para bolas (inspirada en TIM, colores vivos)
Color PALETA_BOLAS[] = {
    {255, 107, 107, 255},   // Coral
    {78,  205, 196, 255},   // Turquesa
    {255, 230, 109, 255},   // Amarillo
    {199, 125, 255, 255},   // Violeta
    {69,  183, 209, 255},   // Cielo
    {255, 179, 71,  255},   // Naranja
    {95,  209, 111, 255},   // Verde
};
const int NUM_COLORES = 7;

// Colores del escenario
const Color COLOR_FONDO       = {22,  22,  42,  255};   // Navy oscuro
const Color COLOR_PARED       = {40,  42,  68,  255};   // Azul-gris oscuro
const Color COLOR_PARED_BORDE = {70,  75,  110, 255};   // Azul-gris claro
const Color COLOR_RAMPA       = {35,  75,  58,  255};   // Verde bosque oscuro
const Color COLOR_RAMPA_BORDE = {65,  140, 100, 255};   // Verde bosque claro
const Color COLOR_HUD         = {200, 200, 220, 255};   // Texto claro
const Color COLOR_CONTROLES   = {130, 130, 160, 200};   // Texto controles

// Estado del prototipo
bool modo_debug = false;
int contador_bolas = 0;

// Feedback visual de spawn fallido
float spawn_error_timer = 0.0f;
Vector2D spawn_error_pos;

// Objeto seleccionado para spawn (0 = Bola, 1 = Trampolín, 2 = Balancín, etc.)
int seleccion_objeto = 0;

// Estado del sistema Drag & Drop
EntidadFisica* entidad_arrastrada = nullptr;
Vector2D offset_arrastre;

bool obtener_datos_circulo(const EntidadFisica* e, Vector2D& pos, double& radio) {
    const Bola* bola = dynamic_cast<const Bola*>(e);
    if (bola) {
        pos = bola->get_posicion();
        radio = bola->get_radio();
        return true;
    }

    const BolaRebotadora* rebotadora = dynamic_cast<const BolaRebotadora*>(e);
    if (rebotadora) {
        pos = rebotadora->get_posicion();
        radio = rebotadora->get_radio();
        return true;
    }

    return false;
}

// Obtener qué objeto móvil/colocable está debajo del cursor del mouse
EntidadFisica* obtener_entidad_bajo_mouse(const MotorFisica& motor, Vector2D mouse_pos) {
    for (auto* e : motor.get_entidades()) {
        TipoForma forma = e->get_tipo_forma();
        if (forma == TipoForma::CIRCULO) {
            const BolaRebotadora* rebotadora = dynamic_cast<const BolaRebotadora*>(e);
            if (rebotadora) {
                double dist = (rebotadora->get_posicion() - mouse_pos).magnitud();
                if (dist < rebotadora->get_radio() + 15.0) {
                    return const_cast<BolaRebotadora*>(rebotadora);
                }
            }

            const Bola* b = dynamic_cast<const Bola*>(e);
            if (b) {
                double dist = (b->get_posicion() - mouse_pos).magnitud();
                if (dist < b->get_radio() + 15.0) {
                    return const_cast<Bola*>(b);
                }
            }
        }
        else if (forma == TipoForma::AABB) {
            const Trampolin* t = dynamic_cast<const Trampolin*>(e);
            if (t) {
                Vector2D pos = t->get_posicion();
                double w = t->get_ancho();
                double h = t->get_alto();
                if (mouse_pos.x >= pos.x - 10 && mouse_pos.x <= pos.x + w + 10 &&
                    mouse_pos.y >= pos.y - 10 && mouse_pos.y <= pos.y + h + 10) {
                    return const_cast<Trampolin*>(t);
                }
            }
            const SeguidorBooster* seg = dynamic_cast<const SeguidorBooster*>(e);
            if (seg) {
                Vector2D pos = seg->get_posicion();
                double w = seg->get_ancho();
                double h = seg->get_alto();
                if (mouse_pos.x >= pos.x - w/2 - 10 && mouse_pos.x <= pos.x + w/2 + 10 &&
                    mouse_pos.y >= pos.y - h/2 - 10 && mouse_pos.y <= pos.y + h/2 + 10) {
                    return const_cast<SeguidorBooster*>(seg);
                }
            }
            const BarrilChavo* bar = dynamic_cast<const BarrilChavo*>(e);
            if (bar) {
                Vector2D pos = bar->get_posicion();
                double w = bar->get_ancho();
                double h = bar->get_alto();
                if (mouse_pos.x >= pos.x - 10 && mouse_pos.x <= pos.x + w + 10 &&
                    mouse_pos.y >= pos.y - 10 && mouse_pos.y <= pos.y + h + 10) {
                    return const_cast<BarrilChavo*>(bar);
                }
            }
            const Ventilador* vent = dynamic_cast<const Ventilador*>(e);
            if (vent) {
                Vector2D pos = vent->get_posicion();
                double w = vent->get_ancho();
                double h = vent->get_alto();
                if (mouse_pos.x >= pos.x - 10 && mouse_pos.x <= pos.x + w + 10 &&
                    mouse_pos.y >= pos.y - 10 && mouse_pos.y <= pos.y + h + 10) {
                    return const_cast<Ventilador*>(vent);
                }
            }
        }
        else if (forma == TipoForma::POLIGONO) {
            const Balancin* bal = dynamic_cast<const Balancin*>(e);
            if (bal) {
                // Pivot central del balancín
                double dist = (bal->get_posicion() - mouse_pos).magnitud();
                if (dist < 45.0) {
                    return const_cast<Balancin*>(bal);
                }
            }
        }
    }
    return nullptr;
}

// ============================================================================
// Crear escena inicial
// ============================================================================
void crear_escena(MotorFisica& motor) {
    int g = 20; // Grosor de paredes límite

    // ---- Paredes del nivel (4 bordes) ----
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(0, ALTO - g), ANCHO, g));          // Suelo
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(0, 0), g, ALTO));                  // Izquierda
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(ANCHO - g, 0), g, ALTO));         // Derecha
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(0, 0), ANCHO, g));                 // Techo

    // ---- Plataformas ----
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(80, 320), 220, 15));               // Plataforma superior izq
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(550, 260), 230, 15));              // Plataforma superior der
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(350, 550), 200, 15));              // Plataforma inferior centro

    // ---- Rampas ----
    // Rampa \ (pendiente de arriba-izq a abajo-der)
    // Conecta visualmente con la plataforma superior izquierda
    motor.agregar_entidad(new PlanoInclinado(
        motor.generar_id(), Vector2D(300, 320), 200, 200, false));

    // Rampa / (pendiente de abajo-izq a arriba-der) 
    motor.agregar_entidad(new PlanoInclinado(
        motor.generar_id(), Vector2D(700, 280), 200, 200, true));

    // ---- Barril Chavo inicial ----
    motor.agregar_entidad(new BarrilChavo(
        motor.generar_id(), Vector2D(420, 470)));

    // ---- Bola Rebotadora inicial ----
    motor.agregar_entidad(new BolaRebotadora(
        motor.generar_id(), Vector2D(640, 470), 48.0));
}

// ============================================================================
// Validación de spawn — no crear objetos dentro de otros
// ============================================================================
bool posicion_valida_para_bola(const MotorFisica& motor, Vector2D pos, double radio) {
    for (const auto* e : motor.get_entidades()) {
        TipoForma forma = e->get_tipo_forma();

        if (forma == TipoForma::CIRCULO) {
            Vector2D pos_circ;
            double radio_circ = 0.0;
            if (obtener_datos_circulo(e, pos_circ, radio_circ)) {
                InfoColision info = Colisiones::circulo_vs_circulo(
                    pos, radio, pos_circ, radio_circ);
                if (info.hay_colision) return false;
            }
        }
        else if (forma == TipoForma::AABB) {
            const ParedRectangular* p = dynamic_cast<const ParedRectangular*>(e);
            if (p) {
                InfoColision info = Colisiones::circulo_vs_aabb(
                    pos, radio, p->get_min(), p->get_max());
                if (info.hay_colision) return false;
            }
            const Trampolin* t = dynamic_cast<const Trampolin*>(e);
            if (t) {
                InfoColision info = Colisiones::circulo_vs_aabb(
                    pos, radio, t->get_min(), t->get_max());
                if (info.hay_colision) return false;
            }
            const BarrilChavo* bar = dynamic_cast<const BarrilChavo*>(e);
            if (bar) {
                InfoColision info = Colisiones::circulo_vs_aabb(
                    pos, radio, bar->get_min(), bar->get_max());
                if (info.hay_colision) return false;
            }
        }
        else if (forma == TipoForma::POLIGONO) {
            const PlanoInclinado* ramp = dynamic_cast<const PlanoInclinado*>(e);
            if (ramp) {
                InfoColision info = Colisiones::circulo_vs_poligono(
                    pos, radio, ramp->get_vertices());
                if (info.hay_colision) return false;
            }
            const Balancin* bal = dynamic_cast<const Balancin*>(e);
            if (bal) {
                InfoColision info = Colisiones::circulo_vs_balancin(
                    pos, radio, bal->get_posicion(), bal->get_angulo(),
                    bal->get_largo(), bal->get_espesor());
                if (info.hay_colision) return false;
            }
        }
    }
    return true;
}

// Validación para crear balancín
bool posicion_valida_para_balancin(const MotorFisica& motor, Vector2D pos, double w, double h) {
    for (const auto* e : motor.get_entidades()) {
        TipoForma forma = e->get_tipo_forma();

        if (forma == TipoForma::CIRCULO) {
            Vector2D pos_circ;
            double radio_circ = 0.0;
            if (obtener_datos_circulo(e, pos_circ, radio_circ)) {
                InfoColision info = Colisiones::circulo_vs_aabb(
                    pos_circ, radio_circ,
                    pos, Vector2D(pos.x + w, pos.y + h));
                if (info.hay_colision) return false;
            }
        }
        else if (forma == TipoForma::AABB) {
            const ParedRectangular* p = dynamic_cast<const ParedRectangular*>(e);
            const Trampolin* t = dynamic_cast<const Trampolin*>(e);
            const BarrilChavo* bar = dynamic_cast<const BarrilChavo*>(e);
            Vector2D min_b = p ? p->get_min() : (t ? t->get_min() : (bar ? bar->get_min() : pos));
            Vector2D max_b = p ? p->get_max() : (t ? t->get_max() : (bar ? bar->get_max() : pos));

            if (p || t || bar) {
                bool overlap = (pos.x < max_b.x && pos.x + w > min_b.x &&
                                pos.y < max_b.y && pos.y + h > min_b.y);
                if (overlap) return false;
            }
        }
        else if (forma == TipoForma::POLIGONO) {
            const PlanoInclinado* ramp = dynamic_cast<const PlanoInclinado*>(e);
            const Balancin* bal = dynamic_cast<const Balancin*>(e);
            if (ramp) {
                for (const auto& v : ramp->get_vertices()) {
                    if (v.x >= pos.x && v.x <= pos.x + w &&
                        v.y >= pos.y && v.y <= pos.y + h) {
                        return false;
                    }
                }
            }
            if (bal) {
                // Evitar superponer pivotes demasiado cerca
                Vector2D diff = (pos + Vector2D(w/2.0, h/2.0)) - bal->get_posicion();
                if (diff.magnitud() < 40.0) return false;
            }
        }
    }
    return true;
}

// Crear balancín en la posición del mouse
bool crear_balancin(MotorFisica& motor, Vector2D pos) {
    double w = 200.0;
    double h = 6.0;

    // Centrar en el mouse (el pivot es su posición)
    Vector2D pivot_pos = pos;
    Vector2D spawn_min(pos.x - w / 2.0, pos.y - h / 2.0);

    if (!posicion_valida_para_balancin(motor, spawn_min, w, h)) {
        spawn_error_timer = 0.5f;
        spawn_error_pos = pos;
        return false;
    }

    Balancin* b = new Balancin(motor.generar_id(), pivot_pos, w, h);
    motor.agregar_entidad(b);
    return true;
}

// Validación para crear trampolín (evita colisiones con paredes, otras rampas, bolas)
bool posicion_valida_para_trampolin(const MotorFisica& motor, Vector2D pos, double w, double h) {
    for (const auto* e : motor.get_entidades()) {
        TipoForma forma = e->get_tipo_forma();

        if (forma == TipoForma::CIRCULO) {
            Vector2D pos_circ;
            double radio_circ = 0.0;
            if (obtener_datos_circulo(e, pos_circ, radio_circ)) {
                InfoColision info = Colisiones::circulo_vs_aabb(
                    pos_circ, radio_circ,
                    pos, Vector2D(pos.x + w, pos.y + h));
                if (info.hay_colision) return false;
            }
        }
        else if (forma == TipoForma::AABB) {
            const ParedRectangular* p = dynamic_cast<const ParedRectangular*>(e);
            const Trampolin* t = dynamic_cast<const Trampolin*>(e);
            const BarrilChavo* bar = dynamic_cast<const BarrilChavo*>(e);
            Vector2D min_b = p ? p->get_min() : (t ? t->get_min() : (bar ? bar->get_min() : pos));
            Vector2D max_b = p ? p->get_max() : (t ? t->get_max() : (bar ? bar->get_max() : pos));

            if (p || t || bar) {
                // Intersección AABB vs AABB
                bool overlap = (pos.x < max_b.x && pos.x + w > min_b.x &&
                                pos.y < max_b.y && pos.y + h > min_b.y);
                if (overlap) return false;
            }
        }
        else if (forma == TipoForma::POLIGONO) {
            const PlanoInclinado* ramp = dynamic_cast<const PlanoInclinado*>(e);
            if (ramp) {
                // Verificación simple para evitar spawn encima de vértices de la rampa
                for (const auto& v : ramp->get_vertices()) {
                    if (v.x >= pos.x && v.x <= pos.x + w &&
                        v.y >= pos.y && v.y <= pos.y + h) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

// ============================================================================
// Crear trampolín en posición del mouse (con validación)
// ============================================================================
bool crear_trampolin(MotorFisica& motor, Vector2D pos) {
    double w = 80.0;
    double h = 20.0;
    // Centrar en el mouse
    Vector2D spawn_pos(pos.x - w / 2.0, pos.y - h / 2.0);

    if (!posicion_valida_para_trampolin(motor, spawn_pos, w, h)) {
        spawn_error_timer = 0.5f;
        spawn_error_pos = pos;
        return false;
    }

    Trampolin* t = new Trampolin(motor.generar_id(), spawn_pos, w, h);
    motor.agregar_entidad(t);
    return true;
}

// ============================================================================
// Crear bola en posición del mouse (con validación)
// ============================================================================
bool crear_bola(MotorFisica& motor, Vector2D pos) {
    double radio = 8.0 + GetRandomValue(0, 8);   // Radio entre 8-16 px

    // Verificar que no colisiona con nada existente
    if (!posicion_valida_para_bola(motor, pos, radio)) {
        spawn_error_timer = 0.5f;  // Flash rojo de 0.5s
        spawn_error_pos = pos;
        return false;
    }

    double masa = radio * radio * 0.01;           // Proporcional al área
    Bola* b = new Bola(motor.generar_id(), pos, radio, masa);
    b->set_color_idx(contador_bolas % NUM_COLORES);
    contador_bolas++;
    motor.agregar_entidad(b);
    return true;
}

// ============================================================================
// Crear BolaRebotadora en posicion del mouse (con validacion)
// ============================================================================
bool crear_bola_rebotadora(MotorFisica& motor, Vector2D pos) {
    double radio = 48.0;

    // Verificar que no nace encima de paredes, rampas u otros mecanismos
    if (!posicion_valida_para_bola(motor, pos, radio)) {
        spawn_error_timer = 0.5f;
        spawn_error_pos = pos;
        return false;
    }

    BolaRebotadora* b = new BolaRebotadora(motor.generar_id(), pos, radio);
    motor.agregar_entidad(b);
    return true;
}

// ============================================================================
// Crear ventilador en posicion del mouse
// ============================================================================
bool crear_ventilador(MotorFisica& motor, Vector2D pos) {
    double w = 42.0;
    double h = 54.0;

    Vector2D spawn_pos(pos.x - w / 2.0, pos.y - h / 2.0);
    Ventilador* v = new Ventilador(motor.generar_id(), spawn_pos, w, h);
    motor.agregar_entidad(v);
    return true;
}

// ============================================================================
// Crear futbolista seguidor en la posición del mouse
// ============================================================================
bool crear_seguidor_booster(MotorFisica& motor, Vector2D pos) {
    double radio = 18.0;
    SeguidorBooster* s = new SeguidorBooster(motor.generar_id(), pos, radio);
    motor.agregar_entidad(s);
    return true;
}

// ============================================================================
// Validación para crear barril (evita colisiones con paredes, trampolines, bolas, etc.)
// ============================================================================
bool posicion_valida_para_barril(const MotorFisica& motor, Vector2D pos, double w, double h) {
    for (const auto* e : motor.get_entidades()) {
        TipoForma forma = e->get_tipo_forma();

        if (forma == TipoForma::CIRCULO) {
            Vector2D pos_circ;
            double radio_circ = 0.0;
            if (obtener_datos_circulo(e, pos_circ, radio_circ)) {
                InfoColision info = Colisiones::circulo_vs_aabb(
                    pos_circ, radio_circ,
                    pos, Vector2D(pos.x + w, pos.y + h));
                if (info.hay_colision) return false;
            }
        }
        else if (forma == TipoForma::AABB) {
            const ParedRectangular* p = dynamic_cast<const ParedRectangular*>(e);
            const Trampolin* t = dynamic_cast<const Trampolin*>(e);
            const BarrilChavo* bar = dynamic_cast<const BarrilChavo*>(e);
            Vector2D min_b = p ? p->get_min() : (t ? t->get_min() : (bar ? bar->get_min() : pos));
            Vector2D max_b = p ? p->get_max() : (t ? t->get_max() : (bar ? bar->get_max() : pos));

            if (p || t || bar) {
                bool overlap = (pos.x < max_b.x && pos.x + w > min_b.x &&
                                pos.y < max_b.y && pos.y + h > min_b.y);
                if (overlap) return false;
            }
        }
    }
    return true;
}

// ============================================================================
// Crear Barril Chavo en posición del mouse (con validación)
// ============================================================================
bool crear_barril_chavo(MotorFisica& motor, Vector2D pos) {
    double w = 60.0;
    double h = 80.0;
    
    // Centrar en el mouse
    Vector2D spawn_pos(pos.x - w / 2.0, pos.y - h / 2.0);

    if (!posicion_valida_para_barril(motor, spawn_pos, w, h)) {
        spawn_error_timer = 0.5f;
        spawn_error_pos = pos;
        return false;
    }

    BarrilChavo* b = new BarrilChavo(motor.generar_id(), spawn_pos, w, h);
    motor.agregar_entidad(b);
    return true;
}

// ============================================================================
// Renderizado de entidades
// ============================================================================
void dibujar_entidad(const EntidadFisica* e) {
    TipoForma forma = e->get_tipo_forma();

    if (forma == TipoForma::CIRCULO) {
        const BolaRebotadora* rebotadora = dynamic_cast<const BolaRebotadora*>(e);
        if (rebotadora) {
            Vector2D pos = rebotadora->get_posicion();
            float r = static_cast<float>(rebotadora->get_radio());
            float def = static_cast<float>(rebotadora->get_deformacion());
            float wobble_x = static_cast<float>(rebotadora->get_offset_vibracion());
            float escala_x = 1.0f + def / (r * 5.0f);
            float escala_y = 1.0f - def / (r * 4.0f);
            float draw_x = static_cast<float>(pos.x) + wobble_x;
            float draw_y = static_cast<float>(pos.y) + def * 0.25f;
            float rx = r * escala_x;
            float ry = r * escala_y;
            float base_top = static_cast<float>(pos.y + r * 0.72f);
            float base_w = r * 1.35f;
            float base_h = r * 0.58f;

            Color col_base = Color{218, 48, 42, 255};
            DrawRectangleRec({draw_x - base_w / 2.0f, base_top,
                              base_w, base_h}, col_base);
            DrawEllipse(static_cast<int>(draw_x), static_cast<int>(draw_y), rx, ry, col_base);

            // 3. PequeÃ±a banda oscura inferior para marcar contacto/curvatura

            if (modo_debug) {
                DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), r, GREEN);
            }
            return;
        }

        const Bola* b = dynamic_cast<const Bola*>(e);
        if (!b) return;

        Vector2D pos = b->get_posicion();
        float r = static_cast<float>(b->get_radio());
        Color col = PALETA_BOLAS[b->get_color_idx()];

        // Círculo relleno + borde más oscuro
        DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), r, col);
        DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), r,
                        ColorBrightness(col, -0.3f));

        // Indicador de rotación (siempre visible)
        // Dos puntos en lados opuestos que giran con el ángulo de la bola
        double ang = b->get_angulo();
        float dot_dist = r * 0.55f;
        float dot_r = std::max(r * 0.2f, 2.0f);
        Color dot_col = ColorBrightness(col, -0.45f);

        // Punto 1
        int dx1 = static_cast<int>(pos.x + std::cos(ang) * dot_dist);
        int dy1 = static_cast<int>(pos.y + std::sin(ang) * dot_dist);
        DrawCircle(dx1, dy1, dot_r, dot_col);

        // Punto 2 (opuesto)
        int dx2 = static_cast<int>(pos.x - std::cos(ang) * dot_dist);
        int dy2 = static_cast<int>(pos.y - std::sin(ang) * dot_dist);
        DrawCircle(dx2, dy2, dot_r, dot_col);

        // Brillo especular (efecto de profundidad sutil)
        DrawCircle(static_cast<int>(pos.x - r * 0.25),
                   static_cast<int>(pos.y - r * 0.25),
                   r * 0.3f,
                   Color{255, 255, 255, 60});

        // Debug extra: línea de velocidad angular
        if (modo_debug) {
            Vector2D dir(std::cos(ang) * r, std::sin(ang) * r);
            DrawLine(static_cast<int>(pos.x), static_cast<int>(pos.y),
                     static_cast<int>(pos.x + dir.x),
                     static_cast<int>(pos.y + dir.y), WHITE);
        }
    }
    else if (forma == TipoForma::AABB) {
        const Ventilador* vent = dynamic_cast<const Ventilador*>(e);
        if (vent) {
            Vector2D pos = vent->get_posicion();
            float px = static_cast<float>(pos.x);
            float py = static_cast<float>(pos.y);
            float w = static_cast<float>(vent->get_ancho());
            float h = static_cast<float>(vent->get_alto());
            float cx = px + w / 2.0f;
            float cy = py + h / 2.0f;
            float fase = static_cast<float>(vent->get_fase_aspas());

            // 1. Cuerpo del ventilador
            DrawRectangleRec({px, py, w, h}, Color{70, 84, 96, 255});
            DrawRectangleLinesEx({px, py, w, h}, 1.5f, Color{170, 190, 205, 255});

            // 2. Rejilla frontal y aspas giratorias
            DrawCircleLines(static_cast<int>(cx), static_cast<int>(cy), h * 0.32f, Color{190, 210, 220, 255});
            for (int i = 0; i < 4; ++i) {
                float ang = fase + i * MathUtils::TIM_PI / 2.0f;
                Vector2 p1 = {cx, cy};
                Vector2 p2 = {
                    cx + std::cos(ang) * h * 0.26f,
                    cy + std::sin(ang) * h * 0.26f
                };
                DrawLineEx(p1, p2, 3.0f, Color{135, 205, 255, 255});
            }
            DrawCircle(static_cast<int>(cx), static_cast<int>(cy), 4.0f, Color{210, 230, 240, 255});

            // 3. Corriente de aire hacia la derecha (animada)
            for (int i = 0; i < 4; ++i) {
                float y = cy - 24.0f + i * 16.0f;
                float offset = std::sin(fase + i * 0.5f) * 15.0f;  // Desplazamiento según fase
                float longitud = 70.0f + std::sin(fase + i * 0.3f) * 25.0f;  // Varía la longitud
                float opacidad = 90 + std::sin(fase + i * 0.4f) * 50;  // Varía transparencia
                DrawLineEx({px + w + 8.0f + offset, y}, {px + w + 8.0f + longitud, y}, 1.5f,
                          Color{120, 200, 255, static_cast<unsigned char>(opacidad)});
            }

            if (modo_debug) {
                DrawRectangleLines(static_cast<int>(px), static_cast<int>(py),
                                   static_cast<int>(w), static_cast<int>(h), GREEN);
                DrawRectangleLines(static_cast<int>(px + w), static_cast<int>(cy - vent->get_ancho_corriente() / 2.0),
                                   static_cast<int>(vent->get_rango()), static_cast<int>(vent->get_ancho_corriente()), GREEN);
            }
            return;
        }

        const SeguidorBooster* seg = dynamic_cast<const SeguidorBooster*>(e);
        if (seg) {
            Vector2D pos = seg->get_posicion();
            float w = static_cast<float>(seg->get_ancho());
            float h = static_cast<float>(seg->get_alto());
            float r = w * 0.75f; // Radio aproximado para dibujar cabeza y camiseta
            EstadoSeguidor estado = seg->get_estado();
            Vector2D pos_init = seg->get_posicion_inicial();
            double ang_pierna = seg->get_angulo_pierna();
            double dir_carr = seg->get_direccion_carrera();
            double kick_f = seg->get_kicker_factor();

            float draw_y = pos.y - 6.0f; // Subir el sprite 6 pixeles para que no se hundan los pies en el suelo

            // 1. Dibujar sombra sutil en el suelo (bajo el jugador en la posición física real)
            DrawEllipse(static_cast<int>(pos.x), static_cast<int>(pos.y + h / 2.0f - 2.0f), w * 0.8f, 3.0f, Color{0, 0, 0, 80});

            // 2. Línea de anclaje (hilo elástico de retorno)
            if (estado != EstadoSeguidor::ESPERANDO) {
                DrawLineEx({(float)pos_init.x, (float)pos_init.y}, {(float)pos.x, (float)draw_y}, 1.5f, Color{100, 150, 255, 100});
            }

            // 3. Piernas y chut (dibujamos dos piernas sencillas oscilando o pateando)
            // Pierna 1
            float leg1_ang = static_cast<float>(ang_pierna);
            float lx1 = pos.x - dir_carr * w * 0.3f + std::sin(leg1_ang) * r * 0.7f;
            float ly1 = draw_y + h / 2.0f - r * 0.2f + std::cos(leg1_ang) * r * 0.5f;
            DrawLineEx({(float)(pos.x - dir_carr * w * 0.2f), (float)(draw_y + h / 2.0f - r * 0.4f)}, {lx1, ly1}, 4.0f, DARKGRAY);
            DrawCircle(lx1, ly1, 3.0f, RED); // Botín rojo

            // Pierna 2 (Pateadora/delantera)
            float leg2_ang = static_cast<float>(-ang_pierna + kick_f * dir_carr * 1.5); // Si patea, se extiende
            float lx2 = pos.x + dir_carr * w * 0.3f + std::sin(leg2_ang) * r * 0.7f;
            float ly2 = draw_y + h / 2.0f - r * 0.2f + std::cos(leg2_ang) * r * 0.5f;
            DrawLineEx({(float)(pos.x + dir_carr * w * 0.2f), (float)(draw_y + h / 2.0f - r * 0.4f)}, {lx2, ly2}, 4.0f, DARKGRAY);
            DrawCircle(lx2, ly2, 3.0f, RED); // Botín rojo

            // 4. Cuerpo / Camiseta rayada de fútbol
            int cx = static_cast<int>(pos.x);
            int cy = static_cast<int>(draw_y + h * 0.1f);
            // Camiseta (cuerpo)
            DrawCircle(cx, cy, r * 0.7f, SKYBLUE);
            DrawCircleLines(cx, cy, r * 0.7f, BLUE);
            // Rayas blancas verticales de la camiseta
            DrawRectangleRec({(float)(cx - r * 0.3f), (float)(cy - r * 0.6f), (float)(r * 0.15f), (float)(r * 1.2f)}, WHITE);
            DrawRectangleRec({(float)(cx + r * 0.15f), (float)(cy - r * 0.6f), (float)(r * 0.15f), (float)(r * 1.2f)}, WHITE);

            // 5. Cabeza del futbolista
            int head_y = static_cast<int>(draw_y - h * 0.2f);
            DrawCircle(cx, head_y, r * 0.45f, Color{250, 200, 160, 255}); // Piel
            DrawCircleLines(cx, head_y, r * 0.45f, DARKGRAY);
            // Pelo marrón
            DrawCircleSector({(float)cx, (float)head_y}, r * 0.46f, 180.0f, 360.0f, 0, Color{130, 80, 40, 255});

            // Ojo indicando la dirección de mirada
            int eye_x = cx + static_cast<int>(dir_carr * r * 0.25f);
            int eye_y = head_y - static_cast<int>(r * 0.05f);
            DrawCircle(eye_x, eye_y, 2.5f, BLACK);

            // 6. LED indicador de estado en la espalda
            Color led_color = GREEN;
            if (estado == EstadoSeguidor::PERSIGUIENDO) led_color = ORANGE;
            else if (estado == EstadoSeguidor::RETRAYENDO) led_color = PURPLE;
            DrawCircle(cx - static_cast<int>(dir_carr * r * 0.3f), cy - r * 0.1f, 3.5f, led_color);

            // Rango de debug
            if (modo_debug) {
                DrawCircleLines(cx, cy, 180.0f, Color{0, 255, 0, 80});
                // Dibujar caja de colisión AABB real
                DrawRectangleLines(static_cast<int>(pos.x - w/2), static_cast<int>(pos.y - h/2), static_cast<int>(w), static_cast<int>(h), GREEN);
            }
            return;
        }

        // Primero verificamos si es un Trampolin
        const Trampolin* tramp = dynamic_cast<const Trampolin*>(e);
        if (tramp) {
            Vector2D pos = tramp->get_posicion();
            float px = static_cast<float>(pos.x);
            float py = static_cast<float>(pos.y);
            float pw = static_cast<float>(tramp->get_ancho());
            float ph = static_cast<float>(tramp->get_alto());
            float def = static_cast<float>(tramp->get_deformacion());

            // 1. Dibujar patas de soporte de acero cromado (estructura triangular estable)
            DrawLineEx({px + 6, py + ph}, {px + 12, py + 8}, 3.0f, DARKGRAY);
            DrawLineEx({px + 18, py + ph}, {px + 12, py + 8}, 3.0f, DARKGRAY);
            DrawLineEx({px + pw - 6, py + ph}, {px + pw - 12, py + 8}, 3.0f, DARKGRAY);
            DrawLineEx({px + pw - 18, py + ph}, {px + pw - 12, py + 8}, 3.0f, DARKGRAY);

            // Barra inferior metálica horizontal de unión
            DrawLineEx({px + 18, py + ph - 2}, {px + pw - 18, py + ph - 2}, 2.5f, GRAY);

            // 2. Dibujar marcos rígidos laterales en los extremos (donde se anclan los resortes)
            DrawRectangleRec({px, py, 6.0f, 10.0f}, GRAY);
            DrawRectangleLinesEx({px, py, 6.0f, 10.0f}, 1.0f, DARKGRAY);
            DrawRectangleRec({px + pw - 6.0f, py, 6.0f, 10.0f}, GRAY);
            DrawRectangleLinesEx({px + pw - 6.0f, py, 6.0f, 10.0f}, 1.0f, DARKGRAY);

            // 3. Dibujar resortes elásticos dorados estirándose hacia la lona curvada
            int num_resortes = 6;
            for (int i = 0; i < num_resortes; ++i) {
                // Anclaje en marco izquierdo
                float lx_start = px + 6.0f;
                float ly_start = py + 3.0f + i * 1.2f;
                
                // Fin en lona izquierda (interpolación hasta el centro)
                float t = static_cast<float>(i) / (num_resortes - 1);
                float lx_end = px + 6.0f + (pw / 2.0f - 14.0f) * t;
                float ly_end = py + 4.0f + def * t;
                
                // Dibujar resorte como zigzag dorado
                float mx = (lx_start + lx_end) / 2.0f;
                float my = (ly_start + ly_end) / 2.0f + 2.5f; // zigzag wave
                DrawLineEx({lx_start, ly_start}, {mx, my}, 1.5f, GOLD);
                DrawLineEx({mx, my}, {lx_end, ly_end}, 1.5f, GOLD);

                // Anclaje en marco derecho
                float rx_start = px + pw - 6.0f;
                float ry_start = py + 3.0f + i * 1.2f;
                
                // Fin en lona derecha
                float rx_end = px + pw - 6.0f - (pw / 2.0f - 14.0f) * t;
                float ry_end = py + 4.0f + def * t;

                float rmx = (rx_start + rx_end) / 2.0f;
                float rmy = (ry_start + ry_end) / 2.0f + 2.5f;
                DrawLineEx({rx_start, ry_start}, {rmx, rmy}, 1.5f, GOLD);
                DrawLineEx({rmx, rmy}, {rx_end, ry_end}, 1.5f, GOLD);
            }

            // 4. Dibujar la lona elástica central curvada por la deformación (rojo intenso con contorno naranja)
            Vector2 p_left = { px + 10.0f, py + 4.0f };
            Vector2 p_center = { px + pw / 2.0f, py + 4.0f + def };
            Vector2 p_right = { px + pw - 10.0f, py + 4.0f };

            // Lona elástica gruesa
            DrawLineEx(p_left, p_center, 6.0f, RED);
            DrawLineEx(p_center, p_right, 6.0f, RED);

            // Borde brillante superior
            DrawLineEx({p_left.x, p_left.y - 2.0f}, {p_center.x, p_center.y - 2.0f}, 1.5f, ORANGE);
            DrawLineEx({p_center.x, p_center.y - 2.0f}, {p_right.x, p_right.y - 2.0f}, 1.5f, ORANGE);
            return;
        }

        // Primero verificamos si es un BarrilChavo
        const BarrilChavo* barril = dynamic_cast<const BarrilChavo*>(e);
        if (barril) {
            Vector2D pos = barril->get_posicion();
            float px = static_cast<float>(pos.x);
            float py = static_cast<float>(pos.y);
            float pw = static_cast<float>(barril->get_ancho());
            float ph = static_cast<float>(barril->get_alto());
            float pop = static_cast<float>(barril->get_pop_factor());

            // 1. Dibujar sombra del barril
            DrawEllipse(static_cast<int>(px + pw / 2.0f), static_cast<int>(py + ph - 2.0f), pw * 0.7f, 4.0f, Color{0, 0, 0, 90});

            // 2. Dibujar al "Chavo" saliendo si pop > 0.0
            if (pop > 0.0f) {
                // La cabeza sale de la tapa superior del barril (py) hacia arriba.
                // El rango de movimiento de la cabeza: desde oculta (y = py + 15) hasta afuera (y = py - 20)
                float chavo_y = py + 15.0f - pop * 35.0f;
                float chavo_x = px + pw / 2.0f;
                float head_r = 15.0f;

                // Dibujar cuello/hombros simples
                DrawRectangleRec({chavo_x - 8.0f, chavo_y + 10.0f, 16.0f, 15.0f}, Color{220, 220, 200, 255}); // Camiseta a rayas beige

                // Dibujar cabeza (piel)
                DrawCircle(static_cast<int>(chavo_x), static_cast<int>(chavo_y), head_r, Color{253, 214, 185, 255});
                DrawCircleLines(static_cast<int>(chavo_x), static_cast<int>(chavo_y), head_r, Color{160, 110, 80, 255});

                // Dibujar gorro del Chavo (gorra verde a cuadros con orejeras)
                // Orejera izquierda
                DrawRectangleRec({chavo_x - head_r - 2.0f, chavo_y - 2.0f, 5.0f, 12.0f}, Color{74, 117, 89, 255}); // Verde oscuro
                DrawRectangleRec({chavo_x - head_r - 2.0f, chavo_y - 2.0f, 5.0f, 12.0f}, Color{126, 170, 140, 255}); // Verde claro alternado
                // Orejera derecha
                DrawRectangleRec({chavo_x + head_r - 3.0f, chavo_y - 2.0f, 5.0f, 12.0f}, Color{74, 117, 89, 255});

                // Cúpula del gorro
                DrawCircleSector({chavo_x, chavo_y}, head_r + 1.0f, 180.0f, 360.0f, 0, Color{84, 137, 101, 255});
                // Visera del gorro
                DrawTriangle(
                    {chavo_x - 4.0f, chavo_y - head_r + 2.0f},
                    {chavo_x + 14.0f, chavo_y - 2.0f},
                    {chavo_x - 2.0f, chavo_y - 2.0f},
                    Color{114, 167, 131, 255}
                );

                // Cuadros/Líneas de la gorra para darle textura
                DrawLineEx({chavo_x - head_r, chavo_y - 6.0f}, {chavo_x + head_r, chavo_y - 6.0f}, 1.5f, Color{40, 70, 50, 180});
                DrawLineEx({chavo_x, chavo_y - head_r}, {chavo_x, chavo_y}, 1.5f, Color{40, 70, 50, 180});

                // Ojos (puntos negros)
                DrawCircle(static_cast<int>(chavo_x - 5.0f), static_cast<int>(chavo_y - 1.0f), 2.0f, BLACK);
                DrawCircle(static_cast<int>(chavo_x + 5.0f), static_cast<int>(chavo_y - 1.0f), 2.0f, BLACK);

                                DrawCircleSector({chavo_x, chavo_y + 3.0f}, 4.0f, 0.0f, 180.0f, 0, Color{200, 80, 80, 255}); // Boca abierta
                DrawCircle(static_cast<int>(chavo_x - 5.0f), static_cast<int>(chavo_y + 2.0f), 2.0f, Color{253, 180, 160, 255}); // Mejillas
                DrawCircle(static_cast<int>(chavo_x + 5.0f), static_cast<int>(chavo_y + 2.0f), 2.0f, Color{253, 180, 160, 255});
            }

            // 3. Dibujar el Barril (cuerpo de madera, franjas metálicas)
            Color col_madera = Color{139, 90, 43, 255};
            Color col_borde = Color{90, 50, 20, 255};

            int num_duelas = 5;
            float duela_w = pw / num_duelas;
            for (int i = 0; i < num_duelas; ++i) {
                float dx = px + i * duela_w;
                Color col_duela = col_madera;
                if (i % 2 == 0) col_duela = ColorBrightness(col_madera, -0.08f);
                DrawRectangleRec({dx, py, duela_w, ph}, col_duela);
                DrawLineEx({dx, py}, {dx, py + ph}, 1.0f, col_borde);
            }
            DrawRectangleLinesEx({px, py, pw, ph}, 1.5f, col_borde);

            // Aros metálicos
            Color col_metal = Color{160, 170, 180, 255};
            Color col_metal_borde = Color{90, 100, 110, 255};
            
            // Aro superior
            DrawRectangleRec({px - 2.0f, py + 12.0f, pw + 4.0f, 6.0f}, col_metal);
            DrawRectangleLinesEx({px - 2.0f, py + 12.0f, pw + 4.0f, 6.0f}, 1.0f, col_metal_borde);

            // Aro central
            DrawRectangleRec({px - 3.0f, py + ph / 2.0f - 3.0f, pw + 6.0f, 6.0f}, col_metal);
            DrawRectangleLinesEx({px - 3.0f, py + ph / 2.0f - 3.0f, pw + 6.0f, 6.0f}, 1.0f, col_metal_borde);

            // Aro inferior
            DrawRectangleRec({px - 2.0f, py + ph - 18.0f, pw + 4.0f, 6.0f}, col_metal);
            DrawRectangleLinesEx({px - 2.0f, py + ph - 18.0f, pw + 4.0f, 6.0f}, 1.0f, col_metal_borde);

            // Dibujar agujero negro de la tapa superior
            DrawEllipse(static_cast<int>(px + pw / 2.0f), static_cast<int>(py + 2.0f), pw * 0.45f, 4.0f, BLACK);

            if (modo_debug) {
                DrawRectangleLines(static_cast<int>(px), static_cast<int>(py), static_cast<int>(pw), static_cast<int>(ph), GREEN);
            }
            return;
        }

        const ParedRectangular* p = dynamic_cast<const ParedRectangular*>(e);
        if (!p) return;

        Vector2D pos = p->get_posicion();
        int px = static_cast<int>(pos.x);
        int py = static_cast<int>(pos.y);
        int pw = static_cast<int>(p->get_ancho());
        int ph = static_cast<int>(p->get_alto());

        DrawRectangle(px, py, pw, ph, COLOR_PARED);
        DrawRectangleLines(px, py, pw, ph, COLOR_PARED_BORDE);
    }
    else if (forma == TipoForma::POLIGONO) {
        const Balancin* bal = dynamic_cast<const Balancin*>(e);
        if (bal) {
            Vector2D pos = bal->get_posicion();
            int px = static_cast<int>(pos.x);
            int py = static_cast<int>(pos.y);
            int largo = static_cast<int>(bal->get_largo());
            int espesor = static_cast<int>(bal->get_espesor());

            // 1. Dibujar soporte del pivot (triángulo metálico)
            DrawTriangle(
                {static_cast<float>(px), static_cast<float>(py)},
                {static_cast<float>(px - 16), static_cast<float>(py + 40)},
                {static_cast<float>(px + 16), static_cast<float>(py + 40)},
                DARKGRAY
            );
            DrawTriangleLines(
                {static_cast<float>(px), static_cast<float>(py)},
                {static_cast<float>(px - 16), static_cast<float>(py + 40)},
                {static_cast<float>(px + 16), static_cast<float>(py + 40)},
                GRAY
            );

            // 2. Dibujar la tabla giratoria (plank)
            Rectangle rec = { static_cast<float>(px), static_cast<float>(py), static_cast<float>(largo), static_cast<float>(espesor) };
            Vector2 origin = { static_cast<float>(largo / 2.0), static_cast<float>(espesor / 2.0) };
            float rot_deg = static_cast<float>(bal->get_angulo() * 180.0 / MathUtils::TIM_PI);

            // Madera ocre con relieve
            DrawRectanglePro(rec, origin, rot_deg, Color{190, 110, 50, 255});
            
            // Dibujar contorno ocre rotado usando las 4 esquinas del tablón
            double cos_a = std::cos(bal->get_angulo());
            double sin_a = std::sin(bal->get_angulo());
            double hl = largo / 2.0;
            double ht = espesor / 2.0;

            // Ejes locales rotados
            Vector2D dir_x(cos_a, sin_a);
            Vector2D dir_y(-sin_a, cos_a);

            // Calcular las 4 esquinas rotadas y trasladadas
            Vector2D c1 = pos - dir_x * hl - dir_y * ht;
            Vector2D c2 = pos + dir_x * hl - dir_y * ht;
            Vector2D c3 = pos + dir_x * hl + dir_y * ht;
            Vector2D c4 = pos - dir_x * hl + dir_y * ht;

            Color color_borde = Color{220, 140, 70, 255};
            DrawLineEx({(float)c1.x, (float)c1.y}, {(float)c2.x, (float)c2.y}, 1.5f, color_borde);
            DrawLineEx({(float)c2.x, (float)c2.y}, {(float)c3.x, (float)c3.y}, 1.5f, color_borde);
            DrawLineEx({(float)c3.x, (float)c3.y}, {(float)c4.x, (float)c4.y}, 1.5f, color_borde);
            DrawLineEx({(float)c4.x, (float)c4.y}, {(float)c1.x, (float)c1.y}, 1.5f, color_borde);

            // 3. Asientos rojos en los extremos
            Vector2 seat_l = {
                static_cast<float>(px - (largo / 2.0 - 5.0) * cos_a),
                static_cast<float>(py - (largo / 2.0 - 5.0) * sin_a)
            };
            DrawCircle(seat_l.x, seat_l.y, 6.0f, RED);
            DrawCircleLines(seat_l.x, seat_l.y, 6.0f, MAROON);

            Vector2 seat_r = {
                static_cast<float>(px + (largo / 2.0 - 5.0) * cos_a),
                static_cast<float>(py + (largo / 2.0 - 5.0) * sin_a)
            };
            DrawCircle(seat_r.x, seat_r.y, 6.0f, RED);
            DrawCircleLines(seat_r.x, seat_r.y, 6.0f, MAROON);

            // 4. Perno central negro/gris
            DrawCircle(px, py, 6, BLACK);
            DrawCircle(px, py, 4, LIGHTGRAY);
            return;
        }

        const PlanoInclinado* ramp = dynamic_cast<const PlanoInclinado*>(e);
        if (!ramp) return;

        const auto& verts = ramp->get_vertices();
        if (verts.size() >= 3) {
            Vector2 v1 = {static_cast<float>(verts[0].x), static_cast<float>(verts[0].y)};
            Vector2 v2 = {static_cast<float>(verts[1].x), static_cast<float>(verts[1].y)};
            Vector2 v3 = {static_cast<float>(verts[2].x), static_cast<float>(verts[2].y)};

            DrawTriangle(v1, v2, v3, COLOR_RAMPA);
            DrawTriangleLines(v1, v2, v3, COLOR_RAMPA_BORDE);
        }
    }
}

// ============================================================================
// Renderizado de debug (vectores de velocidad, hitboxes)
// ============================================================================
void dibujar_debug(const EntidadFisica* e) {
    if (!modo_debug || e->get_es_estatico()) return;

    Vector2D pos = e->get_posicion();
    Vector2D vel = e->get_velocidad();

    // Vector de velocidad (verde, escalado)
    double vel_scale = 0.15;
    DrawLineEx(
        {static_cast<float>(pos.x), static_cast<float>(pos.y)},
        {static_cast<float>(pos.x + vel.x * vel_scale),
         static_cast<float>(pos.y + vel.y * vel_scale)},
        2.0f, GREEN);

    // Velocidad numérica
    double speed = vel.magnitud();
    if (speed > 10.0) {
        DrawText(TextFormat("%.0f", speed),
                 static_cast<int>(pos.x + 15),
                 static_cast<int>(pos.y - 10), 10, GREEN);
    }
}

// ============================================================================
// HUD (información en pantalla)
// ============================================================================
void dibujar_hud(const MotorFisica& motor) {
    int y = 30;
    int margin = 30;

    DrawText(TextFormat("FPS: %d", GetFPS()), margin, y, 16, COLOR_HUD);
    DrawText(TextFormat("Entidades: %d", static_cast<int>(motor.get_entidades().size())),
             margin, y + 22, 16, COLOR_HUD);
    DrawText(TextFormat("Bolas: %d", contador_bolas), margin, y + 44, 16, COLOR_HUD);
    DrawText(TextFormat("Gravedad: %.0f px/s2", motor.get_gravedad().y),
             margin, y + 66, 16, COLOR_HUD);

    // Objeto seleccionado
    const char* obj_name = "BOLA";
    if (seleccion_objeto == 1) obj_name = "TRAMPOLIN";
    else if (seleccion_objeto == 2) obj_name = "BALANCIN";
    else if (seleccion_objeto == 3) obj_name = "FUTBOLISTA";
    else if (seleccion_objeto == 4) obj_name = "BARRIL CHAVO";
    else if (seleccion_objeto == 5) obj_name = "BOLA REBOTADORA";
    else if (seleccion_objeto == 6) obj_name = "VENTILADOR";

    char sel_text[64];
    sprintf(sel_text, "SELECCIONADO [1/2/3/4/5/6/7]: %s", obj_name);
    DrawText(sel_text, margin, y + 90, 16, Color{100, 200, 255, 255});

    // Indicador de pausa
    if (motor.get_pausado()) {
        int pause_w = MeasureText("|| PAUSADO", 30);
        DrawText("|| PAUSADO", ANCHO / 2 - pause_w / 2, ALTO / 2 - 15, 30,
                 Color{255, 255, 100, 200});
    }

    // Indicador de debug
    if (modo_debug) {
        DrawText("[DEBUG]", ANCHO - 100, y, 16, GREEN);
    }

    // Controles
    DrawText("[1/2/3/4/5/6/7] Cambiar Obj  [L-CLICK] Crear Obj  [SPACE] Pausa  [D] Debug  [R] Reset",
             margin, ALTO - 30, 14, COLOR_CONTROLES);
}

// ============================================================================
// Punto de entrada
// ============================================================================
int main() {
    // ---- Inicializar ventana ----
    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Anti-aliasing
    InitWindow(ANCHO, ALTO, "TIM - Motor de Fisica | Prototipo RK4 + Raylib");
    SetTargetFPS(60);

    // ---- Inicializar motor de física ----
    // dt_fijo = 1/120s (120 pasos físicos por segundo)
    // Gravedad = 500 px/s² hacia abajo (Y+ en pantalla)
    MotorFisica motor(1.0 / 120.0, Vector2D(0, 500.0));
    crear_escena(motor);

    // Animación de título (fade-out)
    float titulo_alpha = 1.0f;

    // ---- Bucle principal ----
    while (!WindowShouldClose()) {
        // ======== INPUT ========

        // Cambiar selección de objeto
        if (IsKeyPressed(KEY_ONE)) seleccion_objeto = 0;
        if (IsKeyPressed(KEY_TWO)) seleccion_objeto = 1;
        if (IsKeyPressed(KEY_THREE)) seleccion_objeto = 2;
        if (IsKeyPressed(KEY_FOUR)) seleccion_objeto = 3;
        if (IsKeyPressed(KEY_FIVE)) seleccion_objeto = 4;
        if (IsKeyPressed(KEY_SIX)) seleccion_objeto = 5;
        if (IsKeyPressed(KEY_SEVEN)) seleccion_objeto = 6;

        // Click izquierdo: arrastrar objeto existente o crear uno nuevo
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2D mouse_pos(GetMouseX(), GetMouseY());
            EntidadFisica* clicked = obtener_entidad_bajo_mouse(motor, mouse_pos);
            if (clicked) {
                entidad_arrastrada = clicked;
                offset_arrastre = clicked->get_posicion() - mouse_pos;
            } else {
                if (seleccion_objeto == 0) {
                    crear_bola(motor, mouse_pos);
                }
                else if (seleccion_objeto == 1) {
                    crear_trampolin(motor, mouse_pos);
                }
                else if (seleccion_objeto == 2) {
                    crear_balancin(motor, mouse_pos);
                }
                else if (seleccion_objeto == 3) {
                    crear_seguidor_booster(motor, mouse_pos);
                }
                else if (seleccion_objeto == 4) {
                    crear_barril_chavo(motor, mouse_pos);
                }
                else if (seleccion_objeto == 5) {
                    crear_bola_rebotadora(motor, mouse_pos);
                }
                else if (seleccion_objeto == 6) {
                    crear_ventilador(motor, mouse_pos);
                }
            }
        }

        // Mientras se mantiene presionado el click izquierdo: arrastrar objeto
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && entidad_arrastrada != nullptr) {
            Vector2D mouse_pos(GetMouseX(), GetMouseY());
            Vector2D nueva_pos = mouse_pos + offset_arrastre;

            // Clampear la posición dentro de los límites del escenario
            nueva_pos.x = std::max(30.0, std::min(double(ANCHO - 30.0), nueva_pos.x));
            nueva_pos.y = std::max(30.0, std::min(double(ALTO - 30.0), nueva_pos.y));

            entidad_arrastrada->set_posicion(nueva_pos);
            entidad_arrastrada->set_velocidad(Vector2D(0.0, 0.0));
            entidad_arrastrada->set_velocidad_angular(0.0);
        }

        // Al soltar el click izquierdo: liberar el objeto arrastrado
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            entidad_arrastrada = nullptr;
        }

        // Click derecho heredado: crear trampolín directamente por comodidad
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            Vector2D mouse_pos(GetMouseX(), GetMouseY());
            crear_trampolin(motor, mouse_pos);
        }

        // Spacebar: pausar/reanudar
        if (IsKeyPressed(KEY_SPACE)) {
            motor.set_pausado(!motor.get_pausado());
        }

        // D: toggle debug
        if (IsKeyPressed(KEY_D)) {
            modo_debug = !modo_debug;
        }

        // R: reiniciar escena
        if (IsKeyPressed(KEY_R)) {
            motor.limpiar();
            contador_bolas = 0;
            crear_escena(motor);
        }

        // +/-: ajustar gravedad
        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) {
            Vector2D g = motor.get_gravedad();
            motor.set_gravedad(Vector2D(g.x, g.y + 100.0));
        }
        if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) {
            Vector2D g = motor.get_gravedad();
            motor.set_gravedad(Vector2D(g.x, g.y - 100.0));
        }

        // ======== UPDATE ========
        motor.actualizar(GetFrameTime());

        // Fade-out del título
        if (titulo_alpha > 0.0f) {
            titulo_alpha -= 0.008f;
            if (titulo_alpha < 0.0f) titulo_alpha = 0.0f;
        }

        // Fade-out del error de spawn
        if (spawn_error_timer > 0.0f) {
            spawn_error_timer -= GetFrameTime();
        }

        // ======== RENDER ========
        BeginDrawing();
        ClearBackground(COLOR_FONDO);

        // Dibujar todas las entidades
        for (const auto* e : motor.get_entidades()) {
            dibujar_entidad(e);
            dibujar_debug(e);
        }

        // Feedback visual de spawn fallido (X roja parpadeante)
        if (spawn_error_timer > 0.0f) {
            unsigned char alpha = static_cast<unsigned char>(spawn_error_timer * 2.0f * 255);
            Color err_col = {255, 50, 50, alpha};
            int ex = static_cast<int>(spawn_error_pos.x);
            int ey = static_cast<int>(spawn_error_pos.y);
            DrawLine(ex - 10, ey - 10, ex + 10, ey + 10, err_col);
            DrawLine(ex + 10, ey - 10, ex - 10, ey + 10, err_col);
            DrawCircleLines(ex, ey, 14, err_col);
        }

        // HUD
        dibujar_hud(motor);

        // Splash de título (desaparece gradualmente)
        if (titulo_alpha > 0.01f) {
            unsigned char a = static_cast<unsigned char>(titulo_alpha * 255);
            int tw = MeasureText("TIM: Motor de Fisica", 40);
            DrawText("TIM: Motor de Fisica", ANCHO / 2 - tw / 2, ALTO / 2 - 50, 40,
                     Color{255, 255, 255, a});
            int sw = MeasureText("Prototipo v0.1 | RK4 + Raylib", 20);
            DrawText("Prototipo v0.1 | RK4 + Raylib", ANCHO / 2 - sw / 2, ALTO / 2, 20,
                     Color{180, 180, 200, a});
        }

        EndDrawing();
    }

    // ---- Cleanup ----
    CloseWindow();
    return 0;
}
