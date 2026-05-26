// ============================================================================
// TIM: Motor de Física — Prototipo Interactivo con Raylib
//
// Controles:
//   [Arrastrar]  Desde el menú lateral: soltar en el canvas para crear objetos
//   [Click Izq]  En el canvas: arrastrar objetos existentes
//   [TAB]        Mostrar / Ocultar menú lateral
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
#include "core/animacion.h"
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
#include "objetos/catalogo_menu.gen.h"

#include <cmath>
#include <vector>


// ============================================================================
// Configuración global
// ============================================================================
int ANCHO = 1920;
int ALTO  = 1080;
const int ANCHO_MIN = 640;
const int ALTO_MIN  = 480;

// Paredes del borde del nivel (se ajustan al redimensionar la ventana)
ParedRectangular* borde_suelo = nullptr;
ParedRectangular* borde_izquierda = nullptr;
ParedRectangular* borde_derecha = nullptr;
ParedRectangular* borde_techo = nullptr;

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

// Estado del sistema Drag & Drop (entidades ya en escena)
EntidadFisica* entidad_arrastrada = nullptr;
Vector2D offset_arrastre;

// Estado de selección (para eliminar objetos)
EntidadFisica* entidad_seleccionada = nullptr;

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

// ============================================================================
// Menú lateral derecho — UI y drag-and-drop desde paleta
// ============================================================================
const int MENU_ANCHO = 300;
const int MENU_PESTANA_ALTO = 70;
const int MENU_CATEGORIA_ALTO = 50;
const int MENU_CELDA = 72;
const int MENU_COLS = 2;
const int MENU_MARGEN = 10;
const int MENU_PAGINACION_ALTO = 36;

const Color MENU_FONDO        = {175, 180, 190, 255};
const Color MENU_FONDO_OSCURO = {145, 150, 162, 255};
const Color MENU_BORDE        = {110, 115, 128, 255};
const Color MENU_TEXTO        = {45,  50,  60,  255};
const Color MENU_AZUL         = {55,  130, 210, 255};
const Color MENU_AZUL_CLARO   = {90,  165, 235, 255};
const Color MENU_CELDA_FONDO  = {195, 200, 210, 255};
const Color MENU_INACTIVO     = {130, 135, 145, 180};

bool es_borde_nivel(const EntidadFisica* e);

bool menu_visible = true;
int menu_tab = 0;
int menu_pagina = 0;
bool cat_mecanicas_abierta = true;
bool cat_interactivos_abierta = false;
bool cat_decor_abierta = true;
TipoObjetoMenu arrastrando_spawn = TipoObjetoMenu::NINGUNO;

// Texturas
Texture2D tex_bola[3];  // Array de 3 texturas de pelota
Texture2D tex_fondo;
Texture2D tex_base_central;
Texture2D derecho;
Texture2D tex_barril;   // Textura del barril (parte del BarrilChavo)
Texture2D tex_chavo;    // Textura de El Chavo (personaje que sale)
Texture2D tex_seguidor_quieto;     // Sprite del personaje parado
Texture2D tex_seguidor_corriendo;  // Sprite del personaje corriendo

// Texturas adicionales para assets
Texture2D tex_trampolin;
Texture2D tex_balancin_base;
Texture2D tex_balancin_tabla;
Texture2D tex_plata_larga;
Texture2D tex_plata_peque;
Texture2D tex_plata_rampa_izq;
Texture2D tex_plata_rampa_der;

// Animaciones del SeguidorBooster
Animacion* anim_seguidor_corriendo = nullptr;

int ancho_area_juego() {
    return menu_visible ? (ANCHO - MENU_ANCHO) : ANCHO;
}

bool punto_en_menu(int mx, int my) {
    if (!menu_visible) return false;
    return mx >= ANCHO - MENU_ANCHO && mx < ANCHO && my >= 0 && my < ALTO;
}

bool punto_en_area_juego(int mx, int my) {
    return mx >= 0 && mx < ancho_area_juego() && my >= 0 && my < ALTO;
}

// Verificar si una entidad es un borde del nivel (no se debe arrastrar ni eliminar)
bool es_borde_nivel(const EntidadFisica* e) {
    return e == borde_suelo || e == borde_izquierda || e == borde_derecha || e == borde_techo;
}

// Obtener qué objeto está debajo del cursor del mouse (incluye estáticos)
EntidadFisica* obtener_entidad_bajo_mouse(const MotorFisica& motor, Vector2D mouse_pos) {
    for (auto* e : motor.get_entidades()) {
        // Excluir bordes del nivel del arrastre/selección
        if (es_borde_nivel(e)) continue;

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
            // ParedRectangular (plataformas, paredes colocadas por el usuario)
            const ParedRectangular* p = dynamic_cast<const ParedRectangular*>(e);
            if (p) {
                Vector2D pos = p->get_posicion();
                double w = p->get_ancho();
                double h = p->get_alto();
                if (mouse_pos.x >= pos.x - 10 && mouse_pos.x <= pos.x + w + 10 &&
                    mouse_pos.y >= pos.y - 10 && mouse_pos.y <= pos.y + h + 10) {
                    return const_cast<ParedRectangular*>(p);
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
            // PlanoInclinado (rampas)
            const PlanoInclinado* ramp = dynamic_cast<const PlanoInclinado*>(e);
            if (ramp) {
                const auto& verts = ramp->get_vertices();
                if (verts.size() >= 3) {
                    // Bounding box de los vértices con margen
                    double min_x = verts[0].x, max_x = verts[0].x;
                    double min_y = verts[0].y, max_y = verts[0].y;
                    for (const auto& v : verts) {
                        if (v.x < min_x) min_x = v.x;
                        if (v.x > max_x) max_x = v.x;
                        if (v.y < min_y) min_y = v.y;
                        if (v.y > max_y) max_y = v.y;
                    }
                    if (mouse_pos.x >= min_x - 10 && mouse_pos.x <= max_x + 10 &&
                        mouse_pos.y >= min_y - 10 && mouse_pos.y <= max_y + 10) {
                        return const_cast<PlanoInclinado*>(ramp);
                    }
                }
            }
        }
    }
    return nullptr;
}

// ============================================================================
// Crear escena inicial
// ============================================================================
void actualizar_bordes_nivel() {
    const int g = 20;
    if (!borde_suelo) return;

    borde_suelo->set_posicion(Vector2D(0, ALTO - g));
    borde_suelo->set_dimensiones(ANCHO, g);

    borde_izquierda->set_posicion(Vector2D(0, 0));
    borde_izquierda->set_dimensiones(g, ALTO);

    borde_derecha->set_posicion(Vector2D(ANCHO - g, 0));
    borde_derecha->set_dimensiones(g, ALTO);

    borde_techo->set_posicion(Vector2D(0, 0));
    borde_techo->set_dimensiones(ANCHO, g);
}

void sincronizar_tamano_ventana() {
    if (!IsWindowResized()) return;
    ANCHO = GetScreenWidth();
    ALTO = GetScreenHeight();
    actualizar_bordes_nivel();
}

void crear_escena(MotorFisica& motor) {
    int g = 20; // Grosor de paredes límite

    // ---- Paredes del nivel (4 bordes) ----
    borde_suelo = new ParedRectangular(
        motor.generar_id(), Vector2D(0, ALTO - g), ANCHO, g);
    motor.agregar_entidad(borde_suelo);

    borde_izquierda = new ParedRectangular(
        motor.generar_id(), Vector2D(0, 0), g, ALTO);
    motor.agregar_entidad(borde_izquierda);

    borde_derecha = new ParedRectangular(
        motor.generar_id(), Vector2D(ANCHO - g, 0), g, ALTO);
    motor.agregar_entidad(borde_derecha);

    borde_techo = new ParedRectangular(
        motor.generar_id(), Vector2D(0, 0), ANCHO, g);
    motor.agregar_entidad(borde_techo);

    // ---- Plataformas ----
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(80, 320), 220, 15));               // Plataforma superior izq
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(550, 260), 230, 15));              // Plataforma superior der
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(350, 550), 200, 15));              // Plataforma inferior centro

    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(20, ALTO - 150), 1900, 10));      // Sidebar inferior
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
        motor.generar_id(), Vector2D(420, 470), 90.0, 120.0));

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
    double radio = 35.0 + GetRandomValue(0, 8);   // Radio entre 8-16 px

    // Verificar que no colisiona con nada existente
    if (!posicion_valida_para_bola(motor, pos, radio)) {
        spawn_error_timer = 0.5f;  // Flash rojo de 0.5s
        spawn_error_pos = pos;
        return false;
    }

    double masa = radio * radio * 0.01;           // Proporcional al área
    Bola* b = new Bola(motor.generar_id(), pos, radio, masa);
    b->set_color_idx(contador_bolas % NUM_COLORES);
    b->set_texture_idx(GetRandomValue(0, 2)); 
    contador_bolas++;
    motor.agregar_entidad(b);
    return true;
}

bool crear_rampa(MotorFisica& motor, Vector2D pos, bool invertido) {
    double b = 160.0;
    double h = 120.0;
    Vector2D spawn(pos.x - b / 2.0, pos.y - h / 2.0);
    motor.agregar_entidad(new PlanoInclinado(motor.generar_id(), spawn, b, h, invertido));
    return true;
}

bool crear_plataforma(MotorFisica& motor, Vector2D pos, double w, double h) {
    Vector2D spawn(pos.x - w / 2.0, pos.y - h / 2.0);
    motor.agregar_entidad(new ParedRectangular(motor.generar_id(), spawn, w, h));
    return true;
}

bool crear_bola_rebotadora(MotorFisica& motor, Vector2D pos);
bool crear_ventilador(MotorFisica& motor, Vector2D pos);
bool crear_seguidor_booster(MotorFisica& motor, Vector2D pos);
bool crear_barril_chavo(MotorFisica& motor, Vector2D pos);

bool spawn_desde_menu(MotorFisica& motor, TipoObjetoMenu tipo, Vector2D pos) {
    switch (tipo) {
        case TipoObjetoMenu::BOLA:              return crear_bola(motor, pos);
        case TipoObjetoMenu::BOLA_REBOTADORA:   return crear_bola_rebotadora(motor, pos);
        case TipoObjetoMenu::TRAMPOLIN:         return crear_trampolin(motor, pos);
        case TipoObjetoMenu::BALANCIN:          return crear_balancin(motor, pos);
        case TipoObjetoMenu::RAMPA_IZQUIERDA:   return crear_rampa(motor, pos, false);
        case TipoObjetoMenu::RAMPA_DERECHA:     return crear_rampa(motor, pos, true);
        case TipoObjetoMenu::PLATAFORMA:        return crear_plataforma(motor, pos, 150.0, 15.0);
        case TipoObjetoMenu::PARED_LARGA:       return crear_plataforma(motor, pos, 80.0, 120.0);
        case TipoObjetoMenu::PLATAFORMA_DECOR:   return crear_plataforma(motor, pos, 120.0, 20.0);
        case TipoObjetoMenu::SEGUIDOR_BOOSTER:  return crear_seguidor_booster(motor, pos);
        case TipoObjetoMenu::BARRIL_CHAVO:      return crear_barril_chavo(motor, pos);
        case TipoObjetoMenu::VENTILADOR:        return crear_ventilador(motor, pos);
        default: return false;
    }
}

// ============================================================================
// Íconos del menú (miniaturas para grid y ghost)
// ============================================================================
void dibujar_icono_objeto(TipoObjetoMenu tipo, float cx, float cy, float escala, unsigned char alpha) {
    auto tint = [alpha](Color c) {
        return Color{ c.r, c.g, c.b, static_cast<unsigned char>(
            static_cast<int>(c.a) * alpha / 255) };
    };

    switch (tipo) {
        case TipoObjetoMenu::BOLA: {
            float r = 14.0f * escala;
            Color col = tint(PALETA_BOLAS[contador_bolas % NUM_COLORES]);
            DrawCircle(static_cast<int>(cx), static_cast<int>(cy), r, col);
            DrawCircleLines(static_cast<int>(cx), static_cast<int>(cy), r, tint(DARKGRAY));
            break;
        }
        case TipoObjetoMenu::TRAMPOLIN: {
            if (tex_trampolin.id > 0) {
                float w = 50.0f * escala;
                float h = 16.0f * escala;
                float frame_w = (float)tex_trampolin.width / 4.0f;
                DrawTexturePro(
                    tex_trampolin,
                    { 0, 0, frame_w, (float)tex_trampolin.height },
                    { cx - w / 2.0f, cy - h / 2.0f, w, h },
                    { 0, 0 },
                    0.0f,
                    tint(WHITE)
                );
            } else {
                float w = 44.0f * escala;
                float h = 12.0f * escala;
                float px = cx - w / 2.0f;
                float py = cy - h / 2.0f;
                DrawLineEx({px + 4, py + h}, {px + 10, py + 4}, 2.0f, tint(DARKGRAY));
                DrawLineEx({px + w - 4, py + h}, {px + w - 10, py + 4}, 2.0f, tint(DARKGRAY));
                DrawLineEx({px + 8, py + 2}, {px + w / 2, py + 5}, 4.0f, tint(RED));
                DrawLineEx({px + w / 2, py + 5}, {px + w - 8, py + 2}, 4.0f, tint(RED));
            }
            break;
        }
        case TipoObjetoMenu::BALANCIN: {
            if (tex_balancin_base.id > 0 && tex_balancin_tabla.id > 0) {
                float base_w = 16.0f * escala;
                float base_h = 20.0f * escala;
                DrawTexturePro(
                    tex_balancin_base,
                    { 0, 0, (float)tex_balancin_base.width, (float)tex_balancin_base.height },
                    { cx - base_w / 2.0f, cy - 2.0f * escala, base_w, base_h },
                    { 0, 0 },
                    0.0f,
                    tint(WHITE)
                );

                float largo = 55.0f * escala;
                float esp = 15.0f * escala;
                DrawTexturePro(
                    tex_balancin_tabla,
                    { 0, 0, (float)tex_balancin_tabla.width, (float)tex_balancin_tabla.height },
                    { cx, cy - esp / 2.0f, largo, esp },
                    { largo / 2.0f, esp / 2.0f },
                    0.0f,
                    tint(WHITE)
                );
            } else {
                float largo = 50.0f * escala;
                float esp = 5.0f * escala;
                DrawTriangle(
                    {cx, cy - 8 * escala},
                    {cx - 10 * escala, cy + 14 * escala},
                    {cx + 10 * escala, cy + 14 * escala},
                    tint(DARKGRAY));
                DrawRectangleRec(
                    {cx - largo / 2, cy - esp / 2, largo, esp},
                    tint(Color{190, 110, 50, 255}));
                DrawCircle(static_cast<int>(cx), static_cast<int>(cy), 4 * escala, tint(BLACK));
                DrawCircle(static_cast<int>(cx - largo / 2 + 6), static_cast<int>(cy), 5 * escala, tint(RED));
                DrawCircle(static_cast<int>(cx + largo / 2 - 6), static_cast<int>(cy), 5 * escala, tint(RED));
            }
            break;
        }
        case TipoObjetoMenu::RAMPA_IZQUIERDA: {
            if (tex_plata_rampa_izq.id > 0) {
                float w = 44.0f * escala;
                float h = 44.0f * escala;
                DrawTexturePro(
                    tex_plata_rampa_izq,
                    { 0, 0, (float)tex_plata_rampa_izq.width, (float)tex_plata_rampa_izq.height },
                    { cx - w / 2.0f, cy - h / 2.0f, w, h },
                    { 0, 0 },
                    0.0f,
                    tint(WHITE)
                );
            } else {
                float s = 22.0f * escala;
                Vector2 v1 = {cx - s, cy - s};
                Vector2 v2 = {cx - s, cy + s};
                Vector2 v3 = {cx + s, cy + s};
                DrawTriangle(v1, v2, v3, tint(COLOR_RAMPA));
                DrawTriangleLines(v1, v2, v3, tint(COLOR_RAMPA_BORDE));
            }
            break;
        }
        case TipoObjetoMenu::RAMPA_DERECHA: {
            if (tex_plata_rampa_der.id > 0) {
                float w = 44.0f * escala;
                float h = 44.0f * escala;
                DrawTexturePro(
                    tex_plata_rampa_der,
                    { 0, 0, (float)tex_plata_rampa_der.width, (float)tex_plata_rampa_der.height },
                    { cx - w / 2.0f, cy - h / 2.0f, w, h },
                    { 0, 0 },
                    0.0f,
                    tint(WHITE)
                );
            } else {
                float s = 22.0f * escala;
                Vector2 v1 = {cx - s, cy + s};
                Vector2 v2 = {cx + s, cy + s};
                Vector2 v3 = {cx + s, cy - s};
                DrawTriangle(v1, v2, v3, tint(COLOR_RAMPA));
                DrawTriangleLines(v1, v2, v3, tint(COLOR_RAMPA_BORDE));
            }
            break;
        }
        case TipoObjetoMenu::PLATAFORMA:
        case TipoObjetoMenu::PLATAFORMA_DECOR:
        case TipoObjetoMenu::PARED_LARGA: {
            float w = 44.0f * escala;
            float h = 8.0f * escala;
            if (tipo == TipoObjetoMenu::PARED_LARGA) { w = 18.0f * escala; h = 40.0f * escala; }
            if (tipo == TipoObjetoMenu::PLATAFORMA_DECOR) { w = 36.0f * escala; h = 10.0f * escala; }

            bool drawn = false;
            if (tipo == TipoObjetoMenu::PARED_LARGA) {
                if (tex_plata_peque.id > 0) {
                    DrawTexturePro(
                        tex_plata_peque,
                        { 0, 0, (float)tex_plata_peque.width, (float)tex_plata_peque.height },
                        { cx - w / 2.0f, cy - h / 2.0f, w, h },
                        { 0, 0 },
                        0.0f,
                        tint(WHITE)
                    );
                    drawn = true;
                }
            } else {
                if (tex_plata_larga.id > 0) {
                    DrawTexturePro(
                        tex_plata_larga,
                        { 0, 0, (float)tex_plata_larga.width, (float)tex_plata_larga.height },
                        { cx - w / 2.0f, cy - h / 2.0f, w, h },
                        { 0, 0 },
                        0.0f,
                        tint(WHITE)
                    );
                    drawn = true;
                }
            }

            if (!drawn) {
                DrawRectangleRec({cx - w / 2.0f, cy - h / 2.0f, w, h}, tint(COLOR_PARED));
                DrawRectangleLinesEx({cx - w / 2.0f, cy - h / 2.0f, w, h}, 1.0f, tint(COLOR_PARED_BORDE));
            }
            break;
        }
        case TipoObjetoMenu::BOLA_REBOTADORA: {
            float r = 18.0f * escala;
            DrawCircle(static_cast<int>(cx), static_cast<int>(cy), r, tint(Color{218, 48, 42, 255}));
            DrawCircleLines(static_cast<int>(cx), static_cast<int>(cy), r, tint(MAROON));
            break;
        }
        case TipoObjetoMenu::SEGUIDOR_BOOSTER: {
            DrawRectangleRec({cx - 8 * escala, cy - 16 * escala, 16 * escala,28 * escala}, tint(SKYBLUE));
            DrawCircle(static_cast<int>(cx), static_cast<int>(cy - 14 * escala), 6 * escala, tint(Color{255, 220, 180, 255}));
            break;
        }
        case TipoObjetoMenu::BARRIL_CHAVO: {
            float w = 28.0f * escala;
            float h = 36.0f * escala;
            DrawRectangleRec({cx - w / 2, cy - h / 2 + 6 * escala, w, h},
                             tint(Color{139, 90, 43, 255}));
            DrawCircle(static_cast<int>(cx), static_cast<int>(cy - h / 2 + 2 * escala),
                       8 * escala, tint(Color{255, 220, 180, 255}));
            break;
        }
        case TipoObjetoMenu::VENTILADOR: {
            float w = 28.0f * escala;
            float h = 22.0f * escala;
            DrawRectangleRec({cx - w / 2, cy - h / 2, w, h}, tint(Color{70, 84, 96, 255}));
            DrawCircleLines(static_cast<int>(cx + w / 2), static_cast<int>(cy), 8 * escala,
                            tint(Color{190, 210, 220, 255}));
            break;
        }
        default:
            DrawRectangleRec({cx - 16, cy - 16, 32, 32}, tint(LIGHTGRAY));
            DrawText("?", static_cast<int>(cx - 4), static_cast<int>(cy - 8), 16, tint(DARKGRAY));
            break;
    }
}

struct RectCeldaMenu {
    Rectangle rect;
    TipoObjetoMenu tipo;
    bool disponible;
};

static std::vector<RectCeldaMenu> celdas_menu_cache;

void recolectar_items_visibles(int tab, int pagina, int categoria,
                               std::vector<const ItemCatalogo*>& out) {
    out.clear();
    for (int i = 0; i < CATALOGO_MENU_COUNT; ++i) {
        const ItemCatalogo& it = CATALOGO_MENU[i];
        if (it.tab == tab && it.pagina == pagina && it.categoria == categoria)
            out.push_back(&it);
    }
}

int contar_paginas_tab(int tab) {
    int max_pag = 0;
    for (int i = 0; i < CATALOGO_MENU_COUNT; ++i) {
        if (CATALOGO_MENU[i].tab == tab)
            max_pag = std::max(max_pag, CATALOGO_MENU[i].pagina);
    }
    return max_pag + 1;
}

void layout_celdas_categoria(int panel_x, int y_inicio, int categoria,
                             std::vector<RectCeldaMenu>& celdas) {
    celdas.clear();
    std::vector<const ItemCatalogo*> items;
    int cat_idx = (menu_tab == 0) ? categoria : 0;
    recolectar_items_visibles(menu_tab, menu_pagina, cat_idx, items);

    int x0 = panel_x + MENU_MARGEN;
    int y = y_inicio;
    for (size_t i = 0; i < items.size(); ++i) {
        int col = static_cast<int>(i) % MENU_COLS;
        int row = static_cast<int>(i) / MENU_COLS;
        int cx = x0 + col * (MENU_CELDA + 6);
        int cy = y + row * (MENU_CELDA + 6);
        Rectangle r = {
            static_cast<float>(cx),
            static_cast<float>(cy),
            static_cast<float>(MENU_CELDA),
            static_cast<float>(MENU_CELDA)
        };
        celdas.push_back({ r, items[i]->tipo, items[i]->disponible });
    }
}

TipoObjetoMenu tipo_en_celda(int mx, int my, const std::vector<RectCeldaMenu>& celdas) {
    for (const auto& c : celdas) {
        if (CheckCollisionPointRec({static_cast<float>(mx), static_cast<float>(my)}, c.rect)
            && c.disponible && c.tipo != TipoObjetoMenu::NINGUNO) {
            return c.tipo;
        }
    }
    return TipoObjetoMenu::NINGUNO;
}

void dibujar_celda_menu(const RectCeldaMenu& celda, bool resaltada) {
    Color fondo = resaltada ? MENU_AZUL_CLARO : MENU_CELDA_FONDO;
    if (!celda.disponible) fondo = ColorAlpha(MENU_CELDA_FONDO, 0.5f);

    DrawRectangleRounded(celda.rect, 0.08f, 6, fondo);
    DrawRectangleRoundedLinesEx(celda.rect, 0.08f, 6, 1.0f,
        celda.disponible ? MENU_BORDE : MENU_INACTIVO);

    float cx = celda.rect.x + celda.rect.width / 2.0f;
    float cy = celda.rect.y + celda.rect.height / 2.0f - 4.0f;
    unsigned char alpha = celda.disponible ? 255 : 120;

    if (celda.tipo != TipoObjetoMenu::NINGUNO)
        dibujar_icono_objeto(celda.tipo, cx, cy, 1.0f, alpha);
    else
        dibujar_icono_objeto(TipoObjetoMenu::NINGUNO, cx, cy, 1.0f, alpha);

    DrawText("1",
        static_cast<int>(celda.rect.x + celda.rect.width / 2 - 4),
        static_cast<int>(celda.rect.y + celda.rect.height - 18),
        12, celda.disponible ? MENU_AZUL : MENU_INACTIVO);
}

void dibujar_encabezado_categoria(int panel_x, int y, const char* titulo, bool abierta, int& out_alto) {
    Rectangle hdr = {
        static_cast<float>(panel_x + MENU_MARGEN),
        static_cast<float>(y),
        static_cast<float>(MENU_ANCHO - 2 * MENU_MARGEN),
        static_cast<float>(MENU_CATEGORIA_ALTO)
    };
    DrawRectangleRounded(hdr, 0.06f, 4, MENU_FONDO_OSCURO);
    DrawText(titulo, static_cast<int>(hdr.x + 8), static_cast<int>(hdr.y + 8), 14, MENU_TEXTO);
    const char* chevron = abierta ? "v" : ">";
    int cw = MeasureText(chevron, 14);
    DrawText(chevron, static_cast<int>(hdr.x + hdr.width - cw - 10),
             static_cast<int>(hdr.y + 8), 14, MENU_AZUL);
    out_alto = MENU_CATEGORIA_ALTO + 4;
}

bool click_en_rect(int mx, int my, Rectangle r) {
    return CheckCollisionPointRec({static_cast<float>(mx), static_cast<float>(my)}, r);
}

void reconstruir_celdas_menu() {
    celdas_menu_cache.clear();
    if (!menu_visible) return;

    int px = ANCHO - MENU_ANCHO;
    int y = MENU_PESTANA_ALTO + 36;

    if (menu_tab == 0) {
        y += MENU_CATEGORIA_ALTO + 4;
        if (cat_mecanicas_abierta) {
            std::vector<RectCeldaMenu> celdas;
            layout_celdas_categoria(px, y, 0, celdas);
            for (const auto& c : celdas) celdas_menu_cache.push_back(c);
            int filas = (static_cast<int>(celdas.size()) + MENU_COLS - 1) / MENU_COLS;
            y += filas * (MENU_CELDA + 6) + 8;
        }
        y += MENU_CATEGORIA_ALTO + 4;
        if (cat_interactivos_abierta) {
            std::vector<RectCeldaMenu> celdas;
            layout_celdas_categoria(px, y, 1, celdas);
            for (const auto& c : celdas) celdas_menu_cache.push_back(c);
        }
    } else {
        y += MENU_CATEGORIA_ALTO + 4;
        if (cat_decor_abierta) {
            std::vector<RectCeldaMenu> celdas;
            layout_celdas_categoria(px, y, 0, celdas);
            for (const auto& c : celdas) celdas_menu_cache.push_back(c);
        }
    }
}

void dibujar_menu_lateral() {
    if (!menu_visible) {
        int bx = ANCHO - 26;
        DrawRectangle(bx, ALTO / 2 - 40, 24, 80, MENU_FONDO_OSCURO);
        DrawRectangleLines(bx, ALTO / 2 - 40, 24, 80, MENU_BORDE);
        DrawText("<", bx + 7, ALTO / 2 - 8, 18, MENU_AZUL);
        return;
    }

    int px = ANCHO - MENU_ANCHO;
    
    // Dibujar asset del sidebar derecho si está cargado
    if (derecho.id > 0) {
        float escala_x = static_cast<float>(MENU_ANCHO) / derecho.width;
        float escala_y = static_cast<float>(ALTO) / derecho.height;
        float escala = (escala_x < escala_y) ? escala_x : escala_y;
        DrawTextureEx(derecho, {static_cast<float>(px)-5, 0}, 0, escala, WHITE);
    } else {
        // Fallback si no se carga la textura
        DrawRectangle(px, 0, MENU_ANCHO, ALTO, MENU_FONDO);
    }
    DrawLine(px, 0, px, ALTO, MENU_BORDE);

    // Pestañas OBJETOS / DECORACIÓN
    int tab_w = (MENU_ANCHO - 2 * MENU_MARGEN) / 2;
    Rectangle tab_obj = { static_cast<float>(px + MENU_MARGEN), 8.0f,
                          static_cast<float>(tab_w), static_cast<float>(MENU_PESTANA_ALTO - 8) };
    Rectangle tab_dec = { tab_obj.x + tab_w, tab_obj.y, tab_obj.width, tab_obj.height };

    DrawRectangleRec(tab_obj, menu_tab == 0 ? WHITE : ColorAlpha(WHITE, 0.4f));
    DrawRectangleRec(tab_dec, menu_tab == 1 ? WHITE : ColorAlpha(WHITE, 0.4f));
    DrawText("OBJETOS", static_cast<int>(tab_obj.x + 12), static_cast<int>(tab_obj.y + 10), 14, MENU_TEXTO);
    DrawText("DECORACION", static_cast<int>(tab_dec.x + 4), static_cast<int>(tab_dec.y + 10), 13, MENU_TEXTO);
    if (menu_tab == 0)
        DrawRectangle(static_cast<int>(tab_obj.x), static_cast<int>(tab_obj.y + tab_obj.height - 3), tab_w, 3, MENU_AZUL);
    else
        DrawRectangle(static_cast<int>(tab_dec.x), static_cast<int>(tab_dec.y + tab_dec.height - 3), tab_w, 3, MENU_AZUL);

    // Botón colapsar
    DrawRectangle(px + 4, MENU_PESTANA_ALTO + 4, 28, 24, MENU_FONDO_OSCURO);
    DrawText(">>", px + 10, MENU_PESTANA_ALTO + 8, 14, MENU_AZUL);

    int y = MENU_PESTANA_ALTO + 36;
    int hdr_h = 0;

    if (menu_tab == 0) {
        dibujar_encabezado_categoria(px, y, "MECANICAS", cat_mecanicas_abierta, hdr_h);
        y += hdr_h;
        if (cat_mecanicas_abierta) {
            std::vector<RectCeldaMenu> celdas;
            layout_celdas_categoria(px, y, 0, celdas);
            int filas = (static_cast<int>(celdas.size()) + MENU_COLS - 1) / MENU_COLS;
            y += filas * (MENU_CELDA + 6) + 8;
        }
        dibujar_encabezado_categoria(px, y, "ELEMENTOS INTERACTIVOS", cat_interactivos_abierta, hdr_h);
    } else {
        dibujar_encabezado_categoria(px, y, "DECORACION", cat_decor_abierta, hdr_h);
    }

    reconstruir_celdas_menu();
    Vector2 mouse = GetMousePosition();
    for (const auto& c : celdas_menu_cache) {
        bool hot = (arrastrando_spawn == c.tipo &&
            CheckCollisionPointRec(mouse, c.rect));
        dibujar_celda_menu(c, hot);
    }

    // Paginación
    int paginas = contar_paginas_tab(menu_tab);
    int py = ALTO - MENU_PAGINACION_ALTO - 180;  // 10px de margen desde el fondo del asset
    // Flechas de paginación (más grandes para fácil clic)
    int flecha_w = 80;
    int flecha_h = 50;
    bool hover_prev = CheckCollisionPointRec(mouse, Rectangle{static_cast<float>(px + MENU_MARGEN), static_cast<float>(py), static_cast<float>(flecha_w), static_cast<float>(flecha_h)});
    bool hover_next = CheckCollisionPointRec(mouse, Rectangle{static_cast<float>(px + MENU_ANCHO - MENU_MARGEN - flecha_w), static_cast<float>(py), static_cast<float>(flecha_w), static_cast<float>(flecha_h)});

    DrawRectangleRounded(
        {static_cast<float>(px + MENU_MARGEN), static_cast<float>(py), static_cast<float>(flecha_w), static_cast<float>(flecha_h)},
        0.15f, 4, hover_prev ? Color{110, 125, 145, 230} : Color{80, 90, 105, 180});
    DrawText("<", px + MENU_MARGEN + flecha_w / 2 - 6, py + flecha_h / 2 - 9, 20, hover_prev ? MENU_AZUL_CLARO : MENU_AZUL);

    char pag_txt[32];
    snprintf(pag_txt, sizeof(pag_txt), "PAGINA %d / %d", menu_pagina + 1, paginas);
    int tw = MeasureText(pag_txt, 12);
    DrawText(pag_txt, px + MENU_ANCHO / 2 - tw / 2, py + flecha_h / 2 - 6, 12, MENU_TEXTO);

    DrawRectangleRounded(
        {static_cast<float>(px + MENU_ANCHO - MENU_MARGEN - flecha_w), static_cast<float>(py), static_cast<float>(flecha_w), static_cast<float>(flecha_h)},
        0.15f, 4, hover_next ? Color{110, 125, 145, 230} : Color{80, 90, 105, 180});
    DrawText(">", px + MENU_ANCHO - MENU_MARGEN - flecha_w / 2 - 6, py + flecha_h / 2 - 9, 20, hover_next ? MENU_AZUL_CLARO : MENU_AZUL);
}

bool manejar_click_menu(int mx, int my) {
    if (!menu_visible) {
        Rectangle btn_abrir = { static_cast<float>(ANCHO - 26), static_cast<float>(ALTO / 2 - 40), 24, 80 };
        if (click_en_rect(mx, my, btn_abrir)) {
            menu_visible = true;
            return true;
        }
        return false;
    }

    int px = ANCHO - MENU_ANCHO;
    int tab_w = (MENU_ANCHO - 2 * MENU_MARGEN) / 2;

    Rectangle tab_obj = { static_cast<float>(px + MENU_MARGEN), 8.0f,
                          static_cast<float>(tab_w), static_cast<float>(MENU_PESTANA_ALTO - 8) };
    Rectangle tab_dec = { tab_obj.x + tab_w, tab_obj.y, tab_obj.width, tab_obj.height };
    if (click_en_rect(mx, my, tab_obj)) { menu_tab = 0; menu_pagina = 0; return true; }
    if (click_en_rect(mx, my, tab_dec)) { menu_tab = 1; menu_pagina = 0; return true; }

    Rectangle btn_cerrar = { static_cast<float>(px + 4), static_cast<float>(MENU_PESTANA_ALTO + 4), 28, 24 };
    if (click_en_rect(mx, my, btn_cerrar)) { menu_visible = false; return true; }

    int y = MENU_PESTANA_ALTO + 36;
    if (menu_tab == 0) {
        Rectangle hdr_mec = { static_cast<float>(px + MENU_MARGEN), static_cast<float>(y),
            static_cast<float>(MENU_ANCHO - 2 * MENU_MARGEN), static_cast<float>(MENU_CATEGORIA_ALTO) };
        if (click_en_rect(mx, my, hdr_mec)) { cat_mecanicas_abierta = !cat_mecanicas_abierta; return true; }
        y += MENU_CATEGORIA_ALTO + 4;
        if (cat_mecanicas_abierta) {
            std::vector<RectCeldaMenu> celdas;
            layout_celdas_categoria(px, y, 0, celdas);
            int filas = (static_cast<int>(celdas.size()) + MENU_COLS - 1) / MENU_COLS;
            y += filas * (MENU_CELDA + 6) + 8;
        }
        Rectangle hdr_int = { static_cast<float>(px + MENU_MARGEN), static_cast<float>(y),
            static_cast<float>(MENU_ANCHO - 2 * MENU_MARGEN), static_cast<float>(MENU_CATEGORIA_ALTO) };
        if (click_en_rect(mx, my, hdr_int)) { cat_interactivos_abierta = !cat_interactivos_abierta; return true; }
    } else {
        Rectangle hdr_dec = { static_cast<float>(px + MENU_MARGEN), static_cast<float>(y),
            static_cast<float>(MENU_ANCHO - 2 * MENU_MARGEN), static_cast<float>(MENU_CATEGORIA_ALTO) };
        if (click_en_rect(mx, my, hdr_dec)) { cat_decor_abierta = !cat_decor_abierta; return true; }
    }

    int py = ALTO - MENU_PAGINACION_ALTO - 180; 
    int flecha_w = 80;
    int flecha_h = 50;
    Rectangle btn_prev = { static_cast<float>(px + MENU_MARGEN), static_cast<float>(py), static_cast<float>(flecha_w), static_cast<float>(flecha_h) };
    Rectangle btn_next = { static_cast<float>(px + MENU_ANCHO - MENU_MARGEN - flecha_w), static_cast<float>(py), static_cast<float>(flecha_w), static_cast<float>(flecha_h) };
    int paginas = contar_paginas_tab(menu_tab);
    if (click_en_rect(mx, my, btn_prev) && menu_pagina > 0) { menu_pagina--; return true; }
    if (click_en_rect(mx, my, btn_next) && menu_pagina < paginas - 1) { menu_pagina++; return true; }

    return false;
}

void dibujar_ghost_spawn() {
    if (arrastrando_spawn == TipoObjetoMenu::NINGUNO) return;
    Vector2 mp = GetMousePosition();
    if (!punto_en_area_juego(static_cast<int>(mp.x), static_cast<int>(mp.y))) return;
    dibujar_icono_objeto(arrastrando_spawn, mp.x, mp.y, 1.35f, 140);
    DrawCircleLines(static_cast<int>(mp.x), static_cast<int>(mp.y), 18,
                    Color{255, 255, 255, 80});
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
    double w = 42.0*2;
    double h = 54.0*2;

    Vector2D spawn_pos(pos.x - w / 2.0, pos.y - h / 2.0);
    Ventilador* v = new Ventilador(motor.generar_id(), spawn_pos, w, h);
    motor.agregar_entidad(v);
    return true;
}

// ============================================================================
// Crear futbolista seguidor en la posición del mouse
// ============================================================================
bool crear_seguidor_booster(MotorFisica& motor, Vector2D pos) {
    SeguidorBooster* s = new SeguidorBooster(motor.generar_id(), pos, 60.0, 120.0);
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
    double w = 90.0;
    double h = 120.0;
    
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

// Función helper para dibujar geometría del SeguidorBooster (fallback)
void dibuja_seguidor_geometrico(Vector2D pos, float w, float h, float draw_y, 
                                 EstadoSeguidor estado, Vector2D pos_init,
                                 const SeguidorBooster* seg) {
    float r = w * 0.75f;
    double ang_pierna = seg->get_angulo_pierna();
    double dir_carr = seg->get_direccion_carrera();
    double kick_f = seg->get_kicker_factor();

    // Pierna 1
    float leg1_ang = static_cast<float>(ang_pierna);
    float lx1 = pos.x - dir_carr * w * 0.3f + std::sin(leg1_ang) * r * 0.7f;
    float ly1 = draw_y + h / 2.0f - r * 0.2f + std::cos(leg1_ang) * r * 0.5f;
    DrawLineEx({(float)(pos.x - dir_carr * w * 0.2f), (float)(draw_y + h / 2.0f - r * 0.4f)}, {lx1, ly1}, 4.0f, DARKGRAY);
    DrawCircle(lx1, ly1, 3.0f, RED);

    // Pierna 2
    float leg2_ang = static_cast<float>(-ang_pierna + kick_f * dir_carr * 1.5);
    float lx2 = pos.x + dir_carr * w * 0.3f + std::sin(leg2_ang) * r * 0.7f;
    float ly2 = draw_y + h / 2.0f - r * 0.2f + std::cos(leg2_ang) * r * 0.5f;
    DrawLineEx({(float)(pos.x + dir_carr * w * 0.2f), (float)(draw_y + h / 2.0f - r * 0.4f)}, {lx2, ly2}, 4.0f, DARKGRAY);
    DrawCircle(lx2, ly2, 3.0f, RED);

    // Cuerpo
    int cx = static_cast<int>(pos.x);
    int cy = static_cast<int>(draw_y + h * 0.1f);
    DrawCircle(cx, cy, r * 0.7f, SKYBLUE);
    DrawCircleLines(cx, cy, r * 0.7f, BLUE);
    DrawRectangleRec({(float)(cx - r * 0.3f), (float)(cy - r * 0.6f), (float)(r * 0.15f), (float)(r * 1.2f)}, WHITE);
    DrawRectangleRec({(float)(cx + r * 0.15f), (float)(cy - r * 0.6f), (float)(r * 0.15f), (float)(r * 1.2f)}, WHITE);

    // Cabeza
    int head_y = static_cast<int>(draw_y - h * 0.2f);
    DrawCircle(cx, head_y, r * 0.45f, Color{250, 200, 160, 255});
    DrawCircleLines(cx, head_y, r * 0.45f, DARKGRAY);
    DrawCircleSector({(float)cx, (float)head_y}, r * 0.46f, 180.0f, 360.0f, 0, Color{130, 80, 40, 255});

    int eye_x = cx + static_cast<int>(dir_carr * r * 0.25f);
    int eye_y = head_y - static_cast<int>(r * 0.05f);
    DrawCircle(eye_x, eye_y, 2.5f, BLACK);

    // LED
    Color led_color = GREEN;
    if (estado == EstadoSeguidor::PERSIGUIENDO) led_color = ORANGE;
    else if (estado == EstadoSeguidor::RETRAYENDO) led_color = PURPLE;
    DrawCircle(cx - static_cast<int>(dir_carr * r * 0.3f), cy - r * 0.1f, 3.5f, led_color);
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
        double ang = b->get_angulo();

        // Dibujar sprite de la bola con rotación correcta alrededor del centro
        int tex_idx = b->get_texture_idx();
        if (tex_idx >= 0 && tex_idx < 3 && tex_bola[tex_idx].id > 0) {
            DrawTexturePro(
                tex_bola[tex_idx],
                {0, 0, (float)tex_bola[tex_idx].width, (float)tex_bola[tex_idx].height},
                {static_cast<float>(pos.x), static_cast<float>(pos.y), 2.0f * r, 2.0f * r},
                {r, r},
                static_cast<float>(ang * 180.0 / MathUtils::TIM_PI),
                WHITE
            );
        } else {
            // Fallback a círculo si la textura no se cargó
            Color col = PALETA_BOLAS[b->get_color_idx()];
            DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), r, col);
            DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), r,
                            ColorBrightness(col, -0.3f));
        }

        // Indicador de rotación (dos puntos en lados opuestos)
        float dot_dist = r * 0.55f;
        float dot_r = std::max(r * 0.2f, 2.0f);
        Color dot_col = {100, 100, 100, 150};

        int dx1 = static_cast<int>(pos.x + std::cos(ang) * dot_dist);
        int dy1 = static_cast<int>(pos.y + std::sin(ang) * dot_dist);
        DrawCircle(dx1, dy1, dot_r, dot_col);

        int dx2 = static_cast<int>(pos.x - std::cos(ang) * dot_dist);
        int dy2 = static_cast<int>(pos.y - std::sin(ang) * dot_dist);
        DrawCircle(dx2, dy2, dot_r, dot_col);

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
            EstadoSeguidor estado = seg->get_estado();
            Vector2D pos_init = seg->get_posicion_inicial();
            double dir_carr = seg->get_direccion_carrera();

            float draw_y = pos.y - 6.0f; // Subir el sprite 6 píxeles para que no se hundan los pies en el suelo

            // 1. Dibujar sombra sutil en el suelo
            DrawEllipse(static_cast<int>(pos.x), static_cast<int>(pos.y + h / 2.0f - 2.0f), w * 0.8f, 3.0f, Color{0, 0, 0, 80});

            // 2. Línea de anclaje (hilo elástico de retorno)
            if (estado != EstadoSeguidor::ESPERANDO) {
                DrawLineEx({(float)pos_init.x, (float)pos_init.y}, {(float)pos.x, (float)draw_y}, 1.5f, Color{100, 150, 255, 100});
            }

            // 3. Dibujar sprite del personaje
            float sprite_w = w * 1.5f;
            float sprite_h = h * 1.2f;
            Vector2 pos_draw = {static_cast<float>(pos.x), draw_y};

            if (estado == EstadoSeguidor::ESPERANDO) {
                // Personaje quieto: imagen estática sin animación
                if (tex_seguidor_quieto.id > 0) {
                    Rectangle source = {0, 0, (float)tex_seguidor_quieto.width, (float)tex_seguidor_quieto.height};
                    Rectangle dest = {pos_draw.x - sprite_w/2, pos_draw.y - sprite_h/2, sprite_w, sprite_h};
                    DrawTexturePro(tex_seguidor_quieto, source, dest, {0, 0}, 0.0f, WHITE);
                } else {
                    dibuja_seguidor_geometrico(pos, w, h, draw_y, estado, pos_init, seg);
                }
            } else if (anim_seguidor_corriendo) {
                // Personaje corriendo: animación de sprite sheet
                if (dir_carr > 0) {
                    anim_seguidor_corriendo->dibujar(pos_draw, sprite_w, sprite_h);
                } else {
                    anim_seguidor_corriendo->dibujar_volteado(pos_draw, sprite_w, sprite_h);
                }
            } else if (tex_seguidor_corriendo.id > 0) {
                // Fallback a sprite estático de correr si no hay animación
                Rectangle source = {0, 0, (float)tex_seguidor_corriendo.width, (float)tex_seguidor_corriendo.height};
                Rectangle dest = {pos_draw.x - sprite_w/2, pos_draw.y - sprite_h/2, sprite_w, sprite_h};
                DrawTexturePro(tex_seguidor_corriendo, source, dest, {0, 0}, 0.0f, WHITE);
            } else {
                // Fallback: geometría original si no hay texturas/animaciones
                dibuja_seguidor_geometrico(pos, w, h, draw_y, estado, pos_init, seg);
            }

            if (modo_debug) {
                DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), 180.0f, Color{0, 255, 0, 80});
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

            if (tex_trampolin.id > 0) {
                int num_frames = 4;
                float frame_w = 0.0f;
                float frame_h = 0.0f;
                Rectangle source;
                int frame_idx = 0;
                if (def > 12.0f) frame_idx = 3;
                else if (def > 7.0f) frame_idx = 2;
                else if (def > 2.0f) frame_idx = 1;

                if (tex_trampolin.width > tex_trampolin.height) {
                    frame_w = (float)tex_trampolin.width / num_frames;
                    frame_h = (float)tex_trampolin.height;
                    source = { frame_idx * frame_w, 0, frame_w, frame_h };
                } else {
                    frame_w = (float)tex_trampolin.width;
                    frame_h = (float)tex_trampolin.height / num_frames;
                    source = { 0, frame_idx * frame_h, frame_w, frame_h };
                }

                Rectangle dest = { px - 4.0f, py - 4.0f, pw + 8.0f, ph + 8.0f };
                DrawTexturePro(tex_trampolin, source, dest, {0, 0}, 0.0f, WHITE);
            } else {
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
            }
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
                // Si la textura está disponible, usarla; si no, usar geometría
                if (tex_chavo.id > 0) {
                    // Usar textura: El Chavo se desplaza verticalmente según pop
                    float chavo_y = py + 15.0f - pop * 35.0f;
                    float chavo_x = px + pw / 2.0f;
                    float chavo_w = 70.0f;
                    float chavo_h = 70.0f;
                    
                    DrawTexturePro(
                        tex_chavo,
                        {0, 0, (float)tex_chavo.width, (float)tex_chavo.height},
                        {chavo_x - chavo_w / 2.0f, chavo_y - chavo_h / 2.0f, chavo_w, chavo_h},
                        {0, 0},
                        0.0f,
                        WHITE
                    );
                } else {
                    // Fallback: Geometría original de El Chavo
                    float chavo_y = py + 15.0f - pop * 35.0f;
                    float chavo_x = px + pw / 2.0f;
                    float head_r = 15.0f;

                    DrawRectangleRec({chavo_x - 8.0f, chavo_y + 10.0f, 16.0f, 15.0f}, Color{220, 220, 200, 255});
                    DrawCircle(static_cast<int>(chavo_x), static_cast<int>(chavo_y), head_r, Color{253, 214, 185, 255});
                    DrawCircleLines(static_cast<int>(chavo_x), static_cast<int>(chavo_y), head_r, Color{160, 110, 80, 255});
                    DrawCircleSector({chavo_x, chavo_y}, head_r + 1.0f, 180.0f, 360.0f, 0, Color{84, 137, 101, 255});
                    DrawCircle(static_cast<int>(chavo_x - 5.0f), static_cast<int>(chavo_y - 1.0f), 2.0f, BLACK);
                    DrawCircle(static_cast<int>(chavo_x + 5.0f), static_cast<int>(chavo_y - 1.0f), 2.0f, BLACK);
                    DrawCircleSector({chavo_x, chavo_y + 3.0f}, 4.0f, 0.0f, 180.0f, 0, Color{200, 80, 80, 255});
                    DrawCircle(static_cast<int>(chavo_x - 5.0f), static_cast<int>(chavo_y + 2.0f), 2.0f, Color{253, 180, 160, 255});
                    DrawCircle(static_cast<int>(chavo_x + 5.0f), static_cast<int>(chavo_y + 2.0f), 2.0f, Color{253, 180, 160, 255});
                }
            }

            // 3. Dibujar el Barril
            if (tex_barril.id > 0) {
                // Usar textura del barril
                DrawTexturePro(
                    tex_barril,
                    {0, 0, (float)tex_barril.width, (float)tex_barril.height},
                    {px, py, pw, ph},
                    {0, 0},
                    0.0f,
                    WHITE
                );
            } else {
                // Fallback: Geometría original del barril (duelas de madera + aros metálicos)
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
                
                DrawRectangleRec({px - 2.0f, py + 12.0f, pw + 4.0f, 6.0f}, col_metal);
                DrawRectangleLinesEx({px - 2.0f, py + 12.0f, pw + 4.0f, 6.0f}, 1.0f, col_metal_borde);

                DrawRectangleRec({px - 3.0f, py + ph / 2.0f - 3.0f, pw + 6.0f, 6.0f}, col_metal);
                DrawRectangleLinesEx({px - 3.0f, py + ph / 2.0f - 3.0f, pw + 6.0f, 6.0f}, 1.0f, col_metal_borde);

                DrawRectangleRec({px - 2.0f, py + ph - 18.0f, pw + 4.0f, 6.0f}, col_metal);
                DrawRectangleLinesEx({px - 2.0f, py + ph - 18.0f, pw + 4.0f, 6.0f}, 1.0f, col_metal_borde);

                DrawEllipse(static_cast<int>(px + pw / 2.0f), static_cast<int>(py + 2.0f), pw * 0.45f, 4.0f, BLACK);
            }

            if (modo_debug) {
                DrawRectangleLines(static_cast<int>(px), static_cast<int>(py), static_cast<int>(pw), static_cast<int>(ph), GREEN);
            }
            return;
        }

        const ParedRectangular* p = dynamic_cast<const ParedRectangular*>(e);
        if (!p) return;

        Vector2D pos = p->get_posicion();
        float px = static_cast<float>(pos.x);
        float py = static_cast<float>(pos.y);
        float pw = static_cast<float>(p->get_ancho());
        float ph = static_cast<float>(p->get_alto());

        bool texturizado = false;
        if (!es_borde_nivel(p)) {
            if (pw > ph) {
                if (tex_plata_larga.id > 0) {
                    DrawTexturePro(
                        tex_plata_larga,
                        { 0, 0, (float)tex_plata_larga.width, (float)tex_plata_larga.height },
                        { px, py, pw, ph },
                        { 0, 0 },
                        0.0f,
                        WHITE
                    );
                    texturizado = true;
                }
            } else {
                if (tex_plata_peque.id > 0) {
                    DrawTexturePro(
                        tex_plata_peque,
                        { 0, 0, (float)tex_plata_peque.width, (float)tex_plata_peque.height },
                        { px, py, pw, ph },
                        { 0, 0 },
                        0.0f,
                        WHITE
                    );
                    texturizado = true;
                }
            }
        }

        if (!texturizado) {
            DrawRectangle(static_cast<int>(px), static_cast<int>(py), static_cast<int>(pw), static_cast<int>(ph), COLOR_PARED);
            DrawRectangleLines(static_cast<int>(px), static_cast<int>(py), static_cast<int>(pw), static_cast<int>(ph), COLOR_PARED_BORDE);
        }
    }
    else if (forma == TipoForma::POLIGONO) {
        const Balancin* bal = dynamic_cast<const Balancin*>(e);
        if (bal) {
            Vector2D pos = bal->get_posicion();
            int px = static_cast<int>(pos.x);
            int py = static_cast<int>(pos.y);
            int largo = static_cast<int>(bal->get_largo());
            int espesor = static_cast<int>(bal->get_espesor());
            float rot_deg = static_cast<float>(bal->get_angulo() * 180.0 / MathUtils::TIM_PI);
            double cos_a = std::cos(bal->get_angulo());
            double sin_a = std::sin(bal->get_angulo());

            // 1. Dibujar soporte del pivot
            if (tex_balancin_base.id > 0) {
                float base_w = 34.0f;
                float base_h = 44.0f;
                DrawTexturePro(
                    tex_balancin_base,
                    { 0, 0, (float)tex_balancin_base.width, (float)tex_balancin_base.height },
                    { (float)px - base_w / 2.0f, (float)py - 2.0f, base_w, base_h },
                    { 0, 0 },
                    0.0f,
                    WHITE
                );
            } else {
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
            }

            // 2. Dibujar la tabla giratoria (plank)
            if (tex_balancin_tabla.id > 0) {
                float board_h = espesor * 3.5f;
                Rectangle rec = { static_cast<float>(px), static_cast<float>(py), static_cast<float>(largo), board_h };
                Vector2 origin = { static_cast<float>(largo / 2.0), board_h / 2.0f };
                DrawTexturePro(
                    tex_balancin_tabla,
                    { 0, 0, (float)tex_balancin_tabla.width, (float)tex_balancin_tabla.height },
                    rec,
                    origin,
                    rot_deg,
                    WHITE
                );
            } else {
                Rectangle rec = { static_cast<float>(px), static_cast<float>(py), static_cast<float>(largo), static_cast<float>(espesor) };
                Vector2 origin = { static_cast<float>(largo / 2.0), static_cast<float>(espesor / 2.0) };
                DrawRectanglePro(rec, origin, rot_deg, Color{190, 110, 50, 255});
                
                // Dibujar contorno ocre rotado usando las 4 esquinas del tablón
                double hl = largo / 2.0;
                double ht = espesor / 2.0;
                Vector2D dir_x(cos_a, sin_a);
                Vector2D dir_y(-sin_a, cos_a);
                Vector2D c1 = pos - dir_x * hl - dir_y * ht;
                Vector2D c2 = pos + dir_x * hl - dir_y * ht;
                Vector2D c3 = pos + dir_x * hl + dir_y * ht;
                Vector2D c4 = pos - dir_x * hl + dir_y * ht;

                Color color_borde = Color{220, 140, 70, 255};
                DrawLineEx({(float)c1.x, (float)c1.y}, {(float)c2.x, (float)c2.y}, 1.5f, color_borde);
                DrawLineEx({(float)c2.x, (float)c2.y}, {(float)c3.x, (float)c3.y}, 1.5f, color_borde);
                DrawLineEx({(float)c3.x, (float)c3.y}, {(float)c4.x, (float)c4.y}, 1.5f, color_borde);
                DrawLineEx({(float)c4.x, (float)c4.y}, {(float)c1.x, (float)c1.y}, 1.5f, color_borde);
            }

            // 3. Asientos rojos en los extremos (siempre dibujados encima para mayor detalle)
            Vector2 seat_l = {
                static_cast<float>(px - (largo / 2.0 - 5.0) * cos_a),
                static_cast<float>(py - (largo / 2.0 - 5.0) * sin_a)
            };
            DrawCircle(static_cast<int>(seat_l.x), static_cast<int>(seat_l.y), 6.0f, RED);
            DrawCircleLines(static_cast<int>(seat_l.x), static_cast<int>(seat_l.y), 6.0f, MAROON);

            Vector2 seat_r = {
                static_cast<float>(px + (largo / 2.0 - 5.0) * cos_a),
                static_cast<float>(py + (largo / 2.0 - 5.0) * sin_a)
            };
            DrawCircle(static_cast<int>(seat_r.x), static_cast<int>(seat_r.y), 6.0f, RED);
            DrawCircleLines(static_cast<int>(seat_r.x), static_cast<int>(seat_r.y), 6.0f, MAROON);

            // 4. Perno central negro/gris
            DrawCircle(px, py, 6, BLACK);
            DrawCircle(px, py, 4, LIGHTGRAY);
            return;
        }

        const PlanoInclinado* ramp = dynamic_cast<const PlanoInclinado*>(e);
        if (!ramp) return;

        Vector2D pos = ramp->get_posicion();
        float px = static_cast<float>(pos.x);
        float py = static_cast<float>(pos.y);
        float pw = static_cast<float>(ramp->get_base());
        float ph = static_cast<float>(ramp->get_altura());
        bool invertido = ramp->get_invertido();

        bool texturizado = false;
        if (!invertido && tex_plata_rampa_izq.id > 0) {
            DrawTexturePro(
                tex_plata_rampa_izq,
                { 0, 0, (float)tex_plata_rampa_izq.width, (float)tex_plata_rampa_izq.height },
                { px, py, pw, ph },
                { 0, 0 },
                0.0f,
                WHITE
            );
            texturizado = true;
        } else if (invertido && tex_plata_rampa_der.id > 0) {
            DrawTexturePro(
                tex_plata_rampa_der,
                { 0, 0, (float)tex_plata_rampa_der.width, (float)tex_plata_rampa_der.height },
                { px, py, pw, ph },
                { 0, 0 },
                0.0f,
                WHITE
            );
            texturizado = true;
        }

        if (!texturizado) {
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

    if (arrastrando_spawn != TipoObjetoMenu::NINGUNO) {
        DrawText("Arrastrando objeto... suelta en el area de juego",
                 margin, y + 88, 14, Color{100, 200, 255, 255});
    }

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

    // Indicador de selección
    if (entidad_seleccionada) {
        DrawText(TextFormat("Seleccionado: Entidad #%d  [DEL] Eliminar", entidad_seleccionada->get_id()),
                 margin, y + 88, 14, Color{255, 200, 80, 255});
    }

    // Controles
    DrawText("[TAB] Menu  [Arrastrar] Crear/Mover  [CLICK] Seleccionar  [DEL] Eliminar  [SPACE] Pausa  [D] Debug  [R] Reset",
             margin, ALTO - 30, 14, COLOR_CONTROLES);
}

// ============================================================================
// Cargar texturas
// ============================================================================
void cargandoTexturas() {
    // Cargar las 3 texturas de pelota
    tex_bola[0] = LoadTexture("../Assets/ball/pelota1.png");
    if (tex_bola[0].id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: ../Assets/ball/pelota1.png");
    } else {
        TraceLog(LOG_INFO, "Textura bola 1 cargada: %dx%d", tex_bola[0].width, tex_bola[0].height);
    }

    tex_bola[1] = LoadTexture("../Assets/ball/pelota2.png");
    if (tex_bola[1].id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: ../Assets/ball/pelota2.png");
    } else {
        TraceLog(LOG_INFO, "Textura bola 2 cargada: %dx%d", tex_bola[1].width, tex_bola[1].height);
    }

    tex_bola[2] = LoadTexture("../Assets/ball/pelota3.png");
    if (tex_bola[2].id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: ../Assets/ball/pelota3.png");
    } else {
        TraceLog(LOG_INFO, "Textura bola 3 cargada: %dx%d", tex_bola[2].width, tex_bola[2].height);
    }

    tex_fondo = LoadTexture("../Assets/fondo1.png");
    if (tex_fondo.id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: ../Assets/fondo1.png");
    } else {
        TraceLog(LOG_INFO, "Textura fondo cargada: %dx%d", tex_fondo.width, tex_fondo.height);
    }
    
    tex_base_central = LoadTexture("../Assets/hud/panel.png");
    if (tex_base_central.id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: ../Assets/hud/panel.png");
    } else {
        TraceLog(LOG_INFO, "Textura base central cargada: %dx%d", tex_base_central.width, tex_base_central.height);
    }
    
    derecho = LoadTexture("../Assets/hud/barsidederecho.png");
    if (derecho.id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: ../Assets/hud/barsidederecho.png");
    } else {
        TraceLog(LOG_INFO, "Textura derecho cargada: %dx%d", derecho.width, derecho.height);
    }
    
    // Cargar texturas del BarrilChavo
    tex_barril = LoadTexture("../Assets/chavo/barril.png");
    if (tex_barril.id == 0) {
        TraceLog(LOG_WARNING, "Textura barril no encontrada: ../Assets/chavo/barril.png (usando renderizado geométrico)");
    } else {
        TraceLog(LOG_INFO, "Textura barril cargada: %dx%d", tex_barril.width, tex_barril.height);
    }
    
    tex_chavo = LoadTexture("../Assets/chavo/chavo.png");
    if (tex_chavo.id == 0) {
        TraceLog(LOG_WARNING, "Textura El Chavo no encontrada: ../Assets/characters/chavo.png (usando renderizado geométrico)");
    } else {
        TraceLog(LOG_INFO, "Textura El Chavo cargada: %dx%d", tex_chavo.width, tex_chavo.height);
    }
    
    // Cargar texturas del SeguidorBooster
    tex_seguidor_quieto = LoadTexture("../Assets/messi/messi-normal.png");
    if (tex_seguidor_quieto.id == 0) {
        TraceLog(LOG_WARNING, "Textura seguidor quieto no encontrada: ../Assets/messi/messi-normal.png");
    } else {
        TraceLog(LOG_INFO, "Textura seguidor quieto cargada: %dx%d", tex_seguidor_quieto.width, tex_seguidor_quieto.height);
    }
    
    tex_seguidor_corriendo = LoadTexture("../Assets/messi/mesirve.png");
    if (tex_seguidor_corriendo.id == 0) {
        TraceLog(LOG_WARNING, "Textura seguidor corriendo no encontrada: ../Assets/messi/mesirve.png");
    } else {
        TraceLog(LOG_INFO, "Textura seguidor corriendo cargada: %dx%d", tex_seguidor_corriendo.width, tex_seguidor_corriendo.height);
    }
    
    // Inicializar animaciones del SeguidorBooster
    //Animacion(textura, total_frames, fps, frames_por_fila)
    if (tex_seguidor_corriendo.id > 0) {
        anim_seguidor_corriendo = new Animacion(tex_seguidor_corriendo, 8, 12, 8);
    }

    // Cargar texturas de los nuevos assets
    tex_trampolin = LoadTexture("../Assets/trampolin/trampolin.png");
    if (tex_trampolin.id == 0) {
        TraceLog(LOG_WARNING, "Textura trampolin no encontrada: ../Assets/trampolin/trampolin.png");
    } else {
        TraceLog(LOG_INFO, "Textura trampolin cargada: %dx%d", tex_trampolin.width, tex_trampolin.height);
    }

    tex_balancin_base = LoadTexture("../Assets/balancin/detalle1.png");
    if (tex_balancin_base.id == 0) {
        TraceLog(LOG_WARNING, "Textura balancin base no encontrada: ../Assets/balancin/detalle1.png");
    } else {
        TraceLog(LOG_INFO, "Textura balancin base cargada: %dx%d", tex_balancin_base.width, tex_balancin_base.height);
    }

    tex_balancin_tabla = LoadTexture("../Assets/balancin/balancin2.png");
    if (tex_balancin_tabla.id == 0) {
        TraceLog(LOG_WARNING, "Textura balancin tabla no encontrada: ../Assets/balancin/balancin2.png");
    } else {
        TraceLog(LOG_INFO, "Textura balancin tabla cargada: %dx%d", tex_balancin_tabla.width, tex_balancin_tabla.height);
    }

    tex_plata_larga = LoadTexture("../Assets/plataform/plata_larga.png");
    if (tex_plata_larga.id == 0) {
        TraceLog(LOG_WARNING, "Textura plataforma larga no encontrada: ../Assets/plataform/plata_larga.png");
    } else {
        TraceLog(LOG_INFO, "Textura plataforma larga cargada: %dx%d", tex_plata_larga.width, tex_plata_larga.height);
    }

    tex_plata_peque = LoadTexture("../Assets/plataform/plata_peque.png");
    if (tex_plata_peque.id == 0) {
        TraceLog(LOG_WARNING, "Textura plataforma peque no encontrada: ../Assets/plataform/plata_peque.png");
    } else {
        TraceLog(LOG_INFO, "Textura plataforma peque cargada: %dx%d", tex_plata_peque.width, tex_plata_peque.height);
    }

    tex_plata_rampa_izq = LoadTexture("../Assets/plataform/plataforma1.png");
    if (tex_plata_rampa_izq.id == 0) {
        TraceLog(LOG_WARNING, "Textura rampa izq no encontrada: ../Assets/plataform/plataforma1.png");
    } else {
        TraceLog(LOG_INFO, "Textura rampa izq cargada: %dx%d", tex_plata_rampa_izq.width, tex_plata_rampa_izq.height);
    }

    tex_plata_rampa_der = LoadTexture("../Assets/plataform/plataforma2.png");
    if (tex_plata_rampa_der.id == 0) {
        TraceLog(LOG_WARNING, "Textura rampa der no encontrada: ../Assets/plataform/plataforma2.png");
    } else {
        TraceLog(LOG_INFO, "Textura rampa der cargada: %dx%d", tex_plata_rampa_der.width, tex_plata_rampa_der.height);
    }
}

// ============================================================================
// Punto de entrada
// ============================================================================
int main() {
    // ---- Inicializar ventana ----
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(ANCHO, ALTO, "TIM - Motor de Fisica | Prototipo RK4 + Raylib");
    SetWindowMinSize(ANCHO_MIN, ALTO_MIN);
    SetTargetFPS(60);
    cargandoTexturas();

    // ---- Inicializar motor de física ----
    // dt_fijo = 1/120s (120 pasos físicos por segundo)
    // Gravedad = 500 px/s² hacia abajo (Y+ en pantalla)
    MotorFisica motor(1.0 / 120.0, Vector2D(0, 500.0));
    crear_escena(motor);

    // Animación de título (fade-out)
    float titulo_alpha = 1.0f;

    // ---- Bucle principal ----
    while (!WindowShouldClose()) {
        sincronizar_tamano_ventana();

        // ======== INPUT ========
        reconstruir_celdas_menu();

        if (IsKeyPressed(KEY_TAB)) {
            menu_visible = !menu_visible;
        }

        int mx = GetMouseX();
        int my = GetMouseY();

        // Click izquierdo: menú, spawn drag, o arrastre de entidad en canvas
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (punto_en_menu(mx, my)) {
                if (manejar_click_menu(mx, my)) {
                    // UI consumió el click (pestaña, acordeón, paginación, colapsar)
                } else {
                    TipoObjetoMenu tipo = tipo_en_celda(mx, my, celdas_menu_cache);
                    if (tipo != TipoObjetoMenu::NINGUNO)
                        arrastrando_spawn = tipo;
                }
            } else if (punto_en_area_juego(mx, my)) {
                Vector2D mouse_pos(mx, my);
                EntidadFisica* clicked = obtener_entidad_bajo_mouse(motor, mouse_pos);
                if (clicked) {
                    entidad_arrastrada = clicked;
                    entidad_seleccionada = clicked;  // Seleccionar al hacer click
                    offset_arrastre = clicked->get_posicion() - mouse_pos;
                } else {
                    entidad_seleccionada = nullptr;  // Deseleccionar al hacer click en vacío
                }
            }
        }

        // Arrastre desde paleta: ghost hasta soltar
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && arrastrando_spawn != TipoObjetoMenu::NINGUNO) {
            // No mover entidades mientras se coloca desde el menú
        } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && entidad_arrastrada != nullptr) {
            Vector2D mouse_pos(mx, my);
            Vector2D nueva_pos = mouse_pos + offset_arrastre;
            double limite_x = static_cast<double>(ancho_area_juego()) - 30.0;
            nueva_pos.x = std::max(30.0, std::min(limite_x, nueva_pos.x));
            nueva_pos.y = std::max(30.0, std::min(double(ALTO - 30.0), nueva_pos.y));

            entidad_arrastrada->set_posicion(nueva_pos);
            entidad_arrastrada->set_velocidad(Vector2D(0.0, 0.0));
            entidad_arrastrada->set_velocidad_angular(0.0);

            // Recalcular vértices si es una rampa (PlanoInclinado)
            PlanoInclinado* ramp_drag = dynamic_cast<PlanoInclinado*>(entidad_arrastrada);
            if (ramp_drag) {
                ramp_drag->recalcular_vertices();
            }
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            if (arrastrando_spawn != TipoObjetoMenu::NINGUNO) {
                if (punto_en_area_juego(mx, my)) {
                    spawn_desde_menu(motor, arrastrando_spawn, Vector2D(mx, my));
                }
                arrastrando_spawn = TipoObjetoMenu::NINGUNO;
            }
            entidad_arrastrada = nullptr;
        }

        // Abrir menú con click en pestaña colapsada
        if (!menu_visible && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Rectangle btn_abrir = { static_cast<float>(ANCHO - 26), static_cast<float>(ALTO / 2 - 40), 24, 80 };
            if (click_en_rect(mx, my, btn_abrir))
                menu_visible = true;
        }

        // Spacebar: pausar/reanudar
        if (IsKeyPressed(KEY_SPACE)) {
            motor.set_pausado(!motor.get_pausado());
        }

        // D: toggle debug
        if (IsKeyPressed(KEY_D)) {
            modo_debug = !modo_debug;
        }

        // DELETE / X: eliminar entidad seleccionada
        if ((IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_X)) && entidad_seleccionada) {
            if (!es_borde_nivel(entidad_seleccionada)) {
                int id_eliminar = entidad_seleccionada->get_id();
                // Limpiar punteros antes de eliminar
                if (entidad_arrastrada == entidad_seleccionada) entidad_arrastrada = nullptr;
                entidad_seleccionada = nullptr;
                motor.remover_entidad(id_eliminar);
            }
        }

        // R: reiniciar escena
        if (IsKeyPressed(KEY_R)) {
            motor.limpiar();
            contador_bolas = 0;
            borde_suelo = nullptr;
            borde_izquierda = nullptr;
            borde_derecha = nullptr;
            borde_techo = nullptr;
            entidad_arrastrada = nullptr;
            entidad_seleccionada = nullptr;
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

        // Actualizar animaciones del SeguidorBooster
        if (anim_seguidor_corriendo) {
            anim_seguidor_corriendo->actualizar(GetFrameTime());
        }

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

        // Dibujar fondo
        if (tex_fondo.id > 0) {
            float escala_x = static_cast<float>(ANCHO) / tex_fondo.width;
            float escala_y = static_cast<float>(ALTO) / tex_fondo.height;
            float escala = (escala_x > escala_y) ? escala_x : escala_y;  // Mantener aspecto ratio
            DrawTextureEx(tex_fondo, {0, 0}, 0, escala, WHITE);
        }

        // Dibujar todas las entidades
        for (const auto* e : motor.get_entidades()) {
            dibujar_entidad(e);
            dibujar_debug(e);
        }

        // Dibujar resaltado de selección
        if (entidad_seleccionada) {
            Color sel_color = {255, 200, 50, 180};
            TipoForma sel_forma = entidad_seleccionada->get_tipo_forma();
            if (sel_forma == TipoForma::CIRCULO) {
                Vector2D sel_pos;
                double sel_radio = 0.0;
                if (obtener_datos_circulo(entidad_seleccionada, sel_pos, sel_radio)) {
                    DrawCircleLines(static_cast<int>(sel_pos.x), static_cast<int>(sel_pos.y),
                                    static_cast<float>(sel_radio + 4.0), sel_color);
                    DrawCircleLines(static_cast<int>(sel_pos.x), static_cast<int>(sel_pos.y),
                                    static_cast<float>(sel_radio + 6.0), Color{255, 200, 50, 90});
                }
            } else if (sel_forma == TipoForma::AABB) {
                const ParedRectangular* sp = dynamic_cast<const ParedRectangular*>(entidad_seleccionada);
                if (sp) {
                    Vector2D spos = sp->get_posicion();
                    DrawRectangleLinesEx({(float)spos.x - 3, (float)spos.y - 3,
                        (float)sp->get_ancho() + 6, (float)sp->get_alto() + 6}, 2.0f, sel_color);
                } else {
                    // Genérico AABB: Trampolin, Barril, Ventilador, Seguidor
                    Vector2D spos = entidad_seleccionada->get_posicion();
                    const Trampolin* st = dynamic_cast<const Trampolin*>(entidad_seleccionada);
                    const BarrilChavo* sb = dynamic_cast<const BarrilChavo*>(entidad_seleccionada);
                    const Ventilador* sv = dynamic_cast<const Ventilador*>(entidad_seleccionada);
                    const SeguidorBooster* ss = dynamic_cast<const SeguidorBooster*>(entidad_seleccionada);
                    float sw = 0, sh = 0;
                    if (st) { sw = st->get_ancho(); sh = st->get_alto(); }
                    else if (sb) { sw = sb->get_ancho(); sh = sb->get_alto(); }
                    else if (sv) { sw = sv->get_ancho(); sh = sv->get_alto(); }
                    else if (ss) { sw = ss->get_ancho(); sh = ss->get_alto(); spos.x -= sw/2; spos.y -= sh/2; }
                    if (sw > 0) {
                        DrawRectangleLinesEx({(float)spos.x - 3, (float)spos.y - 3,
                            sw + 6, sh + 6}, 2.0f, sel_color);
                    }
                }
            } else if (sel_forma == TipoForma::POLIGONO) {
                const PlanoInclinado* sr = dynamic_cast<const PlanoInclinado*>(entidad_seleccionada);
                if (sr) {
                    const auto& sv = sr->get_vertices();
                    if (sv.size() >= 3) {
                        for (size_t vi = 0; vi < sv.size(); ++vi) {
                            size_t vj = (vi + 1) % sv.size();
                            DrawLineEx({(float)sv[vi].x, (float)sv[vi].y},
                                       {(float)sv[vj].x, (float)sv[vj].y}, 2.5f, sel_color);
                        }
                    }
                } else {
                    const Balancin* sbal = dynamic_cast<const Balancin*>(entidad_seleccionada);
                    if (sbal) {
                        Vector2D sp = sbal->get_posicion();
                        DrawCircleLines(static_cast<int>(sp.x), static_cast<int>(sp.y), 50.0f, sel_color);
                    }
                }
            }
        }
                // Dibujar panel del HUD (abajo)
        if (tex_base_central.id > 0) {
            // Posición: y = ALTO - 150, altura = 130, ancho = 1900
            float escala_panel_x = 1900.0f / tex_base_central.width;
            float escala_panel_y = 130.0f / tex_base_central.height;
            DrawTextureEx(tex_base_central, {20, static_cast<float>(ALTO - 150)}, 0, 
                         (escala_panel_x > escala_panel_y) ? escala_panel_x : escala_panel_y, WHITE);
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

        // Separador visual del área de juego vs menú
        if (menu_visible) {
            int sep = ancho_area_juego();
            DrawLine(sep, 0, sep, ALTO, Color{60, 65, 90, 180});
        }

        dibujar_ghost_spawn();

        // HUD
        dibujar_hud(motor);

        dibujar_menu_lateral();

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
    for (int i = 0; i < 3; ++i) {
        UnloadTexture(tex_bola[i]);
    }
    UnloadTexture(tex_fondo);
    UnloadTexture(tex_base_central);
    UnloadTexture(derecho);
    if (tex_barril.id > 0) UnloadTexture(tex_barril);
    if (tex_chavo.id > 0) UnloadTexture(tex_chavo);
    if (tex_seguidor_quieto.id > 0) UnloadTexture(tex_seguidor_quieto);
    if (tex_seguidor_corriendo.id > 0) UnloadTexture(tex_seguidor_corriendo);
    if (anim_seguidor_corriendo) {
        delete anim_seguidor_corriendo;
    }
    if (tex_trampolin.id > 0) UnloadTexture(tex_trampolin);
    if (tex_balancin_base.id > 0) UnloadTexture(tex_balancin_base);
    if (tex_balancin_tabla.id > 0) UnloadTexture(tex_balancin_tabla);
    if (tex_plata_larga.id > 0) UnloadTexture(tex_plata_larga);
    if (tex_plata_peque.id > 0) UnloadTexture(tex_plata_peque);
    if (tex_plata_rampa_izq.id > 0) UnloadTexture(tex_plata_rampa_izq);
    if (tex_plata_rampa_der.id > 0) UnloadTexture(tex_plata_rampa_der);
    
    CloseWindow();
    return 0;
}
