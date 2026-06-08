#pragma once
// Guardado / carga de partidas (.tim) y panel UI en el menú lateral.

#include "raylib.h"
#include "../fisica/motor_fisica.h"
#include "../objetos/bola.h"
#include "../objetos/bola_rebotadora.h"
#include "../objetos/trampolin.h"
#include "../objetos/balancin.h"
#include "../objetos/pared_rectangular.h"
#include "../objetos/plano_inclinado.h"
#include "../objetos/ventilador.h"
#include "../objetos/seguidor_booster.h"
#include "../objetos/barril_chavo.h"
#include "../objetos/cubeta.h"
#include "../objetos/soporte_torque.h"
#include "../objetos/cuerda.h"
#include "../objetos/zona_meta.h"
#include "eventos.h"
#include "rutas_datos.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

bool es_borde_nivel(const EntidadFisica* e);
void resetear_punteros_borde();
void crear_bordes_nivel(MotorFisica& motor);
void limpiar_estado_tras_cargar_partida();

enum class ModoPanelGuardado {
    CERRADO,
    PEDIR_NOMBRE_GUARDAR,
    LISTA_PARTIDAS
};

struct InfoPartidaGuardada {
    std::string nombre;
    std::string ruta_archivo;
};

inline ModoPanelGuardado modo_panel_guardado = ModoPanelGuardado::CERRADO;
inline char buffer_nombre_partida[48] = "";
inline std::vector<InfoPartidaGuardada> partidas_guardadas;
inline int indice_partida_lista = -1;
inline std::string mensaje_guardado;
inline float mensaje_guardado_timer = 0.0f;

inline Rectangle rect_btn_guardar_partida;
inline Rectangle rect_btn_ver_partidas;
inline Rectangle rect_panel_guardado;

namespace ui_guardado {
    const Color FONDO_PANEL = {145, 150, 162, 220};
    const Color BORDE = {110, 115, 128, 255};
    const Color TEXTO = {45, 50, 60, 255};
    const Color AZUL = {55, 130, 210, 255};
    const Color AZUL_CLARO = {90, 165, 235, 255};
    const Color CELDA = {195, 200, 210, 255};
    const Color INACTIVO = {130, 135, 145, 180};
}

inline std::string carpeta_partidas() {
    return g_raiz_datos + "saves";
}

inline std::string sanitizar_nombre_partida(const std::string& nombre) {
    std::string out;
    for (unsigned char c : nombre) {
        if (std::isalnum(c) || c == '_' || c == '-') {
            out.push_back(static_cast<char>(c));
        } else if (c == ' ') {
            out.push_back('_');
        }
    }
    if (out.empty()) out = "partida";
    return out;
}

inline int tipo_anclaje_a_int(TipoAnclajeCuerda t) {
    return static_cast<int>(t);
}

inline TipoAnclajeCuerda tipo_anclaje_desde_int(int v) {
    if (v == 1) return TipoAnclajeCuerda::BalancinIzquierdo;
    if (v == 2) return TipoAnclajeCuerda::BalancinDerecho;
    return TipoAnclajeCuerda::Cubeta;
}

inline double leer_valor(const std::string& linea, const char* clave, double defecto) {
    size_t p = linea.find(clave);
    if (p == std::string::npos) return defecto;
    return std::stod(linea.substr(p + std::strlen(clave)));
}

inline int leer_valor_i(const std::string& linea, const char* clave, int defecto) {
    size_t p = linea.find(clave);
    if (p == std::string::npos) return defecto;
    return std::stoi(linea.substr(p + std::strlen(clave)));
}

inline void escribir_dinamica(std::ostream& out, const EntidadFisica* e) {
    Vector2D p = e->get_posicion();
    Vector2D v = e->get_velocidad();
    out << " x=" << p.x << " y=" << p.y
        << " vx=" << v.x << " vy=" << v.y
        << " ang=" << e->get_angulo()
        << " omega=" << e->get_velocidad_angular();
}

inline bool guardar_partida(const MotorFisica& motor, GestorEventos& gestor, const std::string& nombre,
                            int ancho, int alto, int contador_bolas) {
    std::string carpeta = carpeta_partidas();
    std::error_code ec;
    fs::create_directories(carpeta, ec);

    std::string archivo = carpeta + "/" + sanitizar_nombre_partida(nombre) + ".tim";
    std::ofstream out(archivo);
    if (!out) {
        mensaje_guardado = "No se pudo crear el archivo";
        mensaje_guardado_timer = 3.0f;
        return false;
    }

    Vector2D g = motor.get_gravedad();
    out << "tim_save 1\n";
    out << "nombre " << sanitizar_nombre_partida(nombre) << "\n";
    out << "ancho " << ancho << "\n";
    out << "alto " << alto << "\n";
    out << "gravedad_y " << g.y << "\n";
    out << "contador_bolas " << contador_bolas << "\n";
    out << "siguiente_id " << motor.get_siguiente_id() << "\n";

    for (const auto* e : motor.get_entidades()) {
        if (!e || es_borde_nivel(e)) continue;

        if (const auto* b = dynamic_cast<const Bola*>(e)) {
            out << "ent BOLA id=" << b->get_id();
            escribir_dinamica(out, b);
            out << " r=" << b->get_radio() << " m=" << b->get_masa()
                << " ci=" << b->get_color_idx()
                << " ti=" << b->get_texture_idx() << "\n";
        } else if (const auto* br = dynamic_cast<const BolaRebotadora*>(e)) {
            out << "ent BOLA_REBOTADORA id=" << br->get_id();
            escribir_dinamica(out, br);
            out << " r=" << br->get_radio() << "\n";
        } else if (const auto* t = dynamic_cast<const Trampolin*>(e)) {
            Vector2D p = t->get_posicion();
            out << "ent TRAMPOLIN id=" << t->get_id()
                << " x=" << p.x << " y=" << p.y
                << " w=" << t->get_ancho() << " h=" << t->get_alto() << "\n";
        } else if (const auto* bal = dynamic_cast<const Balancin*>(e)) {
            out << "ent BALANCIN id=" << bal->get_id();
            escribir_dinamica(out, bal);
            out << " largo=" << bal->get_largo() << " esp=" << bal->get_espesor() << "\n";
        } else if (const auto* p = dynamic_cast<const ParedRectangular*>(e)) {
            Vector2D pos = p->get_posicion();
            out << "ent PARED id=" << p->get_id()
                << " x=" << pos.x << " y=" << pos.y
                << " w=" << p->get_ancho() << " h=" << p->get_alto() << "\n";
        } else if (const auto* ramp = dynamic_cast<const PlanoInclinado*>(e)) {
            Vector2D pos = ramp->get_posicion();
            out << "ent RAMPA id=" << ramp->get_id()
                << " x=" << pos.x << " y=" << pos.y
                << " b=" << ramp->get_base() << " h=" << ramp->get_altura()
                << " inv=" << (ramp->get_invertido() ? 1 : 0) << "\n";
        } else if (const auto* v = dynamic_cast<const Ventilador*>(e)) {
            Vector2D pos = v->get_posicion();
            out << "ent VENTILADOR id=" << v->get_id()
                << " x=" << pos.x << " y=" << pos.y
                << " w=" << v->get_ancho() << " h=" << v->get_alto()
                << " der=" << (v->mira_derecha() ? 1 : 0) << "\n";
        } else if (const auto* s = dynamic_cast<const SeguidorBooster*>(e)) {
            out << "ent SEGUIDOR id=" << s->get_id();
            escribir_dinamica(out, s);
            out << " w=" << s->get_ancho() << " h=" << s->get_alto() << "\n";
        } else if (const auto* bar = dynamic_cast<const BarrilChavo*>(e)) {
            Vector2D pos = bar->get_posicion();
            out << "ent BARRIL id=" << bar->get_id()
                << " x=" << pos.x << " y=" << pos.y
                << " w=" << bar->get_ancho() << " h=" << bar->get_alto() << "\n";
        } else if (const auto* c = dynamic_cast<const Cubeta*>(e)) {
            out << "ent CUBETA id=" << c->get_id();
            escribir_dinamica(out, c);
            out << " w=" << c->get_ancho() << " h=" << c->get_alto() << "\n";
        } else if (const auto* st = dynamic_cast<const SoporteTorque*>(e)) {
            Vector2D pos = st->get_posicion();
            out << "ent SOPORTE id=" << st->get_id()
                << " x=" << pos.x << " y=" << pos.y
                << " r=" << st->get_radio() << "\n";
        } else if (const auto* cuerda = dynamic_cast<const Cuerda*>(e)) {
            const auto& a = cuerda->get_extremo_a();
            const auto& b = cuerda->get_extremo_b();
            out << "ent CUERDA id=" << cuerda->get_id()
                << " aid=" << a.entidad_id << " at=" << tipo_anclaje_a_int(a.tipo)
                << " bid=" << b.entidad_id << " bt=" << tipo_anclaje_a_int(b.tipo)
                << " len=" << cuerda->get_longitud_inicial() << " sop=";
            const auto& sops = cuerda->get_soportes_id();
            for (size_t i = 0; i < sops.size(); ++i) {
                if (i > 0) out << ",";
                out << sops[i];
            }
        } else if (const auto* zm = dynamic_cast<const ZonaMeta*>(e)) {
            Vector2D pos = zm->get_posicion();
            out << "ent ZONA_META id=" << zm->get_id()
                << " x=" << pos.x << " y=" << pos.y
                << " w=" << zm->ancho << " h=" << zm->alto << "\n";
        }
    }

    out << "eventos_count " << gestor.eventos.size() << "\n";
    for (const auto& ev : gestor.eventos) {
        out << serializar_evento(ev) << "\n";
    }
    out << "siguiente_id_evento " << gestor.siguiente_id_evento << "\n";
    out << "logica_victoria " << (gestor.logica_victoria == TipoLogicaVictoria::TODAS ? "TODAS" : "CUALQUIERA") << "\n";

    mensaje_guardado = "Partida guardada: " + sanitizar_nombre_partida(nombre);
    mensaje_guardado_timer = 3.0f;
    TraceLog(LOG_INFO, "Partida guardada en %s", archivo.c_str());
    return true;
}

inline void refrescar_lista_partidas() {
    partidas_guardadas.clear();
    std::string carpeta = carpeta_partidas();
    std::error_code ec;
    if (!fs::exists(carpeta, ec)) return;

    for (const auto& entry : fs::directory_iterator(carpeta, ec)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".tim") continue;
        InfoPartidaGuardada info;
        info.nombre = entry.path().stem().string();
        info.ruta_archivo = entry.path().string();
        partidas_guardadas.push_back(info);
    }
    std::sort(partidas_guardadas.begin(), partidas_guardadas.end(),
        [](const InfoPartidaGuardada& a, const InfoPartidaGuardada& b) {
            return a.nombre < b.nombre;
        });
}

inline void instanciar_desde_linea(MotorFisica& motor, const std::string& linea, int& max_id) {
    if (linea.rfind("ent ", 0) != 0) return;

    std::string tipo;
    {
        size_t p0 = 4;
        size_t p1 = linea.find(' ', p0);
        tipo = linea.substr(p0, p1 - p0);
    }
    int id = leer_valor_i(linea, "id=", 0);
    max_id = std::max(max_id, id);

    if (tipo == "BOLA") {
        auto* b = new Bola(id, Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
            leer_valor(linea, "r=", 12), leer_valor(linea, "m=", 1));
        b->set_velocidad(Vector2D(leer_valor(linea, "vx=", 0), leer_valor(linea, "vy=", 0)));
        b->set_velocidad_angular(leer_valor(linea, "omega=", 0));
        b->set_color_idx(leer_valor_i(linea, "ci=", 0));
        b->set_texture_idx(leer_valor_i(linea, "ti=", 0));
        motor.agregar_entidad(b);
    } else if (tipo == "BOLA_REBOTADORA") {
        motor.agregar_entidad(new BolaRebotadora(id,
            Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
            leer_valor(linea, "r=", 48)));
    } else if (tipo == "TRAMPOLIN") {
        motor.agregar_entidad(new Trampolin(id,
            Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
            leer_valor(linea, "w=", 80), leer_valor(linea, "h=", 20)));
    } else if (tipo == "BALANCIN") {
        auto* bal = new Balancin(id, Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
            leer_valor(linea, "largo=", 200), leer_valor(linea, "esp=", 6));
        bal->set_velocidad(Vector2D(leer_valor(linea, "vx=", 0), leer_valor(linea, "vy=", 0)));
        bal->set_velocidad_angular(leer_valor(linea, "omega=", 0));
        motor.agregar_entidad(bal);
    } else if (tipo == "PARED") {
        motor.agregar_entidad(new ParedRectangular(id,
            Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
            leer_valor(linea, "w=", 100), leer_valor(linea, "h=", 15)));
    } else if (tipo == "RAMPA") {
        motor.agregar_entidad(new PlanoInclinado(id,
            Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
            leer_valor(linea, "b=", 160), leer_valor(linea, "h=", 120),
            leer_valor_i(linea, "inv=", 0) != 0));
    } else if (tipo == "VENTILADOR") {
        auto* v = new Ventilador(id,
            Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
            leer_valor(linea, "w=", 42), leer_valor(linea, "h=", 54));
        if (leer_valor_i(linea, "der=", 1) == 0) v->invertir_direccion();
        motor.agregar_entidad(v);
    } else if (tipo == "SEGUIDOR") {
        motor.agregar_entidad(new SeguidorBooster(id,
            Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
            leer_valor(linea, "w=", 24), leer_valor(linea, "h=", 48)));
    } else if (tipo == "BARRIL") {
        motor.agregar_entidad(new BarrilChavo(id,
            Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
            leer_valor(linea, "w=", 60), leer_valor(linea, "h=", 80)));
    } else if (tipo == "CUBETA") {
        auto* c = new Cubeta(id, Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
            leer_valor(linea, "w=", 58), leer_valor(linea, "h=", 52));
        c->set_velocidad(Vector2D(leer_valor(linea, "vx=", 0), leer_valor(linea, "vy=", 0)));
        motor.agregar_entidad(c);
    } else if (tipo == "SOPORTE") {
        motor.agregar_entidad(new SoporteTorque(id,
            Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
            leer_valor(linea, "r=", 16)));
    } else if (tipo == "CUERDA") {
        AnclajeCuerda a{leer_valor_i(linea, "aid=", 0), tipo_anclaje_desde_int(leer_valor_i(linea, "at=", 0))};
        AnclajeCuerda b{leer_valor_i(linea, "bid=", 0), tipo_anclaje_desde_int(leer_valor_i(linea, "bt=", 0))};
        std::vector<int> soportes;
        size_t ps = linea.find("sop=");
        if (ps != std::string::npos) {
            std::string lista = linea.substr(ps + 4);
            std::stringstream ls(lista);
            std::string item;
            while (std::getline(ls, item, ',')) {
                if (!item.empty()) soportes.push_back(std::stoi(item));
            }
        }
        motor.agregar_entidad(new Cuerda(id, a, soportes, b, leer_valor(linea, "len=", 200)));
    } else if (tipo == "ZONA_META") {
        motor.agregar_entidad(new ZonaMeta(id,
            Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
            leer_valor(linea, "w=", 80), leer_valor(linea, "h=", 80)));
    }
}

inline bool cargar_partida(MotorFisica& motor, GestorEventos& gestor, const std::string& ruta_archivo,
                           int& ancho, int& alto, int& contador_bolas) {
    std::ifstream in(ruta_archivo);
    if (!in) {
        mensaje_guardado = "No se pudo abrir la partida";
        mensaje_guardado_timer = 3.0f;
        return false;
    }

    resetear_punteros_borde();
    motor.limpiar();
    gestor.limpiar();
    crear_bordes_nivel(motor);

    double gravedad_y = 500.0;
    int max_id = 0;
    std::string linea;

    while (std::getline(in, linea)) {
        if (linea.empty()) continue;
        if (linea.rfind("gravedad_y", 0) == 0) {
            gravedad_y = std::stod(linea.substr(11));
        } else if (linea.rfind("contador_bolas", 0) == 0) {
            contador_bolas = std::stoi(linea.substr(15));
        } else if (linea.rfind("siguiente_id", 0) == 0) {
            max_id = std::stoi(linea.substr(13));
        } else if (linea.rfind("ancho", 0) == 0) {
            ancho = std::stoi(linea.substr(6));
        } else if (linea.rfind("alto", 0) == 0) {
            alto = std::stoi(linea.substr(5));
        } else if (linea.rfind("ent ", 0) == 0) {
            instanciar_desde_linea(motor, linea, max_id);
        } else if (linea.rfind("evt ", 0) == 0) {
            EventoJuego ev = deserializar_evento(linea);
            gestor.eventos.push_back(ev);
        } else if (linea.rfind("siguiente_id_evento", 0) == 0) {
            gestor.siguiente_id_evento = std::stoi(linea.substr(20));
        } else if (linea.rfind("logica_victoria", 0) == 0) {
            if (linea.find("TODAS") != std::string::npos) {
                gestor.logica_victoria = TipoLogicaVictoria::TODAS;
            } else {
                gestor.logica_victoria = TipoLogicaVictoria::CUALQUIERA;
            }
        }
    }

    motor.set_gravedad(Vector2D(0, gravedad_y));
    motor.set_siguiente_id(max_id + 1);
    limpiar_estado_tras_cargar_partida();
    mensaje_guardado = "Partida cargada";
    mensaje_guardado_timer = 3.0f;
    return true;
}

inline void actualizar_mensaje_guardado(float dt) {
    if (mensaje_guardado_timer > 0.0f) {
        mensaje_guardado_timer -= dt;
        if (mensaje_guardado_timer < 0.0f) mensaje_guardado_timer = 0.0f;
    }
}

inline void manejar_texto_nombre_partida() {
    int key = GetCharPressed();
    while (key > 0) {
        size_t len = std::strlen(buffer_nombre_partida);
        if (key >= 32 && key <= 125 && len < sizeof(buffer_nombre_partida) - 1) {
            buffer_nombre_partida[len] = static_cast<char>(key);
            buffer_nombre_partida[len + 1] = '\0';
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE)) {
        size_t len = std::strlen(buffer_nombre_partida);
        if (len > 0) buffer_nombre_partida[len - 1] = '\0';
    }
}

inline void dibujar_panel_guardado(int px, int py_base, int ancho_panel, Font fuente) {
    const int alto_panel = 200;
    Rectangle panel = {static_cast<float>(px + 8), static_cast<float>(py_base),
                       static_cast<float>(ancho_panel - 16), static_cast<float>(alto_panel)};
    rect_panel_guardado = panel;
    DrawRectangleRounded(panel, 0.05f, 6, ui_guardado::FONDO_PANEL);
    DrawRectangleRoundedLinesEx(panel, 0.05f, 6, 1.5f, ui_guardado::BORDE);

    rect_btn_guardar_partida = {panel.x + 10, panel.y + 10, panel.width - 20, 32};
    rect_btn_ver_partidas = {panel.x + 10, panel.y + 50, panel.width - 20, 32};

    auto dibujar_boton = [](Rectangle r, const char* txt, bool hover, Font f) {
        DrawRectangleRounded(r, 0.12f, 4, hover ? ui_guardado::AZUL_CLARO : ui_guardado::AZUL);
        Vector2 ts = MeasureTextEx(f, txt, 14, 1);
        DrawTextEx(f, txt, {r.x + (r.width - ts.x) / 2, r.y + (r.height - ts.y) / 2},
                   14, 1, WHITE);
    };

    Vector2 mouse = GetMousePosition();
    dibujar_boton(rect_btn_guardar_partida, "Guardar Partida",
        CheckCollisionPointRec(mouse, rect_btn_guardar_partida), fuente);
    dibujar_boton(rect_btn_ver_partidas, "Partidas Guardadas",
        CheckCollisionPointRec(mouse, rect_btn_ver_partidas), fuente);

    if (modo_panel_guardado == ModoPanelGuardado::PEDIR_NOMBRE_GUARDAR) {
        DrawTextEx(fuente, "Nombre de la partida:", {panel.x + 10, panel.y + 92}, 12, 1, ui_guardado::TEXTO);
        Rectangle caja = {panel.x + 10, panel.y + 112, panel.width - 20, 28};
        DrawRectangleRounded(caja, 0.08f, 4, WHITE);
        DrawText(buffer_nombre_partida, static_cast<int>(caja.x + 8),
                 static_cast<int>(caja.y + 6), 14, ui_guardado::TEXTO);
        DrawTextEx(fuente, "[ENTER] Guardar  [ESC] Cancelar",
                 {panel.x + 10, panel.y + 148}, 11, 1, ui_guardado::INACTIVO);
    } else if (modo_panel_guardado == ModoPanelGuardado::LISTA_PARTIDAS) {
        DrawTextEx(fuente, "Clic para cargar:", {panel.x + 10, panel.y + 92}, 12, 1, ui_guardado::TEXTO);
        float ly = panel.y + 112;
        for (size_t i = 0; i < partidas_guardadas.size() && i < 4; ++i) {
            Rectangle fila = {panel.x + 10, ly, panel.width - 20, 22};
            bool hot = CheckCollisionPointRec(mouse, fila);
            bool sel = static_cast<int>(i) == indice_partida_lista;
            DrawRectangleRec(fila, sel ? ui_guardado::AZUL : (hot ? ui_guardado::CELDA : ColorAlpha(WHITE, 40)));
            DrawText(partidas_guardadas[i].nombre.c_str(),
                     static_cast<int>(fila.x + 6), static_cast<int>(fila.y + 4), 12, ui_guardado::TEXTO);
            ly += 24;
        }
        if (partidas_guardadas.empty()) {
            DrawText("Sin partidas guardadas", static_cast<int>(panel.x + 10),
                     static_cast<int>(ly), 12, ui_guardado::INACTIVO);
        }
        DrawTextEx(fuente, "[ESC] Cerrar", {panel.x + 10, panel.y + alto_panel - 22},
                   11, 1, ui_guardado::INACTIVO);
    }

    if (mensaje_guardado_timer > 0.0f && !mensaje_guardado.empty()) {
        DrawText(mensaje_guardado.c_str(), static_cast<int>(panel.x + 8),
                 static_cast<int>(panel.y + alto_panel + 4), 12,
                 Color{80, 200, 120, 255});
    }
}

inline bool manejar_click_panel_guardado(int mx, int my, MotorFisica& motor, GestorEventos& gestor,
                                         int& ancho, int& alto, int& contador_bolas) {
    Vector2 p_click = {static_cast<float>(mx), static_cast<float>(my)};

    if (modo_panel_guardado == ModoPanelGuardado::PEDIR_NOMBRE_GUARDAR) {
        if (CheckCollisionPointRec(p_click, rect_panel_guardado)) {
            return true;
        }
        return false;
    }

    if (modo_panel_guardado == ModoPanelGuardado::LISTA_PARTIDAS) {
        float ly = rect_btn_ver_partidas.y + 62;
        for (size_t i = 0; i < partidas_guardadas.size() && i < 4; ++i) {
            Rectangle fila = {rect_btn_ver_partidas.x, ly, rect_btn_ver_partidas.width, 22};
            if (CheckCollisionPointRec(p_click, fila)) {
                cargar_partida(motor, gestor, partidas_guardadas[i].ruta_archivo, ancho, alto, contador_bolas);
                modo_panel_guardado = ModoPanelGuardado::CERRADO;
                return true;
            }
            ly += 24;
        }
        return false;
    }

    if (modo_panel_guardado != ModoPanelGuardado::CERRADO) return false;

    if (CheckCollisionPointRec(p_click, rect_btn_guardar_partida)) {
        buffer_nombre_partida[0] = '\0';
        modo_panel_guardado = ModoPanelGuardado::PEDIR_NOMBRE_GUARDAR;
        return true;
    }
    if (CheckCollisionPointRec(p_click, rect_btn_ver_partidas)) {
        refrescar_lista_partidas();
        indice_partida_lista = -1;
        modo_panel_guardado = ModoPanelGuardado::LISTA_PARTIDAS;
        return true;
    }
    return false;
}

inline void manejar_teclas_panel_guardado(MotorFisica& motor, GestorEventos& gestor, int& ancho, int& alto,
                                          int& contador_bolas) {
    if (modo_panel_guardado == ModoPanelGuardado::PEDIR_NOMBRE_GUARDAR) {
        manejar_texto_nombre_partida();
        if (IsKeyPressed(KEY_ENTER) && buffer_nombre_partida[0] != '\0') {
            guardar_partida(motor, gestor, buffer_nombre_partida, ancho, alto, contador_bolas);
            modo_panel_guardado = ModoPanelGuardado::CERRADO;
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            modo_panel_guardado = ModoPanelGuardado::CERRADO;
        }
    } else if (modo_panel_guardado == ModoPanelGuardado::LISTA_PARTIDAS) {
        if (IsKeyPressed(KEY_ESCAPE)) {
            modo_panel_guardado = ModoPanelGuardado::CERRADO;
        }
    }
}
