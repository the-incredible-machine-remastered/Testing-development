#pragma once
#include "raylib.h"
#include "../fisica/motor_fisica.h"
#include "eventos.h"
#include "../objetos/zona_meta.h"
#include "../core/math_utils.h"
#include <string>
#include <vector>
#include <unordered_map>

extern EntidadFisica* entidad_arrastrada;
inline bool panel_izquierdo_visible = true;

enum class ModoEventoUI {
    INACTIVO,
    SELECCIONAR_TIPO,
    ESPERANDO_ENTIDAD_1,
    ESPERANDO_ENTIDAD_2,
    SELECCIONAR_TIPO_A,
    SELECCIONAR_TIPO_B,
    SELECCIONAR_TIPO_ZONA
};

inline ModoEventoUI modo_evento_ui = ModoEventoUI::INACTIVO;
inline TipoCondicion condicion_en_creacion_ui = TipoCondicion::CONTACTO_ENTIDADES;
inline int obj1_id_ui = -1;

inline int nodo_padre_creacion_ui = -1;
inline TipoEntidadJuego tipo_a_ui = TipoEntidadJuego::BOLA;
inline TipoEntidadJuego tipo_b_ui = TipoEntidadJuego::BOLA;
inline TipoEntidadJuego tipo_zona_ui = TipoEntidadJuego::BOLA;

inline void dibujar_nodo_ui(
    const MotorFisica& motor,
    GestorEventos& gestor,
    NodoEvento& nodo,
    int px, int& py, int w,
    int indent,
    bool interactivo,
    int& nodo_a_eliminar,
    int& nodo_a_toggle,
    int& nodo_padre_agregar
) {
    int margin = 6;
    int indent_px = 12;
    int current_x = px + 10 + indent * indent_px;
    int current_w = w - 20 - indent * indent_px;

    if (nodo.tipo == TipoNodo::CONDICION) {
        // pastilla de condicion
        Rectangle r_pill = { (float)current_x, (float)py, (float)current_w, 24 };
        
        Color bg_color = Color{120, 120, 130, 255};
        switch (nodo.condicion.tipo) {
            case TipoCondicion::CONTACTO_ENTIDADES:
            case TipoCondicion::CONTACTO_TIPOS:
                bg_color = Color{90, 105, 140, 255};
                break;
            case TipoCondicion::CABEZAZO:
            case TipoCondicion::PATADA:
                bg_color = Color{80, 140, 100, 255};
                break;
            case TipoCondicion::ENTIDAD_EN_ZONA:
            case TipoCondicion::TIPO_EN_ZONA:
                bg_color = Color{60, 120, 160, 255};
                break;
            case TipoCondicion::BARRIL_ACTIVADO:
                bg_color = Color{180, 100, 50, 255};
                break;
        }
        
        DrawRectangleRounded(r_pill, 0.25f, 4, bg_color);
        DrawRectangleRoundedLinesEx(r_pill, 0.25f, 4, 1.0f, ColorAlpha(WHITE, 0.2f));

        std::string desc = nodo.condicion.describir(motor.get_entidades());
        if (desc.length() > 28) desc = desc.substr(0, 25) + "...";
        DrawText(desc.c_str(), r_pill.x + 8, r_pill.y + 6, 10, WHITE);

        // Indicador de estado (✓ / -)
        Rectangle r_status = { r_pill.x + r_pill.width - 40, r_pill.y + 4, 16, 16 };
        if (nodo.cumplido) {
            DrawRectangleRounded(r_status, 0.5f, 4, GREEN);
            DrawText("V", r_status.x + 5, r_status.y + 3, 10, WHITE);
        } else {
            DrawRectangleRounded(r_status, 0.5f, 4, Color{80, 80, 90, 255});
            DrawText("-", r_status.x + 6, r_status.y + 3, 10, LIGHTGRAY);
        }

        // Boton Eliminar
        if (interactivo) {
            Rectangle r_del = { r_pill.x + r_pill.width - 20, r_pill.y + 4, 16, 16 };
            DrawRectangleRounded(r_del, 0.5f, 4, RED);
            DrawText("x", r_del.x + 5, r_del.y + 2, 10, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r_del)) {
                nodo_a_eliminar = nodo.id;
            }
        }

        py += 24 + margin;
    } else {
        // AND / OR
        int start_y = py;
        
        Rectangle r_header = { (float)current_x, (float)py, (float)current_w, 22 };
        Color header_color = (nodo.tipo == TipoNodo::OPERADOR_AND) ? Color{55, 80, 110, 255} : Color{110, 70, 110, 255};
        
        DrawRectangleRounded(r_header, 0.25f, 4, header_color);
        DrawRectangleRoundedLinesEx(r_header, 0.25f, 4, 1.0f, ColorAlpha(WHITE, 0.3f));
        
        std::string op_text = (nodo.tipo == TipoNodo::OPERADOR_AND) ? "AND (Todas)" : "OR (Alguna)";
        if (nodo.cumplido) op_text += " [OK]";
        DrawText(op_text.c_str(), r_header.x + 8, r_header.y + 5, 11, WHITE);
        
        if (interactivo && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r_header)) {
            nodo_a_toggle = nodo.id;
        }

        py += 22 + margin;
        
        if (nodo.hijos.empty()) {
            DrawText("(vacio)", current_x + 12, py, 10, GRAY);
            py += 14 + margin;
        } else {
            for (auto& h : nodo.hijos) {
                dibujar_nodo_ui(motor, gestor, h, px, py, w, indent + 1, interactivo, nodo_a_eliminar, nodo_a_toggle, nodo_padre_agregar);
            }
        }

        if (interactivo) {
            Rectangle r_add = { (float)(current_x + 10), (float)py, (float)(current_w - 20), 20 };
            DrawRectangleRec(r_add, Color{60, 60, 70, 255});
            DrawRectangleLinesEx(r_add, 1.0f, Color{80, 80, 90, 255});
            DrawText("+ AGREGAR", r_add.x + (r_add.width - MeasureText("+ AGREGAR", 10)) / 2, r_add.y + 5, 10, LIGHTGRAY);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r_add)) {
                nodo_padre_agregar = nodo.id;
            }
            py += 20 + margin;
        }

        if (interactivo && nodo.id != -1) {
            Rectangle r_del_g = { r_header.x + r_header.width - 20, r_header.y + 3, 16, 16 };
            DrawRectangleRounded(r_del_g, 0.5f, 4, RED);
            DrawText("x", r_del_g.x + 5, r_del_g.y + 2, 10, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r_del_g)) {
                nodo_a_eliminar = nodo.id;
            }
        }

        // Borde vertical para indicar anidado
        Rectangle r_border = { (float)current_x, (float)start_y, (float)current_w, (float)(py - start_y) };
        DrawRectangleLinesEx(r_border, 1.0f, ColorAlpha(header_color, 0.4f));
    }
}

inline void dibujar_seleccion_tipo_ui(int px, int& py, int w, const char* titulo, TipoEntidadJuego& out_tipo, bool& listo) {
    DrawText(titulo, px + 10, py, 11, YELLOW);
    py += 18;

    std::vector<TipoEntidadJuego> tipos = {
        TipoEntidadJuego::BOLA,
        TipoEntidadJuego::BOLA_REBOTADORA,
        TipoEntidadJuego::TRAMPOLIN,
        TipoEntidadJuego::BALANCIN,
        TipoEntidadJuego::PARED,
        TipoEntidadJuego::RAMPA,
        TipoEntidadJuego::VENTILADOR,
        TipoEntidadJuego::SEGUIDOR,
        TipoEntidadJuego::BARRIL,
        TipoEntidadJuego::CUBETA,
        TipoEntidadJuego::SOPORTE,
        TipoEntidadJuego::ZONA_META
    };

    int btn_w = (w - 30) / 2;
    int btn_h = 22;
    int start_x = px + 10;

    for (size_t i = 0; i < tipos.size(); ++i) {
        int col = i % 2;
        int row = i / 2;
        Rectangle r = { (float)(start_x + col * (btn_w + 10)), (float)(py + row * (btn_h + 6)), (float)btn_w, (float)btn_h };
        
        DrawRectangleRec(r, Color{80, 130, 200, 255});
        DrawRectangleLinesEx(r, 1.0f, Color{100, 150, 220, 255});
        
        const char* name = tipo_entidad_juego_a_str(tipos[i]);
        DrawText(name, r.x + (r.width - MeasureText(name, 9)) / 2, r.y + 6, 9, WHITE);
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r)) {
            out_tipo = tipos[i];
            listo = true;
        }
    }
    py += ((tipos.size() + 1) / 2) * (btn_h + 6) + 10;
}

inline void dibujar_panel_eventos_izquierdo(const MotorFisica& motor, GestorEventos& gestor, int alto_pantalla) {
    int px = 0;
    int w = 260; 

    // Botón abrir en el borde izquierdo si está oculto
    if (!panel_izquierdo_visible) {
        Rectangle btn_abrir = { 0, (float)(alto_pantalla / 2 - 40), 24, 80 };
        bool hover = CheckCollisionPointRec(GetMousePosition(), btn_abrir);
        DrawRectangle(btn_abrir.x, btn_abrir.y, btn_abrir.width, btn_abrir.height, hover ? Color{165, 170, 182, 255} : Color{30, 32, 40, 255});
        DrawRectangleLines(btn_abrir.x, btn_abrir.y, btn_abrir.width, btn_abrir.height, Color{50, 52, 65, 255});
        DrawText(">", btn_abrir.x + 7, btn_abrir.y + 32, 18, hover ? Color{100, 150, 220, 255} : Color{55, 80, 110, 255});
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hover) {
            panel_izquierdo_visible = true;
        }
        return;
    }

    DrawRectangle(px, 0, w, alto_pantalla, Color{30, 32, 40, 255});
    DrawRectangleLines(px, 0, w, alto_pantalla, Color{50, 52, 65, 255});

    // Botón cerrar en el borde derecho del panel si está visible
    Rectangle btn_cerrar = { (float)w, (float)(alto_pantalla / 2 - 40), 24, 80 };
    bool hover_cerrar = CheckCollisionPointRec(GetMousePosition(), btn_cerrar);
    DrawRectangle(btn_cerrar.x, btn_cerrar.y, btn_cerrar.width, btn_cerrar.height, hover_cerrar ? Color{165, 170, 182, 255} : Color{30, 32, 40, 255});
    DrawRectangleLines(btn_cerrar.x, btn_cerrar.y, btn_cerrar.width, btn_cerrar.height, Color{50, 52, 65, 255});
    DrawText("<", btn_cerrar.x + 7, btn_cerrar.y + 32, 18, hover_cerrar ? Color{100, 150, 220, 255} : Color{55, 80, 110, 255});

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && hover_cerrar) {
        panel_izquierdo_visible = false;
        return;
    }

    DrawText("ECUACION DE VICTORIA", px + 15, 20, 16, SKYBLUE);

    int py = 60;
    
    if (modo_evento_ui == ModoEventoUI::INACTIVO) {
        int nodo_a_eliminar = -2;
        int nodo_a_toggle = -2;
        int nodo_padre_agregar = -2;

        dibujar_nodo_ui(motor, gestor, gestor.raiz, px, py, w, 0, (estado_actual == EstadoJuego::JUEGO_CREATIVO), nodo_a_eliminar, nodo_a_toggle, nodo_padre_agregar);

        if (nodo_a_eliminar != -2) {
            gestor.remover_nodo(nodo_a_eliminar);
        }
        if (nodo_a_toggle != -2) {
            NodoEvento* n = gestor.buscar_nodo(nodo_a_toggle);
            if (n) n->toggle_operador();
        }
        if (nodo_padre_agregar != -2) {
            nodo_padre_creacion_ui = nodo_padre_agregar;
            modo_evento_ui = ModoEventoUI::SELECCIONAR_TIPO;
        }

        // Mostrar ecuacion textual abajo
        py = alto_pantalla - 110;
        DrawLine(px + 10, py, px + w - 10, py, Color{60, 60, 70, 255});
        py += 10;
        DrawText("Formula lógica:", px + 12, py, 11, GRAY);
        py += 18;

        std::string eq = describir_nodo_ecuacion(gestor.raiz, motor.get_entidades());
        // wrap a varias lineas si es muy larga
        if (eq.length() > 38) {
            std::string eq1 = eq.substr(0, 35) + "...";
            DrawText(eq1.c_str(), px + 15, py, 10, GOLD);
        } else {
            DrawText(eq.c_str(), px + 15, py, 10, GOLD);
        }

    } else if (modo_evento_ui == ModoEventoUI::SELECCIONAR_TIPO) {
        DrawText("Seleccionar Condición:", px + 12, py, 13, LIGHTGRAY);
        py += 22;

        auto boton_tipo = [&](const char* txt, TipoCondicion tc) {
            Rectangle r = { (float)px + 15, (float)py, (float)w - 30, 25 };
            DrawRectangleRec(r, Color{60, 110, 180, 255});
            DrawText(txt, r.x + 10, r.y + 7, 10, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r)) {
                condicion_en_creacion_ui = tc;
                obj1_id_ui = -1;
                if (tc == TipoCondicion::CONTACTO_TIPOS) {
                    modo_evento_ui = ModoEventoUI::SELECCIONAR_TIPO_A;
                } else {
                    modo_evento_ui = ModoEventoUI::ESPERANDO_ENTIDAD_1;
                }
            }
            py += 30;
        };

        boton_tipo("Colisión entre Objetos", TipoCondicion::CONTACTO_ENTIDADES);
        boton_tipo("Contacto por Tipos", TipoCondicion::CONTACTO_TIPOS);
        boton_tipo("Cabezazo (Messi)", TipoCondicion::CABEZAZO);
        boton_tipo("Patada (Messi)", TipoCondicion::PATADA);
        boton_tipo("Objeto en Zona Meta", TipoCondicion::ENTIDAD_EN_ZONA);
        boton_tipo("Tipo en Zona Meta", TipoCondicion::TIPO_EN_ZONA);
        boton_tipo("Barril Activado", TipoCondicion::BARRIL_ACTIVADO);

        py += 10;
        DrawText("--- Sub-Grupos ---", px + 15, py, 11, GRAY);
        py += 18;

        auto boton_grupo = [&](const char* txt, TipoNodo tn) {
            Rectangle r = { (float)px + 15, (float)py, (float)w - 30, 25 };
            DrawRectangleRec(r, Color{90, 80, 140, 255});
            DrawText(txt, r.x + 10, r.y + 7, 10, WHITE);
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), r)) {
                gestor.agregar_grupo(nodo_padre_creacion_ui, tn);
                modo_evento_ui = ModoEventoUI::INACTIVO;
            }
            py += 30;
        };

        boton_grupo("Nuevo Subgrupo AND", TipoNodo::OPERADOR_AND);
        boton_grupo("Nuevo Subgrupo OR", TipoNodo::OPERADOR_OR);

        py += 15;
        Rectangle btn_cancel = { (float)px + 15, (float)py, (float)w - 30, 25 };
        DrawRectangleRec(btn_cancel, GRAY);
        DrawText("Cancelar", btn_cancel.x + 95, btn_cancel.y + 7, 10, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), btn_cancel)) {
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }
    } else if (modo_evento_ui == ModoEventoUI::ESPERANDO_ENTIDAD_1) {
        DrawText("SELECCIONA EN LA ESCENA", px + 12, py, 12, YELLOW);
        py += 20;
        
        std::string txt_guia = "Selecciona el primer objeto...";
        if (condicion_en_creacion_ui == TipoCondicion::ENTIDAD_EN_ZONA ||
            condicion_en_creacion_ui == TipoCondicion::TIPO_EN_ZONA) {
            txt_guia = "Selecciona la Zona Meta...";
        } else if (condicion_en_creacion_ui == TipoCondicion::BARRIL_ACTIVADO) {
            txt_guia = "Selecciona el Barril...";
        } else if (condicion_en_creacion_ui == TipoCondicion::CABEZAZO ||
                   condicion_en_creacion_ui == TipoCondicion::PATADA) {
            txt_guia = "Selecciona a Messi (Seguidor)...";
        }
        
        DrawText(txt_guia.c_str(), px + 12, py, 10, LIGHTGRAY);
        
        py += 60;
        Rectangle btn_cancel = { (float)px + 15, (float)py, (float)w - 30, 25 };
        DrawRectangleRec(btn_cancel, GRAY);
        DrawText("Cancelar", btn_cancel.x + 95, btn_cancel.y + 7, 10, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), btn_cancel)) {
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }
    } else if (modo_evento_ui == ModoEventoUI::ESPERANDO_ENTIDAD_2) {
        DrawText("SELECCIONA EN LA ESCENA", px + 12, py, 12, YELLOW);
        py += 20;

        std::string txt_guia = "Selecciona el segundo objeto...";
        if (condicion_en_creacion_ui == TipoCondicion::ENTIDAD_EN_ZONA) {
            txt_guia = "Selecciona el Objeto que entra...";
        }
        
        DrawText(txt_guia.c_str(), px + 12, py, 10, LIGHTGRAY);

        py += 60;
        Rectangle btn_cancel = { (float)px + 15, (float)py, (float)w - 30, 25 };
        DrawRectangleRec(btn_cancel, GRAY);
        DrawText("Cancelar", btn_cancel.x + 95, btn_cancel.y + 7, 10, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), btn_cancel)) {
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }
    } else if (modo_evento_ui == ModoEventoUI::SELECCIONAR_TIPO_A) {
        bool listo = false;
        dibujar_seleccion_tipo_ui(px, py, w, "Seleccionar primer Tipo (A):", tipo_a_ui, listo);
        if (listo) {
            modo_evento_ui = ModoEventoUI::SELECCIONAR_TIPO_B;
        }
        
        py += 10;
        Rectangle btn_cancel = { (float)px + 15, (float)py, (float)w - 30, 25 };
        DrawRectangleRec(btn_cancel, GRAY);
        DrawText("Cancelar", btn_cancel.x + 95, btn_cancel.y + 7, 10, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), btn_cancel)) {
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }
    } else if (modo_evento_ui == ModoEventoUI::SELECCIONAR_TIPO_B) {
        bool listo = false;
        dibujar_seleccion_tipo_ui(px, py, w, "Seleccionar segundo Tipo (B):", tipo_b_ui, listo);
        if (listo) {
            CondicionEvento cond;
            cond.tipo = TipoCondicion::CONTACTO_TIPOS;
            cond.tipo_a = tipo_a_ui;
            cond.tipo_b = tipo_b_ui;
            gestor.agregar_condicion(nodo_padre_creacion_ui, cond);
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }

        py += 10;
        Rectangle btn_cancel = { (float)px + 15, (float)py, (float)w - 30, 25 };
        DrawRectangleRec(btn_cancel, GRAY);
        DrawText("Cancelar", btn_cancel.x + 95, btn_cancel.y + 7, 10, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), btn_cancel)) {
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }
    } else if (modo_evento_ui == ModoEventoUI::SELECCIONAR_TIPO_ZONA) {
        bool listo = false;
        dibujar_seleccion_tipo_ui(px, py, w, "Tipo que debe entrar:", tipo_zona_ui, listo);
        if (listo) {
            CondicionEvento cond;
            cond.tipo = TipoCondicion::TIPO_EN_ZONA;
            cond.id_zona_meta = obj1_id_ui;
            cond.tipo_zona = tipo_zona_ui;
            gestor.agregar_condicion(nodo_padre_creacion_ui, cond);
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }

        py += 10;
        Rectangle btn_cancel = { (float)px + 15, (float)py, (float)w - 30, 25 };
        DrawRectangleRec(btn_cancel, GRAY);
        DrawText("Cancelar", btn_cancel.x + 95, btn_cancel.y + 7, 10, WHITE);
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), btn_cancel)) {
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }
    }

    // Inventario del Jugador (abajo a la izquierda)
    if (estado_actual == EstadoJuego::JUEGO_CREATIVO) {
        int panel_x = px + 12;
        int panel_y = alto_pantalla - 390;
        int panel_w = w - 24;
        int panel_h = 260;

        Rectangle rect_panel = { (float)panel_x, (float)panel_y, (float)panel_w, (float)panel_h };
        
        bool is_dragging = (entidad_arrastrada != nullptr);
        bool is_hover = is_dragging && CheckCollisionPointRec(GetMousePosition(), rect_panel);

        DrawRectangleRounded(rect_panel, 0.05f, 4, Color{22, 24, 30, 255});
        DrawRectangleRoundedLinesEx(rect_panel, 0.05f, 4, 1.0f, is_hover ? GREEN : Color{60, 65, 80, 255});

        DrawText("INVENTARIO DEL JUGADOR", panel_x + 10, panel_y + 10, 11, is_hover ? GREEN : SKYBLUE);
        DrawLine(panel_x + 10, panel_y + 26, panel_x + panel_w - 10, panel_y + 26, Color{50, 50, 60, 255});

        std::unordered_map<TipoEntidadJuego, int> items_inventario;
        for (const auto* e : motor.get_entidades()) {
            if (!e->get_es_fijo()) {
                items_inventario[e->get_tipo_entidad()]++;
            }
        }

        int item_y = panel_y + 36;
        if (items_inventario.empty()) {
            DrawText("Arrastra objetos aqui", panel_x + 15, item_y, 10, GRAY);
            DrawText("para agregarlos al", panel_x + 15, item_y + 14, 10, GRAY);
            DrawText("inventario (no fijos)", panel_x + 15, item_y + 28, 10, GRAY);
        } else {
            for (auto const& [tipo, cant] : items_inventario) {
                if (item_y + 16 > panel_y + panel_h - 10) {
                    DrawText("...", panel_x + 15, item_y, 10, GRAY);
                    break;
                }
                const char* nombre = nombre_tipo_entidad(tipo);
                DrawText(TextFormat("- %s:", nombre), panel_x + 15, item_y, 11, LIGHTGRAY);
                DrawText(TextFormat("x%d", cant), panel_x + panel_w - 40, item_y, 11, ORANGE);
                item_y += 18;
            }
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
        } else if (condicion_en_creacion_ui == TipoCondicion::TIPO_EN_ZONA) {
            // Ir a la seleccion de tipo en vez de la entidad 2
            modo_evento_ui = ModoEventoUI::SELECCIONAR_TIPO_ZONA;
        } else {
            CondicionEvento cond;
            cond.tipo = condicion_en_creacion_ui;
            if (cond.tipo == TipoCondicion::CABEZAZO || cond.tipo == TipoCondicion::PATADA) {
                cond.id_seguidor = obj1_id_ui;
            } else if (cond.tipo == TipoCondicion::BARRIL_ACTIVADO) {
                cond.id_barril = obj1_id_ui;
            }
            gestor.agregar_condicion(nodo_padre_creacion_ui, cond);
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
            gestor.agregar_condicion(nodo_padre_creacion_ui, cond);
            modo_evento_ui = ModoEventoUI::INACTIVO;
        }
        return true;
    }

    return false;
}
