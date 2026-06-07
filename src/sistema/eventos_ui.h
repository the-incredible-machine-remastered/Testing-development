#pragma once
#include "raylib.h"
#include "../fisica/motor_fisica.h"
#include "eventos.h"
#include "../objetos/zona_meta.h"
#include "../core/math_utils.h"
#include <string>

enum class ModoEventoUI {
    INACTIVO,
    SELECCIONAR_TIPO,
    ESPERANDO_ENTIDAD_1,
    ESPERANDO_ENTIDAD_2
};

inline ModoEventoUI modo_evento_ui = ModoEventoUI::INACTIVO;
inline TipoCondicion condicion_en_creacion_ui = TipoCondicion::CONTACTO_ENTIDADES;
inline int obj1_id_ui = -1;

inline void dibujar_panel_eventos_izquierdo(const MotorFisica& motor, GestorEventos& gestor, int alto_pantalla) {
    int px = 0;
    int w = 260; 
    DrawRectangle(px, 0, w, alto_pantalla, Color{145, 150, 162, 220});
    DrawRectangleLines(px, 0, w, alto_pantalla, Color{110, 115, 128, 255});

    DrawText("PANEL DE EVENTOS", px + 20, 20, 20, DARKGRAY);

    int py = 60;
    
    if (modo_evento_ui == ModoEventoUI::INACTIVO) {
        DrawText("Eventos Activos:", px + 10, py, 14, DARKGRAY);
        py += 25;
        for (const auto& ev : gestor.eventos) {
            std::string desc = "ID " + std::to_string(ev.id) + ": " + ev.condicion.describir(motor.get_entidades());
            DrawText(desc.c_str(), px + 15, py, 10, DARKGRAY);
            
            Rectangle btn_del = { (float)px + w - 30, (float)py - 2, 20, 20 };
            DrawRectangleRec(btn_del, RED);
            DrawText("X", btn_del.x + 6, btn_del.y + 4, 10, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), btn_del)) {
                gestor.remover_evento(ev.id);
            }
            py += 25;
        }

        py += 20;
        Rectangle btn_nuevo = { (float)px + 20, (float)py, (float)w - 40, 30 };
        DrawRectangleRec(btn_nuevo, Color{55, 130, 210, 255});
        DrawText("NUEVO EVENTO", btn_nuevo.x + 40, btn_nuevo.y + 10, 12, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), btn_nuevo)) {
            modo_evento_ui = ModoEventoUI::SELECCIONAR_TIPO;
        }
    } else if (modo_evento_ui == ModoEventoUI::SELECCIONAR_TIPO) {
        DrawText("Seleccionar Tipo:", px + 10, py, 14, DARKGRAY);
        py += 25;

        auto boton_tipo = [&](const char* txt, TipoCondicion tc) {
            Rectangle r = { (float)px + 20, (float)py, (float)w - 40, 25 };
            DrawRectangleRec(r, Color{90, 165, 235, 255});
            DrawText(txt, r.x + 10, r.y + 7, 10, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r)) {
                condicion_en_creacion_ui = tc;
                obj1_id_ui = -1;
                modo_evento_ui = ModoEventoUI::ESPERANDO_ENTIDAD_1;
            }
            py += 30;
        };

        boton_tipo("Colision entre Entidades", TipoCondicion::CONTACTO_ENTIDADES);
        boton_tipo("Cabezazo (Seguidor)", TipoCondicion::CABEZAZO);
        boton_tipo("Patada (Seguidor)", TipoCondicion::PATADA);
        boton_tipo("Entidad en Zona Meta", TipoCondicion::ENTIDAD_EN_ZONA);
        boton_tipo("Barril Activado", TipoCondicion::BARRIL_ACTIVADO);

        py += 10;
        Rectangle btn_cancel = { (float)px + 20, (float)py, (float)w - 40, 25 };
        DrawRectangleRec(btn_cancel, GRAY);
        DrawText("Cancelar", btn_cancel.x + 50, btn_cancel.y + 7, 10, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), btn_cancel)) {
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }
    } else if (modo_evento_ui == ModoEventoUI::ESPERANDO_ENTIDAD_1) {
        DrawText("Selecciona el primer objeto", px + 10, py, 12, YELLOW);
        py += 20;
        DrawText("Haciendo click en la escena...", px + 10, py, 10, LIGHTGRAY);
        
        py += 40;
        Rectangle btn_cancel = { (float)px + 20, (float)py, (float)w - 40, 25 };
        DrawRectangleRec(btn_cancel, GRAY);
        DrawText("Cancelar", btn_cancel.x + 50, btn_cancel.y + 7, 10, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), btn_cancel)) {
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }
    } else if (modo_evento_ui == ModoEventoUI::ESPERANDO_ENTIDAD_2) {
        DrawText("Selecciona el segundo objeto", px + 10, py, 12, YELLOW);
        py += 20;
        DrawText("Haciendo click en la escena...", px + 10, py, 10, LIGHTGRAY);

        py += 40;
        Rectangle btn_cancel = { (float)px + 20, (float)py, (float)w - 40, 25 };
        DrawRectangleRec(btn_cancel, GRAY);
        DrawText("Cancelar", btn_cancel.x + 50, btn_cancel.y + 7, 10, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), btn_cancel)) {
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }
    }
}

inline bool manejar_click_evento_ui(GestorEventos& gestor, EntidadFisica* clicked) {
    if (modo_evento_ui == ModoEventoUI::INACTIVO) return false;
    if (!clicked) return false;

    if (modo_evento_ui == ModoEventoUI::ESPERANDO_ENTIDAD_1) {
        obj1_id_ui = clicked->get_id();
        if (condicion_en_creacion_ui == TipoCondicion::CONTACTO_ENTIDADES || 
            condicion_en_creacion_ui == TipoCondicion::ENTIDAD_EN_ZONA) {
            modo_evento_ui = ModoEventoUI::ESPERANDO_ENTIDAD_2;
        } else {
            CondicionEvento cond;
            cond.tipo = condicion_en_creacion_ui;
            if (cond.tipo == TipoCondicion::CABEZAZO || cond.tipo == TipoCondicion::PATADA) {
                cond.id_seguidor = obj1_id_ui;
            } else if (cond.tipo == TipoCondicion::BARRIL_ACTIVADO) {
                cond.id_barril = obj1_id_ui;
            }
            gestor.agregar_evento(cond, AccionEvento{TipoAccion::VICTORIA});
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }
        return true;
    } else if (modo_evento_ui == ModoEventoUI::ESPERANDO_ENTIDAD_2) {
        if (clicked->get_id() != obj1_id_ui) {
            CondicionEvento cond;
            cond.tipo = condicion_en_creacion_ui;
            if (cond.tipo == TipoCondicion::CONTACTO_ENTIDADES) {
                cond.id_entidad_a = obj1_id_ui;
                cond.id_entidad_b = clicked->get_id();
            } else if (cond.tipo == TipoCondicion::ENTIDAD_EN_ZONA) {
                cond.id_entidad_zona = obj1_id_ui;
                cond.id_zona_meta = clicked->get_id();
            }
            gestor.agregar_evento(cond, AccionEvento{TipoAccion::VICTORIA});
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }
        return true;
    }

    return false;
}
