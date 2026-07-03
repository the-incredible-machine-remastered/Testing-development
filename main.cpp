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
#include "objetos/cubeta.h"
#include "objetos/cuerda.h"
#include "objetos/soporte_torque.h"
#include "objetos/seguidor_booster.h"
#include "objetos/barril_chavo.h"
#include "objetos/ventilador.h"
#include "objetos/globo.h"
#include "objetos/gancho.h"
#include "objetos/pistola.h"
#include "objetos/tijera.h"
#include "objetos/bola_beisbol.h"
#include "objetos/caja_hamster.h"
#include "objetos/banda.h"
#include "objetos/caja_sorpresa.h"
#include "objetos/caminadora.h"
#include "objetos/foco.h"
#include "objetos/lupa.h"
#include "objetos/canon.h"
#include "objetos/ladrillo.h"
#include "objetos/ladrillo_vertical.h"
#include "objetos/ladrillo_horizontal.h"
#include "objetos/dinamita.h"
#include "objetos/dinamita_detonador.h"
#include "objetos/raton.h"
#include "objetos/gato.h"
#include "objetos/escalon.h"
#include "fisica/colisiones.h"
#include "fisica/motor_fisica.h"
#include "objetos/catalogo_menu.gen.h"
#include "sistema/rutas_datos.h"
#include "sistema/eventos.h"
#include "sistema/eventos_ui.h"
#include "sistema/guardado_partida.h"

#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <cstdio>

// Compatibilidad con versiones de Raylib sin IsMusicReady
inline bool IsMusicReady(Music music) {
    return music.ctxData != nullptr;
}

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
// COLOR_PARED, COLOR_PARED_BORDE, COLOR_RAMPA, COLOR_RAMPA_BORDE son definidos en assets_extern.h
const Color COLOR_HUD         = {200, 200, 220, 255};   // Texto claro
const Color COLOR_CONTROLES   = {130, 130, 160, 200};   // Texto controles

// Estado del prototipo
std::unordered_map<TipoObjetoMenu, int> inventario_maximo;
std::unordered_map<TipoObjetoMenu, int> inventario_actual;
std::unordered_map<TipoObjetoMenu, std::vector<std::unique_ptr<EntidadFisica>>> inventario_entidades;
int nivel_campana_actual = -1;
std::string nivel_usuario_actual_path = "";

EstadoJuego estado_actual = EstadoJuego::MENU_PRINCIPAL;
EstadoJuego estado_previo = EstadoJuego::MENU_PRINCIPAL;
float titulo_alpha = 1.0f;

enum class TabOpciones {
    JUGABILIDAD,
    CONTROLES,
    VIDEO,
    SONIDO,
    IDIOMA
};
TabOpciones pestana_opciones_actual = TabOpciones::CONTROLES;

enum class TabNiveles {
    CAMPANA,
    MIS_NIVELES,
    IMPORTAR
};
TabNiveles pestana_niveles_actual = TabNiveles::CAMPANA;

bool salir_juego = false;
bool mostrar_ayuda_overlay = false;
bool mostrar_pausa_overlay = false;

bool modo_debug = false;
int contador_bolas = 0;
GestorEventos gestor_eventos;

// Cronómetro y rastreo de nivel actual para reinicios
double tiempo_nivel = 0.0;
std::string ruta_nivel_actual = "";
int campana_nivel_actual = -1;

// Audio del juego
Music musica_menu;
bool sonido_mutado = false;


// Feedback visual de spawn fallido
float spawn_error_timer = 0.0f;
Vector2D spawn_error_pos;

// Estado del sistema Drag & Drop (entidades ya en escena)
EntidadFisica* entidad_arrastrada = nullptr;
Vector2D offset_arrastre;

// Estado de selección (para eliminar objetos)
EntidadFisica* entidad_seleccionada = nullptr;

enum class HandleResize {
    NINGUNO,
    TOP_LEFT, TOP_CENTER, TOP_RIGHT,
    MID_LEFT,             MID_RIGHT,
    BOT_LEFT, BOT_CENTER, BOT_RIGHT
};
HandleResize handle_activo = HandleResize::NINGUNO;
Vector2D     handle_pos_inicial_mouse;
double       handle_w_inicial = 0;
double       handle_h_inicial = 0;
Vector2D     handle_pos_inicial_ent;

bool es_borde_nivel(const EntidadFisica* e);

enum class EstadoColocacionCuerda {
    INACTIVA,
    ESPERANDO_EXTREMO_A,
    ESPERANDO_SOPORTE,
    ESPERANDO_EXTREMO_B
};

EstadoColocacionCuerda estado_cuerda = EstadoColocacionCuerda::INACTIVA;
AnclajeCuerda cuerda_extremo_a = { -1, TipoAnclajeCuerda::Cubeta };
Vector2D cuerda_punto_a_preview;
std::vector<int> cuerda_soportes_id;
std::vector<Vector2D> cuerda_soportes_preview;

enum class EstadoConexionBanda {
    INACTIVA,
    ESPERANDO_HAMSTER,
    ESPERANDO_VENTILADOR
};
EstadoConexionBanda estado_banda = EstadoConexionBanda::INACTIVA;
int banda_hamster_id  = -1;
int banda_ventilador_id = -1;
Vector2D banda_origen_preview; // posición del hámster seleccionado para preview

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

Vector2D posicion_anclaje_cuerda(const EntidadFisica* e, TipoAnclajeCuerda tipo) {
    if (!e) return Vector2D();

    if (tipo == TipoAnclajeCuerda::Cubeta) {
        const Cubeta* cubeta = dynamic_cast<const Cubeta*>(e);
        return cubeta ? cubeta->get_punto_cuerda() : e->get_posicion();
    }

    if (tipo == TipoAnclajeCuerda::Globo || tipo == TipoAnclajeCuerda::SoporteFijo || tipo == TipoAnclajeCuerda::Gancho) {
        return e->get_posicion();
    }

    const Balancin* bal = dynamic_cast<const Balancin*>(e);
    if (!bal) return e->get_posicion();
    return tipo == TipoAnclajeCuerda::BalancinIzquierdo
        ? bal->get_punto_extremo_izquierdo()
        : bal->get_punto_extremo_derecho();
}

const EntidadFisica* buscar_entidad_por_id(const MotorFisica& motor, int id) {
    for (const auto* e : motor.get_entidades()) {
        if (e && e->get_id() == id) return e;
    }
    return nullptr;
}

bool detectar_anclaje_cuerda(const MotorFisica& motor, Vector2D mouse_pos,
                             AnclajeCuerda& out, Vector2D& punto_out) {
    const double radio_hit = 30.0;
    double mejor_dist2 = radio_hit * radio_hit;
    bool encontrado = false;

    for (const auto* e : motor.get_entidades()) {
        if (es_borde_nivel(e)) continue;

        const Cubeta* cubeta = dynamic_cast<const Cubeta*>(e);
        if (cubeta) {
            Vector2D p = cubeta->get_punto_cuerda();
            double d2 = (p - mouse_pos).magnitud_cuadrada();
            if (d2 <= mejor_dist2) {
                mejor_dist2 = d2;
                out = { cubeta->get_id(), TipoAnclajeCuerda::Cubeta };
                punto_out = p;
                encontrado = true;
            }
        }

        if (e->get_tipo_entidad() == TipoEntidadJuego::GLOBO) {
            Vector2D p = e->get_posicion();
            double d2 = (p - mouse_pos).magnitud_cuadrada();
            if (d2 <= mejor_dist2) {
                mejor_dist2 = d2;
                out = { e->get_id(), TipoAnclajeCuerda::Globo };
                punto_out = p;
                encontrado = true;
            }
        }

        const SoporteTorque* soporte = dynamic_cast<const SoporteTorque*>(e);
        if (soporte) {
            Vector2D p = soporte->get_punto_cuerda();
            double d2 = (p - mouse_pos).magnitud_cuadrada();
            if (d2 <= mejor_dist2) {
                mejor_dist2 = d2;
                out = { soporte->get_id(), TipoAnclajeCuerda::SoporteFijo };
                punto_out = p;
                encontrado = true;
            }
        }

        const Gancho* gancho = dynamic_cast<const Gancho*>(e);
        if (gancho) {
            Vector2D p = gancho->get_punto_cuerda();
            double d2 = (p - mouse_pos).magnitud_cuadrada();
            if (d2 <= mejor_dist2) {
                mejor_dist2 = d2;
                out = { gancho->get_id(), TipoAnclajeCuerda::Gancho };
                punto_out = p;
                encontrado = true;
            }
        }

        const Pistola* pistola_anc = dynamic_cast<const Pistola*>(e);
        if (pistola_anc) {
            Vector2D p = pistola_anc->get_posicion();
            double d2 = (p - mouse_pos).magnitud_cuadrada();
            if (d2 <= mejor_dist2) {
                mejor_dist2 = d2;
                out = { pistola_anc->get_id(), TipoAnclajeCuerda::Gancho };
                punto_out = p;
                encontrado = true;
            }
        }

        // Foco y Lupa: activables como la pistola; se anclan con el mismo tipo
        // de anclaje genérico (Gancho, punto = posición, masa 0).
        const Foco* foco_anc = dynamic_cast<const Foco*>(e);
        if (foco_anc) {
            Vector2D p = foco_anc->get_posicion();
            double d2 = (p - mouse_pos).magnitud_cuadrada();
            if (d2 <= mejor_dist2) {
                mejor_dist2 = d2;
                out = { foco_anc->get_id(), TipoAnclajeCuerda::Gancho };
                punto_out = p;
                encontrado = true;
            }
        }

        const Lupa* lupa_anc = dynamic_cast<const Lupa*>(e);
        if (lupa_anc) {
            Vector2D p = lupa_anc->get_posicion();
            double d2 = (p - mouse_pos).magnitud_cuadrada();
            if (d2 <= mejor_dist2) {
                mejor_dist2 = d2;
                out = { lupa_anc->get_id(), TipoAnclajeCuerda::Gancho };
                punto_out = p;
                encontrado = true;
            }
        }

        const Balancin* bal = dynamic_cast<const Balancin*>(e);
        if (bal) {
            Vector2D puntos[2] = {
                bal->get_punto_extremo_izquierdo(),
                bal->get_punto_extremo_derecho()
            };
            TipoAnclajeCuerda tipos[2] = {
                TipoAnclajeCuerda::BalancinIzquierdo,
                TipoAnclajeCuerda::BalancinDerecho
            };

            for (int i = 0; i < 2; ++i) {
                double d2 = (puntos[i] - mouse_pos).magnitud_cuadrada();
                if (d2 <= mejor_dist2) {
                    mejor_dist2 = d2;
                    out = { bal->get_id(), tipos[i] };
                    punto_out = puntos[i];
                    encontrado = true;
                }
            }
        }
    }

    return encontrado;
}

bool detectar_soporte_torque(const MotorFisica& motor, Vector2D mouse_pos,
                             int& soporte_id_out, Vector2D& punto_out) {
    const double radio_extra = 16.0;
    for (const auto* e : motor.get_entidades()) {
        const SoporteTorque* soporte = dynamic_cast<const SoporteTorque*>(e);
        if (!soporte) continue;

        Vector2D p = soporte->get_punto_cuerda();
        double radio_hit = soporte->get_radio() + radio_extra;
        if ((p - mouse_pos).magnitud_cuadrada() <= radio_hit * radio_hit) {
            soporte_id_out = soporte->get_id();
            punto_out = p;
            return true;
        }
    }

    return false;
}

void cancelar_colocacion_cuerda() {
    estado_cuerda = EstadoColocacionCuerda::INACTIVA;
    cuerda_extremo_a = { -1, TipoAnclajeCuerda::Cubeta };
    cuerda_punto_a_preview = Vector2D();
    cuerda_soportes_id.clear();
    cuerda_soportes_preview.clear();
}

bool soporte_ya_agregado(int soporte_id) {
    return std::find(cuerda_soportes_id.begin(), cuerda_soportes_id.end(), soporte_id)
        != cuerda_soportes_id.end();
}

double calcular_longitud_ruta_cuerda(Vector2D punto_a,
                                     const std::vector<Vector2D>& soportes,
                                     Vector2D punto_b) {
    double longitud = 0.0;
    Vector2D anterior = punto_a;
    for (const Vector2D& soporte : soportes) {
        longitud += Vector2D::distancia(anterior, soporte);
        anterior = soporte;
    }
    longitud += Vector2D::distancia(anterior, punto_b);
    return longitud;
}

bool manejar_click_colocacion_cuerda(MotorFisica& motor, Vector2D mouse_pos) {
    if (estado_cuerda == EstadoColocacionCuerda::INACTIVA) return false;

    if (!motor.get_pausado()) {
        cancelar_colocacion_cuerda();
        spawn_error_timer = 0.5f;
        spawn_error_pos = mouse_pos;
        return true;
    }

    if (estado_cuerda == EstadoColocacionCuerda::ESPERANDO_EXTREMO_A) {
        AnclajeCuerda anclaje;
        Vector2D punto;
        if (detectar_anclaje_cuerda(motor, mouse_pos, anclaje, punto)) {
            // El torque solo puede ser soporte intermedio, no extremo_a
            if (anclaje.tipo == TipoAnclajeCuerda::SoporteFijo) return true;
            cuerda_extremo_a = anclaje;
            cuerda_punto_a_preview = punto;
            estado_cuerda = EstadoColocacionCuerda::ESPERANDO_SOPORTE;
        }
        return true;
    }

    if (estado_cuerda == EstadoColocacionCuerda::ESPERANDO_SOPORTE) {
        int soporte_id = -1;
        Vector2D punto_soporte;
        if (detectar_soporte_torque(motor, mouse_pos, soporte_id, punto_soporte)) {
            cuerda_soportes_id.push_back(soporte_id);
            cuerda_soportes_preview.push_back(punto_soporte);
            estado_cuerda = EstadoColocacionCuerda::ESPERANDO_EXTREMO_B;
            return true;
        }

        AnclajeCuerda extremo_b;
        Vector2D punto_b;
        if (detectar_anclaje_cuerda(motor, mouse_pos, extremo_b, punto_b)) {
            if (extremo_b.entidad_id == cuerda_extremo_a.entidad_id && extremo_b.tipo == cuerda_extremo_a.tipo) {
                return true; // Evitar anclar al mismo punto
            }
            const EntidadFisica* ent_a = buscar_entidad_por_id(motor, cuerda_extremo_a.entidad_id);
            if (ent_a) {
                Vector2D punto_a = posicion_anclaje_cuerda(ent_a, cuerda_extremo_a.tipo);
                double longitud = calcular_longitud_ruta_cuerda(punto_a, cuerda_soportes_preview, punto_b);
                motor.agregar_entidad(new Cuerda(
                    motor.generar_id(), cuerda_extremo_a, cuerda_soportes_id, extremo_b, longitud));
            }
            cancelar_colocacion_cuerda();
            return true;
        }
        return true;
    }

    if (estado_cuerda == EstadoColocacionCuerda::ESPERANDO_EXTREMO_B) {
        int soporte_id = -1;
        Vector2D punto_soporte;
        if (detectar_soporte_torque(motor, mouse_pos, soporte_id, punto_soporte)) {
            if (!soporte_ya_agregado(soporte_id)) {
                cuerda_soportes_id.push_back(soporte_id);
                cuerda_soportes_preview.push_back(punto_soporte);
            }
            return true;
        }

        AnclajeCuerda extremo_b;
        Vector2D punto_b;
        if (detectar_anclaje_cuerda(motor, mouse_pos, extremo_b, punto_b)) {
            if (extremo_b.entidad_id == cuerda_extremo_a.entidad_id && extremo_b.tipo == cuerda_extremo_a.tipo) {
                return true; // Evitar anclar al mismo punto
            }
            const EntidadFisica* ent_a = buscar_entidad_por_id(motor, cuerda_extremo_a.entidad_id);
            if (ent_a) {
                Vector2D punto_a = posicion_anclaje_cuerda(ent_a, cuerda_extremo_a.tipo);
                double longitud = calcular_longitud_ruta_cuerda(punto_a, cuerda_soportes_preview, punto_b);
                motor.agregar_entidad(new Cuerda(
                    motor.generar_id(), cuerda_extremo_a, cuerda_soportes_id, extremo_b, longitud));
            }
            cancelar_colocacion_cuerda();
            return true;
        }
        return true;
    }

    return false;
}

bool manejar_click_conexion_banda(MotorFisica& motor, Vector2D mouse_pos) {
    if (estado_banda == EstadoConexionBanda::INACTIVA) return false;

    if (!motor.get_pausado()) {
        estado_banda = EstadoConexionBanda::INACTIVA;
        banda_hamster_id = -1;
        return true;
    }

    const auto& ents = motor.get_entidades();

    if (estado_banda == EstadoConexionBanda::ESPERANDO_HAMSTER) {
        for (auto* e : ents) {
            auto* h = dynamic_cast<CajaHamster*>(e);
            if (!h) continue;
            if (!e->contiene_punto(mouse_pos)) continue;
            Vector2D p = e->get_posicion();
            banda_hamster_id = e->get_id();
            banda_origen_preview = Vector2D(p.x + h->get_ancho() * 0.5, p.y + h->get_alto() * 0.5);
            estado_banda = EstadoConexionBanda::ESPERANDO_VENTILADOR;
            return true;
        }
        return true;
    }

    if (estado_banda == EstadoConexionBanda::ESPERANDO_VENTILADOR) {
        for (auto* e : ents) {
            // No conectar con el mismo hámster origen
            if (e->get_id() == banda_hamster_id) continue;

            if (!e->contiene_punto(mouse_pos)) continue;

            auto* v = dynamic_cast<Ventilador*>(e);
            if (v) {
                motor.agregar_entidad(new Banda(motor.generar_id(), banda_hamster_id, e->get_id()));
                v->set_controlado_por_banda(true);
                estado_banda = EstadoConexionBanda::INACTIVA;
                banda_hamster_id = -1;
                return true;
            }

            auto* cs = dynamic_cast<CajaSorpresa*>(e);
            if (cs) {
                motor.agregar_entidad(new Banda(motor.generar_id(), banda_hamster_id, e->get_id()));
                estado_banda = EstadoConexionBanda::INACTIVA;
                banda_hamster_id = -1;
                return true;
            }

            auto* conv = dynamic_cast<Caminadora*>(e);
            if (conv) {
                motor.agregar_entidad(new Banda(motor.generar_id(), banda_hamster_id, e->get_id()));
                estado_banda = EstadoConexionBanda::INACTIVA;
                banda_hamster_id = -1;
                return true;
            }
        }
        return true;
    }

    return false;
}

// ============================================================================
// Menú lateral derecho — UI y drag-and-drop desde paleta
// ============================================================================
const int MENU_ANCHO = 300;
const int MENU_PESTANA_ALTO = 50;
const int MENU_CATEGORIA_ALTO = 40;
const int MENU_CELDA = 85;
const int MENU_COLS = 3;
const int MENU_MARGEN = 10;
const int MENU_PAGINACION_ALTO = 50;

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
Texture2D tex_celda_menu;          // Asset para las celdas del menú
Texture2D tex_barra_encabezado;    // Asset para los encabezados de categoría
Font fuente_menu = {0}; 
Texture2D tex_seguidor_cabezazo;   // Sprite del personaje cabeceando

// Texturas adicionales para assets
Texture2D tex_trampolin;
Texture2D tex_balancin_base;
Texture2D tex_balancin_tabla;
Texture2D tex_plata_larga;
Texture2D tex_plata_peque;
Texture2D tex_plata_rampa_izq;
Texture2D tex_plata_rampa_der;
Texture2D tex_hud_opciones;
Texture2D tex_hud_opciones_hover;
Texture2D tex_hud_ayuda;
Texture2D tex_hud_salir;
Texture2D tex_hud_play;
Texture2D tex_hud_reset;
Texture2D tex_hud_reset_hover;
Texture2D tex_menu_fondo;
Texture2D tex_menu_box;
Texture2D tex_btn_jugar1;
Texture2D tex_btn_jugar2;
Texture2D tex_btn_creativo1;
Texture2D tex_btn_creativo2;
Texture2D tex_btn_opciones1;
Texture2D tex_btn_opciones2;
Texture2D tex_btn_salir1;
Texture2D tex_btn_salir2;
Texture2D tex_menu_titulo;
Texture2D tex_robote_soporte;  // Soporte de BolaRebotadora (robot rojo)
Texture2D tex_robote_pelota;   // Pelota roja de BolaRebotadora
Texture2D tex_ventilador_cuerpo; // Cuerpo del ventilador
Texture2D tex_ventilador_aspa;   // Aspa del ventilador
Texture2D tex_caminadora;        // Sprite caminadora/conveyor

// Animaciones del SeguidorBooster
Animacion* anim_seguidor_corriendo = nullptr;
Texture2D tex_menu_inicio_anim;
Animacion* anim_menu_inicio = nullptr;
float pos_x_anim_menu = 0.0f;
Texture2D tex_menu_fede_anim;
Animacion* anim_menu_fede = nullptr;
Texture2D tex_menu_moto_anim;
Animacion* anim_menu_moto = nullptr;
float pos_x_anim_moto = 0.0f;
Texture2D tex_menu_jose_anim;
Animacion* anim_menu_jose = nullptr;
float pos_x_anim_jose = 0.0f;
Texture2D tex_menu_gusano_anim;
Animacion* anim_menu_gusano = nullptr;
float pos_x_anim_gusano = 0.0f;
float timer_espera_menu = 0.0f;
float timer_espera_moto = 0.0f;
Texture2D tex_menu_drom_anim;
Animacion* anim_menu_drom = nullptr;
float pos_x_anim_drom = 0.0f;
float timer_espera_drom = 0.0f;

int ancho_area_juego() {
    return menu_visible ? (ANCHO - MENU_ANCHO) : ANCHO;
}

bool punto_en_menu(int mx, int my) {
    // Siempre incluir el área del botón de abrir/cerrar
    int px = ANCHO - MENU_ANCHO;
    int bx = menu_visible ? (px - 28) : (ANCHO - 26);
    
    // Área del botón de abrir/cerrar
    if (mx >= bx && mx < bx + 24 && my >= ALTO / 2 - 40 && my < ALTO / 2 + 40) {
        return true;
    }
    
    // Área del menú Principal (solo si está visible)
    if (!menu_visible) return false;
    if (my >= ALTO - 150) return false; // El HUD está sobre el menú lateral en la parte inferior
    return mx >= px && mx < ANCHO && my >= 0 && my < ALTO;
}

bool punto_en_panel_izquierdo(int mx, int my) {
    if (estado_actual == EstadoJuego::JUEGO_NIVEL) return false;
    int w = 260;
    int bx = panel_izquierdo_visible ? w : 0;
    
    // Área del botón de abrir/cerrar del panel izquierdo
    if (mx >= bx && mx < bx + 24 && my >= ALTO / 2 - 40 && my < ALTO / 2 + 40) {
        return true;
    }
    
    if (!panel_izquierdo_visible) return false;
    return mx >= 0 && mx < w && my >= 0 && my < ALTO;
}

bool punto_en_area_juego(int mx, int my) {
    return mx >= 0 && mx < ancho_area_juego() && my >= 0 && my < ALTO;
}

// Verificar si una entidad es un borde del nivel (no se debe arrastrar ni eliminar)
bool es_borde_nivel(const EntidadFisica* e) {
    return e == borde_suelo || e == borde_izquierda || e == borde_derecha || e == borde_techo;
}

struct InfoHandles {
    Rectangle rects[8];
    HandleResize tipos[8];
};
InfoHandles calcular_handles_xywh(float x, float y, float w, float h) {
    const float S = 10.0f;
    float hs = S / 2.0f;
    InfoHandles out;
    out.tipos[0] = HandleResize::TOP_LEFT;    out.rects[0] = {x-hs,       y-hs,       S,S};
    out.tipos[1] = HandleResize::TOP_CENTER;  out.rects[1] = {x+w/2-hs,   y-hs,       S,S};
    out.tipos[2] = HandleResize::TOP_RIGHT;   out.rects[2] = {x+w-hs,     y-hs,       S,S};
    out.tipos[3] = HandleResize::MID_LEFT;    out.rects[3] = {x-hs,       y+h/2-hs,   S,S};
    out.tipos[4] = HandleResize::MID_RIGHT;   out.rects[4] = {x+w-hs,     y+h/2-hs,   S,S};
    out.tipos[5] = HandleResize::BOT_LEFT;    out.rects[5] = {x-hs,       y+h-hs,     S,S};
    out.tipos[6] = HandleResize::BOT_CENTER;  out.rects[6] = {x+w/2-hs,   y+h-hs,     S,S};
    out.tipos[7] = HandleResize::BOT_RIGHT;   out.rects[7] = {x+w-hs,     y+h-hs,     S,S};
    return out;
}
InfoHandles calcular_handles(const ParedRectangular* p) {
    return calcular_handles_xywh((float)p->get_posicion().x, (float)p->get_posicion().y,
                                 (float)p->get_ancho(), (float)p->get_alto());
}

// Obtener qué objeto está debajo del cursor del mouse (incluye estáticos)
EntidadFisica* obtener_entidad_bajo_mouse(const MotorFisica& motor, Vector2D mouse_pos) {
    for (auto* e : motor.get_entidades()) {
        if (es_borde_nivel(e)) continue;
        if (e->contiene_punto(mouse_pos)) {
            return e;
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
    int current_w = GetScreenWidth();
    int current_h = GetScreenHeight();
    if (current_w != ANCHO || current_h != ALTO) {
        ANCHO = current_w;
        ALTO = current_h;
        actualizar_bordes_nivel();
    }
}

void resetear_punteros_borde() {
    borde_suelo = nullptr;
    borde_izquierda = nullptr;
    borde_derecha = nullptr;
    borde_techo = nullptr;
}

void limpiar_estado_tras_cargar_partida() {
    entidad_arrastrada = nullptr;
    entidad_seleccionada = nullptr;
    arrastrando_spawn = TipoObjetoMenu::NINGUNO;
    cancelar_colocacion_cuerda();
    handle_activo = HandleResize::NINGUNO;
    inventario_maximo.clear();
    inventario_actual.clear();
    inventario_entidades.clear();
    snapshot_simulacion = "";
    menu_tab = 0;
    menu_pagina = 0;
}

void crear_bordes_nivel(MotorFisica& motor) {
    const int g = 20;
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
}

void crear_escena(MotorFisica& motor) {
    crear_bordes_nivel(motor);
}

// ============================================================================
// Validación de spawn — no crear objetos dentro de otros
// ============================================================================
bool posicion_valida_para_spawn(const MotorFisica& motor, Vector2D pos, TipoForma forma, double w_r, double h) {
    for (const auto* e : motor.get_entidades()) {
        TipoForma forma_e = e->get_tipo_forma();

        if (forma == TipoForma::CIRCULO) {
            double radio = w_r;
            if (forma_e == TipoForma::CIRCULO) {
                Vector2D pos_circ;
                double radio_circ = 0.0;
                if (obtener_datos_circulo(e, pos_circ, radio_circ)) {
                    InfoColision info = Colisiones::circulo_vs_circulo(pos, radio, pos_circ, radio_circ);
                    if (info.hay_colision) return false;
                }
            }
            else if (forma_e == TipoForma::AABB) {
                InfoColision info = Colisiones::circulo_vs_aabb(pos, radio, e->get_min(), e->get_max());
                if (info.hay_colision) return false;
            }
            else if (forma_e == TipoForma::POLIGONO) {
                const PlanoInclinado* ramp = dynamic_cast<const PlanoInclinado*>(e);
                if (ramp) {
                    InfoColision info = Colisiones::circulo_vs_poligono(pos, radio, ramp->get_vertices());
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
        else if (forma == TipoForma::AABB) {
            double w = w_r;
            if (forma_e == TipoForma::CIRCULO) {
                Vector2D pos_circ;
                double radio_circ = 0.0;
                if (obtener_datos_circulo(e, pos_circ, radio_circ)) {
                    InfoColision info = Colisiones::circulo_vs_aabb(pos_circ, radio_circ, pos, Vector2D(pos.x + w, pos.y + h));
                    if (info.hay_colision) return false;
                }
            }
            else if (forma_e == TipoForma::AABB) {
                Vector2D min_b = e->get_min();
                Vector2D max_b = e->get_max();
                bool overlap = (pos.x < max_b.x && pos.x + w > min_b.x &&
                                pos.y < max_b.y && pos.y + h > min_b.y);
                if (overlap) return false;
            }
            else if (forma_e == TipoForma::POLIGONO) {
                const PlanoInclinado* ramp = dynamic_cast<const PlanoInclinado*>(e);
                if (ramp) {
                    for (const auto& v : ramp->get_vertices()) {
                        if (v.x >= pos.x && v.x <= pos.x + w &&
                            v.y >= pos.y && v.y <= pos.y + h) {
                            return false;
                        }
                    }
                }
                const Balancin* bal = dynamic_cast<const Balancin*>(e);
                if (bal) {
                    Vector2D diff = (pos + Vector2D(w / 2.0, h / 2.0)) - bal->get_posicion();
                    if (diff.magnitud() < 40.0) return false;
                }
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

    if (!posicion_valida_para_spawn(motor, spawn_min, TipoForma::AABB, w, h)) {
        spawn_error_timer = 0.5f;
        spawn_error_pos = pos;
        return false;
    }

    Balancin* b = new Balancin(motor.generar_id(), pivot_pos, w, h);
    motor.agregar_entidad(b);
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

    if (!posicion_valida_para_spawn(motor, spawn_pos, TipoForma::AABB, w, h)) {
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
    if (!posicion_valida_para_spawn(motor, pos, TipoForma::CIRCULO, radio, 0.0)) {
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

bool crear_gancho(MotorFisica& motor, Vector2D pos) {
    motor.agregar_entidad(new Gancho(motor.generar_id(), pos));
    return true;
}

bool crear_pistola(MotorFisica& motor, Vector2D pos) {
    motor.agregar_entidad(new Pistola(motor.generar_id(), pos, 0.0));
    return true;
}

bool crear_foco(MotorFisica& motor, Vector2D pos) {
    motor.agregar_entidad(new Foco(motor.generar_id(), pos));
    return true;
}

bool crear_lupa(MotorFisica& motor, Vector2D pos) {
    motor.agregar_entidad(new Lupa(motor.generar_id(), pos, 0.0, 200.0));
    return true;
}

bool crear_canon(MotorFisica& motor, Vector2D pos) {
    motor.agregar_entidad(new Canon(motor.generar_id(), pos, 180.0)); // apunta izquierda
    return true;
}


bool crear_dinamita(MotorFisica& motor, Vector2D pos) {
    Vector2D spawn(pos.x - 13.0, pos.y - 23.0);
    motor.agregar_entidad(new Dinamita(motor.generar_id(), spawn, 26.0, 46.0));
    return true;
}

bool crear_ladrillo_vertical(MotorFisica& motor, Vector2D pos) {
    Vector2D spawn(pos.x - 20.0, pos.y - 60.0);
    motor.agregar_entidad(new LadrilloVertical(motor.generar_id(), spawn, 40.0, 120.0));
    return true;
}

bool crear_ladrillo_horizontal(MotorFisica& motor, Vector2D pos) {
    Vector2D spawn(pos.x - 60.0, pos.y - 20.0);
    motor.agregar_entidad(new LadrilloHorizontal(motor.generar_id(), spawn, 120.0, 40.0));
    return true;
}

bool crear_dinamita_detonador(MotorFisica& motor, Vector2D pos) {
    Vector2D spawn(pos.x - 13.0, pos.y - 23.0);
    motor.agregar_entidad(new DinamitaDetonador(motor.generar_id(), spawn, 26.0, 46.0));
    return true;
}

bool crear_gato(MotorFisica& motor, Vector2D pos) {
    motor.agregar_entidad(new Gato(motor.generar_id(), pos, 54.0, 58.0));
    return true;
}

bool crear_raton(MotorFisica& motor, Vector2D pos) {
    motor.agregar_entidad(new Raton(motor.generar_id(), pos, 22.0, 12.0));
    return true;
}

bool crear_escalon(MotorFisica& motor, Vector2D pos) {
    Vector2D spawn(pos.x - 25.0, pos.y - 13.0);
    motor.agregar_entidad(new Escalon(motor.generar_id(), spawn, 50.0, 26.0));
    return true;
}

bool crear_rampa(MotorFisica& motor, Vector2D pos, bool invertido) {
    double b = 160.0;
    double h = 120.0;
    Vector2D spawn(pos.x - b / 2.0, pos.y - h / 2.0);
    motor.agregar_entidad(new PlanoInclinado(motor.generar_id(), spawn, b, h, invertido));
    return true;
}

bool crear_plataforma(MotorFisica& motor, Vector2D pos, double w, double h, TipoObjetoMenu t_menu = TipoObjetoMenu::PLATAFORMA) {
    Vector2D spawn(pos.x - w / 2.0, pos.y - h / 2.0);
    motor.agregar_entidad(new ParedRectangular(motor.generar_id(), spawn, w, h, t_menu));
    return true;
}

bool crear_bola_rebotadora(MotorFisica& motor, Vector2D pos);
bool crear_ventilador(MotorFisica& motor, Vector2D pos);
bool crear_seguidor_booster(MotorFisica& motor, Vector2D pos);
bool crear_barril_chavo(MotorFisica& motor, Vector2D pos);
bool crear_cubeta(MotorFisica& motor, Vector2D pos);
bool crear_soporte_torque(MotorFisica& motor, Vector2D pos);
bool crear_globo(MotorFisica& motor, Vector2D pos);
bool crear_bola_beisbol(MotorFisica& motor, Vector2D pos);
bool crear_tijera(MotorFisica& motor, Vector2D pos);
bool crear_caja_hamster(MotorFisica& motor, Vector2D pos);
bool crear_banda(MotorFisica& motor, Vector2D pos);
bool crear_caja_sorpresa(MotorFisica& motor, Vector2D pos);
bool crear_caminadora(MotorFisica& motor, Vector2D pos);

bool crear_zona_meta(MotorFisica& motor, Vector2D pos) {
    motor.agregar_entidad(new ZonaMeta(motor.generar_id(), pos, 80.0, 80.0));
    return true;
}

bool spawn_desde_menu(MotorFisica& motor, TipoObjetoMenu tipo, Vector2D pos) {
    switch (tipo) {
        case TipoObjetoMenu::BOLA:              return crear_bola(motor, pos);
        case TipoObjetoMenu::BOLA_REBOTADORA:   return crear_bola_rebotadora(motor, pos);
        case TipoObjetoMenu::TRAMPOLIN:         return crear_trampolin(motor, pos);
        case TipoObjetoMenu::BALANCIN:          return crear_balancin(motor, pos);
        case TipoObjetoMenu::RAMPA:             return crear_rampa(motor, pos, false);
        case TipoObjetoMenu::PLATAFORMA:        return crear_plataforma(motor, pos, 150.0, 15.0, TipoObjetoMenu::PLATAFORMA);
        case TipoObjetoMenu::PARED_LARGA:       return crear_plataforma(motor, pos, 80.0, 120.0, TipoObjetoMenu::PARED_LARGA);
        case TipoObjetoMenu::PLATAFORMA_DECOR:   return crear_plataforma(motor, pos, 120.0, 20.0, TipoObjetoMenu::PLATAFORMA_DECOR);
        case TipoObjetoMenu::SEGUIDOR_BOOSTER:  return crear_seguidor_booster(motor, pos);
        case TipoObjetoMenu::BARRIL_CHAVO:      return crear_barril_chavo(motor, pos);
        case TipoObjetoMenu::VENTILADOR:        return crear_ventilador(motor, pos);
        case TipoObjetoMenu::CUBETA:            return crear_cubeta(motor, pos);
        case TipoObjetoMenu::SOPORTE_TORQUE:    return crear_soporte_torque(motor, pos);
        case TipoObjetoMenu::GANCHO:            return crear_gancho(motor, pos);
        case TipoObjetoMenu::PISTOLA:           return crear_pistola(motor, pos);
        case TipoObjetoMenu::FOCO:              return crear_foco(motor, pos);
        case TipoObjetoMenu::LUPA:              return crear_lupa(motor, pos);
        case TipoObjetoMenu::CANON:             return crear_canon(motor, pos);
        case TipoObjetoMenu::LADRILLO_VERTICAL:  return crear_ladrillo_vertical(motor, pos);
        case TipoObjetoMenu::LADRILLO_HORIZONTAL: return crear_ladrillo_horizontal(motor, pos);
        case TipoObjetoMenu::DINAMITA:          return crear_dinamita(motor, pos);
        case TipoObjetoMenu::DINAMITA_DETONADOR: return crear_dinamita_detonador(motor, pos);
        case TipoObjetoMenu::GATO:              return crear_gato(motor, pos);
        case TipoObjetoMenu::RATON:             return crear_raton(motor, pos);
        case TipoObjetoMenu::ESCALON:           return crear_escalon(motor, pos);
        case TipoObjetoMenu::GLOBO:             return crear_globo(motor, pos);
        case TipoObjetoMenu::BOLA_BEISBOL:      return crear_bola_beisbol(motor, pos);
        case TipoObjetoMenu::TIJERA:            return crear_tijera(motor, pos);
        case TipoObjetoMenu::ZONA_META:         return crear_zona_meta(motor, pos);
        case TipoObjetoMenu::CUERDA:            return false;
        case TipoObjetoMenu::CAJA_HAMSTER:      return crear_caja_hamster(motor, pos);
        case TipoObjetoMenu::BANDA:             return crear_banda(motor, pos);
        case TipoObjetoMenu::CAJA_SORPRESA:     return crear_caja_sorpresa(motor, pos);
        case TipoObjetoMenu::CAMINADORA:          return crear_caminadora(motor, pos);
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
        case TipoObjetoMenu::RAMPA: {
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
        case TipoObjetoMenu::ESCALON: {
            // Triángulo pequeño (rampita)
            float s = 13.0f * escala;
            Vector2 v1 = {cx - s, cy + s*0.6f};
            Vector2 v2 = {cx + s, cy + s*0.6f};
            Vector2 v3 = {cx + s, cy - s*0.9f};
            DrawTriangle(v1, v2, v3, tint(COLOR_RAMPA));
            DrawTriangleLines(v1, v2, v3, tint(COLOR_RAMPA_BORDE));
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
        case TipoObjetoMenu::CUBETA: {
            float w = 34.0f * escala;
            float h = 30.0f * escala;
            float x = cx - w / 2.0f;
            float y = cy - h / 2.0f + 4.0f * escala;
            DrawRectangleRec({x + 4 * escala, y + 6 * escala, w - 8 * escala, h - 6 * escala},
                             tint(Color{120, 145, 160, 255}));
            DrawRectangleLinesEx({x + 4 * escala, y + 6 * escala, w - 8 * escala, h - 6 * escala},
                                 1.5f, tint(Color{50, 60, 70, 255}));
            DrawLineEx({x + 6 * escala, y + 8 * escala}, {x + w / 2, y - 4 * escala},
                       2.0f, tint(Color{210, 220, 225, 255}));
            DrawLineEx({x + w - 6 * escala, y + 8 * escala}, {x + w / 2, y - 4 * escala},
                       2.0f, tint(Color{210, 220, 225, 255}));
            DrawCircle(static_cast<int>(cx), static_cast<int>(y - 4 * escala), 3.5f * escala,
                       tint(Color{40, 45, 50, 255}));
            break;
        }
        case TipoObjetoMenu::CUERDA: {
            Color cuerda_col = tint(Color{214, 184, 112, 255});
            Color sombra = tint(Color{116, 86, 42, 255});
            float amp = 5.0f * escala;
            float paso = 6.0f * escala;
            Vector2 prev = {cx - 28.0f * escala, cy};
            for (int i = 1; i <= 10; ++i) {
                float x = cx - 28.0f * escala + paso * i;
                float y = cy + std::sin(i * 0.9f) * amp;
                DrawLineEx(prev, {x, y}, 5.0f * escala, sombra);
                DrawLineEx(prev, {x, y}, 3.0f * escala, cuerda_col);
                prev = {x, y};
            }
            DrawCircleLines(static_cast<int>(cx - 15.0f * escala), static_cast<int>(cy + 13.0f * escala),
                            9.0f * escala, cuerda_col);
            DrawCircleLines(static_cast<int>(cx), static_cast<int>(cy + 14.0f * escala),
                            10.0f * escala, cuerda_col);
            DrawCircleLines(static_cast<int>(cx + 16.0f * escala), static_cast<int>(cy + 13.0f * escala),
                            9.0f * escala, cuerda_col);
            break;
        }
        case TipoObjetoMenu::SOPORTE_TORQUE: {
            float r = 15.0f * escala;
            DrawCircleV({cx, cy}, r, tint(Color{135, 140, 145, 255}));
            DrawCircleLines(static_cast<int>(cx), static_cast<int>(cy), r,
                            tint(Color{65, 70, 78, 255}));
            DrawCircleV({cx, cy}, 5.0f * escala, tint(Color{35, 38, 42, 255}));
            break;
        }
        case TipoObjetoMenu::GANCHO: {
            // Eye bolt: aro arriba + cuerpo con rosca abajo
            Color gc = tint(Color{170, 178, 186, 255});
            Color gd = tint(Color{100, 108, 116, 255});
            Color gb = tint(Color{210, 215, 220, 255});
            float r = 10.0f * escala;
            // cuerpo con rosca
            float bw = r * 1.1f, bh = r * 1.1f;
            DrawRectangleRec({cx - bw/2, cy, bw, bh}, gc);
            DrawRectangleLinesEx({cx - bw/2, cy, bw, bh}, 1.2f, gd);
            for (int i = 1; i <= 2; i++)
                DrawLineEx({cx - bw/2 + 1, cy + bh*i/3.0f}, {cx + bw/2 - 1, cy + bh*i/3.0f}, 1.0f, gd);
            // cuello
            float nw = bw * 0.7f, nh = r * 0.3f;
            DrawRectangleRec({cx - nw/2, cy - nh, nw, nh}, gc);
            DrawRectangleLinesEx({cx - nw/2, cy - nh, nw, nh}, 1.0f, gd);
            // aro
            float ar = r * 0.72f;
            float ay = cy - nh - ar;
            DrawRing({cx, ay}, ar * 0.52f, ar, 0, 360, 20, gc);
            DrawCircleLines((int)cx, (int)ay, ar, gd);
            DrawCircleLines((int)cx, (int)ay, ar * 0.52f, gd);
            DrawCircleLines((int)(cx - ar*0.2f), (int)(ay - ar*0.2f), ar*0.18f, gb);
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
        case TipoObjetoMenu::PISTOLA: {
            Color pc = tint(Color{60, 65, 70, 255});
            Color pm = tint(Color{100, 60, 30, 255});
            Color pml = tint(Color{140, 148, 155, 255});
            float s = escala;
            // mango
            DrawRectangleRec({cx - 4*s, cy, 14*s, 16*s}, pm);
            // cuerpo
            DrawRectangleRec({cx - 18*s, cy - 8*s, 36*s, 13*s}, pc);
            // cañón
            DrawRectangleRec({cx + 6*s, cy - 5*s, 20*s, 8*s}, pml);
            // aros rojos (gatillo)
            DrawCircle((int)(cx - 2*s), (int)(cy + 4*s), 4*s, tint(Color{200, 40, 40, 255}));
            break;
        }
        case TipoObjetoMenu::FOCO: {
            float r = 11.0f * escala;
            DrawCircle((int)cx, (int)(cy - 2*escala), r * 1.5f, tint(Color{255, 230, 120, 70}));
            DrawCircle((int)cx, (int)(cy - 2*escala), r, tint(Color{255, 235, 130, 255}));
            DrawCircleLines((int)cx, (int)(cy - 2*escala), r, tint(Color{210, 170, 40, 255}));
            DrawLineEx({cx - r*0.35f, cy - escala}, {cx, cy - r*0.4f - 2*escala}, 1.6f, tint(Color{255,150,30,255}));
            DrawLineEx({cx, cy - r*0.4f - 2*escala}, {cx + r*0.35f, cy - escala}, 1.6f, tint(Color{255,150,30,255}));
            DrawRectangleRec({cx - r*0.45f, cy + r*0.7f - 2*escala, r*0.9f, r*0.55f}, tint(Color{150,155,160,255}));
            break;
        }
        case TipoObjetoMenu::LUPA: {
            // Lente vertical + rayo horizontal
            float rx = 6.0f * escala, ry = 11.0f * escala;
            DrawLineEx({cx, cy}, {cx + 16*escala, cy}, 2.0f, tint(Color{255,240,150,160}));
            DrawEllipse((int)cx, (int)cy, rx, ry, tint(Color{200,230,255,120}));
            DrawEllipseLines((int)cx, (int)cy, rx, ry, tint(Color{110,90,60,255}));
            DrawLineEx({cx, cy + ry}, {cx, cy + ry + 6*escala}, 2.5f, tint(Color{110,90,60,255}));
            break;
        }
        case TipoObjetoMenu::CANON: {
            float s = escala;
            Color metal = tint(Color{70, 75, 82, 255});
            Color metal2 = tint(Color{45, 48, 54, 255});
            Color madera = tint(Color{110, 70, 35, 255});
            // base
            DrawRectangleRec({cx - 12*s, cy + 4*s, 24*s, 7*s}, madera);
            DrawCircle((int)(cx - 7*s), (int)(cy + 11*s), 3.5f*s, metal2);
            DrawCircle((int)(cx + 7*s), (int)(cy + 11*s), 3.5f*s, metal2);
            // tubo apuntando a la izquierda
            DrawLineEx({cx + 4*s, cy - 2*s}, {cx - 16*s, cy - 2*s}, 11*s, metal);
            DrawCircle((int)(cx - 16*s), (int)(cy - 2*s), 5.5f*s, metal);
            DrawCircle((int)(cx - 16*s), (int)(cy - 2*s), 3*s, tint(Color{20,20,24,255}));
            // mecha atras con chispa
            DrawLineEx({cx + 8*s, cy - 4*s}, {cx + 14*s, cy - 10*s}, 1.8f, tint(Color{60,45,30,255}));
            DrawCircle((int)(cx + 14*s), (int)(cy - 10*s), 2.5f*s, tint(Color{255,200,60,255}));
            break;
        }
        case TipoObjetoMenu::LADRILLO_VERTICAL: {
            float w = 14.0f * escala, h = 30.0f * escala;
            float x0 = cx - w/2, y0 = cy - h/2;
            DrawRectangleRec({x0, y0, w, h}, tint(Color{205,195,180,255}));
            int filas = 5;
            for (int fila=0; fila<filas; ++fila) {
                float ry = y0 + fila*(h/filas);
                float off = (fila%2==0)?0.0f:w*0.5f;
                for (float bx=x0-off; bx<x0+w; bx+=w) {
                    float a=fmaxf(bx,x0), b=fminf(bx+w-1.5f, x0+w);
                    if (b>a) DrawRectangleRec({a, ry+1, b-a, h/filas-2}, tint(Color{170,70,55,255}));
                }
            }
            DrawRectangleLinesEx({x0,y0,w,h}, 1.5f, tint(Color{110,45,35,255}));
            // flechas verticales (indica que crece en alto)
            DrawLineEx({cx, y0-4*escala},{cx, y0-1*escala}, 1.5f, tint(Color{60,60,70,255}));
            DrawLineEx({cx, y0+h+1*escala},{cx, y0+h+4*escala}, 1.5f, tint(Color{60,60,70,255}));
            break;
        }
        case TipoObjetoMenu::LADRILLO_HORIZONTAL: {
            float w = 34.0f * escala, h = 14.0f * escala;
            float x0 = cx - w/2, y0 = cy - h/2;
            DrawRectangleRec({x0, y0, w, h}, tint(Color{205,195,180,255}));
            int filas = 2;
            for (int fila=0; fila<filas; ++fila) {
                float ry = y0 + fila*(h/filas);
                float off = (fila%2==0)?0.0f:w*0.16f;
                for (float bx=x0-off; bx<x0+w; bx+=w*0.33f) {
                    float a=fmaxf(bx,x0), b=fminf(bx+w*0.33f-1.5f, x0+w);
                    if (b>a) DrawRectangleRec({a, ry+1, b-a, h/filas-2}, tint(Color{170,70,55,255}));
                }
            }
            DrawRectangleLinesEx({x0,y0,w,h}, 1.5f, tint(Color{110,45,35,255}));
            // flechas horizontales (indica que crece en ancho)
            DrawLineEx({x0-4*escala, cy},{x0-1*escala, cy}, 1.5f, tint(Color{60,60,70,255}));
            DrawLineEx({x0+w+1*escala, cy},{x0+w+4*escala, cy}, 1.5f, tint(Color{60,60,70,255}));
            break;
        }
        case TipoObjetoMenu::DINAMITA: {
            float w = 16.0f*escala, h = 24.0f*escala;
            float x0 = cx - w/2, y0 = cy - h/2 + 2*escala;
            for (int i=0;i<3;++i) {
                float bx = x0 + i*(w/3.0f);
                DrawRectangleRec({bx+0.5f, y0, w/3.0f-1.0f, h}, tint(Color{190,55,45,255}));
            }
            DrawRectangleRec({x0, y0 + h*0.45f, w, 3.0f}, tint(Color{90,60,30,255}));
            // mecha con chispa
            DrawLineEx({cx, y0}, {cx + 4*escala, y0 - 6*escala}, 1.6f, tint(Color{70,55,35,255}));
            DrawCircle((int)(cx + 4*escala), (int)(y0 - 6*escala), 2.2f*escala, tint(Color{255,200,60,255}));
            break;
        }
        case TipoObjetoMenu::DINAMITA_DETONADOR: {
            float w = 12.0f*escala, h = 22.0f*escala;
            float x0 = cx - 10*escala, y0 = cy - h/2 + 2*escala;
            for (int i=0;i<3;++i) {
                float bx = x0 + i*(w/3.0f);
                DrawRectangleRec({bx+0.5f, y0, w/3.0f-1.0f, h}, tint(Color{190,55,45,255}));
            }
            // cable + detonador (émbolo) a la derecha
            float dx = cx + 12*escala, dy = cy;
            DrawLineBezier({x0+w, y0+h*0.5f}, {dx, dy}, 1.8f, tint(Color{40,40,45,255}));
            DrawRectangleRec({dx-5*escala, dy-3*escala, 10*escala, 8*escala}, tint(Color{120,70,30,255}));
            DrawRectangleRec({dx-1*escala, dy-9*escala, 2*escala, 6*escala}, tint(Color{150,155,160,255}));
            DrawRectangleRec({dx-4*escala, dy-10*escala, 8*escala, 2*escala}, tint(Color{150,155,160,255}));
            break;
        }
        case TipoObjetoMenu::GATO: {
            Color c = tint(Color{235,235,240,255});
            Color o = tint(Color{170,170,180,255});
            float bw = 16*escala, bh = 11*escala;
            // cola
            DrawLineBezier({cx - bw*0.6f, cy}, {cx - bw*0.9f, cy - bh*1.2f}, 2.5f, c);
            // cuerpo
            DrawEllipse((int)cx, (int)(cy+2*escala), bw*0.6f, bh*0.5f, c);
            // cabeza
            float hx = cx + bw*0.4f, hy = cy - bh*0.3f;
            DrawCircle((int)hx, (int)hy, bh*0.55f, c);
            DrawTriangle({hx-4*escala, hy-3*escala},{hx-1*escala,hy-8*escala},{hx+1*escala,hy-2*escala}, c);
            DrawTriangle({hx+4*escala, hy-3*escala},{hx+2*escala,hy-8*escala},{hx-1*escala,hy-2*escala}, c);
            DrawCircle((int)(hx+2*escala),(int)hy, 1.5f, tint(Color{80,150,90,255}));
            DrawEllipseLines((int)cx, (int)(cy+2*escala), bw*0.6f, bh*0.5f, o);
            break;
        }
        case TipoObjetoMenu::RATON: {
            Color c = tint(Color{150,150,158,255});
            Color rosa = tint(Color{235,150,160,255});
            float bw = 15*escala, bh = 8*escala;
            // cola
            DrawLineBezier({cx - bw*0.5f, cy}, {cx - bw*0.8f, cy - bh}, 1.5f, rosa);
            // cuerpo
            DrawEllipse((int)cx, (int)cy, bw*0.5f, bh*0.5f, c);
            // cabeza + oreja
            float hx = cx + bw*0.4f;
            DrawCircle((int)hx, (int)cy, bh*0.5f, c);
            DrawCircle((int)(hx - 2*escala), (int)(cy - bh*0.5f), bh*0.4f, c);
            DrawCircle((int)(hx - 2*escala), (int)(cy - bh*0.5f), bh*0.22f, rosa);
            DrawCircle((int)(hx + bh*0.4f), (int)cy, 1.2f, rosa);
            break;
        }
        case TipoObjetoMenu::GLOBO: {
            float r = 14.0f * escala;
            DrawCircle((int)cx, (int)(cy + 4 * escala), r, tint(Color{220, 50, 50, 255}));
            DrawCircleLines((int)cx, (int)(cy + 4 * escala), r, tint(Color{150, 20, 20, 255}));
            // hilo
            DrawLineEx({cx, cy + 4 * escala + r}, {cx, cy + r * 2.2f + 4 * escala}, 1.5f, tint(Color{80, 80, 80, 255}));
            break;
        }
        case TipoObjetoMenu::TIJERA: {
            // Tijera horizontal: aros a la izquierda, puntas a la derecha
            float hx = 18.0f * escala;
            float hy = 9.0f * escala;
            float ra = 6.5f * escala;
            Color sc = tint(Color{175, 182, 195, 255});
            Color sd = tint(Color{80, 88, 100, 255});
            // cuchilla superior: aro izq arriba -> punta derecha arriba
            DrawLineEx({cx - hx + ra, cy - hy * 0.3f}, {cx + hx, cy - hy}, 2.8f * escala, sc);
            // cuchilla inferior: aro izq abajo -> punta derecha abajo
            DrawLineEx({cx - hx + ra, cy + hy * 0.3f}, {cx + hx, cy + hy}, 2.8f * escala, sc);
            // aros izquierda (rojos como mangos)
            DrawCircle((int)(cx - hx + ra), (int)(cy - hy * 0.55f), ra, tint(Color{200, 40, 40, 255}));
            DrawCircleLines((int)(cx - hx + ra), (int)(cy - hy * 0.55f), ra, sd);
            DrawCircle((int)(cx - hx + ra), (int)(cy + hy * 0.55f), ra, tint(Color{200, 40, 40, 255}));
            DrawCircleLines((int)(cx - hx + ra), (int)(cy + hy * 0.55f), ra, sd);
            // pivote central
            DrawCircle((int)(cx - hx * 0.1f), (int)cy, 2.5f * escala, sd);
            // línea verde de corte vertical
            DrawLineEx({cx + hx * 0.15f, cy - hy * 1.4f}, {cx + hx * 0.15f, cy + hy * 1.4f}, 1.5f * escala, tint(Color{80, 200, 80, 255}));
            break;
        }
        case TipoObjetoMenu::BOLA_BEISBOL: {
            float r = 14.0f * escala;
            DrawCircle((int)cx, (int)cy, r, tint(Color{240, 235, 220, 255}));
            DrawCircleLines((int)cx, (int)cy, r, tint(Color{180, 170, 150, 255}));
            // costuras
            DrawLineEx({cx - r * 0.3f, cy - r * 0.8f}, {cx - r * 0.3f, cy + r * 0.8f}, 1.5f, tint(Color{200, 60, 60, 255}));
            DrawLineEx({cx + r * 0.3f, cy - r * 0.8f}, {cx + r * 0.3f, cy + r * 0.8f}, 1.5f, tint(Color{200, 60, 60, 255}));
            break;
        }
        case TipoObjetoMenu::ZONA_META: {
            float w = 36.0f * escala;
            float h = 36.0f * escala;
            // bandera ajedrez verde/blanco
            int tiles = 3;
            float tw = w / tiles, th = h / tiles;
            for (int row = 0; row < tiles; row++) {
                for (int col = 0; col < tiles; col++) {
                    Color tc = ((row + col) % 2 == 0) ? tint(Color{60, 180, 80, 200}) : tint(Color{240, 240, 240, 200});
                    DrawRectangleRec({cx - w/2 + col * tw, cy - h/2 + row * th, tw, th}, tc);
                }
            }
            DrawRectangleLinesEx({cx - w/2, cy - h/2, w, h}, 1.5f, tint(Color{30, 120, 50, 255}));
            break;
        }
        case TipoObjetoMenu::CAJA_HAMSTER: {
            // Caja izquierda + rueda derecha
            float bw = 14.0f * escala, bh = 24.0f * escala;
            float rr = 12.0f * escala;
            DrawRectangleRec({cx - 20*escala, cy - bh/2, bw, bh}, tint(Color{140, 100, 50, 255}));
            DrawCircle((int)(cx + 2*escala), (int)cy, rr, tint(Color{210, 170, 80, 255}));
            DrawCircleLines((int)(cx + 2*escala), (int)cy, rr, tint(Color{80, 50, 20, 255}));
            // hámster mini
            DrawCircle((int)(cx + 2*escala), (int)(cy + rr*0.3f), rr*0.3f, tint(Color{230, 200, 160, 255}));
            break;
        }
        case TipoObjetoMenu::BANDA: {
            float hw = 22.0f * escala;
            DrawLineEx({cx - hw, cy - 4*escala}, {cx + hw, cy - 4*escala}, 2.5f*escala, tint(Color{255, 180, 50, 220}));
            DrawLineEx({cx - hw, cy + 4*escala}, {cx + hw, cy + 4*escala}, 2.5f*escala, tint(Color{255, 180, 50, 220}));
            DrawCircle((int)(cx), (int)cy, 4*escala, tint(Color{255, 200, 80, 255}));
            break;
        }
        case TipoObjetoMenu::CAJA_SORPRESA: {
            float bw = 26.0f * escala, bh = 26.0f * escala;
            // Caja roja con lunares amarillos
            DrawRectangleRec({cx - bw/2, cy - bh/2 + 4*escala, bw, bh * 0.85f}, tint(Color{210,40,40,255}));
            DrawCircle((int)(cx - bw*0.25f),(int)(cy + 4*escala), bw*0.1f, tint(Color{255,200,50,255}));
            DrawCircle((int)(cx + bw*0.25f),(int)(cy + 4*escala), bw*0.1f, tint(Color{255,200,50,255}));
            // Tapa
            DrawRectangleRec({cx - bw/2 - 2*escala, cy - bh/2, bw + 4*escala, bh*0.18f}, tint(Color{180,30,30,255}));
            // Cabeza asomando
            DrawCircle((int)cx, (int)(cy - bh/2 - 8*escala), 9*escala, tint(Color{255,220,170,255}));
            DrawCircle((int)cx, (int)(cy - bh/2 - 9*escala), 4*escala, tint(Color{220,40,40,255}));
            break;
        }
        case TipoObjetoMenu::CAMINADORA: {
            float cw = 40.0f * escala, ch = 10.0f * escala;
            float rr = ch * 0.5f;
            // Cuerpo
            DrawRectangleRec({cx - cw/2 + rr, cy - ch/2, cw - rr*2, ch}, tint(Color{60,80,100,255}));
            // Cinta central
            DrawRectangleRec({cx - cw/2 + rr, cy - ch*0.25f, cw - rr*2, ch*0.5f}, tint(Color{80,100,120,255}));
            // Ruedas
            DrawCircle((int)(cx - cw/2 + rr), (int)cy, rr, tint(Color{70,70,80,255}));
            DrawCircle((int)(cx + cw/2 - rr), (int)cy, rr, tint(Color{70,70,80,255}));
            // Flecha
            DrawLineEx({cx - 8*escala, cy}, {cx + 8*escala, cy}, 1.5f*escala, tint(Color{255,200,80,220}));
            DrawLineEx({cx + 8*escala, cy}, {cx + 4*escala, cy - 4*escala}, 1.5f*escala, tint(Color{255,200,80,220}));
            DrawLineEx({cx + 8*escala, cy}, {cx + 4*escala, cy + 4*escala}, 1.5f*escala, tint(Color{255,200,80,220}));
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
    if (estado_actual == EstadoJuego::JUEGO_NIVEL) {
        if (pagina != 0) return;
        for (int i = 0; i < CATALOGO_MENU_COUNT; ++i) {
            const ItemCatalogo& it = CATALOGO_MENU[i];
            if (it.tab == tab && it.categoria == categoria) {
                if (inventario_maximo[it.tipo] > 0) {
                    out.push_back(&it);
                }
            }
        }
    } else {
        for (int i = 0; i < CATALOGO_MENU_COUNT; ++i) {
            const ItemCatalogo& it = CATALOGO_MENU[i];
            if (it.tab == tab && it.pagina == pagina && it.categoria == categoria)
                out.push_back(&it);
        }
    }
}

int contar_paginas_tab(int tab) {
    if (estado_actual == EstadoJuego::JUEGO_NIVEL) {
        return 1;
    }
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

void dibujar_celda_menu(const RectCeldaMenu& celda, bool resaltada){
    int cant = 1;
    bool disp = celda.disponible;
    if (estado_actual == EstadoJuego::JUEGO_NIVEL) {
        cant = inventario_actual[celda.tipo];
        if (cant <= 0) {
            disp = false;
        }
    }

    Color fondo = resaltada ? MENU_AZUL_CLARO : MENU_CELDA_FONDO;
    if (!disp) fondo = ColorAlpha(MENU_CELDA_FONDO, 0.5f);

    // Dibujar asset de celda si está cargado
    if (tex_celda_menu.id > 0) {
        DrawTexturePro(tex_celda_menu, 
                      {0, 0, (float)tex_celda_menu.width, (float)tex_celda_menu.height},
                      celda.rect,
                      {0, 0}, 0.0f, fondo);
    } else {
        // Fallback si no se carga la textura
        DrawRectangleRounded(celda.rect, 0.08f, 6, fondo);
    }
    DrawRectangleRoundedLinesEx(celda.rect, 0.08f, 6, 1.0f,
        disp ? MENU_BORDE : MENU_INACTIVO);

    float cx = celda.rect.x + celda.rect.width / 2.0f;
    float cy = celda.rect.y + celda.rect.height / 2.0f - 4.0f;
    unsigned char alpha = disp ? 255 : 120;

    if (celda.tipo != TipoObjetoMenu::NINGUNO)
        dibujar_icono_objeto(celda.tipo, cx, cy, 1.0f, alpha);
    else
        dibujar_icono_objeto(TipoObjetoMenu::NINGUNO, cx, cy, 1.0f, alpha);

    std::string cant_str = (estado_actual == EstadoJuego::JUEGO_CREATIVO) ? "inf" : "x" + std::to_string(cant);
    
    DrawText(cant_str.c_str(),
        static_cast<int>(celda.rect.x + celda.rect.width / 2 - MeasureText(cant_str.c_str(), 12) / 2),
        static_cast<int>(celda.rect.y + celda.rect.height - 18),
        12, disp ? MENU_AZUL : MENU_INACTIVO);
}

void dibujar_encabezado_categoria(int panel_x, int y, const char* titulo, bool abierta, int& out_alto) {
    Rectangle hdr = {
        static_cast<float>(panel_x + MENU_MARGEN)+10,
        static_cast<float>(y),
        static_cast<float>(MENU_ANCHO - 2 * MENU_MARGEN)-25,
        static_cast<float>(MENU_CATEGORIA_ALTO)
    };
    
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, hdr);
    Color tint_color = hover ? Color{230, 235, 245, 255} : WHITE;
    Color fallback_color = hover ? Color{165, 170, 182, 255} : MENU_FONDO_OSCURO;

    // Dibujar asset de encabezado si está cargado
    if (tex_barra_encabezado.id > 0) {
        DrawTexturePro(tex_barra_encabezado, 
                      {0, 0, (float)tex_barra_encabezado.width, (float)tex_barra_encabezado.height},
                      hdr,
                      {0, 0}, 0.0f, tint_color);
    } else {
        // Fallback si no se carga la textura
        DrawRectangleRounded(hdr, 0.06f, 4, fallback_color);
    }



    int tamanio_titulo = 17;
    Vector2 title_size = MeasureTextEx(fuente_menu, titulo, tamanio_titulo, 1);
    Vector2 title_pos = {hdr.x +5+ (hdr.width - title_size.x) / 2 , hdr.y + (hdr.height - title_size.y) / 2 -8};
    DrawTextEx(fuente_menu, titulo, title_pos, tamanio_titulo, 1, MENU_TEXTO);
    

    const char* chevron = abierta ? "v" : ">";
    
    int cw = MeasureText(chevron, 16);
    
    DrawText(chevron, static_cast<int>(hdr.x + hdr.width - cw - 5)-235, static_cast<int>(hdr.y + 6), 22, MENU_AZUL);
    
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
    Vector2 mouse = GetMousePosition();
    if (!menu_visible) {
        int bx = ANCHO - 26;
        bool hover = CheckCollisionPointRec(mouse, Rectangle{static_cast<float>(bx), static_cast<float>(ALTO / 2 - 40), 24, 80});
        DrawRectangle(bx, ALTO / 2 - 40, 24, 80, hover ? Color{165, 170, 182, 255} : MENU_FONDO_OSCURO);
        DrawRectangleLines(bx, ALTO / 2 - 40, 24, 80, MENU_BORDE);
        DrawText("<", bx + 7, ALTO / 2 - 8, 18, hover ? MENU_AZUL_CLARO : MENU_AZUL);
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
    Rectangle tab_obj = { static_cast<float>(px + MENU_MARGEN)+10, 30.0f,
                          static_cast<float>(tab_w)-15, static_cast<float>(MENU_PESTANA_ALTO - 8) };
    Rectangle tab_dec = { tab_obj.x + tab_w, tab_obj.y, tab_obj.width, tab_obj.height };

    bool hover_obj = CheckCollisionPointRec(mouse, tab_obj);
    bool hover_dec = CheckCollisionPointRec(mouse, tab_dec);

    DrawRectangleRec(tab_obj, menu_tab == 0 ? WHITE : (hover_obj ? ColorAlpha(WHITE, 0.7f) : ColorAlpha(WHITE, 0.4f)));
    DrawRectangleRec(tab_dec, menu_tab == 1 ? WHITE : (hover_dec ? ColorAlpha(WHITE, 0.7f) : ColorAlpha(WHITE, 0.4f)));
    
    // Centrar texto OBJETOS
    Vector2 obj_size = MeasureTextEx(fuente_menu, "OBJETOS", 14, 1);

    Vector2 obj_pos = {tab_obj.x + (tab_obj.width - obj_size.x) / 2 , tab_obj.y + (tab_obj.height - obj_size.y) / 2};
    DrawTextEx(fuente_menu, "OBJETOS", obj_pos, 17, 1, MENU_TEXTO);
    
    // Centrar texto DECORACION
    Vector2 dec_size = MeasureTextEx(fuente_menu, "DECORACION", 13, 1);

    Vector2 dec_pos = {tab_dec.x + (tab_dec.width - dec_size.x) / 2, tab_dec.y + (tab_dec.height - dec_size.y) / 2};
    DrawTextEx(fuente_menu, "DECORACION", dec_pos, 17, 1, MENU_TEXTO);
    if (menu_tab == 0)
        DrawRectangle(static_cast<int>(tab_obj.x), static_cast<int>(tab_obj.y + tab_obj.height - 3), tab_w -15, 3, MENU_AZUL);
    else
        DrawRectangle(static_cast<int>(tab_dec.x), static_cast<int>(tab_dec.y + tab_dec.height - 3), tab_w, 3, MENU_AZUL);

    // Botón cerrar a la izquierda del menú
    int bx = px - 28;
    bool hover_cerrar = CheckCollisionPointRec(mouse, Rectangle{static_cast<float>(bx), static_cast<float>(ALTO / 2 - 40), 24, 80});
    DrawRectangle(bx, ALTO / 2 - 40, 24, 80, hover_cerrar ? Color{165, 170, 182, 255} : MENU_FONDO_OSCURO);
    DrawRectangleLines(bx, ALTO / 2 - 40, 24, 80, MENU_BORDE);
    DrawText(">", bx + 7, ALTO / 2 - 8, 18, hover_cerrar ? MENU_AZUL_CLARO : MENU_AZUL);

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
    for (const auto& c : celdas_menu_cache) {
        bool hover_cell = CheckCollisionPointRec(mouse, c.rect);
        bool hot = hover_cell || (arrastrando_spawn == c.tipo);
        dibujar_celda_menu(c, hot);
    }

    // Guardado / carga de partidas
    if (estado_actual != EstadoJuego::JUEGO_NIVEL) {
        int py_guardado = ALTO - MENU_PAGINACION_ALTO - 180 - 215;
        dibujar_panel_guardado(px, py_guardado, MENU_ANCHO, fuente_menu);
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

    // ==================== RENDERING DE DEBUG DE HITBOXES ====================
    if (modo_debug) {
        // Pestañas (Rojo)
        DrawRectangleLinesEx(tab_obj, 1.5f, RED);
        DrawRectangleLinesEx(tab_dec, 1.5f, RED);

        // Botón de cerrar
        DrawRectangleLinesEx(Rectangle{static_cast<float>(bx), static_cast<float>(ALTO / 2 - 40), 24, 80}, 1.5f, RED);

        // Categorías
        int y_debug = MENU_PESTANA_ALTO + 36;
        if (menu_tab == 0) {
            Rectangle h1 = { static_cast<float>(px + MENU_MARGEN) + 10, static_cast<float>(y_debug),
                static_cast<float>(MENU_ANCHO - 2 * MENU_MARGEN) - 25, static_cast<float>(MENU_CATEGORIA_ALTO) };
            DrawRectangleLinesEx(h1, 1.5f, RED);
            y_debug += MENU_CATEGORIA_ALTO + 4;
            if (cat_mecanicas_abierta) {
                std::vector<RectCeldaMenu> temp_celdas;
                layout_celdas_categoria(px, y_debug, 0, temp_celdas);
                int filas = (static_cast<int>(temp_celdas.size()) + MENU_COLS - 1) / MENU_COLS;
                y_debug += filas * (MENU_CELDA + 6) + 8;
            }
            Rectangle h2 = { static_cast<float>(px + MENU_MARGEN) + 10, static_cast<float>(y_debug),
                static_cast<float>(MENU_ANCHO - 2 * MENU_MARGEN) - 25, static_cast<float>(MENU_CATEGORIA_ALTO) };
            DrawRectangleLinesEx(h2, 1.5f, RED);
        } else {
            Rectangle h1 = { static_cast<float>(px + MENU_MARGEN) + 10, static_cast<float>(y_debug),
                static_cast<float>(MENU_ANCHO - 2 * MENU_MARGEN) - 25, static_cast<float>(MENU_CATEGORIA_ALTO) };
            DrawRectangleLinesEx(h1, 1.5f, RED);
        }

        // Celdas de items (Verde)
        for (const auto& c : celdas_menu_cache) {
            DrawRectangleLinesEx(c.rect, 1.5f, GREEN);
        }

        // Botones de página (Rojo)
        DrawRectangleLinesEx(Rectangle{static_cast<float>(px + MENU_MARGEN), static_cast<float>(py), static_cast<float>(flecha_w), static_cast<float>(flecha_h)}, 1.5f, RED);
        DrawRectangleLinesEx(Rectangle{static_cast<float>(px + MENU_ANCHO - MENU_MARGEN - flecha_w), static_cast<float>(py), static_cast<float>(flecha_w), static_cast<float>(flecha_h)}, 1.5f, RED);

        // Panel de guardado y sus botones (Rojo)
        DrawRectangleLinesEx(rect_panel_guardado, 2.0f, RED);
        DrawRectangleLinesEx(rect_btn_guardar_partida, 1.5f, RED);
        DrawRectangleLinesEx(rect_btn_ver_partidas, 1.5f, RED);
    }
}

bool manejar_click_menu(int mx, int my, MotorFisica& motor) {
    if (estado_actual != EstadoJuego::JUEGO_NIVEL) {
        if (manejar_click_panel_guardado(mx, my, motor, gestor_eventos, ANCHO, ALTO, contador_bolas)) {
            return true;
        }
    }


    if (!menu_visible) {
        int bx = ANCHO - 26;
        Rectangle btn_abrir = { static_cast<float>(bx), static_cast<float>(ALTO / 2 - 40), 24, 80 };
        if (click_en_rect(mx, my, btn_abrir)) {
            menu_visible = true;
            return true;
        }
        return false;
    }

    int px = ANCHO - MENU_ANCHO;
    int tab_w = (MENU_ANCHO - 2 * MENU_MARGEN) / 2;

    Rectangle tab_obj = { static_cast<float>(px + MENU_MARGEN) + 10, 30.0f,
                          static_cast<float>(tab_w) - 15, static_cast<float>(MENU_PESTANA_ALTO - 8) };
    Rectangle tab_dec = { tab_obj.x + tab_w, tab_obj.y, tab_obj.width, tab_obj.height };
    if (click_en_rect(mx, my, tab_obj)) { menu_tab = 0; menu_pagina = 0; return true; }
    if (click_en_rect(mx, my, tab_dec)) { menu_tab = 1; menu_pagina = 0; return true; }

    // Botón cerrar a la izquierda del menú
    int bx = px - 28;
    Rectangle btn_cerrar = { static_cast<float>(bx), static_cast<float>(ALTO / 2 - 40), 24, 80 };
    if (click_en_rect(mx, my, btn_cerrar)) { menu_visible = false; return true; }

    int y = MENU_PESTANA_ALTO + 36;
    if (menu_tab == 0) {
        Rectangle hdr_mec = { static_cast<float>(px + MENU_MARGEN) + 10, static_cast<float>(y),
            static_cast<float>(MENU_ANCHO - 2 * MENU_MARGEN) - 25, static_cast<float>(MENU_CATEGORIA_ALTO) };
        if (click_en_rect(mx, my, hdr_mec)) { cat_mecanicas_abierta = !cat_mecanicas_abierta; return true; }
        y += MENU_CATEGORIA_ALTO + 4;
        if (cat_mecanicas_abierta) {
            std::vector<RectCeldaMenu> celdas;
            layout_celdas_categoria(px, y, 0, celdas);
            int filas = (static_cast<int>(celdas.size()) + MENU_COLS - 1) / MENU_COLS;
            y += filas * (MENU_CELDA + 6) + 8;
        }
        Rectangle hdr_int = { static_cast<float>(px + MENU_MARGEN) + 10, static_cast<float>(y),
            static_cast<float>(MENU_ANCHO - 2 * MENU_MARGEN) - 25, static_cast<float>(MENU_CATEGORIA_ALTO) };
        if (click_en_rect(mx, my, hdr_int)) { cat_interactivos_abierta = !cat_interactivos_abierta; return true; }
    } else {
        Rectangle hdr_dec = { static_cast<float>(px + MENU_MARGEN) + 10, static_cast<float>(y),
            static_cast<float>(MENU_ANCHO - 2 * MENU_MARGEN) - 25, static_cast<float>(MENU_CATEGORIA_ALTO) };
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

void dibujar_cuerdas(const MotorFisica& motor) {
    for (const auto* e : motor.get_entidades()) {
        const Cuerda* cuerda = dynamic_cast<const Cuerda*>(e);
        if (!cuerda) continue;

        std::vector<Vector2D> puntos;
        if (!cuerda->obtener_puntos(motor.get_entidades(), puntos)) continue;

        Color cuerda_col = cuerda->get_ultima_tension() > 0.0
            ? Color{235, 220, 155, 255}
            : Color{185, 175, 130, 230};
        for (size_t i = 1; i < puntos.size(); ++i) {
            DrawLineEx({(float)puntos[i - 1].x, (float)puntos[i - 1].y},
                       {(float)puntos[i].x, (float)puntos[i].y}, 3.0f, cuerda_col);
        }
        Vector2D a = puntos.front();
        Vector2D b = puntos.back();
        DrawCircle(static_cast<int>(a.x), static_cast<int>(a.y), 5.0f, Color{220, 60, 60, 255});
        DrawCircle(static_cast<int>(b.x), static_cast<int>(b.y), 5.0f, Color{220, 60, 60, 255});
    }
}

void dibujar_tramo_cuerda_preview(Vector2D a, Vector2D b) {
    Vector2 va = {static_cast<float>(a.x), static_cast<float>(a.y)};
    Vector2 vb = {static_cast<float>(b.x), static_cast<float>(b.y)};
    DrawLineEx(va, vb, 7.0f, Color{35, 28, 18, 190});
    DrawLineEx(va, vb, 4.0f, Color{255, 224, 130, 255});
    DrawLineEx(va, vb, 1.5f, Color{255, 255, 220, 220});
}

void dibujar_previsualizacion_cuerda(const MotorFisica& motor) {
    if (estado_cuerda == EstadoColocacionCuerda::INACTIVA) return;

    Vector2D mouse(GetMouseX(), GetMouseY());
    dibujar_icono_objeto(TipoObjetoMenu::CUERDA,
                         static_cast<float>(mouse.x + 34.0),
                         static_cast<float>(mouse.y - 28.0),
                         0.72f, 190);

    if (estado_cuerda == EstadoColocacionCuerda::ESPERANDO_EXTREMO_A) {
        AnclajeCuerda anclaje;
        Vector2D punto;
        if (detectar_anclaje_cuerda(motor, mouse, anclaje, punto)) {
            DrawCircleLines(static_cast<int>(punto.x), static_cast<int>(punto.y), 13.0f,
                            Color{255, 230, 120, 230});
        }
        return;
    }

    if (estado_cuerda == EstadoColocacionCuerda::ESPERANDO_SOPORTE) {
        int soporte_id = -1;
        Vector2D punto_soporte;
        dibujar_tramo_cuerda_preview(cuerda_punto_a_preview, mouse);
        DrawCircle(static_cast<int>(cuerda_punto_a_preview.x), static_cast<int>(cuerda_punto_a_preview.y),
                   8.0f, Color{255, 90, 60, 245});
        DrawCircleLines(static_cast<int>(cuerda_punto_a_preview.x), static_cast<int>(cuerda_punto_a_preview.y),
                        13.0f, Color{255, 240, 150, 245});
        if (detectar_soporte_torque(motor, mouse, soporte_id, punto_soporte)) {
            DrawCircleLines(static_cast<int>(punto_soporte.x), static_cast<int>(punto_soporte.y),
                            24.0f, Color{255, 240, 120, 255});
        }
        return;
    }

    if (estado_cuerda == EstadoColocacionCuerda::ESPERANDO_EXTREMO_B) {
        Vector2D anterior = cuerda_punto_a_preview;
        for (const Vector2D& soporte : cuerda_soportes_preview) {
            dibujar_tramo_cuerda_preview(anterior, soporte);
            anterior = soporte;
        }
        dibujar_tramo_cuerda_preview(anterior, mouse);
        DrawCircle(static_cast<int>(cuerda_punto_a_preview.x), static_cast<int>(cuerda_punto_a_preview.y),
                   7.0f, Color{255, 90, 60, 245});
        for (const Vector2D& soporte : cuerda_soportes_preview) {
            DrawCircleLines(static_cast<int>(soporte.x), static_cast<int>(soporte.y),
                            22.0f, Color{255, 240, 120, 240});
        }
    }
}

void dibujar_previsualizacion_banda(const MotorFisica& motor) {
    if (estado_banda == EstadoConexionBanda::INACTIVA) return;

    Vector2D mouse(GetMouseX(), GetMouseY());

    if (estado_banda == EstadoConexionBanda::ESPERANDO_HAMSTER) {
        // Highlight hámster bajo el cursor
        for (const auto* e : motor.get_entidades()) {
            const auto* h = dynamic_cast<const CajaHamster*>(e);
            if (!h) continue;
            Vector2D p = e->get_posicion();
            if (mouse.x >= p.x && mouse.x <= p.x + h->get_ancho() &&
                mouse.y >= p.y && mouse.y <= p.y + h->get_alto()) {
                DrawRectangleLinesEx({(float)p.x, (float)p.y,
                    (float)h->get_ancho(), (float)h->get_alto()}, 3.0f, Color{255,200,50,230});
            }
        }
        return;
    }

    if (estado_banda == EstadoConexionBanda::ESPERANDO_VENTILADOR) {
        // Línea desde el hámster hasta el cursor (como cuerda)
        Vector2D a = banda_origen_preview;
        Vector2D b = mouse;
        Vector2 va = {(float)a.x, (float)a.y};
        Vector2 vb = {(float)b.x, (float)b.y};
        DrawLineEx(va, vb, 5.0f, Color{40, 30, 10, 180});
        DrawLineEx(va, vb, 3.0f, Color{255, 200, 50, 220});
        DrawCircle((int)a.x, (int)a.y, 7.0f, Color{255, 180, 30, 230});

        // Highlight destinos válidos bajo el cursor
        for (const auto* e : motor.get_entidades()) {
            Vector2D p = e->get_posicion();
            const auto* v = dynamic_cast<const Ventilador*>(e);
            if (v && mouse.x >= p.x && mouse.x <= p.x + v->get_ancho() &&
                     mouse.y >= p.y && mouse.y <= p.y + v->get_alto()) {
                DrawRectangleLinesEx({(float)p.x, (float)p.y,
                    (float)v->get_ancho(), (float)v->get_alto()}, 3.0f, Color{255,200,50,230});
            }
            const auto* cs = dynamic_cast<const CajaSorpresa*>(e);
            if (cs && mouse.x >= p.x && mouse.x <= p.x + cs->get_ancho() &&
                      mouse.y >= p.y && mouse.y <= p.y + cs->get_alto()) {
                DrawRectangleLinesEx({(float)p.x, (float)p.y,
                    (float)cs->get_ancho(), (float)cs->get_alto()}, 3.0f, Color{255,200,50,230});
            }
            const auto* conv = dynamic_cast<const Caminadora*>(e);
            if (conv && e->contiene_punto(mouse)) {
                DrawRectangleLinesEx({(float)p.x, (float)p.y,
                    (float)conv->get_ancho(), (float)conv->get_alto()}, 3.0f, Color{255,200,50,230});
            }
        }
    }
}

bool crear_cubeta(MotorFisica& motor, Vector2D pos) {
    double w = 58.0;
    double h = 52.0;
    Vector2D spawn(pos.x - w / 2.0, pos.y - h / 2.0);
    motor.agregar_entidad(new Cubeta(motor.generar_id(), spawn, w, h));
    return true;
}

bool crear_soporte_torque(MotorFisica& motor, Vector2D pos) {
    motor.agregar_entidad(new SoporteTorque(motor.generar_id(), pos, 16.0));
    return true;
}

bool crear_globo(MotorFisica& motor, Vector2D pos) {
    double radio = 36.0;
    if (!posicion_valida_para_spawn(motor, pos, TipoForma::CIRCULO, radio, 0.0)) {
        spawn_error_timer = 0.5f;
        spawn_error_pos = pos;
        return false;
    }
    Globo* g = new Globo(motor.generar_id(), pos, radio);
    motor.agregar_entidad(g);
    return true;
}

bool crear_bola_beisbol(MotorFisica& motor, Vector2D pos) {
    double radio = 12.0;
    if (!posicion_valida_para_spawn(motor, pos, TipoForma::CIRCULO, radio, 0.0)) {
        spawn_error_timer = 0.5f;
        spawn_error_pos = pos;
        return false;
    }
    BolaBeisbol* b = new BolaBeisbol(motor.generar_id(), pos, radio);
    motor.agregar_entidad(b);
    return true;
}

bool crear_tijera(MotorFisica& motor, Vector2D pos) {
    double w = 100.0;
    double h = 51.0;
    Vector2D spawn_pos(pos.x - w / 2.0, pos.y - h / 2.0);
    Tijera* t = new Tijera(motor.generar_id(), spawn_pos, w, h);
    motor.agregar_entidad(t);
    return true;
}

bool crear_caja_hamster(MotorFisica& motor, Vector2D pos) {
    double w = 90.0, h = 80.0;
    Vector2D spawn_pos(pos.x - w / 2.0, pos.y - h / 2.0);
    CajaHamster* ham = new CajaHamster(motor.generar_id(), spawn_pos, w, h);
    motor.agregar_entidad(ham);
    return true;
}

bool crear_banda(MotorFisica& motor, Vector2D pos) {
    (void)pos;
    return false; // se maneja con el flujo de conexión de la banda
}

bool crear_caja_sorpresa(MotorFisica& motor, Vector2D pos) {
    double w = 70.0, h = 70.0;
    Vector2D spawn_pos(pos.x - w / 2.0, pos.y - h / 2.0);
    motor.agregar_entidad(new CajaSorpresa(motor.generar_id(), spawn_pos, w, h));
    return true;
}

bool crear_caminadora(MotorFisica& motor, Vector2D pos) {
    double w = 150.0, h = 24.0;
    Vector2D spawn_pos(pos.x - w / 2.0, pos.y - h / 2.0);
    motor.agregar_entidad(new Caminadora(motor.generar_id(), spawn_pos, w, h, true));
    return true;
}

// ============================================================================
// Crear BolaRebotadora en posicion del mouse (con validacion)
// ============================================================================
bool crear_bola_rebotadora(MotorFisica& motor, Vector2D pos) {
    double radio = 48.0;

    // Verificar que no nace encima de paredes, rampas u otros mecanismos
    if (!posicion_valida_para_spawn(motor, pos, TipoForma::CIRCULO, radio, 0.0)) {
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
// ============================================================================
// Crear Barril Chavo en posición del mouse (con validación)
// ============================================================================
bool crear_barril_chavo(MotorFisica& motor, Vector2D pos) {
    double w = 90.0;
    double h = 120.0;
    
    // Centrar en el mouse
    Vector2D spawn_pos(pos.x - w / 2.0, pos.y - h / 2.0);

    if (!posicion_valida_para_spawn(motor, spawn_pos, TipoForma::AABB, w, h)) {
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
    if (e) {
        e->dibujar(modo_debug);
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

    if (estado_cuerda != EstadoColocacionCuerda::INACTIVA) {
        const char* paso = "Selecciona primer extremo";
        if (estado_cuerda == EstadoColocacionCuerda::ESPERANDO_SOPORTE) paso = "Selecciona primer Torque";
        if (estado_cuerda == EstadoColocacionCuerda::ESPERANDO_EXTREMO_B) paso = "Torque extra o extremo final";
        DrawText(TextFormat("Cuerda: %s", paso), margin, y + 110, 14,
                 Color{235, 220, 155, 255});
    }

    if (estado_banda != EstadoConexionBanda::INACTIVA) {
        const char* paso_banda = estado_banda == EstadoConexionBanda::ESPERANDO_HAMSTER
            ? "Banda: Click en la Caja Hamster"
            : "Banda: Click en Ventilador o CajaSorpresa";
        DrawText(paso_banda, margin, y + 128, 14, Color{255, 200, 80, 255});
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
        bool rotable = dynamic_cast<Ventilador*>(entidad_seleccionada) != nullptr ||
                       dynamic_cast<PlanoInclinado*>(entidad_seleccionada) != nullptr;
        bool redimensionable = dynamic_cast<ParedRectangular*>(entidad_seleccionada) != nullptr &&
                               !es_borde_nivel(entidad_seleccionada);
        const char* rotar_txt = (rotable && (!entidad_seleccionada->get_es_fijo() || estado_actual == EstadoJuego::JUEGO_CREATIVO)) ? "  [F] Rotar" : "";
        const char* resize_txt = (redimensionable && estado_actual == EstadoJuego::JUEGO_CREATIVO) ? "  [Flechas] Resize" : "";
        
        std::string info_txt;
        if (estado_actual == EstadoJuego::JUEGO_CREATIVO) {
            const char* fijo_str = entidad_seleccionada->get_es_fijo() ? "Fijo" : "Inventario del Jugador";
            const char* toggle_fijo_txt = !es_borde_nivel(entidad_seleccionada) ? "  [P/G] Cambiar Tipo (Fijo/Inventario)" : "";
            info_txt = TextFormat("Seleccionado: %s #%d (%s)%s%s%s  [DEL] Eliminar  [Click Der/ESC] Deseleccionar",
                                  nombre_tipo_entidad(entidad_seleccionada->get_tipo_entidad()),
                                  entidad_seleccionada->get_id(), fijo_str, toggle_fijo_txt, rotar_txt, resize_txt);
        } else {
            const char* fijo_str = entidad_seleccionada->get_es_fijo() ? "Fijo (Bloqueado)" : "Colocado por ti";
            const char* del_txt = entidad_seleccionada->get_es_fijo() ? "" : "  [DEL] Eliminar";
            info_txt = TextFormat("Seleccionado: %s #%d (%s)%s%s  [Click Der/ESC] Deseleccionar",
                                  nombre_tipo_entidad(entidad_seleccionada->get_tipo_entidad()),
                                  entidad_seleccionada->get_id(), fijo_str, del_txt, rotar_txt);
        }
        
        DrawText(info_txt.c_str(), margin, y + 88, 14, Color{255, 200, 80, 255});
    }

    // Controles
    DrawText("[TAB] Menu  [Arrastrar] Crear/Mover  [CLICK] Seleccionar  [Click Der/ESC] Deseleccionar  [DEL] Eliminar  [Flechas] Resize  [SPACE] Pausa  [D] Debug",
             margin, ALTO - 30, 14, COLOR_CONTROLES);
}

// ============================================================================
// Cargar texturas
// ============================================================================
void cargandoTexturas() {
    inicializar_raiz_datos();

    // Cargar las 3 texturas de pelota
    tex_bola[0] = cargar_textura_datos("Assets/ball/pelota1.png");
    if (tex_bola[0].id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: pelota1.png");
    } else {
        TraceLog(LOG_INFO, "Textura bola 1 cargada: %dx%d", tex_bola[0].width, tex_bola[0].height);
    }

    tex_bola[1] = cargar_textura_datos("Assets/ball/pelota2.png");
    if (tex_bola[1].id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: pelota2.png");
    } else {
        TraceLog(LOG_INFO, "Textura bola 2 cargada: %dx%d", tex_bola[1].width, tex_bola[1].height);
    }

    tex_bola[2] = cargar_textura_datos("Assets/ball/pelota3.png");
    if (tex_bola[2].id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: pelota3.png");
    } else {
        TraceLog(LOG_INFO, "Textura bola 3 cargada: %dx%d", tex_bola[2].width, tex_bola[2].height);
    }

    tex_fondo = cargar_textura_datos("Assets/fondo1.png");
    if (tex_fondo.id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: fondo1.png");
    } else {
        TraceLog(LOG_INFO, "Textura fondo cargada: %dx%d", tex_fondo.width, tex_fondo.height);
    }
    
    tex_base_central = cargar_textura_datos("Assets/hud/panel.png");
    if (tex_base_central.id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: panel.png");
    } else {
        TraceLog(LOG_INFO, "Textura base central cargada: %dx%d", tex_base_central.width, tex_base_central.height);
    }
    
    derecho = cargar_textura_datos("Assets/hud/barsidederecho.png");
    if (derecho.id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: barsidederecho.png");
    } else {
        TraceLog(LOG_INFO, "Textura derecho cargada: %dx%d", derecho.width, derecho.height);
    }
    
    tex_hud_opciones = cargar_textura_datos("Assets/hud/opciones.png");
    tex_hud_opciones_hover = cargar_textura_datos("Assets/hud/opciones2.png");
    tex_hud_ayuda = cargar_textura_datos("Assets/hud/nose.png");
    tex_hud_salir = cargar_textura_datos("Assets/hud/ver.png");
    tex_hud_play = cargar_textura_datos("Assets/hud/inicio.png");
    tex_hud_reset = cargar_textura_datos("Assets/hud/deshacer.png");
    tex_hud_reset_hover = cargar_textura_datos("Assets/hud/deshacer2.png");
    
    tex_menu_fondo = cargar_textura_datos("Assets/menu/fondo.png");
    if (tex_menu_fondo.id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: Assets/menu/fondo.png");
    } else {
        TraceLog(LOG_INFO, "Textura menu fondo cargada: %dx%d", tex_menu_fondo.width, tex_menu_fondo.height);
    }
    
    tex_menu_box = cargar_textura_datos("Assets/menu/menu.png");
    if (tex_menu_box.id == 0) {
        TraceLog(LOG_ERROR, "Error cargando textura: Assets/menu/menu.png");
    } else {
        TraceLog(LOG_INFO, "Textura menu box cargada: %dx%d", tex_menu_box.width, tex_menu_box.height);
    }
    
    tex_btn_jugar1 = cargar_textura_datos("Assets/menu/jugarniveles1.png");
    tex_btn_jugar2 = cargar_textura_datos("Assets/menu/jugarniveles2.png");
    tex_btn_creativo1 = cargar_textura_datos("Assets/menu/modocreativo1.png");
    tex_btn_creativo2 = cargar_textura_datos("Assets/menu/modocreativo2.png");
    tex_btn_opciones1 = cargar_textura_datos("Assets/menu/opciones1.png");
    tex_btn_opciones2 = cargar_textura_datos("Assets/menu/opciones2.png");
    tex_btn_salir1 = cargar_textura_datos("Assets/menu/salir1.png");
    tex_btn_salir2 = cargar_textura_datos("Assets/menu/salir2.png");
    tex_menu_titulo = cargar_textura_datos("Assets/menu/titulo.png");
    
    // Cargar musica de fondo
    if (FileExists("Assets/Music/menu.wav")) {
        musica_menu = LoadMusicStream("Assets/Music/menu.wav");
        if (IsMusicReady(musica_menu)) {
            musica_menu.looping = true;
            PlayMusicStream(musica_menu);
            TraceLog(LOG_INFO, "Musica de menu cargada y reproduciendose (menu.wav).");
        } else {
            TraceLog(LOG_ERROR, "Error al preparar stream de musica: Assets/Music/menu.wav");
        }
    } else {
        TraceLog(LOG_WARNING, "Archivo de musica no encontrado: Assets/Music/menu.wav");
    }
    
    // Cargar texturas del BarrilChavo
    tex_barril = cargar_textura_datos("Assets/chavo/barril.png");
    if (tex_barril.id == 0) {
        TraceLog(LOG_WARNING, "Textura barril no encontrada: barril.png (usando renderizado geométrico)");
    } else {
        TraceLog(LOG_INFO, "Textura barril cargada: %dx%d", tex_barril.width, tex_barril.height);
    }
    
    tex_chavo = cargar_textura_datos("Assets/chavo/chavo.png");
    if (tex_chavo.id == 0) {
        TraceLog(LOG_WARNING, "Textura El Chavo no encontrada: chavo.png (usando renderizado geométrico)");
    } else {
        TraceLog(LOG_INFO, "Textura El Chavo cargada: %dx%d", tex_chavo.width, tex_chavo.height);
    }
    
    // Cargar texturas del SeguidorBooster
    tex_seguidor_quieto = cargar_textura_datos("Assets/messi/messi-normal.png");
    if (tex_seguidor_quieto.id == 0) {
        TraceLog(LOG_WARNING, "Textura seguidor quieto no encontrada: messi-normal.png");
    } else {
        TraceLog(LOG_INFO, "Textura seguidor quieto cargada: %dx%d", tex_seguidor_quieto.width, tex_seguidor_quieto.height);
    }
    
    tex_seguidor_corriendo = cargar_textura_datos("Assets/messi/mesirve.png");
    if (tex_seguidor_corriendo.id == 0) {
        TraceLog(LOG_WARNING, "Textura seguidor corriendo no encontrada: mesirve.png");
    } else {
        TraceLog(LOG_INFO, "Textura seguidor corriendo cargada: %dx%d", tex_seguidor_corriendo.width, tex_seguidor_corriendo.height);
    }

    tex_seguidor_cabezazo = cargar_textura_datos("Assets/messi/cabezazo2.png");
    if (tex_seguidor_cabezazo.id == 0) {
        TraceLog(LOG_WARNING, "Textura seguidor cabezazo no encontrada: cabezazo2.png");
    } else {
        TraceLog(LOG_INFO, "Textura seguidor cabezazo cargada: %dx%d", tex_seguidor_cabezazo.width, tex_seguidor_cabezazo.height);
    }
    
    // Inicializar animaciones del SeguidorBooster
    if (tex_seguidor_corriendo.id > 0) {
        anim_seguidor_corriendo = new Animacion(tex_seguidor_corriendo, 8, 12, 8);
    }

    // Inicializar animaciones del Menú de Inicio
    tex_menu_inicio_anim = cargar_textura_datos("Assets/animation/manuel-n.png");
    if (tex_menu_inicio_anim.id == 0) {
        TraceLog(LOG_WARNING, "Textura de animacion de menu no encontrada: Assets/animation/manuel-n.png");
    } else {
        TraceLog(LOG_INFO, "Textura de animacion de menu cargada: %dx%d", tex_menu_inicio_anim.width, tex_menu_inicio_anim.height);
        anim_menu_inicio = new Animacion(tex_menu_inicio_anim, 6, 8, 6);
        pos_x_anim_menu = ANCHO * 0.25f;
    }

    tex_menu_fede_anim = cargar_textura_datos("Assets/animation/fede-ani.png");
    if (tex_menu_fede_anim.id == 0) {
        TraceLog(LOG_WARNING, "Textura de animacion fede no encontrada: Assets/animation/fede.png");
    } else {
        TraceLog(LOG_INFO, "Textura de animacion fede cargada: %dx%d", tex_menu_fede_anim.width, tex_menu_fede_anim.height);
        anim_menu_fede = new Animacion(tex_menu_fede_anim, 8, 4, 8);
    }

    tex_menu_moto_anim = cargar_textura_datos("Assets/animation/moto-ani.png");
    if (tex_menu_moto_anim.id == 0) {
        TraceLog(LOG_WARNING, "Textura de animacion moto no encontrada: Assets/animation/moto-ani.png");
    } else {
        TraceLog(LOG_INFO, "Textura de animacion moto cargada: %dx%d", tex_menu_moto_anim.width, tex_menu_moto_anim.height);
        anim_menu_moto = new Animacion(tex_menu_moto_anim, 8, 4, 8);
        pos_x_anim_moto = ANCHO + 150.0f;
    }

    tex_menu_jose_anim = cargar_textura_datos("Assets/animation/jose-ani.png");
    if (tex_menu_jose_anim.id == 0) {
        TraceLog(LOG_WARNING, "Textura de animacion jose no encontrada: Assets/animation/jose-ani.png");
    } else {
        TraceLog(LOG_INFO, "Textura de animacion jose cargada: %dx%d", tex_menu_jose_anim.width, tex_menu_jose_anim.height);
        anim_menu_jose = new Animacion(tex_menu_jose_anim, 8, 8, 8);
        pos_x_anim_jose = ANCHO * 0.5f;
    }

    tex_menu_gusano_anim = cargar_textura_datos("Assets/animation/gusa-ani.png");
    if (tex_menu_gusano_anim.id == 0) {
        TraceLog(LOG_WARNING, "Textura de animacion gusano no encontrada: Assets/animation/gusa-ani.png");
    } else {
        TraceLog(LOG_INFO, "Textura de animacion gusano cargada: %dx%d", tex_menu_gusano_anim.width, tex_menu_gusano_anim.height);
        anim_menu_gusano = new Animacion(tex_menu_gusano_anim, 8, 8, 8);
        pos_x_anim_gusano = ANCHO * 0.5f + 100.0f;
    }

    tex_menu_drom_anim = cargar_textura_datos("Assets/animation/drom.png");
    if (tex_menu_drom_anim.id == 0) {
        TraceLog(LOG_WARNING, "Textura de animacion drone no encontrada: Assets/animation/drom.png");
    } else {
        TraceLog(LOG_INFO, "Textura de animacion drone cargada: %dx%d", tex_menu_drom_anim.width, tex_menu_drom_anim.height);
        anim_menu_drom = new Animacion(tex_menu_drom_anim, 8, 8, 8);
        pos_x_anim_drom = -150.0f;
    }

    // Cargar texturas de los nuevos assets
    tex_trampolin = cargar_textura_datos("Assets/trampolin/trampolin.png");
    if (tex_trampolin.id == 0) {
        TraceLog(LOG_WARNING, "Textura trampolin no encontrada: trampolin.png");
    } else {
        TraceLog(LOG_INFO, "Textura trampolin cargada: %dx%d", tex_trampolin.width, tex_trampolin.height);
    }

    tex_balancin_base = cargar_textura_datos("Assets/balancin/detalle1.png");
    if (tex_balancin_base.id == 0) {
        TraceLog(LOG_WARNING, "Textura balancin base no encontrada: detalle1.png");
    } else {
        TraceLog(LOG_INFO, "Textura balancin base cargada: %dx%d", tex_balancin_base.width, tex_balancin_base.height);
    }

    tex_balancin_tabla = cargar_textura_datos("Assets/balancin/balancin2.png");
    if (tex_balancin_tabla.id == 0) {
        TraceLog(LOG_WARNING, "Textura balancin tabla no encontrada: balancin2.png");
    } else {
        TraceLog(LOG_INFO, "Textura balancin tabla cargada: %dx%d", tex_balancin_tabla.width, tex_balancin_tabla.height);
    }

    tex_plata_larga = cargar_textura_datos("Assets/plataform/plata_larga.png");
    if (tex_plata_larga.id == 0) {
        TraceLog(LOG_WARNING, "Textura plataforma larga no encontrada: plata_larga.png");
    } else {
        TraceLog(LOG_INFO, "Textura plataforma larga cargada: %dx%d", tex_plata_larga.width, tex_plata_larga.height);
    }

    tex_plata_peque = cargar_textura_datos("Assets/plataform/plata_peque.png");
    if (tex_plata_peque.id == 0) {
        TraceLog(LOG_WARNING, "Textura plataforma peque no encontrada: plata_peque.png");
    } else {
        TraceLog(LOG_INFO, "Textura plataforma peque cargada: %dx%d", tex_plata_peque.width, tex_plata_peque.height);
    }

    tex_plata_rampa_izq = cargar_textura_datos("Assets/plataform/rampa.png");
    if (tex_plata_rampa_izq.id == 0) {
        TraceLog(LOG_WARNING, "Textura rampa izq no encontrada: rampa.png");
    } else {
        TraceLog(LOG_INFO, "Textura rampa izq cargada: %dx%d", tex_plata_rampa_izq.width, tex_plata_rampa_izq.height);
    }

    tex_plata_rampa_der = cargar_textura_datos("Assets/plataform/rampa_2.png");
    if (tex_plata_rampa_der.id == 0) {
        TraceLog(LOG_WARNING, "Textura rampa der no encontrada: rampa_2.png");
    } else {
        TraceLog(LOG_INFO, "Textura rampa der cargada: %dx%d", tex_plata_rampa_der.width, tex_plata_rampa_der.height);
    }

    // Texturas de BolaRebotadora (robot rojo)
    tex_robote_soporte = cargar_textura_datos("Assets/rebote/proto-2.png");
    if (tex_robote_soporte.id == 0) {
        TraceLog(LOG_WARNING, "Textura robote soporte no encontrada: Assets/rebote/proto-2.png");
    } else {
        TraceLog(LOG_INFO, "Textura robote soporte cargada: %dx%d", tex_robote_soporte.width, tex_robote_soporte.height);
    }
    tex_robote_pelota = cargar_textura_datos("Assets/rebote/proto-1.png");
    if (tex_robote_pelota.id == 0) {
        TraceLog(LOG_WARNING, "Textura robote pelota no encontrada: Assets/rebote/proto-1.png");
    } else {
        TraceLog(LOG_INFO, "Textura robote pelota cargada: %dx%d", tex_robote_pelota.width, tex_robote_pelota.height);
    }

    // Texturas del ventilador (cuerpo + aspa)
    tex_ventilador_cuerpo = cargar_textura_datos("Assets/ventilador/cuerpo.png");
    if (tex_ventilador_cuerpo.id == 0) {
        TraceLog(LOG_WARNING, "Textura ventilador cuerpo no encontrada: Assets/ventilador/cuerpo.png");
    } else {
        TraceLog(LOG_INFO, "Textura ventilador cuerpo cargada: %dx%d", tex_ventilador_cuerpo.width, tex_ventilador_cuerpo.height);
    }
    tex_ventilador_aspa = cargar_textura_datos("Assets/ventilador/aspa.png");
    if (tex_ventilador_aspa.id == 0) {
        TraceLog(LOG_WARNING, "Textura ventilador aspa no encontrada: Assets/ventilador/aspa.png");
    } else {
        TraceLog(LOG_INFO, "Textura ventilador aspa cargada: %dx%d", tex_ventilador_aspa.width, tex_ventilador_aspa.height);
    }
    tex_caminadora = cargar_textura_datos("Assets/caminadora/caminadora1.png");

    // Cargar asset de celda de menú
    tex_celda_menu = cargar_textura_datos("Assets/hud/obj-vacio.png");
    if (tex_celda_menu.id == 0) {
        TraceLog(LOG_WARNING, "Textura celda menú no encontrada: Assets/hud/obj-vacio.png");
    } else {
        TraceLog(LOG_INFO, "Textura celda menú cargada: %dx%d", tex_celda_menu.width, tex_celda_menu.height);
    }

    // Cargar asset de encabezado de categoría
    tex_barra_encabezado = cargar_textura_datos("Assets/hud/barra.png");
    if (tex_barra_encabezado.id == 0) {
        TraceLog(LOG_WARNING, "Textura barra encabezado no encontrada: Assets/hud/barra.png");
    } else {
        TraceLog(LOG_INFO, "Textura barra encabezado cargada: %dx%d", tex_barra_encabezado.width, tex_barra_encabezado.height);
    }
    
    // Cargar fuente personalizada
    fuente_menu = cargar_fuente_datos("fonts/Gamer.ttf", 32);
    if (fuente_menu.baseSize == 0) {
        TraceLog(LOG_WARNING, "Fuente Gamer.ttf no encontrada: usando fuente por defecto");
    } else {
        TraceLog(LOG_INFO, "Fuente Gamer.ttf cargada correctamente");
    }

}

// ============================================================================
// UI Helper para Botones Interactivos
// ============================================================================
bool dibujar_boton_interactivo(Rectangle rect, const char* texto, Color color_base, Color color_hover, Font fuente, float font_size = 20.0f, Color color_texto = WHITE) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rect);
    
    Rectangle draw_rect = rect;
    if (hover) {
        draw_rect.x -= 2;
        draw_rect.y -= 2;
        draw_rect.width += 4;
        draw_rect.height += 4;
    }
    
    DrawRectangleRounded(draw_rect, 0.15f, 6, hover ? color_hover : color_base);
    DrawRectangleRoundedLinesEx(draw_rect, 0.15f, 6, 2.0f, hover ? WHITE : ColorAlpha(WHITE, 0.5f));
    
    Vector2 text_size = MeasureTextEx(fuente, texto, font_size, 1);
    Vector2 text_pos = {
        draw_rect.x + (draw_rect.width - text_size.x) / 2.0f,
        draw_rect.y + (draw_rect.height - text_size.y) / 2.0f
    };
    
    DrawTextEx(fuente, texto, text_pos, font_size, 1, color_texto);
    
    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

bool dibujar_boton_imagen_interactivo(Rectangle rect, Texture2D tex_normal, Texture2D tex_hover) {
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rect);
    
    Rectangle draw_rect = rect;
    if (hover) {
        draw_rect.x -= 2;
        draw_rect.y -= 2;
        draw_rect.width += 4;
        draw_rect.height += 4;
    }
    
    Texture2D tex = (hover && tex_hover.id > 0) ? tex_hover : tex_normal;
    if (tex.id > 0) {
        DrawTexturePro(tex, {0.0f, 0.0f, (float)tex.width, (float)tex.height}, draw_rect, {0.0f, 0.0f}, 0.0f, WHITE);
    }
    
    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

// ============================================================================
// Funciones del Menú Principal
// ============================================================================
void actualizar_menu_principal() {
    if (anim_menu_inicio) {
        if (timer_espera_menu > 0.0f) {
            timer_espera_menu -= GetFrameTime();
            if (timer_espera_menu <= 0.0f) {
                pos_x_anim_menu = ANCHO + 150.0f;
            }
        } else {
            anim_menu_inicio->actualizar(GetFrameTime());
            pos_x_anim_menu -= 70.0f * GetFrameTime();
            if (pos_x_anim_menu < -150.0f) {
                // Tiempo de espera aleatorio entre 3.0 y 8.0 segundos
                timer_espera_menu = (float)GetRandomValue(80, 100) / 10.0f;
                pos_x_anim_menu = -1000.0f;
            }
        }
    }
    if (anim_menu_fede) {
        anim_menu_fede->actualizar(GetFrameTime());
    }
    if (anim_menu_moto) {
        if (timer_espera_moto > 0.0f) {
            timer_espera_moto -= GetFrameTime();
            if (timer_espera_moto <= 0.0f) {
                pos_x_anim_moto = ANCHO + 150.0f;
            }
        } else {
            anim_menu_moto->actualizar(GetFrameTime());
            pos_x_anim_moto -= 500.0f * GetFrameTime();
            if (pos_x_anim_moto < -150.0f) {
                // Tiempo de espera aleatorio entre 5.0 y 12.0 segundos
                timer_espera_moto = (float)GetRandomValue(100, 150) / 10.0f;
                pos_x_anim_moto = -1000.0f;
            }
        }
    }
    if (anim_menu_jose) {
        anim_menu_jose->actualizar(GetFrameTime());
    }
    if (anim_menu_gusano) {
        anim_menu_gusano->actualizar(GetFrameTime());
    }
    if (anim_menu_drom) {
        if (timer_espera_drom > 0.0f) {
            timer_espera_drom -= GetFrameTime();
            if (timer_espera_drom <= 0.0f) {
                pos_x_anim_drom = -150.0f;
            }
        } else {
            anim_menu_drom->actualizar(GetFrameTime());
            pos_x_anim_drom += 150.0f * GetFrameTime();
            if (pos_x_anim_drom > ANCHO + 150.0f) {
                // Tiempo de espera aleatorio entre 4.0 y 10.0 segundos
                timer_espera_drom = (float)GetRandomValue(80, 100) / 10.0f;
                pos_x_anim_drom = -1000.0f;
            }
        }
    }
}




void dibujar_menu_principal(MotorFisica& motor) {
    if (tex_menu_fondo.id > 0) {
        float escala_x = static_cast<float>(ANCHO) / tex_menu_fondo.width;
        float escala_y = static_cast<float>(ALTO) / tex_menu_fondo.height;
        float escala = (escala_x > escala_y) ? escala_x : escala_y;
        DrawTextureEx(tex_menu_fondo, {0, 0}, 0, escala, WHITE);
    } else {
        // Fondo degradado fallback
        DrawRectangleGradientV(0, 0, ANCHO, ALTO, COLOR_FONDO, Color{10, 10, 25, 255});
    }
    
    // Dibujar personaje animado Fede
    if (anim_menu_fede) {
        Vector2 pos_dibujo = { ANCHO * 0.75f + 150, ALTO * 0.65f + 100.0f };
        anim_menu_fede->dibujar(pos_dibujo, 110.0f * 1.5f, 200.0f * 1.5f);
    }

    // Dibujar personaje animado Jose
    if (anim_menu_jose) {
        Vector2 pos_dibujo = { pos_x_anim_jose - 400, ALTO * 0.65f + 150.0f - 50};
        anim_menu_jose->dibujar(pos_dibujo, 110.0f * 1.5f, 200.0f * 1.5f);
    }

    // Dibujar personaje animado Gusano
    if (anim_menu_gusano) {
        Vector2 pos_dibujo = { pos_x_anim_gusano + 730, ALTO * 0.65f + 50};
        anim_menu_gusano->dibujar(pos_dibujo, 110.0f * 1.5f+50, 200.0f * 1.5f + 100);
    }

    // Dibujar personaje animado Manuel
    if (anim_menu_inicio) {
        Vector2 pos_dibujo = { pos_x_anim_menu, ALTO * 0.65f + 150.0f };
        anim_menu_inicio->dibujar(pos_dibujo, 110.0f * 1.5f, 200.0f * 1.5f);
    }

    // Dibujar personaje animado Drone (Drom) - Capa superior
    if (anim_menu_drom) {
        Vector2 pos_dibujo = { pos_x_anim_drom, ALTO * 0.15f };
        anim_menu_drom->dibujar(pos_dibujo, 110.0f * 1.5f, 200.0f * 1.5f);
    }
    
    // Título y Subtítulo
    if (tex_menu_titulo.id > 0) {
        float logo_w = 600.0f ; // scaled size
        float logo_h = logo_w * (static_cast<float>(tex_menu_titulo.height) / tex_menu_titulo.width);
        Rectangle logo_rect = { (ANCHO - logo_w)/2.0f, ALTO * 0.15f - 100.0f, logo_w, logo_h };
        DrawTexturePro(tex_menu_titulo, {0.0f, 0.0f, (float)tex_menu_titulo.width, (float)tex_menu_titulo.height}, logo_rect, {0.0f, 0.0f}, 0.0f, WHITE);
    } else {
        const char* titulo = "THE INCREDIBLE MACHINE";
        const char* subtitulo = "REMASTERED";
        Vector2 tsize = MeasureTextEx(fuente_menu, titulo, 60, 2);
        DrawTextEx(fuente_menu, titulo, {(ANCHO - tsize.x)/2.0f, ALTO * 0.2f}, 60, 2, GOLD);
        
        Vector2 size_sub = MeasureTextEx(fuente_menu, subtitulo, 24, 1);
        DrawTextEx(fuente_menu, subtitulo, {(ANCHO - size_sub.x)/2.0f, ALTO * 0.2f + 70.0f}, 24, 1, SKYBLUE);
    }
    
    // Botones
    float btn_w = 300.0f;
    float btn_h = 50.0f;
    float start_y = ALTO * 0.45f + 100.0f;
    float spacing = 40.0f;
    
    Rectangle rect_jugar = {(ANCHO - btn_w)/2.0f, start_y + 80.0f, btn_w, btn_h};
    Rectangle rect_creativo = {(ANCHO - btn_w)/2.0f, start_y + spacing + 80.0f, btn_w, btn_h};
    Rectangle rect_opciones = {(ANCHO - btn_w)/2.0f, start_y + spacing * 2.0f + 80.0f, btn_w, btn_h};
    Rectangle rect_salir = {(ANCHO - btn_w)/2.0f, start_y + spacing * 3.0f + 80.0f, btn_w, btn_h};
    
    // Contenedor de opciones del menú (menu.png)
    if (tex_menu_box.id > 0) {
        float panel_w = btn_w + 80.0f ; 
        float panel_h = spacing * 3.0f + btn_h + 60.0f;
        Rectangle panel_rect = { (ANCHO - panel_w)/2.0f, start_y  +50.0f, panel_w, panel_h };
        DrawTexturePro(tex_menu_box, {0.0f, 0.0f, (float)tex_menu_box.width, (float)tex_menu_box.height}, panel_rect, {0.0f, 0.0f}, 0.0f, WHITE);
    }
    
    if (dibujar_boton_imagen_interactivo(rect_jugar, tex_btn_jugar1, tex_btn_jugar2)) {
        refrescar_lista_partidas();
        estado_actual = EstadoJuego::SELECCION_NIVELES;
    }
    
    if (dibujar_boton_imagen_interactivo(rect_creativo, tex_btn_creativo1, tex_btn_creativo2)) {
        motor.limpiar();
        contador_bolas = 0;
        resetear_punteros_borde();
        limpiar_estado_tras_cargar_partida();
        crear_escena(motor);
        gestor_eventos.limpiar();
        modo_evento_ui = ModoEventoUI::INACTIVO;
        estado_actual = EstadoJuego::JUEGO_CREATIVO;
        motor.set_pausado(true);
    }
    
    if (dibujar_boton_imagen_interactivo(rect_opciones, tex_btn_opciones1, tex_btn_opciones2)) {
        estado_previo = estado_actual;
        estado_actual = EstadoJuego::MENU_OPCIONES;
    }
    
    if (dibujar_boton_imagen_interactivo(rect_salir, tex_btn_salir1, tex_btn_salir2)) {
        salir_juego = true;
    }

    // Dibujar personaje animado Moto (Frans) - Adelante del cuadro de opciones
    if (anim_menu_moto) {
        Vector2 pos_dibujo = { pos_x_anim_moto, ALTO * 0.65f + 150.0f + 75.0f };
        anim_menu_moto->dibujar(pos_dibujo, 110.0f * 1.5f + 200 , 200.0f * 1.5f +50);
    }
}

// ============================================================================
// Funciones del Menú de Opciones
// ============================================================================
void actualizar_menu_opciones() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        estado_actual = estado_previo;
    }
}

void dibujar_menu_opciones() {
    if (tex_menu_fondo.id > 0) {
        float escala_x = static_cast<float>(ANCHO) / tex_menu_fondo.width;
        float escala_y = static_cast<float>(ALTO) / tex_menu_fondo.height;
        float escala = (escala_x > escala_y) ? escala_x : escala_y;
        DrawTextureEx(tex_menu_fondo, {0, 0}, 0, escala, WHITE);
    } else {
        DrawRectangleGradientV(0, 0, ANCHO, ALTO, COLOR_FONDO, Color{10, 10, 25, 255});
    }
    
    // Geometría y variables
    float sidebar_x = ANCHO * 0.10f;
    float sidebar_w = 270.0f;
    float divider_x = sidebar_x + sidebar_w + 30.0f;
    float content_x = divider_x + 50.0f;
    float start_y = ALTO * 0.25f;
    float btn_h = 45.0f;
    float spacing_y = 52.0f;
    Vector2 mouse = GetMousePosition();

    // 1. Título principal izquierdo
    float title_y = ALTO * 0.15f;
    DrawRectangle(static_cast<int>(sidebar_x), static_cast<int>(title_y), 4, 32, SKYBLUE);
    DrawTextEx(fuente_menu, "OPCIONES", { sidebar_x + 15.0f, title_y }, 32.0f, 2.0f, WHITE);

    // 2. Definir pestanas del sidebar
    struct TabButton {
        TabOpciones tab;
        const char* label;
    };
    TabButton tabs[] = {
        { TabOpciones::JUGABILIDAD, "JUGABILIDAD" },
        { TabOpciones::CONTROLES, "CONTROLES" },
        { TabOpciones::VIDEO, "VIDEO" },
        { TabOpciones::SONIDO, "SONIDO" },
        { TabOpciones::IDIOMA, "IDIOMA" }
    };
    int num_tabs = 5;

    // Dibujar pestanas principales
    for (int i = 0; i < num_tabs; ++i) {
        Rectangle btn_rect = { sidebar_x, start_y + i * spacing_y, sidebar_w, btn_h };
        bool is_active = (pestana_opciones_actual == tabs[i].tab);
        bool hover = CheckCollisionPointRec(mouse, btn_rect);
        
        if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            pestana_opciones_actual = tabs[i].tab;
        }
        
        if (is_active) {
            DrawRectangleRec(btn_rect, Color{ 55, 130, 210, 200 });
            // Triángulo de selección (indicador de la derecha)
            DrawTriangle(
                { btn_rect.x + btn_rect.width - 20.0f, btn_rect.y + btn_rect.height/2.0f - 5.0f },
                { btn_rect.x + btn_rect.width - 20.0f, btn_rect.y + btn_rect.height/2.0f + 5.0f },
                { btn_rect.x + btn_rect.width - 12.0f, btn_rect.y + btn_rect.height/2.0f },
                WHITE
            );
        } else if (hover) {
            DrawRectangleRec(btn_rect, Color{ 255, 255, 255, 30 });
        }
        
        Color text_color = is_active ? WHITE : (hover ? LIGHTGRAY : Color{ 180, 180, 200, 255 });
        Vector2 text_size = MeasureTextEx(fuente_menu, tabs[i].label, 20.0f, 1.0f);
        DrawTextEx(fuente_menu, tabs[i].label, { btn_rect.x + 15.0f, btn_rect.y + (btn_rect.height - text_size.y)/2.0f }, 20.0f, 1.0f, text_color);
    }

    // 3. Botón Restaurar Predeterminados
    Rectangle btn_reset = { sidebar_x, start_y + 5.3f * spacing_y, sidebar_w, btn_h };
    bool hover_reset = CheckCollisionPointRec(mouse, btn_reset);
    if (hover_reset && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        sonido_mutado = false;
        if (IsMusicReady(musica_menu)) {
            SetMusicVolume(musica_menu, 0.6f);
        }
    }
    if (hover_reset) {
        DrawRectangleRec(btn_reset, Color{ 255, 255, 255, 30 });
    }
    Color text_color_reset = hover_reset ? WHITE : Color{ 160, 160, 180, 255 };
    Vector2 ts_reset = MeasureTextEx(fuente_menu, "RESTAURAR PREDET.", 18.0f, 1.0f);
    DrawTextEx(fuente_menu, "RESTAURAR PREDET.", { btn_reset.x + 15.0f, btn_reset.y + (btn_reset.height - ts_reset.y)/2.0f }, 18.0f, 1.0f, text_color_reset);

    // 4. Botón Volver
    Rectangle btn_volver = { sidebar_x, start_y + 6.3f * spacing_y, sidebar_w, btn_h };
    bool hover_volver = CheckCollisionPointRec(mouse, btn_volver);
    if (hover_volver && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        estado_actual = estado_previo;
    }
    if (hover_volver) {
        DrawRectangleRec(btn_volver, Color{ 255, 255, 255, 30 });
    }
    Color text_color_volver = hover_volver ? WHITE : Color{ 180, 180, 200, 255 };
    Vector2 ts_volver = MeasureTextEx(fuente_menu, "VOLVER", 20.0f, 1.0f);
    DrawTextEx(fuente_menu, "VOLVER", { btn_volver.x + 15.0f, btn_volver.y + (btn_volver.height - ts_volver.y)/2.0f }, 20.0f, 1.0f, text_color_volver);

    // 5. Línea divisoria vertical
    DrawLineEx({ divider_x, ALTO * 0.15f }, { divider_x, ALTO * 0.82f }, 1.5f, Color{ 70, 75, 110, 100 });

    // 6. Columna derecha (Contenido dinámico de la pestana)
    float content_y = ALTO * 0.25f;

    if (pestana_opciones_actual == TabOpciones::CONTROLES) {
        DrawTextEx(fuente_menu, "CONTROLES", { content_x, ALTO * 0.15f }, 32.0f, 2.0f, SKYBLUE);
        
        float dy = 38.0f;
        auto dibujar_linea_opcion = [&](const char* tecla, const char* descripcion, float y) {
            DrawTextEx(fuente_menu, tecla, { content_x, y }, 22.0f, 1.0f, GOLD);
            DrawTextEx(fuente_menu, descripcion, { content_x + 180.0f, y }, 22.0f, 1.0f, LIGHTGRAY);
        };
        
        dibujar_linea_opcion("[Arrastrar]", "Crear objetos desde el menu lateral y soltarlos en el canvas", content_y);
        dibujar_linea_opcion("[Click Izq]", "Arrastrar y mover objetos ya colocados en la escena", content_y + dy);
        dibujar_linea_opcion("[TAB]", "Mostrar u ocultar el menu lateral de herramientas", content_y + dy * 2);
        dibujar_linea_opcion("[ESPACIO]", "Pausar / Reanudar la simulacion fisica", content_y + dy * 3);
        dibujar_linea_opcion("[D]", "Activar / Desactivar modo debug (wireframes y vectores)", content_y + dy * 4);
        dibujar_linea_opcion("[F / T]", "Rotar (F) / Alternar tamano (T) (Rampas y Ventilador)", content_y + dy * 5);
        dibujar_linea_opcion("[R]", "Reiniciar escena (borra todo y recarga inicial)", content_y + dy * 6);
        dibujar_linea_opcion("[+/-]", "Aumentar / Disminuir gravedad", content_y + dy * 7);
        dibujar_linea_opcion("[SUPR/X]", "Eliminar el objeto seleccionado", content_y + dy * 8);
        dibujar_linea_opcion("[ESC]", "Regresar / Pausar el juego", content_y + dy * 9);
    } else {
        const char* section_title = "";
        if (pestana_opciones_actual == TabOpciones::JUGABILIDAD) section_title = "JUGABILIDAD";
        else if (pestana_opciones_actual == TabOpciones::VIDEO) section_title = "VIDEO";
        else if (pestana_opciones_actual == TabOpciones::SONIDO) section_title = "SONIDO";
        else if (pestana_opciones_actual == TabOpciones::IDIOMA) section_title = "IDIOMA";
        
        DrawTextEx(fuente_menu, section_title, { content_x, ALTO * 0.15f }, 32.0f, 2.0f, SKYBLUE);
        
        DrawTextEx(fuente_menu, "Esta categoria no tiene opciones configurables en esta version.", { content_x, content_y }, 20.0f, 1.0f, GRAY);
        DrawTextEx(fuente_menu, "Proximamente estaran disponibles mas parametros de configuracion.", { content_x, content_y + 35.0f }, 18.0f, 1.0f, DARKGRAY);
    }
}

// ============================================================================
// Funciones del Selector de Niveles
// ============================================================================
void actualizar_seleccion_niveles(MotorFisica& motor) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        estado_actual = EstadoJuego::MENU_PRINCIPAL;
    }
}

// Inicializa un nivel de campaña oficial en memoria
void cargar_nivel_campana(MotorFisica& motor, int lvl_idx) {
    tiempo_nivel = 0.0;
    nivel_campana_actual = lvl_idx;
    nivel_usuario_actual_path = "";

    motor.limpiar();
    resetear_punteros_borde();
    limpiar_estado_tras_cargar_partida();
    crear_bordes_nivel(motor);
    
    // Ruta del archivo dinámico para la campaña
    std::string path;
    if (lvl_idx == 0) path = "Assets/campaign/1_primer_impacto.tim";
    else if (lvl_idx == 1) path = "Assets/campaign/2_rebote_perfecto.tim";
    else if (lvl_idx == 2) path = "Assets/campaign/3_el_soplido.tim";
    else path = "Assets/campaign/4_messi_gol_gol_gol.tim";

    if (FileExists(path.c_str())) {
        int w = ANCHO;
        int h = ALTO;
        int cont = contador_bolas;
        estado_actual = EstadoJuego::JUEGO_NIVEL;
        if (cargar_partida(motor, gestor_eventos, path, w, h, cont)) {
            ANCHO = w;
            ALTO = h;
            contador_bolas = cont;
        } else {
            estado_actual = EstadoJuego::SELECCION_NIVELES;
            TraceLog(LOG_WARNING, "Error al cargar archivo de campana: %s", path.c_str());
        }
    } else {
        TraceLog(LOG_WARNING, "Archivo de campana no encontrado: %s", path.c_str());
    }
}

void dibujar_seleccion_niveles(MotorFisica& motor) {
    if (tex_menu_fondo.id > 0) {
        float escala_x = static_cast<float>(ANCHO) / tex_menu_fondo.width;
        float escala_y = static_cast<float>(ALTO) / tex_menu_fondo.height;
        float escala = (escala_x > escala_y) ? escala_x : escala_y;
        DrawTextureEx(tex_menu_fondo, {0, 0}, 0, escala, WHITE);
    } else {
        DrawRectangleGradientV(0, 0, ANCHO, ALTO, COLOR_FONDO, Color{10, 10, 25, 255});
    }
    
    // Geometría y variables
    float sidebar_x = ANCHO * 0.10f;
    float sidebar_w = 270.0f;
    float divider_x = sidebar_x + sidebar_w + 30.0f;
    float content_x = divider_x + 50.0f;
    float start_y = ALTO * 0.25f;
    float btn_h = 45.0f;
    float spacing_y = 52.0f;
    Vector2 mouse = GetMousePosition();

    // 1. Título principal izquierdo
    float title_y = ALTO * 0.15f;
    DrawRectangle(static_cast<int>(sidebar_x), static_cast<int>(title_y), 4, 32, SKYBLUE);
    DrawTextEx(fuente_menu, "NIVELES", { sidebar_x + 15.0f, title_y }, 32.0f, 2.0f, WHITE);

    // 2. Definir pestanas del sidebar
    struct TabButton {
        TabNiveles tab;
        const char* label;
    };
    TabButton tabs[] = {
        { TabNiveles::CAMPANA, "CAMPANA" },
        { TabNiveles::MIS_NIVELES, "MIS NIVELES" },
        { TabNiveles::IMPORTAR, "IMPORTAR (.tim)" }
    };
    int num_tabs = 3;

    // Dibujar pestanas principales
    for (int i = 0; i < num_tabs; ++i) {
        Rectangle btn_rect = { sidebar_x, start_y + i * spacing_y, sidebar_w, btn_h };
        bool is_active = (pestana_niveles_actual == tabs[i].tab);
        bool hover = CheckCollisionPointRec(mouse, btn_rect);
        
        if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            pestana_niveles_actual = tabs[i].tab;
        }
        
        if (is_active) {
            DrawRectangleRec(btn_rect, Color{ 55, 130, 210, 200 });
            DrawTriangle(
                { btn_rect.x + btn_rect.width - 20.0f, btn_rect.y + btn_rect.height/2.0f - 5.0f },
                { btn_rect.x + btn_rect.width - 20.0f, btn_rect.y + btn_rect.height/2.0f + 5.0f },
                { btn_rect.x + btn_rect.width - 12.0f, btn_rect.y + btn_rect.height/2.0f },
                WHITE
            );
        } else if (hover) {
            DrawRectangleRec(btn_rect, Color{ 255, 255, 255, 30 });
        }
        
        Color text_color = is_active ? WHITE : (hover ? LIGHTGRAY : Color{ 180, 180, 200, 255 });
        Vector2 text_size = MeasureTextEx(fuente_menu, tabs[i].label, 20.0f, 1.0f);
        DrawTextEx(fuente_menu, tabs[i].label, { btn_rect.x + 15.0f, btn_rect.y + (btn_rect.height - text_size.y)/2.0f }, 20.0f, 1.0f, text_color);
    }

    // Botón Volver
    Rectangle btn_volver = { sidebar_x, start_y + 5.0f * spacing_y, sidebar_w, btn_h };
    bool hover_volver = CheckCollisionPointRec(mouse, btn_volver);
    if (hover_volver && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        estado_actual = EstadoJuego::MENU_PRINCIPAL;
    }
    if (hover_volver) {
        DrawRectangleRec(btn_volver, Color{ 255, 255, 255, 30 });
    }
    Color text_color_volver = hover_volver ? WHITE : Color{ 180, 180, 200, 255 };
    Vector2 ts_volver = MeasureTextEx(fuente_menu, "VOLVER", 20.0f, 1.0f);
    DrawTextEx(fuente_menu, "VOLVER", { btn_volver.x + 15.0f, btn_volver.y + (btn_volver.height - ts_volver.y)/2.0f }, 20.0f, 1.0f, text_color_volver);

    // 3. Línea divisoria vertical
    DrawLineEx({ divider_x, ALTO * 0.15f }, { divider_x, ALTO * 0.82f }, 1.5f, Color{ 70, 75, 110, 100 });

    // 4. Panel derecho de contenido
    float content_y = ALTO * 0.25f;

    if (pestana_niveles_actual == TabNiveles::CAMPANA) {
        DrawTextEx(fuente_menu, "CAMPANA OFICIAL", { content_x, ALTO * 0.15f }, 32.0f, 2.0f, SKYBLUE);
        
        // Grid de 4 niveles oficiales pre-diseñados
        float card_w = 380.0f;
        float card_h = 160.0f;
        float spacing_x = 40.0f;
        float spacing_y = 30.0f;
        int cols = 3;
        
        struct LevelData {
            const char* name;
            const char* desc1;
            const char* desc2;
            const char* diff;
            Color diff_color;
        };
        LevelData official_lvls[] = {
            { "1. Primer Impacto", "Entiende la gravedad haciendo caer", "una pelota dentro de la cubeta.", "FACIL", GREEN },
            { "2. Rebote Perfecto", "Utiliza el trampolin para desviar", "la trayectoria de la bola.", "FACIL", GREEN },
            { "3. El Soplido", "Usa la corriente de viento del", "ventilador para empujar objetos.", "MEDIO", ORANGE },
            { "4. Messi gol gol gol", "Ayuda a Messi a meter un golazo", "usando los trampolines.", "DIFICIL", RED }
        };
        
        for (int i = 0; i < 4; ++i) {
            int r = i / cols;
            int c = i % cols;
            float x = content_x + c * (card_w + spacing_x);
            float y = content_y + r * (card_h + spacing_y);
            Rectangle card = { x, y, card_w, card_h };
            
            bool hover_card = CheckCollisionPointRec(mouse, card);
            Rectangle draw_card = card;
            if (hover_card) {
                draw_card.x -= 2; draw_card.y -= 2; draw_card.width += 4; draw_card.height += 4;
            }
            
            DrawRectangleRounded(draw_card, 0.08f, 4, hover_card ? Color{55, 130, 210, 180} : Color{40, 42, 68, 200});
            DrawRectangleRoundedLinesEx(draw_card, 0.08f, 4, 2.0f, hover_card ? WHITE : Color{70, 75, 110, 255});
            
            // Dibujar textos en la tarjeta
            DrawTextEx(fuente_menu, official_lvls[i].name, { draw_card.x + 15.0f, draw_card.y + 15.0f }, 24.0f, 1.0f, GOLD);
            DrawTextEx(fuente_menu, official_lvls[i].diff, { draw_card.x + 15.0f, draw_card.y + 40.0f }, 16.0f, 1.0f, official_lvls[i].diff_color);
            
            // Texto descriptivo en dos líneas fijas
            DrawTextEx(fuente_menu, official_lvls[i].desc1, { draw_card.x + 15.0f, draw_card.y + 65.0f }, 18.0f, 1.0f, LIGHTGRAY);
            DrawTextEx(fuente_menu, official_lvls[i].desc2, { draw_card.x + 15.0f, draw_card.y + 85.0f }, 18.0f, 1.0f, LIGHTGRAY);
            
            if (hover_card && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                cargar_nivel_campana(motor, i);
            }
        }
    } 
    else if (pestana_niveles_actual == TabNiveles::MIS_NIVELES) {
        DrawTextEx(fuente_menu, "MIS NIVELES", { content_x, ALTO * 0.15f }, 32.0f, 2.0f, SKYBLUE);
        
        float card_w = 260.0f;
        float card_h = 100.0f;
        float spacing_x = 30.0f;
        float spacing_y = 20.0f;
        int cols = 3;
        
        for (size_t i = 0; i < partidas_guardadas.size(); ++i) {
            int r = i / cols;
            int c = i % cols;
            float x = content_x + c * (card_w + spacing_x);
            float y = content_y + r * (card_h + spacing_y);
            
            Rectangle card_rect = {x, y, card_w, card_h};
            bool hover = CheckCollisionPointRec(mouse, card_rect);
            Rectangle draw_rect = card_rect;
            if (hover) {
                draw_rect.x -= 2; draw_rect.y -= 2; draw_rect.width += 4; draw_rect.height += 4;
            }
            
            Color card_color = hover ? Color{55, 130, 210, 180} : Color{40, 42, 68, 200};
            DrawRectangleRounded(draw_rect, 0.1f, 4, card_color);
            DrawRectangleRoundedLinesEx(draw_rect, 0.1f, 4, 2.0f, hover ? WHITE : Color{70, 75, 110, 255});
            
            std::string lvl_num_str = "CREACION " + std::to_string(i + 1);
            Vector2 text_size_num = MeasureTextEx(fuente_menu, lvl_num_str.c_str(), 18, 1);
            DrawTextEx(fuente_menu, lvl_num_str.c_str(), 
                       {draw_rect.x + (draw_rect.width - text_size_num.x)/2.0f, draw_rect.y + 15.0f},
                       18, 1, GOLD);
                       
            std::string lvl_name = partidas_guardadas[i].nombre;
            std::replace(lvl_name.begin(), lvl_name.end(), '_', ' ');
            Vector2 text_size_name = MeasureTextEx(fuente_menu, lvl_name.c_str(), 20, 1);
            if (text_size_name.x > card_w - 20) {
                lvl_name = lvl_name.substr(0, 15) + "...";
                text_size_name = MeasureTextEx(fuente_menu, lvl_name.c_str(), 20, 1);
            }
            DrawTextEx(fuente_menu, lvl_name.c_str(),
                       {draw_rect.x + (draw_rect.width - text_size_name.x)/2.0f, draw_rect.y + 45.0f},
                       20, 1, WHITE);
                       
            if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                tiempo_nivel = 0.0;
                campana_nivel_actual = -1;
                ruta_nivel_actual = partidas_guardadas[i].ruta_archivo;

                int ancho_cargado = ANCHO;
                int alto_cargado = ALTO;
                nivel_campana_actual = -1;
                nivel_usuario_actual_path = partidas_guardadas[i].ruta_archivo;
                estado_actual = EstadoJuego::JUEGO_NIVEL;
                if (cargar_partida(motor, gestor_eventos, partidas_guardadas[i].ruta_archivo, ancho_cargado, alto_cargado, contador_bolas)) {
                    ANCHO = ancho_cargado;
                    ALTO = alto_cargado;
                } else {
                    estado_actual = EstadoJuego::SELECCION_NIVELES;
                }
            }
        }
        
        if (partidas_guardadas.empty()) {
            DrawTextEx(fuente_menu, "No hay niveles creados por el usuario en 'saves/'", { content_x, content_y }, 20.0f, 1.0f, GRAY);
            DrawTextEx(fuente_menu, "Entra a 'Modo Creativo' desde el menu de inicio para disenar y guardar tus puzles.", { content_x, content_y + 30.0f }, 16.0f, 1.0f, LIGHTGRAY);
        }
    }
    else if (pestana_niveles_actual == TabNiveles::IMPORTAR) {
        DrawTextEx(fuente_menu, "IMPORTAR NIVEL (.tim)", { content_x, ALTO * 0.15f }, 32.0f, 2.0f, SKYBLUE);
        
        // Área interactiva de Drag and Drop
        float zone_w = 640.0f;
        float zone_h = 320.0f;
        Rectangle import_zone = { content_x, content_y, zone_w, zone_h };
        
        bool hover_zone = CheckCollisionPointRec(mouse, import_zone);
        
        // Dibujar borde punteado o discontinuo
        DrawRectangleRounded(import_zone, 0.04f, 6, hover_zone ? Color{55, 130, 210, 40} : Color{255, 255, 255, 10});
        DrawRectangleRoundedLinesEx(import_zone, 0.04f, 6, 2.5f, hover_zone ? SKYBLUE : Color{120, 125, 150, 150});
        
        // Icono de carga procedimental
        float cx = import_zone.x + import_zone.width / 2.0f;
        float cy = import_zone.y + import_zone.height / 2.0f - 30.0f;
        
        // Dibujar caja de archivo / flecha de importación
        DrawRectangleLines(cx - 30, cy - 30, 60, 60, hover_zone ? SKYBLUE : LIGHTGRAY);
        DrawTriangle({ cx - 15, cy + 10 }, { cx + 15, cy + 10 }, { cx, cy - 15 }, hover_zone ? SKYBLUE : LIGHTGRAY);
        
        const char* text_imp = "ARRASTRA Y SUELTA UN ARCHIVO .tim AQUI";
        Vector2 ts_imp = MeasureTextEx(fuente_menu, text_imp, 22.0f, 1.0f);
        DrawTextEx(fuente_menu, text_imp, { cx - ts_imp.x/2.0f, cy + 55.0f }, 22.0f, 1.0f, hover_zone ? WHITE : LIGHTGRAY);
        
        const char* text_sub = "(O copia tus archivos de nivel directamente en la carpeta 'saves/')";
        Vector2 ts_sub = MeasureTextEx(fuente_menu, text_sub, 16.0f, 1.0f);
        DrawTextEx(fuente_menu, text_sub, { cx - ts_sub.x/2.0f, cy + 85.0f }, 16.0f, 1.0f, GRAY);
    }
}

struct BotonesHUD {
    Rectangle rect_play;
    Rectangle rect_reset;
    Rectangle rect_opciones;
    Rectangle rect_ayuda;
    Rectangle rect_salir;
};

BotonesHUD calcular_rectangulos_botones_hud() {
    float escala_panel_x = (ANCHO - 40) / 1611.0f;
    float escala_panel_y = 130.0f / 138.0f;
    float escala = (escala_panel_x > escala_panel_y) ? escala_panel_x : escala_panel_y;
    
    float y_offset = ALTO - 150.0f;
    
    BotonesHUD btns;
    btns.rect_play = { 20.0f + 1110.0f * escala + 35, y_offset + 20.0f * escala + 25 , 70.0f * escala - 20 , 70.0f * escala - 20 };
    btns.rect_reset = { 20.0f + 1210.0f * escala + 35 - 1260, y_offset + 20.0f * escala + 25, 70.0f * escala - 20 +10, 70.0f * escala - 20 +10};
    btns.rect_opciones = { 20.0f + 1310.0f * escala+35, y_offset + 20.0f * escala + 25 , 70.0f * escala-20 , 70.0f * escala - 20 };
    btns.rect_ayuda = { 20.0f + 1410.0f * escala+26, y_offset + 20.0f * escala + 25 , 70.0f * escala-20 , 70.0f * escala - 20};
    btns.rect_salir = { 20.0f + 1510.0f * escala+26, y_offset + 20.0f * escala + 25 , 70.0f * escala-20 , 70.0f * escala - 20};
    btns.rect_play = { 20.0f * escala+25 , y_offset + 10.0f * escala + 15 , 70.0f * escala + 40 ,92.0f * escala};
    return btns;
}


// ============================================================================
// Núcleo del Juego (Común para Sandbox y Niveles)
// ============================================================================
void actualizar_juego_core(MotorFisica& motor, bool es_modo_nivel) {
    if (mostrar_pausa_overlay) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            mostrar_pausa_overlay = false;
        }
        return;
    }

    if (mostrar_ayuda_overlay) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            mostrar_ayuda_overlay = false;
        } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int mx = GetMouseX();
            int my = GetMouseY();
            float w = 600.0f;
            float h = 400.0f;
            Rectangle box = {(ANCHO - w)/2.0f, (ALTO - h)/2.0f, w, h};
            Rectangle close_btn = {box.x + box.width - 40, box.y + 10, 30, 30};
            if (CheckCollisionPointRec({(float)mx, (float)my}, close_btn) || !CheckCollisionPointRec({(float)mx, (float)my}, box)) {
                mostrar_ayuda_overlay = false;
            }
        }
        return;
    }

    if (es_modo_nivel && !gestor_eventos.victoria_alcanzada) {
        tiempo_nivel += GetFrameTime();
    }

    reconstruir_celdas_menu();

    if (IsKeyPressed(KEY_TAB)) {
        menu_visible = !menu_visible;
    }

    int mx = GetMouseX();
    int my = GetMouseY();

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (modo_panel_guardado != ModoPanelGuardado::CERRADO) {
            modo_panel_guardado = ModoPanelGuardado::CERRADO;
        } else if (estado_cuerda != EstadoColocacionCuerda::INACTIVA) {
            cancelar_colocacion_cuerda();
        } else {
            mostrar_pausa_overlay = true;
        }
        return;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) &&
        estado_cuerda != EstadoColocacionCuerda::INACTIVA) {
        cancelar_colocacion_cuerda();
    }

    if (estado_actual != EstadoJuego::JUEGO_NIVEL) {
        manejar_teclas_panel_guardado(motor, gestor_eventos, ANCHO, ALTO, contador_bolas);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        // Interceptar clicks en los botones del HUD primero
        BotonesHUD btns = calcular_rectangulos_botones_hud();
        Vector2 mouse_pos = {(float)mx, (float)my};
        if (!punto_en_panel_izquierdo(mx, my) && !punto_en_menu(mx, my)) {
            if (CheckCollisionPointRec(mouse_pos, btns.rect_play)) {
                if (motor.get_pausado()) {
                    guardar_snapshot_simulacion(motor, gestor_eventos);
                    motor.aplicar_transmision_bandas(); // apagar ventiladores controlados antes del primer frame
                    motor.set_pausado(false);
                    cancelar_colocacion_cuerda();
                } else {
                    spawn_error_timer = 0.5f;
                    spawn_error_pos = Vector2D(mx, my);
                }
                return;
            }
            if (CheckCollisionPointRec(mouse_pos, btns.rect_reset)) {
                restaurar_snapshot_simulacion(motor, gestor_eventos);
                motor.aplicar_transmision_bandas();
                motor.set_pausado(true);
                return;
            }
            if (CheckCollisionPointRec(mouse_pos, btns.rect_opciones)) {
                mostrar_pausa_overlay = true;
                return;
            }
            if (CheckCollisionPointRec(mouse_pos, btns.rect_ayuda)) {
                mostrar_ayuda_overlay = true;
                return;
            }
            if (CheckCollisionPointRec(mouse_pos, btns.rect_salir)) {
                menu_visible = !menu_visible;
                return;
            }
            if (CheckCollisionPointRec(mouse_pos, btns.rect_play)) {
                motor.set_pausado(!motor.get_pausado());
                if (!motor.get_pausado()) cancelar_colocacion_cuerda();
                return;
            }
        }


        if (punto_en_panel_izquierdo(mx, my)) {
            entidad_seleccionada = nullptr;  // Deseleccionar al interactuar con el panel de eventos
        } else if (punto_en_menu(mx, my)) {
            entidad_seleccionada = nullptr;  // Deseleccionar al clickear en el menú
            if (manejar_click_menu(mx, my, motor)) {
                // UI consumió el click
            } else {
                TipoObjetoMenu tipo = tipo_en_celda(mx, my, celdas_menu_cache);
                if (tipo == TipoObjetoMenu::CUERDA) {
                    if (motor.get_pausado()) {
                        if (estado_actual == EstadoJuego::JUEGO_NIVEL && inventario_actual[TipoObjetoMenu::CUERDA] <= 0) {
                            spawn_error_timer = 0.5f;
                            spawn_error_pos = Vector2D(mx, my);
                        } else {
                            cancelar_colocacion_cuerda();
                            estado_cuerda = EstadoColocacionCuerda::ESPERANDO_EXTREMO_A;
                        }
                    } else {
                        spawn_error_timer = 0.5f;
                        spawn_error_pos = Vector2D(mx, my);
                    }
                } else if (tipo == TipoObjetoMenu::BANDA) {
                    if (motor.get_pausado()) {
                        estado_banda = EstadoConexionBanda::ESPERANDO_HAMSTER;
                        banda_hamster_id = -1;
                    } else {
                        spawn_error_timer = 0.5f;
                        spawn_error_pos = Vector2D(mx, my);
                    }
                } else if (tipo != TipoObjetoMenu::NINGUNO) {
                    if (!motor.get_pausado()) {
                        spawn_error_timer = 0.5f;
                        spawn_error_pos = Vector2D(mx, my);
                    } else if (estado_actual == EstadoJuego::JUEGO_NIVEL && inventario_actual[tipo] <= 0) {
                        spawn_error_timer = 0.5f;
                        spawn_error_pos = Vector2D(mx, my);
                    } else {
                        cancelar_colocacion_cuerda();
                        arrastrando_spawn = tipo;
                    }
                }
            }
        } else if (punto_en_area_juego(mx, my)) {
            Vector2D mouse_pos(mx, my);
            if (arrastrando_spawn != TipoObjetoMenu::NINGUNO && arrastrando_spawn != TipoObjetoMenu::BANDA) {
                spawn_desde_menu(motor, arrastrando_spawn, mouse_pos);
                arrastrando_spawn = TipoObjetoMenu::NINGUNO;
            } else if (arrastrando_spawn == TipoObjetoMenu::BANDA) {
                arrastrando_spawn = TipoObjetoMenu::NINGUNO;
                manejar_click_conexion_banda(motor, mouse_pos);
            } else if (manejar_click_colocacion_cuerda(motor, mouse_pos)) {
                entidad_arrastrada = nullptr;
            } else if (manejar_click_conexion_banda(motor, mouse_pos)) {
                entidad_arrastrada = nullptr;
            } else {
                bool handle_click_detectado = false;
                if (estado_actual == EstadoJuego::JUEGO_CREATIVO && entidad_seleccionada && motor.get_pausado()) {
                    ParedRectangular* p = dynamic_cast<ParedRectangular*>(entidad_seleccionada);
                    Caminadora* camh = dynamic_cast<Caminadora*>(entidad_seleccionada);
                    if (p && !es_borde_nivel(p)) {
                        auto handles = calcular_handles(p);
                        for (int i = 0; i < 8; ++i) {
                            if (CheckCollisionPointRec({(float)mx, (float)my}, handles.rects[i])) {
                                handle_activo = handles.tipos[i];
                                handle_pos_inicial_mouse = Vector2D(mx, my);
                                handle_w_inicial = p->get_ancho();
                                handle_h_inicial = p->get_alto();
                                handle_pos_inicial_ent = p->get_posicion();
                                handle_click_detectado = true;
                                break;
                            }
                        }
                    } else if (camh) {
                        auto handles = calcular_handles_xywh((float)camh->get_posicion().x, (float)camh->get_posicion().y,
                                                             (float)camh->get_ancho(), (float)camh->get_alto());
                        for (int i = 0; i < 8; ++i) {
                            if (CheckCollisionPointRec({(float)mx, (float)my}, handles.rects[i])) {
                                handle_activo = handles.tipos[i];
                                handle_pos_inicial_mouse = Vector2D(mx, my);
                                handle_w_inicial = camh->get_ancho();
                                handle_h_inicial = camh->get_alto();
                                handle_pos_inicial_ent = camh->get_posicion();
                                handle_click_detectado = true;
                                break;
                            }
                        }
                    }
                }

                if (!handle_click_detectado) {
                    EntidadFisica* clicked = obtener_entidad_bajo_mouse(motor, mouse_pos);
                    if (clicked) {
                        if (estado_actual == EstadoJuego::JUEGO_NIVEL && clicked->get_es_fijo()) {
                            entidad_seleccionada = clicked;
                        } else {
                            bool manejado_por_evento = false;
                            if (estado_actual == EstadoJuego::JUEGO_CREATIVO) {
                                manejado_por_evento = manejar_click_evento_ui(gestor_eventos, clicked);
                            }
                            if (!manejado_por_evento) {
                                if (motor.get_pausado()) {
                                    entidad_arrastrada = clicked;
                                    offset_arrastre = clicked->get_posicion() - mouse_pos;
                                } else {
                                    spawn_error_timer = 0.5f;
                                    spawn_error_pos = clicked->get_posicion();
                                }
                                entidad_seleccionada = clicked;
                            }
                        }
                    } else {
                        entidad_seleccionada = nullptr;
                    }
                }
            }
        }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && arrastrando_spawn != TipoObjetoMenu::NINGUNO) {
        // Ghost spawn
    } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && handle_activo != HandleResize::NINGUNO && entidad_seleccionada) {
        ParedRectangular* p = dynamic_cast<ParedRectangular*>(entidad_seleccionada);
        if (p) {
            double dx = mx - handle_pos_inicial_mouse.x;
            double dy = my - handle_pos_inicial_mouse.y;
            double nw = handle_w_inicial, nh = handle_h_inicial;
            Vector2D npos = handle_pos_inicial_ent;
            bool izq  = handle_activo==HandleResize::TOP_LEFT||handle_activo==HandleResize::MID_LEFT||handle_activo==HandleResize::BOT_LEFT;
            bool der  = handle_activo==HandleResize::TOP_RIGHT||handle_activo==HandleResize::MID_RIGHT||handle_activo==HandleResize::BOT_RIGHT;
            bool arr  = handle_activo==HandleResize::TOP_LEFT||handle_activo==HandleResize::TOP_CENTER||handle_activo==HandleResize::TOP_RIGHT;
            bool abaj = handle_activo==HandleResize::BOT_LEFT||handle_activo==HandleResize::BOT_CENTER||handle_activo==HandleResize::BOT_RIGHT;
            if (der)  nw = std::max(20.0, handle_w_inicial + dx);
            if (izq)  { nw = std::max(20.0, handle_w_inicial - dx); npos.x = handle_pos_inicial_ent.x + (handle_w_inicial - nw); }
            if (abaj) nh = std::max(8.0, handle_h_inicial + dy);
            if (arr)  { nh = std::max(8.0, handle_h_inicial - dy); npos.y = handle_pos_inicial_ent.y + (handle_h_inicial - nh); }
            p->set_posicion(npos);
            p->set_dimensiones(nw, nh);
        } else if (Caminadora* camd = dynamic_cast<Caminadora*>(entidad_seleccionada)) {
            // La caminadora SOLO cambia el ancho (con su límite interno). Alto y Y fijos.
            double dx = mx - handle_pos_inicial_mouse.x;
            double nw = handle_w_inicial;
            Vector2D npos = handle_pos_inicial_ent;
            bool izq = handle_activo==HandleResize::TOP_LEFT||handle_activo==HandleResize::MID_LEFT||handle_activo==HandleResize::BOT_LEFT;
            bool der = handle_activo==HandleResize::TOP_RIGHT||handle_activo==HandleResize::MID_RIGHT||handle_activo==HandleResize::BOT_RIGHT;
            if (der) nw = handle_w_inicial + dx;
            if (izq) { nw = handle_w_inicial - dx; }
            double antes = camd->get_ancho();
            camd->set_dimensiones(nw, camd->get_alto());
            // Si se arrastró el lado izquierdo, mover x para que el borde derecho quede fijo.
            if (izq) npos.x = handle_pos_inicial_ent.x + (handle_w_inicial - camd->get_ancho());
            camd->set_posicion(npos);
            (void)antes;
        }
    } else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && entidad_arrastrada != nullptr) {
        Vector2D mouse_pos(mx, my);
        Vector2D nueva_pos = mouse_pos + offset_arrastre;
        double limite_x = static_cast<double>(ancho_area_juego()) - 30.0;
        nueva_pos.x = std::max(30.0, std::min(limite_x, nueva_pos.x));
        nueva_pos.y = std::max(30.0, std::min(double(ALTO - 30.0), nueva_pos.y));

        entidad_arrastrada->set_posicion(nueva_pos);
        entidad_arrastrada->set_velocidad(Vector2D(0.0, 0.0));
        entidad_arrastrada->set_velocidad_angular(0.0);

        PlanoInclinado* ramp_drag = dynamic_cast<PlanoInclinado*>(entidad_arrastrada);
        if (ramp_drag) {
            ramp_drag->recalcular_vertices();
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (arrastrando_spawn != TipoObjetoMenu::NINGUNO) {
            if (punto_en_area_juego(mx, my)) {
                bool spawn_exito = false;
                if (estado_actual == EstadoJuego::JUEGO_NIVEL) {
                    if (!inventario_entidades[arrastrando_spawn].empty()) {
                        auto e = std::move(inventario_entidades[arrastrando_spawn].back());
                        inventario_entidades[arrastrando_spawn].pop_back();

                        double w_or_r = 0.0, h_val = 0.0;
                        TipoForma forma = e->get_tipo_forma();
                        if (forma == TipoForma::CIRCULO) {
                            auto* b = dynamic_cast<Bola*>(e.get());
                            if (b) w_or_r = b->get_radio();
                            auto* br = dynamic_cast<BolaRebotadora*>(e.get());
                            if (br) w_or_r = br->get_radio();
                        } else if (forma == TipoForma::AABB) {
                            Vector2D size = e->get_max() - e->get_min();
                            w_or_r = size.x;
                            h_val = size.y;
                        } else if (forma == TipoForma::POLIGONO) {
                            auto* ramp = dynamic_cast<PlanoInclinado*>(e.get());
                            if (ramp) {
                                w_or_r = ramp->get_base();
                                h_val = ramp->get_altura();
                            }
                            auto* bal = dynamic_cast<Balancin*>(e.get());
                            if (bal) {
                                w_or_r = bal->get_largo();
                                h_val = bal->get_espesor();
                            }
                        }

                        Vector2D pos_destino(mx, my);
                        if (forma == TipoForma::AABB || forma == TipoForma::POLIGONO) {
                            pos_destino.x = mx - w_or_r / 2.0;
                            pos_destino.y = my - h_val / 2.0;
                        }

                        if (posicion_valida_para_spawn(motor, pos_destino, forma, w_or_r, h_val)) {
                            e->set_posicion(pos_destino);
                            e->set_velocidad(Vector2D(0, 0));
                            e->set_velocidad_angular(0);
                            
                            auto* ramp = dynamic_cast<PlanoInclinado*>(e.get());
                            if (ramp) ramp->recalcular_vertices();

                            motor.agregar_entidad(std::move(e));
                            inventario_actual[arrastrando_spawn]--;
                            spawn_exito = true;
                        } else {
                            inventario_entidades[arrastrando_spawn].push_back(std::move(e));
                            spawn_error_timer = 0.5f;
                            spawn_error_pos = Vector2D(mx, my);
                        }
                    }
                } else {
                    spawn_exito = spawn_desde_menu(motor, arrastrando_spawn, Vector2D(mx, my));
                }

                if (spawn_exito) {
                    arrastrando_spawn = TipoObjetoMenu::NINGUNO;
                }
            } else {
                arrastrando_spawn = TipoObjetoMenu::NINGUNO;
            }
        }
        if (entidad_arrastrada != nullptr) {
            if (estado_actual == EstadoJuego::JUEGO_CREATIVO && !es_borde_nivel(entidad_arrastrada)) {
                int panel_x = 12;
                int panel_y = ALTO - 390;
                int panel_w = 236; // 260 - 24
                int panel_h = 260;
                Rectangle rect_panel = { (float)panel_x, (float)panel_y, (float)panel_w, (float)panel_h };
                if (panel_izquierdo_visible && CheckCollisionPointRec(GetMousePosition(), rect_panel)) {
                    entidad_arrastrada->set_es_fijo(false);
                    TraceLog(LOG_INFO, "Entidad #%d arrastrada al inventario, es_fijo = false", entidad_arrastrada->get_id());
                }
            }
            entidad_arrastrada = nullptr;
        }
        handle_activo = HandleResize::NINGUNO;
    }

    if (!menu_visible && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Rectangle btn_abrir = { static_cast<float>(ANCHO - 26), static_cast<float>(ALTO / 2 - 40), 24, 80 };
        if (click_en_rect(mx, my, btn_abrir))
            menu_visible = true;
    }

    if (IsKeyPressed(KEY_SPACE)) {
        if (motor.get_pausado()) {
            guardar_snapshot_simulacion(motor, gestor_eventos);
            motor.aplicar_transmision_bandas();
            motor.set_pausado(false);
            cancelar_colocacion_cuerda();
        } else {
            restaurar_snapshot_simulacion(motor, gestor_eventos);
            motor.aplicar_transmision_bandas();
            motor.set_pausado(true);
        }
    }

    if (IsKeyPressed(KEY_D)) {
        modo_debug = !modo_debug;
    }

    if ((IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_X)) && entidad_seleccionada) {
        if (!motor.get_pausado()) {
            spawn_error_timer = 0.5f;
            spawn_error_pos = entidad_seleccionada->get_posicion();
        } else if (!es_borde_nivel(entidad_seleccionada)) {
            if (estado_actual == EstadoJuego::JUEGO_NIVEL && entidad_seleccionada->get_es_fijo()) {
                spawn_error_timer = 0.5f;
                spawn_error_pos = entidad_seleccionada->get_posicion();
            } else {
                int id_eliminar = entidad_seleccionada->get_id();
                if (estado_actual == EstadoJuego::JUEGO_NIVEL) {
                    TipoObjetoMenu t_menu = entidad_seleccionada->get_tipo_menu();
                    inventario_actual[t_menu]++;
                    auto ptr = motor.transferir_entidad(id_eliminar);
                    if (ptr) {
                        inventario_entidades[t_menu].push_back(std::move(ptr));
                    }
                } else {
                    motor.remover_entidad(id_eliminar);
                }
                if (entidad_arrastrada == entidad_seleccionada) entidad_arrastrada = nullptr;
                entidad_seleccionada = nullptr;
            }
        }
    }

    if (IsKeyPressed(KEY_F) && entidad_seleccionada) {
        if (!motor.get_pausado()) {
            spawn_error_timer = 0.5f;
            spawn_error_pos = entidad_seleccionada->get_posicion();
        } else if (estado_actual == EstadoJuego::JUEGO_NIVEL && entidad_seleccionada->get_es_fijo()) {
            spawn_error_timer = 0.5f;
            spawn_error_pos = entidad_seleccionada->get_posicion();
        } else {
            auto* vent = dynamic_cast<Ventilador*>(entidad_seleccionada);
            if (vent) {
                vent->invertir_direccion();
            } else {
                auto* ramp = dynamic_cast<PlanoInclinado*>(entidad_seleccionada);
                if (ramp) {
                    ramp->invertir();
                } else {
                    auto* bal = dynamic_cast<Balancin*>(entidad_seleccionada);
                    if (bal) bal->ciclar_inclinacion();
                    else {
                        auto* pist = dynamic_cast<Pistola*>(entidad_seleccionada);
                        if (pist) pist->invertir();
                        else {
                            // La lupa NO rota (su haz depende del foco). El cañón sí.
                            auto* can = dynamic_cast<Canon*>(entidad_seleccionada);
                            if (can) can->invertir();
                        }
                    }
                }
            }
        }
    }

    if (IsKeyPressed(KEY_T) && entidad_seleccionada) {
        if (!motor.get_pausado()) {
            spawn_error_timer = 0.5f;
            spawn_error_pos = entidad_seleccionada->get_posicion();
        } else if (estado_actual == EstadoJuego::JUEGO_NIVEL && entidad_seleccionada->get_es_fijo()) {
            spawn_error_timer = 0.5f;
            spawn_error_pos = entidad_seleccionada->get_posicion();
        } else {
            auto* ramp = dynamic_cast<PlanoInclinado*>(entidad_seleccionada);
            if (ramp) {
                ramp->alternar_tamano();
            }
        }
    }

    if ((IsKeyPressed(KEY_P) || IsKeyPressed(KEY_G)) && entidad_seleccionada && estado_actual == EstadoJuego::JUEGO_CREATIVO) {
        if (!motor.get_pausado()) {
            spawn_error_timer = 0.5f;
            spawn_error_pos = entidad_seleccionada->get_posicion();
        } else if (!es_borde_nivel(entidad_seleccionada)) {
            entidad_seleccionada->set_es_fijo(!entidad_seleccionada->get_es_fijo());
            TraceLog(LOG_INFO, "Entidad #%d es_fijo toggled to %d", entidad_seleccionada->get_id(), entidad_seleccionada->get_es_fijo());
        }
    }

    if (estado_actual == EstadoJuego::JUEGO_CREATIVO && entidad_seleccionada && motor.get_pausado()) {
        ParedRectangular* p = dynamic_cast<ParedRectangular*>(entidad_seleccionada);
        if (p && !es_borde_nivel(p)) {
            double paso = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) ? 1.0 : 10.0;
            double nw = p->get_ancho(), nh = p->get_alto();
            bool cambiado = false;
            if (IsKeyPressed(KEY_RIGHT)) { nw = std::max(20.0, nw + paso); cambiado = true; }
            if (IsKeyPressed(KEY_LEFT))  { nw = std::max(20.0, nw - paso); cambiado = true; }
            if (IsKeyPressed(KEY_DOWN))  { nh = std::max(8.0,  nh + paso); cambiado = true; }
            if (IsKeyPressed(KEY_UP))    { nh = std::max(8.0,  nh - paso); cambiado = true; }
            if (cambiado) {
                p->set_dimensiones(nw, nh);
            }
        } else {
            PlanoInclinado* ramp = dynamic_cast<PlanoInclinado*>(entidad_seleccionada);
            if (ramp) {
                double paso = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) ? 1.0 : 10.0;
                double nb = ramp->get_base(), nh = ramp->get_altura();
                bool cambiado = false;
                if (IsKeyPressed(KEY_RIGHT)) { nb = std::max(20.0, nb + paso); cambiado = true; }
                if (IsKeyPressed(KEY_LEFT))  { nb = std::max(20.0, nb - paso); cambiado = true; }
                if (IsKeyPressed(KEY_DOWN))  { nh = std::max(8.0,  nh + paso); cambiado = true; }
                if (IsKeyPressed(KEY_UP))    { nh = std::max(8.0,  nh - paso); cambiado = true; }
                if (cambiado) {
                    ramp->set_dimensiones(nb, nh);
                }
            } else if (Caminadora* cam = dynamic_cast<Caminadora*>(entidad_seleccionada)) {
                // Caminadora: RIGHT/LEFT expanden el ANCHO (con límite interno);
                // UP/DOWN ajustan la VELOCIDAD de la cinta.
                double paso = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) ? 1.0 : 10.0;
                if (IsKeyPressed(KEY_RIGHT)) cam->set_dimensiones(cam->get_ancho() + paso, cam->get_alto());
                if (IsKeyPressed(KEY_LEFT))  cam->set_dimensiones(cam->get_ancho() - paso, cam->get_alto());
                if (IsKeyPressed(KEY_UP))    cam->ajustar_velocidad(20.0);
                if (IsKeyPressed(KEY_DOWN))  cam->ajustar_velocidad(-20.0);
            }
        }
    }

    if (IsKeyPressed(KEY_R)) {
        if (estado_actual == EstadoJuego::JUEGO_NIVEL) {
            tiempo_nivel = 0.0;
            if (nivel_campana_actual != -1) {
                cargar_nivel_campana(motor, nivel_campana_actual);
            } else if (!nivel_usuario_actual_path.empty()) {
                int ancho_cargado = ANCHO;
                int alto_cargado = ALTO;
                cargar_partida(motor, gestor_eventos, nivel_usuario_actual_path, ancho_cargado, alto_cargado, contador_bolas);
                ANCHO = ancho_cargado;
                ALTO = alto_cargado;
            } else {
                motor.limpiar();
                contador_bolas = 0;
                resetear_punteros_borde();
                limpiar_estado_tras_cargar_partida();
                crear_escena(motor);
                gestor_eventos.limpiar();
            }
        } else {
            motor.limpiar();
            contador_bolas = 0;
            resetear_punteros_borde();
            limpiar_estado_tras_cargar_partida();
            crear_escena(motor);
            gestor_eventos.limpiar();
        }
        modo_evento_ui = ModoEventoUI::INACTIVO;
        motor.set_pausado(true);
    }

    if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) {
        Vector2D g = motor.get_gravedad();
        motor.set_gravedad(Vector2D(g.x, g.y + 100.0));
    }
    if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) {
        Vector2D g = motor.get_gravedad();
        motor.set_gravedad(Vector2D(g.x, g.y - 100.0));
    }

    motor.actualizar(GetFrameTime());
    gestor_eventos.evaluar(motor.get_colisiones_frame(), motor.get_eventos_especiales_frame(), motor.get_entidades(), GetFrameTime());

    if (anim_seguidor_corriendo) {
        anim_seguidor_corriendo->actualizar(GetFrameTime());
    }

    if (titulo_alpha > 0.0f) {
        titulo_alpha -= 0.008f;
        if (titulo_alpha < 0.0f) titulo_alpha = 0.0f;
    }

    if (spawn_error_timer > 0.0f) {
        spawn_error_timer -= GetFrameTime();
    }

    actualizar_mensaje_guardado(GetFrameTime());
}

void dibujar_juego_core(MotorFisica& motor, bool es_modo_nivel) {
    if (tex_fondo.id > 0) {
        float escala_x = static_cast<float>(ANCHO) / tex_fondo.width;
        float escala_y = static_cast<float>(ALTO) / tex_fondo.height;
        float escala = (escala_x > escala_y) ? escala_x : escala_y;
        DrawTextureEx(tex_fondo, {0, 0}, 0, escala, WHITE);
    }

    dibujar_cuerdas(motor);
    motor.dibujar_bandas(modo_debug);

    for (const auto* e : motor.get_entidades()) {
        dibujar_entidad(e);
        dibujar_debug(e);
        
        if (estado_actual == EstadoJuego::JUEGO_CREATIVO && !e->get_es_fijo()) {
            Color inv_color = Color{76, 219, 138, 130};
            TipoForma forma = e->get_tipo_forma();
            if (forma == TipoForma::CIRCULO) {
                Vector2D pos;
                double radio = 0.0;
                if (obtener_datos_circulo(e, pos, radio)) {
                    DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y),
                                    static_cast<float>(radio + 3.0), inv_color);
                    DrawText("I", static_cast<int>(pos.x - 3), static_cast<int>(pos.y - 5), 10, inv_color);
                }
            } else if (forma == TipoForma::AABB) {
                const ParedRectangular* pr = dynamic_cast<const ParedRectangular*>(e);
                if (pr) {
                    Vector2D pos = pr->get_posicion();
                    DrawRectangleLinesEx({(float)pos.x - 2, (float)pos.y - 2,
                        (float)pr->get_ancho() + 4, (float)pr->get_alto() + 4}, 1.0f, inv_color);
                    DrawText("I", static_cast<int>(pos.x + 2), static_cast<int>(pos.y + 2), 10, inv_color);
                }
            } else if (forma == TipoForma::POLIGONO) {
                const PlanoInclinado* ramp = dynamic_cast<const PlanoInclinado*>(e);
                if (ramp) {
                    Vector2D pos = ramp->get_posicion();
                    DrawRectangleLinesEx({(float)pos.x - 2, (float)pos.y - 2,
                        (float)ramp->get_base() + 4, (float)ramp->get_altura() + 4}, 1.0f, inv_color);
                    DrawText("I", static_cast<int>(pos.x + 2), static_cast<int>(pos.y + 2), 10, inv_color);
                }
                const Balancin* bal = dynamic_cast<const Balancin*>(e);
                if (bal) {
                    Vector2D pos = bal->get_posicion();
                    DrawRectangleLinesEx({(float)pos.x - 2, (float)pos.y - 2,
                        (float)bal->get_largo() + 4, (float)bal->get_espesor() + 4}, 1.0f, inv_color);
                    DrawText("I", static_cast<int>(pos.x + 2), static_cast<int>(pos.y + 2), 10, inv_color);
                }
            }
        }
    }
    dibujar_previsualizacion_cuerda(motor);
    dibujar_previsualizacion_banda(motor);

    if (entidad_seleccionada) {
        Color sel_color = {255, 200, 50, 180};
        const SoporteTorque* soporte_sel = dynamic_cast<const SoporteTorque*>(entidad_seleccionada);
        if (soporte_sel) {
            Vector2D sp = soporte_sel->get_punto_cuerda();
            DrawCircleLines(static_cast<int>(sp.x), static_cast<int>(sp.y),
                            static_cast<float>(soporte_sel->get_radio() + 5.0), sel_color);
            DrawCircleLines(static_cast<int>(sp.x), static_cast<int>(sp.y),
                            static_cast<float>(soporte_sel->get_radio() + 8.0),
                            Color{255, 200, 50, 90});
        }
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

                // Draw resize handles if in JUEGO_CREATIVO and paused
                if (estado_actual == EstadoJuego::JUEGO_CREATIVO && motor.get_pausado() && !es_borde_nivel(sp)) {
                    auto handles = calcular_handles(sp);
                    for (int i = 0; i < 8; ++i) {
                        bool activo = (handle_activo == handles.tipos[i]);
                        Color col = activo ? Color{255, 200, 50, 255} : Color{80, 160, 255, 200};
                        DrawRectangleRec(handles.rects[i], col);
                        DrawRectangleLinesEx(handles.rects[i], 1.5f, {20, 80, 200, 255});
                    }
                }
            } else if (const Caminadora* scam = dynamic_cast<const Caminadora*>(entidad_seleccionada)) {
                // Caminadora: contorno + handles (redimensionable con el mouse, solo ancho).
                Vector2D spos = scam->get_posicion();
                float sw = (float)scam->get_ancho(), sh = (float)scam->get_alto();
                DrawRectangleLinesEx({(float)spos.x - 3, (float)spos.y - 3, sw + 6, sh + 6}, 2.0f, sel_color);
                if (estado_actual == EstadoJuego::JUEGO_CREATIVO && motor.get_pausado()) {
                    auto handles = calcular_handles_xywh((float)spos.x, (float)spos.y, sw, sh);
                    for (int i = 0; i < 8; ++i) {
                        // Solo los handles laterales (izq/der) son útiles (solo ancho).
                        bool lateral = handles.tipos[i]==HandleResize::MID_LEFT || handles.tipos[i]==HandleResize::MID_RIGHT;
                        if (!lateral) continue;
                        bool activo = (handle_activo == handles.tipos[i]);
                        Color col = activo ? Color{255, 200, 50, 255} : Color{80, 160, 255, 200};
                        DrawRectangleRec(handles.rects[i], col);
                        DrawRectangleLinesEx(handles.rects[i], 1.5f, {20, 80, 200, 255});
                    }
                }
            } else {
                Vector2D spos = entidad_seleccionada->get_posicion();
                const Trampolin* st = dynamic_cast<const Trampolin*>(entidad_seleccionada);
                const BarrilChavo* sb = dynamic_cast<const BarrilChavo*>(entidad_seleccionada);
                const Ventilador* sv = dynamic_cast<const Ventilador*>(entidad_seleccionada);
                const SeguidorBooster* ss = dynamic_cast<const SeguidorBooster*>(entidad_seleccionada);
                const Cubeta* sc = dynamic_cast<const Cubeta*>(entidad_seleccionada);
                float sw = 0, sh = 0;
                if (st) { sw = st->get_ancho(); sh = st->get_alto(); }
                else if (sb) { sw = sb->get_ancho(); sh = sb->get_alto(); }
                else if (sv) { sw = sv->get_ancho(); sh = sv->get_alto(); }
                else if (ss) { sw = ss->get_ancho(); sh = ss->get_alto(); spos.x -= sw/2; spos.y -= sh/2; }
                else if (sc) { sw = sc->get_ancho(); sh = sc->get_alto(); }
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

        const Ventilador* sv_rot = dynamic_cast<const Ventilador*>(entidad_seleccionada);
        const PlanoInclinado* sr_rot = dynamic_cast<const PlanoInclinado*>(entidad_seleccionada);
        if (sv_rot || sr_rot) {
            Vector2 badge_pos;
            if (sv_rot) {
                Vector2D spos = sv_rot->get_posicion();
                badge_pos = { (float)spos.x + (float)sv_rot->get_ancho() + 10.0f, (float)spos.y - 10.0f };
            } else {
                Vector2D spos = sr_rot->get_posicion();
                badge_pos = { (float)spos.x + (float)sr_rot->get_base() + 10.0f, (float)spos.y - 10.0f };
            }

            DrawCircleV(badge_pos, 14.0f, Color{255, 200, 50, 240});
            DrawCircleLines(badge_pos.x, badge_pos.y, 14.0f, WHITE);

            float radio_arco = 7.0f;
            for (int d = 0; d < 270; d += 15) {
                float rad1 = d * DEG2RAD;
                float rad2 = (d + 15) * DEG2RAD;
                DrawLineEx(
                    { badge_pos.x + std::cos(rad1) * radio_arco, badge_pos.y + std::sin(rad1) * radio_arco },
                    { badge_pos.x + std::cos(rad2) * radio_arco, badge_pos.y + std::sin(rad2) * radio_arco },
                    2.0f, Color{40, 40, 40, 255}
                );
            }
            float rad_punta = 270.0f * DEG2RAD;
            Vector2 p_ext = { badge_pos.x + std::cos(rad_punta) * radio_arco, badge_pos.y + std::sin(rad_punta) * radio_arco };
            DrawTriangle(
                { p_ext.x - 3, p_ext.y - 1 },
                { p_ext.x + 3, p_ext.y - 4 },
                { p_ext.x + 1, p_ext.y + 4 },
                Color{40, 40, 40, 255}
            );

            // Si es una rampa, dibujar también el badge de cambiar tamaño [T]
            if (sr_rot) {
                Vector2 size_badge_pos = { badge_pos.x + 32.0f, badge_pos.y };
                DrawCircleV(size_badge_pos, 14.0f, Color{50, 180, 255, 240});
                DrawCircleLines(size_badge_pos.x, size_badge_pos.y, 14.0f, WHITE);
                DrawTextEx(fuente_menu, "T", { size_badge_pos.x - 5.0f, size_badge_pos.y - 8.0f }, 16.0f, 1.0f, Color{40, 40, 40, 255});
            }
        }
    }

    dibujar_menu_lateral();

    if (tex_base_central.id > 0) {
        float escala_panel_x = 1900.0f / tex_base_central.width;
        float escala_panel_y = 130.0f / tex_base_central.height;
        DrawTextureEx(tex_base_central, {20, static_cast<float>(ALTO - 150)}, 0, 
                     (escala_panel_x > escala_panel_y) ? escala_panel_x : escala_panel_y, WHITE);
    }

    if (spawn_error_timer > 0.0f) {
        unsigned char alpha = static_cast<unsigned char>(spawn_error_timer * 2.0f * 255);
        Color err_col = {255, 50, 50, alpha};
        int ex = static_cast<int>(spawn_error_pos.x);
        int ey = static_cast<int>(spawn_error_pos.y);
        DrawLine(ex - 10, ey - 10, ex + 10, ey + 10, err_col);
        DrawLine(ex + 10, ey - 10, ex - 10, ey + 10, err_col);
        DrawCircleLines(ex, ey, 14, err_col);
    }

    if (menu_visible) {
        int sep = ancho_area_juego();
        DrawLine(sep, 0, sep, ALTO, Color{60, 65, 90, 180});
    }

    dibujar_ghost_spawn();

    if (gestor_eventos.victoria_alcanzada) {
        DrawRectangle(0, 0, ANCHO, ALTO, ColorAlpha(BLACK, 0.7f));
        DrawText("VICTORIA!", ANCHO/2 - 100, ALTO/2 - 60, 40, GREEN);
        
        if (es_modo_nivel) {
            int min = (int)(tiempo_nivel / 60);
            int seg = (int)(tiempo_nivel) % 60;
            int ms = (int)(tiempo_nivel * 100) % 100;
            const char* final_time_txt = TextFormat("Tiempo de completado: %02d:%02d.%02d", min, seg, ms);
            int tw = MeasureText(final_time_txt, 24);
            DrawText(final_time_txt, ANCHO/2 - tw/2, ALTO/2, 24, GOLD);
            DrawText("Presiona R para reiniciar el nivel o ESC para salir", ANCHO/2 - 240, ALTO/2 + 50, 20, WHITE);
        } else {
            DrawText("Presiona R para reiniciar el nivel", ANCHO/2 - 160, ALTO/2 + 20, 20, WHITE);
        }
    }

    if (titulo_alpha > 0.01f) {
        unsigned char a = static_cast<unsigned char>(titulo_alpha * 255);
        int tw = MeasureText("TIM: Motor de Fisica", 40);
        DrawText("TIM: Motor de Fisica", ANCHO / 2 - tw / 2, ALTO / 2 - 50, 40,
                 Color{255, 255, 255, a});
        int sw = MeasureText("Prototipo v0.1 | RK4 + Raylib", 20);
        DrawText("Prototipo v0.1 | RK4 + Raylib", ANCHO / 2 - sw / 2, ALTO / 2, 20,
                 Color{180, 180, 200, a});
    }

    // Dibujar banner indicador de Modo
    DrawRectangle(280, 30, 240, 35, ColorAlpha(Color{15, 15, 30, 255}, 0.85f));
    DrawRectangleLines(280, 30, 240, 35, es_modo_nivel ? SKYBLUE : GOLD);
    const char* txt_modo = es_modo_nivel ? "MODO JUEGO: NIVEL" : "MODO CREATIVO (SANDBOX)";
    Vector2 ts = MeasureTextEx(fuente_menu, txt_modo, 16, 1);
    DrawTextEx(fuente_menu, txt_modo, {280 + (240 - ts.x)/2.0f, 30 + (35 - ts.y)/2.0f}, 16, 1, es_modo_nivel ? SKYBLUE : GOLD);

    // Dibujar banner del Cronómetro si estamos en Modo Nivel
    if (es_modo_nivel) {
        int min = (int)(tiempo_nivel / 60);
        int seg = (int)(tiempo_nivel) % 60;
        int ms = (int)(tiempo_nivel * 100) % 100;
        
        const char* txt_tiempo = TextFormat("%02d:%02d.%02d", min, seg, ms);
        Vector2 ts_time = MeasureTextEx(fuente_menu, txt_tiempo, 16, 1);
        DrawTextEx(fuente_menu, txt_tiempo, {715 + (180 - ts_time.x)/2.0f, 970 + (35 - ts_time.y)/2.0f}, 54, 9, WHITE);
    }

    BotonesHUD btns = calcular_rectangulos_botones_hud();
    Vector2 mouse = GetMousePosition();
    
    bool mouse_en_paneles = punto_en_panel_izquierdo((int)mouse.x, (int)mouse.y) || punto_en_menu((int)mouse.x, (int)mouse.y);
    
    // Play button
    bool hover_play = !mouse_en_paneles && CheckCollisionPointRec(mouse, btns.rect_play);
    bool play_activo = motor.get_pausado();
    Color play_tint = play_activo ? WHITE : Color{120, 120, 120, 255};
    if (tex_hud_play.id > 0) {
        DrawTexturePro(tex_hud_play, {0.0f, 0.0f, (float)tex_hud_play.width, (float)tex_hud_play.height}, btns.rect_play, {0.0f, 0.0f}, 0.0f, play_tint);
    }
    if (hover_play && play_activo) {
        DrawRectangleRoundedLinesEx(btns.rect_play, 0.2f, 4, 2.0f, SKYBLUE);
    }

    // Reset button
    bool hover_reset = !mouse_en_paneles && CheckCollisionPointRec(mouse, btns.rect_reset);
    if (tex_hud_reset.id > 0) {
        Texture2D tex = hover_reset && (tex_hud_reset_hover.id > 0) ? tex_hud_reset_hover : tex_hud_reset;
        DrawTexturePro(tex, {0.0f, 0.0f, (float)tex.width, (float)tex.height}, btns.rect_reset, {0.0f, 0.0f}, 0.0f, WHITE);
    }
    if (hover_reset) {
        DrawRectangleRoundedLinesEx(btns.rect_reset, 0.2f, 4, 2.0f, SKYBLUE);
    }

    // Opciones (Engrane)
    bool hover_opciones = !mouse_en_paneles && CheckCollisionPointRec(mouse, btns.rect_opciones);

    if (tex_hud_opciones.id > 0) {
        Texture2D tex = hover_opciones && (tex_hud_opciones_hover.id > 0) ? tex_hud_opciones_hover : tex_hud_opciones;
        DrawTexturePro(tex, {0.0f, 0.0f, (float)tex.width, (float)tex.height}, btns.rect_opciones, {0.0f, 0.0f}, 0.0f, WHITE);
    }
    if (hover_opciones) {
        DrawRectangleRoundedLinesEx(btns.rect_opciones, 0.2f, 4, 2.0f, SKYBLUE);
    }
    
    // Ayuda (?)
    bool hover_ayuda = !mouse_en_paneles && CheckCollisionPointRec(mouse, btns.rect_ayuda);
    if (tex_hud_ayuda.id > 0) {
        DrawTexturePro(tex_hud_ayuda, {0.0f, 0.0f, (float)tex_hud_ayuda.width, (float)tex_hud_ayuda.height}, btns.rect_ayuda, {0.0f, 0.0f}, 0.0f, WHITE);
    }
    if (hover_ayuda) {
        DrawRectangleRoundedLinesEx(btns.rect_ayuda, 0.2f, 4, 2.0f, SKYBLUE);
    }
    
    // Salir (Menú)
    bool hover_salir = !mouse_en_paneles && CheckCollisionPointRec(mouse, btns.rect_salir);
    if (tex_hud_salir.id > 0) {
        DrawTexturePro(tex_hud_salir, {0.0f, 0.0f, (float)tex_hud_salir.width, (float)tex_hud_salir.height}, btns.rect_salir, {0.0f, 0.0f}, 0.0f, WHITE);
    }
    if (hover_salir) {
        DrawRectangleRoundedLinesEx(btns.rect_salir, 0.2f, 4, 2.0f, SKYBLUE);
    }

    // Dibujar paneles laterales (sobre el HUD)
    if (!es_modo_nivel) {
        dibujar_panel_eventos_izquierdo(motor, gestor_eventos, ALTO);
    }

    // Dibujar Overlay de Ayuda
    if (mostrar_ayuda_overlay) {
        DrawRectangle(0, 0, ANCHO, ALTO, ColorAlpha(BLACK, 0.75f));
        
        float w = 600.0f;
        float h = 400.0f;
        Rectangle box = {(ANCHO - w)/2.0f, (ALTO - h)/2.0f, w, h};
        DrawRectangleRounded(box, 0.05f, 6, Color{40, 42, 68, 255});
        DrawRectangleRoundedLinesEx(box, 0.05f, 6, 2.0f, SKYBLUE);
        
        Rectangle close_btn = {box.x + box.width - 40, box.y + 10, 30, 30};
        bool hover_close = CheckCollisionPointRec(mouse, close_btn);
        DrawRectangleRec(close_btn, hover_close ? RED : ColorAlpha(RED, 0.7f));
        DrawText("X", close_btn.x + 10, close_btn.y + 6, 16, WHITE);
        
        Vector2 t_size = MeasureTextEx(fuente_menu, "GUIA DE AYUDA RAPIDA", 28, 2);
        DrawTextEx(fuente_menu, "GUIA DE AYUDA RAPIDA", {box.x + (box.width - t_size.x)/2.0f, box.y + 30.0f}, 28, 2, GOLD);
        
        float start_y = box.y + 90.0f;
        float dy = 32.0f;
        
        auto draw_help_line = [](const char* key, const char* desc, float y, float x_offset) {
            DrawTextEx(fuente_menu, key, {x_offset, y}, 18, 1, GOLD);
            DrawTextEx(fuente_menu, desc, {x_offset + 120.0f, y}, 18, 1, LIGHTGRAY);
        };
        
        draw_help_line("Objetivo:", "Coloca objetos para que la pelota llegue a la zona meta.", start_y, box.x + 30.0f);
        draw_help_line("Arrastrar:", "Arrastra objetos del menu lateral al canvas.", start_y + dy, box.x + 30.0f);
        draw_help_line("Mover:", "Haz click izquierdo en un objeto y arrastralo.", start_y + dy * 2, box.x + 30.0f);
        draw_help_line("Rotar / Tamano:", "F: rotar/invertir. T: alternar tamano de rampas.", start_y + dy * 3, box.x + 30.0f);
        draw_help_line("Borrar (SUPR/X):", "Selecciona un objeto y presiona X o SUPR para eliminarlo.", start_y + dy * 4, box.x + 30.0f);
        draw_help_line("Simulacion:", "Presiona ESPACIO para correr o pausar la fisica.", start_y + dy * 5, box.x + 30.0f);
        draw_help_line("Menu Lateral:", "Presiona TAB para ocultar/mostrar el catalogo de objetos.", start_y + dy * 6, box.x + 30.0f);
        draw_help_line("Reiniciar (R):", "Presiona R para limpiar el nivel y reiniciar.", start_y + dy * 7, box.x + 30.0f);
        
        DrawTextEx(fuente_menu, "Haz click en 'X' o fuera de este panel para cerrar.", {box.x + 30.0f, box.y + h - 40.0f}, 14, 1, GRAY);
    }

    // Dibujar Overlay de Pausa
    if (mostrar_pausa_overlay) {
        DrawRectangle(0, 0, ANCHO, ALTO, ColorAlpha(BLACK, 0.75f));
        
        float w = 340.0f;
        float h = 260.0f;
        Rectangle box = {(ANCHO - w)/2.0f, (ALTO - h)/2.0f, w, h};
        DrawRectangleRounded(box, 0.08f, 6, Color{40, 42, 68, 255});
        DrawRectangleRoundedLinesEx(box, 0.08f, 6, 2.0f, SKYBLUE);
        
        Vector2 t_size = MeasureTextEx(fuente_menu, "PAUSA", 28, 2);
        DrawTextEx(fuente_menu, "PAUSA", {box.x + (box.width - t_size.x)/2.0f, box.y + 20.0f}, 28, 2, GOLD);
        
        float btn_w = 260.0f;
        float btn_h = 42.0f;
        float start_y = box.y + 70.0f;
        float spacing_y = 55.0f;
        
        Rectangle rect_continuar = {box.x + (w - btn_w)/2.0f, start_y, btn_w, btn_h};
        Rectangle rect_opciones  = {box.x + (w - btn_w)/2.0f, start_y + spacing_y, btn_w, btn_h};
        Rectangle rect_salir     = {box.x + (w - btn_w)/2.0f, start_y + spacing_y * 2.0f, btn_w, btn_h};
        
        if (dibujar_boton_interactivo(rect_continuar, "CONTINUAR", {45, 50, 80, 255}, {70, 80, 130, 255}, fuente_menu, 20.0f)) {
            mostrar_pausa_overlay = false;
        }
        if (dibujar_boton_interactivo(rect_opciones, "OPCIONES", {45, 50, 80, 255}, {70, 80, 130, 255}, fuente_menu, 20.0f)) {
            mostrar_pausa_overlay = false;
            estado_previo = estado_actual;
            estado_actual = EstadoJuego::MENU_OPCIONES;
        }
        if (dibujar_boton_interactivo(rect_salir, "SALIR AL MENU", {180, 50, 50, 255}, {220, 70, 70, 255}, fuente_menu, 20.0f)) {
            mostrar_pausa_overlay = false;
            estado_actual = es_modo_nivel ? EstadoJuego::SELECCION_NIVELES : EstadoJuego::MENU_PRINCIPAL;
        }
    }
}

// Dibuja un boton interactivo en la esquina superior derecha para silenciar el sonido.
void dibujar_y_actualizar_boton_silencio() {
    float size = 45.0f;
    Rectangle rect_mute = { (float)ANCHO - size - 20.0f, 20.0f, size, size };
    Vector2 mouse = GetMousePosition();
    bool hover = CheckCollisionPointRec(mouse, rect_mute);
    
    if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        sonido_mutado = !sonido_mutado;
        if (IsMusicReady(musica_menu)) {
            SetMusicVolume(musica_menu, sonido_mutado ? 0.0f : 0.6f);
        }
    }
    
    Color color_base = sonido_mutado ? Color{180, 50, 50, 255} : Color{50, 180, 50, 255};
    Color color_hover = sonido_mutado ? Color{220, 70, 70, 255} : Color{70, 220, 70, 255};
    Color color_dibujo = hover ? color_hover : color_base;
    
    // Dibujar el cuadro de referencia estilo glassmorphism
    DrawRectangleRounded(rect_mute, 0.2f, 4, ColorAlpha(color_dibujo, 0.4f));
    DrawRectangleRoundedLinesEx(rect_mute, 0.2f, 4, 2.0f, color_dibujo);
    
    // Dibujar indicador textual "MUTE" o "SOUND"
    const char* txt = sonido_mutado ? "MUTED" : "SOUND";
    float text_size = 11.0f;
    Vector2 t_size = MeasureTextEx(fuente_menu, txt, text_size, 1);
    DrawTextEx(fuente_menu, txt, { rect_mute.x + (rect_mute.width - t_size.x)/2.0f, rect_mute.y + (rect_mute.height - t_size.y)/2.0f }, text_size, 1, WHITE);
}

// ============================================================================
// Punto de entrada
// ============================================================================
int main() {
    // ---- Inicializar ventana ----
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI);
    //SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(ANCHO, ALTO, "TIM - Motor de Fisica | Prototipo RK4 + Raylib");
    InitAudioDevice(); // Inicializar dispositivo de audio de Raylib
    ANCHO = GetScreenWidth();
    ALTO = GetScreenHeight();
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
    while (!WindowShouldClose() && !salir_juego) {
        sincronizar_tamano_ventana();
        
        // Actualizar el stream de musica
        if (IsMusicReady(musica_menu)) {
            UpdateMusicStream(musica_menu);
        }

        // Manejar importación de niveles mediante drag & drop
        if (IsFileDropped()) {
            FilePathList archivos_soltados = LoadDroppedFiles();
            if (archivos_soltados.count > 0) {
                std::string ruta_origen = archivos_soltados.paths[0];
                if (ruta_origen.length() > 4 && ruta_origen.substr(ruta_origen.length() - 4) == ".tim") {
                    std::string nombre_archivo = std::filesystem::path(ruta_origen).filename().string();
                    std::string ruta_destino = carpeta_partidas() + "/" + nombre_archivo;
                    
                    std::error_code ec;
                    std::filesystem::copy_file(ruta_origen, ruta_destino, std::filesystem::copy_options::overwrite_existing, ec);
                    
                    refrescar_lista_partidas();
                    pestana_niveles_actual = TabNiveles::MIS_NIVELES;
                    estado_actual = EstadoJuego::SELECCION_NIVELES;
                }
            }
            UnloadDroppedFiles(archivos_soltados);
        }

        // ======== UPDATE ========
        switch (estado_actual) {
            case EstadoJuego::MENU_PRINCIPAL:
                actualizar_menu_principal();
                break;
            case EstadoJuego::MENU_OPCIONES:
                actualizar_menu_opciones();
                break;
            case EstadoJuego::SELECCION_NIVELES:
                actualizar_seleccion_niveles(motor);
                break;
            case EstadoJuego::JUEGO_CREATIVO:
                actualizar_juego_core(motor, false);
                break;
            case EstadoJuego::JUEGO_NIVEL:
                actualizar_juego_core(motor, true);
                break;
        }

        // ======== RENDER ========
        BeginDrawing();
        ClearBackground(COLOR_FONDO);

        switch (estado_actual) {
            case EstadoJuego::MENU_PRINCIPAL:
                dibujar_menu_principal(motor);
                break;
            case EstadoJuego::MENU_OPCIONES:
                dibujar_menu_opciones();
                break;
            case EstadoJuego::SELECCION_NIVELES:
                dibujar_seleccion_niveles(motor);
                break;
            case EstadoJuego::JUEGO_CREATIVO:
                dibujar_juego_core(motor, false);
                break;
            case EstadoJuego::JUEGO_NIVEL:
                dibujar_juego_core(motor, true);
                break;
        }

        // Dibujar boton de silencio en la capa superior
        dibujar_y_actualizar_boton_silencio();

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
    if (tex_celda_menu.id > 0) UnloadTexture(tex_celda_menu);
    if (tex_barra_encabezado.id > 0) UnloadTexture(tex_barra_encabezado);
    if (tex_seguidor_cabezazo.id > 0) UnloadTexture(tex_seguidor_cabezazo);
    if (anim_seguidor_corriendo) {
        delete anim_seguidor_corriendo;
    }
    if (anim_menu_inicio) {
        delete anim_menu_inicio;
        anim_menu_inicio = nullptr;
    }
    if (tex_menu_inicio_anim.id > 0) {
        UnloadTexture(tex_menu_inicio_anim);
    }
    if (anim_menu_fede) {
        delete anim_menu_fede;
        anim_menu_fede = nullptr;
    }
    if (tex_menu_fede_anim.id > 0) {
        UnloadTexture(tex_menu_fede_anim);
    }
    if (anim_menu_moto) {
        delete anim_menu_moto;
        anim_menu_moto = nullptr;
    }
    if (tex_menu_moto_anim.id > 0) {
        UnloadTexture(tex_menu_moto_anim);
    }
    if (anim_menu_jose) {
        delete anim_menu_jose;
        anim_menu_jose = nullptr;
    }
    if (tex_menu_jose_anim.id > 0) {
        UnloadTexture(tex_menu_jose_anim);
    }
    if (anim_menu_gusano) {
        delete anim_menu_gusano;
        anim_menu_gusano = nullptr;
    }
    if (tex_menu_gusano_anim.id > 0) {
        UnloadTexture(tex_menu_gusano_anim);
    }
    if (anim_menu_drom) {
        delete anim_menu_drom;
        anim_menu_drom = nullptr;
    }
    if (tex_menu_drom_anim.id > 0) {
        UnloadTexture(tex_menu_drom_anim);
    }


    if (tex_trampolin.id > 0) UnloadTexture(tex_trampolin);
    if (tex_balancin_base.id > 0) UnloadTexture(tex_balancin_base);
    if (tex_balancin_tabla.id > 0) UnloadTexture(tex_balancin_tabla);
    if (tex_plata_larga.id > 0) UnloadTexture(tex_plata_larga);
    if (tex_plata_peque.id > 0) UnloadTexture(tex_plata_peque);
    if (tex_plata_rampa_izq.id > 0) UnloadTexture(tex_plata_rampa_izq);
    if (tex_plata_rampa_der.id > 0) UnloadTexture(tex_plata_rampa_der);
    if (tex_robote_soporte.id > 0) UnloadTexture(tex_robote_soporte);
    if (tex_robote_pelota.id > 0) UnloadTexture(tex_robote_pelota);
    if (tex_ventilador_cuerpo.id > 0) UnloadTexture(tex_ventilador_cuerpo);
    if (tex_ventilador_aspa.id > 0) UnloadTexture(tex_ventilador_aspa);
    if (tex_caminadora.id > 0) UnloadTexture(tex_caminadora);
    
    if (tex_hud_opciones.id > 0) UnloadTexture(tex_hud_opciones);
    if (tex_hud_opciones_hover.id > 0) UnloadTexture(tex_hud_opciones_hover);
    if (tex_hud_ayuda.id > 0) UnloadTexture(tex_hud_ayuda);
    if (tex_hud_salir.id > 0) UnloadTexture(tex_hud_salir);
    if (tex_hud_play.id > 0) UnloadTexture(tex_hud_play);
    if (tex_hud_reset.id > 0) UnloadTexture(tex_hud_reset);
    if (tex_hud_reset_hover.id > 0) UnloadTexture(tex_hud_reset_hover);
    if (tex_menu_fondo.id > 0) UnloadTexture(tex_menu_fondo);
    if (tex_menu_box.id > 0) UnloadTexture(tex_menu_box);
    
    if (tex_btn_jugar1.id > 0) UnloadTexture(tex_btn_jugar1);
    if (tex_btn_jugar2.id > 0) UnloadTexture(tex_btn_jugar2);
    if (tex_btn_creativo1.id > 0) UnloadTexture(tex_btn_creativo1);
    if (tex_btn_creativo2.id > 0) UnloadTexture(tex_btn_creativo2);
    if (tex_btn_opciones1.id > 0) UnloadTexture(tex_btn_opciones1);
    if (tex_btn_opciones2.id > 0) UnloadTexture(tex_btn_opciones2);
    if (tex_btn_salir1.id > 0) UnloadTexture(tex_btn_salir1);
    if (tex_btn_salir2.id > 0) UnloadTexture(tex_btn_salir2);
    if (tex_menu_titulo.id > 0) UnloadTexture(tex_menu_titulo);
    
    // Descargar fuente personalizada
    if (fuente_menu.baseSize > 0) UnloadFont(fuente_menu);
    
    // Descargar stream de musica y cerrar dispositivo de audio
    if (IsMusicReady(musica_menu)) {
        UnloadMusicStream(musica_menu);
    }
    CloseAudioDevice();
    
    CloseWindow();
    return 0;
}
