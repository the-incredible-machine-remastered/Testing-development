#pragma once
#include "raylib.h"
#include "core/animacion.h"

// Variables y funciones globales externas
extern bool modo_debug;
extern bool es_borde_nivel(const EntidadFisica* e);
extern Color PALETA_BOLAS[];
inline constexpr Color COLOR_PARED       = {40,  42,  68,  255};
inline constexpr Color COLOR_PARED_BORDE = {70,  75,  110, 255};
inline constexpr Color COLOR_RAMPA       = {35,  75,  58,  255};
inline constexpr Color COLOR_RAMPA_BORDE = {65,  140, 100, 255};

extern Texture2D tex_bola[3];
extern Texture2D tex_robote_soporte;
extern Texture2D tex_robote_pelota;
extern Texture2D tex_ventilador_cuerpo;
extern Texture2D tex_ventilador_aspa;
extern Texture2D tex_caminadora;
extern Texture2D tex_seguidor_quieto;
extern Texture2D tex_seguidor_corriendo;
extern Texture2D tex_seguidor_cabezazo;
extern Animacion* anim_seguidor_corriendo;
extern Texture2D tex_trampolin;
extern Texture2D tex_balancin_base;
extern Texture2D tex_balancin_tabla;
extern Texture2D tex_plata_larga;
extern Texture2D tex_plata_peque;
extern Texture2D tex_plata_rampa_izq;
extern Texture2D tex_plata_rampa_der;
extern Texture2D tex_chavo;
extern Texture2D tex_barril;
extern Texture2D tex_pistola;
extern Texture2D tex_pistola_gira;
extern Texture2D tex_tv_play;
extern Texture2D tex_tv_pause;
extern Font fuente_menu;

// Nuevas texturas de objetos agregados
extern Texture2D tex_globo;
extern Texture2D tex_beisbol;
extern Texture2D tex_caja_dinamita;
extern Texture2D tex_caja_base;
extern Texture2D tex_caja_tapa;
extern Texture2D tex_canon_tubo;
extern Texture2D tex_canon_base;
extern Texture2D tex_dinamita;
extern Texture2D tex_foco_apagado;
extern Texture2D tex_foco_prendido;
extern Texture2D tex_ladrillos_cuadrado;
extern Texture2D tex_ladrillos_horizontal;
extern Texture2D tex_ladrillos_vertical;
extern Texture2D tex_payaso;
extern Texture2D tex_rueda_hamster_externa;
extern Texture2D tex_rueda_hamster_rueda;
extern Texture2D tex_cubo;
extern Texture2D tex_tijera_cerrada;
extern Texture2D tex_tijera_abierta;
extern Texture2D tex_gato_quieto;
extern Texture2D tex_gato_caminando;
extern Texture2D tex_gato_corriendo;
extern Texture2D tex_rata_quieta;
extern Texture2D tex_rata_caminando;

