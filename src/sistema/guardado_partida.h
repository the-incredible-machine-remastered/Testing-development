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
#include "../objetos/gancho.h"
#include "../objetos/pistola.h"
#include "../objetos/tijera.h"
#include "../objetos/globo.h"
#include "../objetos/bola_beisbol.h"
#include "../objetos/caja_hamster.h"
#include "../objetos/banda.h"
#include "../objetos/caja_sorpresa.h"
#include "../objetos/caminadora.h"
#include "../objetos/foco.h"
#include "../objetos/lupa.h"
#include "../objetos/canon.h"
#include "../objetos/ladrillo.h"
#include "../objetos/dinamita.h"
#include "../objetos/dinamita_detonador.h"
#include "../objetos/raton.h"
#include "../objetos/gato.h"
#include "eventos.h"
#include "rutas_datos.h"
#include "../objetos/catalogo_menu.gen.h"
#include "../core/tipo_entidad.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>

namespace fs = std::filesystem;

bool es_borde_nivel(const EntidadFisica* e);
void resetear_punteros_borde();
void crear_bordes_nivel(MotorFisica& motor);
void limpiar_estado_tras_cargar_partida();

extern std::unordered_map<TipoObjetoMenu, int> inventario_maximo;
extern std::unordered_map<TipoObjetoMenu, int> inventario_actual;
extern std::unordered_map<TipoObjetoMenu, std::vector<std::unique_ptr<EntidadFisica>>> inventario_entidades;
extern EstadoJuego estado_actual;

inline TipoObjetoMenu mapear_tipo_entidad_a_menu(TipoEntidadJuego t, const EntidadFisica* e) {
    switch (t) {
        case TipoEntidadJuego::BOLA: return TipoObjetoMenu::BOLA;
        case TipoEntidadJuego::BOLA_REBOTADORA: return TipoObjetoMenu::BOLA_REBOTADORA;
        case TipoEntidadJuego::TRAMPOLIN: return TipoObjetoMenu::TRAMPOLIN;
        case TipoEntidadJuego::BALANCIN: return TipoObjetoMenu::BALANCIN;
        case TipoEntidadJuego::RAMPA: return TipoObjetoMenu::RAMPA;
        case TipoEntidadJuego::VENTILADOR: return TipoObjetoMenu::VENTILADOR;
        case TipoEntidadJuego::SEGUIDOR: return TipoObjetoMenu::SEGUIDOR_BOOSTER;
        case TipoEntidadJuego::BARRIL: return TipoObjetoMenu::BARRIL_CHAVO;
        case TipoEntidadJuego::CUBETA: return TipoObjetoMenu::CUBETA;
        case TipoEntidadJuego::SOPORTE: return TipoObjetoMenu::SOPORTE_TORQUE;
        case TipoEntidadJuego::ZONA_META: return TipoObjetoMenu::ZONA_META;
        case TipoEntidadJuego::CUERDA: return TipoObjetoMenu::CUERDA;
        case TipoEntidadJuego::TIJERA: return TipoObjetoMenu::TIJERA;
        case TipoEntidadJuego::GLOBO: return TipoObjetoMenu::GLOBO;
        case TipoEntidadJuego::BOLA_BEISBOL: return TipoObjetoMenu::BOLA_BEISBOL;
        case TipoEntidadJuego::CAJA_HAMSTER: return TipoObjetoMenu::CAJA_HAMSTER;
        case TipoEntidadJuego::BANDA: return TipoObjetoMenu::BANDA;
        case TipoEntidadJuego::CAJA_SORPRESA: return TipoObjetoMenu::CAJA_SORPRESA;
        case TipoEntidadJuego::CAMINADORA: return TipoObjetoMenu::CAMINADORA;
        case TipoEntidadJuego::FOCO: return TipoObjetoMenu::FOCO;
        case TipoEntidadJuego::LUPA: return TipoObjetoMenu::LUPA;
        case TipoEntidadJuego::CANON: return TipoObjetoMenu::CANON;
        case TipoEntidadJuego::LADRILLO: return TipoObjetoMenu::LADRILLO;
        case TipoEntidadJuego::DINAMITA: return TipoObjetoMenu::DINAMITA;
        case TipoEntidadJuego::DINAMITA_DETONADOR: return TipoObjetoMenu::DINAMITA_DETONADOR;
        case TipoEntidadJuego::GATO: return TipoObjetoMenu::GATO;
        case TipoEntidadJuego::RATON: return TipoObjetoMenu::RATON;
        case TipoEntidadJuego::PARED: {
            const ParedRectangular* p = dynamic_cast<const ParedRectangular*>(e);
            if (p) {
                double w = p->get_ancho();
                double h = p->get_alto();
                if (std::abs(w - 150.0) < 10.0 && std::abs(h - 15.0) < 5.0) return TipoObjetoMenu::PLATAFORMA;
                if (std::abs(w - 80.0) < 10.0 && std::abs(h - 120.0) < 10.0) return TipoObjetoMenu::PARED_LARGA;
                if (std::abs(w - 120.0) < 10.0 && std::abs(h - 20.0) < 5.0) return TipoObjetoMenu::PLATAFORMA_DECOR;
            }
            return TipoObjetoMenu::PLATAFORMA;
        }
        default: return TipoObjetoMenu::NINGUNO;
    }
}

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
    switch (v) {
        case 0: return TipoAnclajeCuerda::Cubeta;
        case 1: return TipoAnclajeCuerda::BalancinIzquierdo;
        case 2: return TipoAnclajeCuerda::BalancinDerecho;
        case 3: return TipoAnclajeCuerda::SoporteFijo;
        case 4: return TipoAnclajeCuerda::Globo;
        case 5: return TipoAnclajeCuerda::Gancho;
        default: return TipoAnclajeCuerda::Cubeta;
    }
}

inline double leer_valor(const std::string& linea, const char* clave, double defecto) {
    size_t p = linea.find(clave);
    if (p == std::string::npos) return defecto;
    try {
        return std::stod(linea.substr(p + std::strlen(clave)));
    } catch (...) {
        return defecto;
    }
}

inline int leer_valor_i(const std::string& linea, const char* clave, int defecto) {
    size_t p = linea.find(clave);
    if (p == std::string::npos) return defecto;
    try {
        return std::stoi(linea.substr(p + std::strlen(clave)));
    } catch (...) {
        return defecto;
    }
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

        std::string s = e->serializar();
        if (!s.empty()) {
            out << s << "\n";
        }
    }

    out << "siguiente_id_evento " << gestor.siguiente_id_evento << "\n";
    serializar_nodo_recursivo(gestor.raiz, out);

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

using CreadorEntidad = std::function<std::unique_ptr<EntidadFisica>(int id, const std::string& linea)>;

inline const std::unordered_map<std::string, CreadorEntidad>& obtener_registro_fabrica() {
    static const std::unordered_map<std::string, CreadorEntidad> registro = {
        {"BOLA", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            auto b = std::make_unique<Bola>(id, Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "r=", 12), leer_valor(linea, "m=", 1));
            b->set_velocidad(Vector2D(leer_valor(linea, "vx=", 0), leer_valor(linea, "vy=", 0)));
            b->set_velocidad_angular(leer_valor(linea, "omega=", 0));
            b->set_color_idx(leer_valor_i(linea, "ci=", 0));
            b->set_texture_idx(leer_valor_i(linea, "ti=", 0));
            return b;
        }},
        {"BOLA_REBOTADORA", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<BolaRebotadora>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "r=", 48));
        }},
        {"TRAMPOLIN", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<Trampolin>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 80), leer_valor(linea, "h=", 20));
        }},
        {"BALANCIN", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            auto bal = std::make_unique<Balancin>(id, Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "largo=", 200), leer_valor(linea, "esp=", 6));
            double ang0 = leer_valor(linea, "ang0=", 0.0);
            if (ang0 != 0.0) {
                bal->ciclar_inclinacion();
                if (ang0 < 0.0 && bal->get_angulo_inicial() > 0.0) bal->ciclar_inclinacion();
                if (ang0 > 0.0 && bal->get_angulo_inicial() < 0.0) bal->ciclar_inclinacion();
            }
            bal->set_velocidad(Vector2D(leer_valor(linea, "vx=", 0), leer_valor(linea, "vy=", 0)));
            bal->set_velocidad_angular(leer_valor(linea, "omega=", 0));
            return bal;
        }},
        {"PARED", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<ParedRectangular>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 100), leer_valor(linea, "h=", 15));
        }},
        {"RAMPA", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<PlanoInclinado>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "b=", 160), leer_valor(linea, "h=", 120),
                leer_valor_i(linea, "inv=", 0) != 0);
        }},
        {"VENTILADOR", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            auto v = std::make_unique<Ventilador>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 42), leer_valor(linea, "h=", 54));
            if (leer_valor_i(linea, "der=", 1) == 0) v->invertir_direccion();
            if (leer_valor_i(linea, "banda=", 0) == 1) v->set_controlado_por_banda(true);
            return v;
        }},
        {"SEGUIDOR", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<SeguidorBooster>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 24), leer_valor(linea, "h=", 48));
        }},
        {"BARRIL", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<BarrilChavo>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 60), leer_valor(linea, "h=", 80));
        }},
        {"CUBETA", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            auto c = std::make_unique<Cubeta>(id, Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 58), leer_valor(linea, "h=", 52));
            c->set_velocidad(Vector2D(leer_valor(linea, "vx=", 0), leer_valor(linea, "vy=", 0)));
            return c;
        }},
        {"SOPORTE", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<SoporteTorque>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "r=", 16));
        }},
        {"CUERDA", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            AnclajeCuerda a{leer_valor_i(linea, "aid=", 0), tipo_anclaje_desde_int(leer_valor_i(linea, "at=", 0))};
            AnclajeCuerda b{leer_valor_i(linea, "bid=", 0), tipo_anclaje_desde_int(leer_valor_i(linea, "bt=", 0))};
            std::vector<int> soportes;
            size_t ps = linea.find("sop=");
            if (ps != std::string::npos) {
                std::string lista = linea.substr(ps + 4);
                std::stringstream ls(lista);
                std::string item;
                while (std::getline(ls, item, ',')) {
                    if (!item.empty()) {
                        try {
                            soportes.push_back(std::stoi(item));
                        } catch (...) {}
                    }
                }
            }
            return std::make_unique<Cuerda>(id, a, soportes, b, leer_valor(linea, "len=", 200));
        }},
        {"GANCHO", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<Gancho>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)));
        }},
        {"PISTOLA", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<Pistola>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "ang=", 0.0));
        }},
        {"FOCO", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<Foco>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "r=", 18.0));
        }},
        {"LUPA", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<Lupa>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "ang=", 0.0), leer_valor(linea, "rango=", 200.0));
        }},
        {"CANON", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<Canon>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "ang=", 180.0));
        }},
        {"LADRILLO", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<Ladrillo>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 60.0), leer_valor(linea, "h=", 40.0));
        }},
        {"DINAMITA", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<Dinamita>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 26.0), leer_valor(linea, "h=", 46.0));
        }},
        {"DINAMITA_DETONADOR", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<DinamitaDetonador>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 26.0), leer_valor(linea, "h=", 46.0));
        }},
        {"GATO", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<Gato>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 54.0), leer_valor(linea, "h=", 40.0));
        }},
        {"RATON", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<Raton>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 22.0), leer_valor(linea, "h=", 12.0));
        }},
        {"ZONA_META", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<ZonaMeta>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 80), leer_valor(linea, "h=", 80));
        }},
        {"TIJERA", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<Tijera>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 119.0), leer_valor(linea, "h=", 51.0));
        }},
        {"GLOBO", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            auto g = std::make_unique<Globo>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "r=", 36.0));
            g->set_velocidad(Vector2D(leer_valor(linea, "vx=", 0), leer_valor(linea, "vy=", 0)));
            return g;
        }},
        {"BOLA_BEISBOL", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            auto b = std::make_unique<BolaBeisbol>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "r=", 24.0));
            b->set_velocidad(Vector2D(leer_valor(linea, "vx=", 0), leer_valor(linea, "vy=", 0)));
            b->set_velocidad_angular(leer_valor(linea, "omega=", 0));
            return b;
        }},
        {"CAJA_HAMSTER", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            auto h = std::make_unique<CajaHamster>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 90.0), leer_valor(linea, "h=", 80.0));
            // Leer IDs de ventiladores conectados
            size_t pv = linea.find("vents=");
            if (pv != std::string::npos) {
                std::string lista = linea.substr(pv + 6);
                std::stringstream ls(lista);
                std::string item;
                while (std::getline(ls, item, ',')) {
                    try { if (!item.empty()) h->agregar_ventilador(std::stoi(item)); } catch(...) {}
                }
            }
            return h;
        }},
        {"BANDA", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            int orig = leer_valor_i(linea, "orig=", 0);
            int dest = leer_valor_i(linea, "dest=", 0);
            return std::make_unique<Banda>(id, orig, dest);
        }},
        {"CAJA_SORPRESA", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<CajaSorpresa>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 70.0), leer_valor(linea, "h=", 70.0));
        }},
        {"CONVEYOR", [](int id, const std::string& linea) -> std::unique_ptr<EntidadFisica> {
            return std::make_unique<Caminadora>(id,
                Vector2D(leer_valor(linea, "x=", 0), leer_valor(linea, "y=", 0)),
                leer_valor(linea, "w=", 150.0), leer_valor(linea, "h=", 24.0),
                leer_valor_i(linea, "der=", 1) != 0);
        }}
    };
    return registro;
}

inline void instanciar_desde_linea(MotorFisica& motor, const std::string& linea, int& max_id, bool es_snapshot = false) {
    if (linea.rfind("ent ", 0) != 0) return;

    std::string tipo;
    {
        size_t p0 = 4;
        size_t p1 = linea.find(' ', p0);
        tipo = linea.substr(p0, p1 - p0);
    }
    int id = leer_valor_i(linea, "id=", 0);
    max_id = std::max(max_id, id);

    const auto& registro = obtener_registro_fabrica();
    auto it = registro.find(tipo);
    if (it != registro.end()) {
        auto e = it->second(id, linea);
        if (e) {
            bool fijo = (leer_valor_i(linea, "fijo=", 1) != 0);
            e->set_es_fijo(fijo);
            
            int tm = leer_valor_i(linea, "tipo_menu=", -1);
            TipoObjetoMenu tipo_menu = TipoObjetoMenu::NINGUNO;
            if (tm >= 0 && tm < static_cast<int>(TipoObjetoMenu::COUNT)) {
                tipo_menu = static_cast<TipoObjetoMenu>(tm);
            } else {
                tipo_menu = mapear_tipo_entidad_a_menu(e->get_tipo_entidad(), e.get());
            }
            e->set_tipo_menu(tipo_menu);

            if (!fijo && !es_snapshot) {
                inventario_maximo[tipo_menu]++;
                inventario_actual[tipo_menu]++;
                if (estado_actual == EstadoJuego::JUEGO_NIVEL) {
                    inventario_entidades[tipo_menu].push_back(std::move(e));
                    return;
                }
            }

            motor.agregar_entidad(std::move(e));
        }
    } else {
        TraceLog(LOG_WARNING, "Tipo de entidad desconocido al cargar partida: '%s'", tipo.c_str());
    }
}

inline bool cargar_partida(MotorFisica& motor, GestorEventos& gestor, const std::string& ruta_archivo,
                           int& ancho, int& alto, int& contador_bolas) {
    limpiar_estado_tras_cargar_partida();

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

    std::vector<CondicionEvento> old_condiciones;
    TipoLogicaVictoria old_logica = TipoLogicaVictoria::CUALQUIERA;
    bool arbol_nuevo_cargado = false;
    std::vector<NodoEvento> stack_nodos;

    while (std::getline(in, linea)) {
        if (linea.empty()) continue;
        try {
            if (linea.rfind("gravedad_y", 0) == 0) {
                gravedad_y = std::stod(linea.substr(11));
            } else if (linea.rfind("contador_bolas", 0) == 0) {
                contador_bolas = std::stoi(linea.substr(15));
            } else if (linea.rfind("siguiente_id_evento", 0) == 0) {
                gestor.siguiente_id_evento = std::stoi(linea.substr(20));
            } else if (linea.rfind("siguiente_id", 0) == 0) {
                max_id = std::stoi(linea.substr(13));
            } else if (linea.rfind("ancho", 0) == 0) {
                ancho = std::stoi(linea.substr(6));
            } else if (linea.rfind("alto", 0) == 0) {
                alto = std::stoi(linea.substr(5));
            } else if (linea.rfind("ent ", 0) == 0) {
                instanciar_desde_linea(motor, linea, max_id, false);
            } else if (linea.rfind("evt ", 0) == 0) {
                CondicionEvento cond = deserializar_condicion_desde_linea(linea);
                old_condiciones.push_back(cond);
            } else if (linea.rfind("logica_victoria", 0) == 0) {
                if (linea.find("TODAS") != std::string::npos) {
                    old_logica = TipoLogicaVictoria::TODAS;
                } else {
                    old_logica = TipoLogicaVictoria::CUALQUIERA;
                }
            } else if (linea.find("evt_grupo id=") != std::string::npos) {
                auto extraer_str = [&](const std::string& clave) -> std::string {
                    std::string buscar = clave + "=";
                    size_t pos = linea.find(buscar);
                    if (pos == std::string::npos) return "";
                    pos += buscar.length();
                    size_t fin = linea.find(' ', pos);
                    if (fin == std::string::npos) fin = linea.length();
                    return linea.substr(pos, fin - pos);
                };
                int node_id = -1;
                try { node_id = std::stoi(extraer_str("id")); } catch(...) {}
                std::string op_str = extraer_str("op");
                TipoNodo tn = (op_str == "OR") ? TipoNodo::OPERADOR_OR : TipoNodo::OPERADOR_AND;
                
                NodoEvento nuevo_grupo;
                nuevo_grupo.id = node_id;
                nuevo_grupo.tipo = tn;
                nuevo_grupo.cumplido = false;
                
                stack_nodos.push_back(nuevo_grupo);
                arbol_nuevo_cargado = true;
            } else if (linea.find("evt_cond ") != std::string::npos) {
                auto extraer_str = [&](const std::string& clave) -> std::string {
                    std::string buscar = clave + "=";
                    size_t pos = linea.find(buscar);
                    if (pos == std::string::npos) return "";
                    pos += buscar.length();
                    size_t fin = linea.find(' ', pos);
                    if (fin == std::string::npos) fin = linea.length();
                    return linea.substr(pos, fin - pos);
                };
                int node_id = -1;
                try { node_id = std::stoi(extraer_str("id")); } catch(...) {}
                CondicionEvento cond = deserializar_condicion_desde_linea(linea);
                
                NodoEvento nodo_cond;
                nodo_cond.id = node_id;
                nodo_cond.tipo = TipoNodo::CONDICION;
                nodo_cond.condicion = cond;
                nodo_cond.cumplido = false;
                
                if (!stack_nodos.empty()) {
                    stack_nodos.back().hijos.push_back(nodo_cond);
                } else {
                    gestor.raiz = nodo_cond;
                }
                arbol_nuevo_cargado = true;
            } else if (linea.find("evt_grupo_fin") != std::string::npos) {
                if (stack_nodos.size() > 1) {
                    NodoEvento hijo = stack_nodos.back();
                    stack_nodos.pop_back();
                    stack_nodos.back().hijos.push_back(hijo);
                } else if (stack_nodos.size() == 1) {
                    gestor.raiz = stack_nodos.back();
                    stack_nodos.pop_back();
                }
                arbol_nuevo_cargado = true;
            }
        } catch (...) {
            TraceLog(LOG_WARNING, "Linea de guardado malformada ignorada de forma segura: '%s'", linea.c_str());
        }
    }

    if (!arbol_nuevo_cargado) {
        gestor.limpiar();
        gestor.raiz.tipo = (old_logica == TipoLogicaVictoria::TODAS) ? TipoNodo::OPERADOR_AND : TipoNodo::OPERADOR_OR;
        gestor.raiz.id = -1;
        for (const auto& cond : old_condiciones) {
            gestor.agregar_condicion(gestor.raiz.id, cond);
        }
    }

    motor.set_gravedad(Vector2D(0, gravedad_y));
    motor.set_siguiente_id(max_id + 1);
    
    // Asegurar que todos los objetos comiencen completamente en reposo
    for (auto* e : motor.get_entidades()) {
        e->set_velocidad(Vector2D(0.0, 0.0));
        e->set_velocidad_angular(0.0);
    }
    
    // Apagar ventiladores que están controlados por una Banda
    {
        const auto& ents = motor.get_entidades();
        for (auto* e : ents) {
            auto* banda = dynamic_cast<Banda*>(e);
            if (!banda) continue;
            for (auto* e2 : ents) {
                if (e2->get_id() != banda->get_id_destino()) continue;
                auto* vent = dynamic_cast<Ventilador*>(e2);
                if (vent) vent->set_controlado_por_banda(true);
                break;
            }
        }
    }

    mensaje_guardado = "Partida cargada";
    mensaje_guardado_timer = 3.0f;
    motor.set_pausado(true);
    return true;
}

inline std::string snapshot_simulacion = "";

inline void guardar_snapshot_simulacion(const MotorFisica& motor, GestorEventos& gestor) {
    std::stringstream out;
    out << "tim_save 1\n";
    out << "gravedad_y " << motor.get_gravedad().y << "\n";
    out << "siguiente_id " << motor.get_siguiente_id() << "\n";
    
    for (const auto& pair : inventario_actual) {
        out << "inv_item " << static_cast<int>(pair.first) << " " << pair.second << "\n";
    }

    for (const auto* e : motor.get_entidades()) {
        if (!e || es_borde_nivel(e)) continue;

        std::string s = e->serializar();
        if (!s.empty()) {
            out << s << "\n";
        }
    }

    out << "siguiente_id_evento " << gestor.siguiente_id_evento << "\n";
    serializar_nodo_recursivo(gestor.raiz, out);
    snapshot_simulacion = out.str();
    TraceLog(LOG_INFO, "Snapshot de simulación guardado (tamaño: %d bytes)", static_cast<int>(snapshot_simulacion.size()));
}

inline void restaurar_snapshot_simulacion(MotorFisica& motor, GestorEventos& gestor) {
    if (snapshot_simulacion.empty()) return;

    resetear_punteros_borde();
    motor.limpiar();
    gestor.limpiar();
    crear_bordes_nivel(motor);

    for (auto& pair : inventario_actual) {
        pair.second = 0;
    }

    std::stringstream in(snapshot_simulacion);
    double gravedad_y = 500.0;
    int max_id = 0;
    std::string linea;

    std::vector<CondicionEvento> old_condiciones;
    TipoLogicaVictoria old_logica = TipoLogicaVictoria::CUALQUIERA;
    bool arbol_nuevo_cargado = false;
    std::vector<NodoEvento> stack_nodos;

    while (std::getline(in, linea)) {
        if (linea.empty()) continue;
        try {
            if (linea.rfind("gravedad_y", 0) == 0) {
                gravedad_y = std::stod(linea.substr(11));
            } else if (linea.rfind("siguiente_id_evento", 0) == 0) {
                gestor.siguiente_id_evento = std::stoi(linea.substr(20));
            } else if (linea.rfind("siguiente_id", 0) == 0) {
                max_id = std::stoi(linea.substr(13));
            } else if (linea.rfind("inv_item ", 0) == 0) {
                std::stringstream ss(linea.substr(9));
                int tipo_val, cant_val;
                if (ss >> tipo_val >> cant_val) {
                    inventario_actual[static_cast<TipoObjetoMenu>(tipo_val)] = cant_val;
                }
            } else if (linea.rfind("ent ", 0) == 0) {
                instanciar_desde_linea(motor, linea, max_id, true);
            } else if (linea.rfind("evt ", 0) == 0) {
                CondicionEvento cond = deserializar_condicion_desde_linea(linea);
                old_condiciones.push_back(cond);
            } else if (linea.rfind("logica_victoria", 0) == 0) {
                if (linea.find("TODAS") != std::string::npos) {
                    old_logica = TipoLogicaVictoria::TODAS;
                } else {
                    old_logica = TipoLogicaVictoria::CUALQUIERA;
                }
            } else if (linea.find("evt_grupo id=") != std::string::npos) {
                auto extraer_str = [&](const std::string& clave) -> std::string {
                    std::string buscar = clave + "=";
                    size_t pos = linea.find(buscar);
                    if (pos == std::string::npos) return "";
                    pos += buscar.length();
                    size_t fin = linea.find(' ', pos);
                    if (fin == std::string::npos) fin = linea.length();
                    return linea.substr(pos, fin - pos);
                };
                int node_id = -1;
                try { node_id = std::stoi(extraer_str("id")); } catch(...) {}
                std::string op_str = extraer_str("op");
                TipoNodo tn = (op_str == "OR") ? TipoNodo::OPERADOR_OR : TipoNodo::OPERADOR_AND;
                
                NodoEvento nuevo_grupo;
                nuevo_grupo.id = node_id;
                nuevo_grupo.tipo = tn;
                nuevo_grupo.cumplido = false;
                
                stack_nodos.push_back(nuevo_grupo);
                arbol_nuevo_cargado = true;
            } else if (linea.find("evt_cond ") != std::string::npos) {
                auto extraer_str = [&](const std::string& clave) -> std::string {
                    std::string buscar = clave + "=";
                    size_t pos = linea.find(buscar);
                    if (pos == std::string::npos) return "";
                    pos += buscar.length();
                    size_t fin = linea.find(' ', pos);
                    if (fin == std::string::npos) fin = linea.length();
                    return linea.substr(pos, fin - pos);
                };
                int node_id = -1;
                try { node_id = std::stoi(extraer_str("id")); } catch(...) {}
                CondicionEvento cond = deserializar_condicion_desde_linea(linea);
                
                NodoEvento nodo_cond;
                nodo_cond.id = node_id;
                nodo_cond.tipo = TipoNodo::CONDICION;
                nodo_cond.condicion = cond;
                nodo_cond.cumplido = false;
                
                if (!stack_nodos.empty()) {
                    stack_nodos.back().hijos.push_back(nodo_cond);
                } else {
                    gestor.raiz = nodo_cond;
                }
                arbol_nuevo_cargado = true;
            } else if (linea.find("evt_grupo_fin") != std::string::npos) {
                if (stack_nodos.size() > 1) {
                    NodoEvento hijo = stack_nodos.back();
                    stack_nodos.pop_back();
                    stack_nodos.back().hijos.push_back(hijo);
                } else if (stack_nodos.size() == 1) {
                    gestor.raiz = stack_nodos.back();
                    stack_nodos.pop_back();
                }
                arbol_nuevo_cargado = true;
            }
        } catch (...) {
            TraceLog(LOG_WARNING, "Error al procesar línea de snapshot: %s", linea.c_str());
        }
    }

    if (!arbol_nuevo_cargado) {
        gestor.limpiar();
        gestor.raiz.tipo = (old_logica == TipoLogicaVictoria::TODAS) ? TipoNodo::OPERADOR_AND : TipoNodo::OPERADOR_OR;
        gestor.raiz.id = -1;
        for (const auto& cond : old_condiciones) {
            gestor.agregar_condicion(gestor.raiz.id, cond);
        }
    }

    motor.set_gravedad(Vector2D(0, gravedad_y));
    motor.set_siguiente_id(max_id + 1);
    limpiar_estado_tras_cargar_partida();

    // Apagar ventiladores controlados por Banda
    {
        const auto& ents = motor.get_entidades();
        for (auto* e : ents) {
            auto* banda = dynamic_cast<Banda*>(e);
            if (!banda) continue;
            for (auto* e2 : ents) {
                if (e2->get_id() != banda->get_id_destino()) continue;
                auto* vent = dynamic_cast<Ventilador*>(e2);
                if (vent) vent->set_controlado_por_banda(true);
                break;
            }
        }
    }

    TraceLog(LOG_INFO, "Snapshot de simulación restaurado.");
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

    // Calcular las mismas dimensiones de panel y botones que en dibujar_panel_guardado
    int px = ancho - 300; // MENU_ANCHO = 300
    int py_base = alto - 50 - 180 - 215; // MENU_PAGINACION_ALTO = 50, MENU_PAGINACION_ALTO - 180 - 215
    const int alto_panel = 200;
    Rectangle panel = {static_cast<float>(px + 8), static_cast<float>(py_base),
                       static_cast<float>(300 - 16), static_cast<float>(alto_panel)};
    
    Rectangle btn_guardar = {panel.x + 10, panel.y + 10, panel.width - 20, 32};
    Rectangle btn_ver = {panel.x + 10, panel.y + 50, panel.width - 20, 32};

    // Sincronizar variables globales por consistencia
    rect_panel_guardado = panel;
    rect_btn_guardar_partida = btn_guardar;
    rect_btn_ver_partidas = btn_ver;

    if (modo_panel_guardado == ModoPanelGuardado::PEDIR_NOMBRE_GUARDAR) {
        if (CheckCollisionPointRec(p_click, panel)) {
            return true;
        }
        return false;
    }

    if (modo_panel_guardado == ModoPanelGuardado::LISTA_PARTIDAS) {
        float ly = btn_ver.y + 62;
        for (size_t i = 0; i < partidas_guardadas.size() && i < 4; ++i) {
            Rectangle fila = {btn_ver.x, ly, btn_ver.width, 22};
            if (CheckCollisionPointRec(p_click, fila)) {
                cargar_partida(motor, gestor, partidas_guardadas[i].ruta_archivo, ancho, alto, contador_bolas);
                modo_panel_guardado = ModoPanelGuardado::CERRADO;
                return true;
            }
            ly += 24;
        }
        if (CheckCollisionPointRec(p_click, panel)) {
            return true; // Consumir click para que no actúe sobre el canvas
        }
        return false;
    }

    if (modo_panel_guardado != ModoPanelGuardado::CERRADO) return false;

    if (CheckCollisionPointRec(p_click, btn_guardar)) {
        buffer_nombre_partida[0] = '\0';
        modo_panel_guardado = ModoPanelGuardado::PEDIR_NOMBRE_GUARDAR;
        return true;
    }
    if (CheckCollisionPointRec(p_click, btn_ver)) {
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
