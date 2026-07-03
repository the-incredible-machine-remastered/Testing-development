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
