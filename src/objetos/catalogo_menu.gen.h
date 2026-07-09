#pragma once
// ============================================================================
// GENERADO AUTOMATICAMENTE — NO EDITAR A MANO
// Ejecutar: python tools/generar_catalogo_menu.py
// Fuente: etiquetas // TIM_MENU_SPAWN en src/objetos/*.h
// ============================================================================

#include <string>
#include <unordered_map>

enum class TipoObjetoMenu {
    NINGUNO = -1,
    BALANCIN,
    BOLA_BOLOS,
    BOLA_NORMAL,
    BOLA_PLAYA,
    BOLA_TENIS,
    BOLA_BEISBOL,
    BOLA_REBOTADORA,
    CANON,
    CUBETA,
    CUERDA,
    ESCALON,
    FOCO,
    GANCHO,
    GLOBO,
    LUPA,
    PARED_LARGA,
    PLATAFORMA,
    PISTOLA,
    RAMPA,
    SOPORTE_TORQUE,
    TIJERA,
    TRAMPOLIN,
    BARRIL_CHAVO,
    CAJA_SORPRESA,
    CAMINADORA,
    GATO,
    GENERADOR_MOTOR,
    RATON,
    RUEDA_HAMSTER,
    SEGUIDOR_BOOSTER,
    VENTILADOR,
    ZONA_META,
    CINTA_TRANSPORTADORA,
    CORREA,
    DINAMITA,
    DINAMITA_DETONADOR,
    LADRILLO_HORIZONTAL,
    LADRILLO_VERTICAL,
    PLATAFORMA_DECOR,
    COUNT
};

struct ItemCatalogo {
    TipoObjetoMenu tipo;
    const char* etiqueta;
    int pagina;
    int tab;
    int categoria;
    bool disponible;
};

static const ItemCatalogo CATALOGO_MENU[] = {
    { TipoObjetoMenu::BALANCIN, "Balancin", 0, 0, 0, true },
    { TipoObjetoMenu::BOLA_BOLOS, "Balon Bolos", 0, 0, 0, true },
    { TipoObjetoMenu::BOLA_NORMAL, "Balon Normal", 0, 0, 0, true },
    { TipoObjetoMenu::BOLA_PLAYA, "Balon Playa", 0, 0, 0, true },
    { TipoObjetoMenu::BOLA_TENIS, "Pelota Tenis", 0, 0, 0, true },
    { TipoObjetoMenu::BOLA_BEISBOL, "Beisbol", 0, 0, 0, true },
    { TipoObjetoMenu::BOLA_REBOTADORA, "Bola Reb.", 1, 0, 0, true },
    { TipoObjetoMenu::CANON, "Canon", 1, 0, 0, true },
    { TipoObjetoMenu::CUBETA, "Cubeta", 1, 0, 0, true },
    { TipoObjetoMenu::CUERDA, "Cuerda", 1, 0, 0, true },
    { TipoObjetoMenu::ESCALON, "Escalon", 1, 0, 0, true },
    { TipoObjetoMenu::FOCO, "Foco", 1, 0, 0, true },
    { TipoObjetoMenu::GANCHO, "Gancho", 2, 0, 0, true },
    { TipoObjetoMenu::GLOBO, "Globo", 2, 0, 0, true },
    { TipoObjetoMenu::LUPA, "Lupa", 2, 0, 0, true },
    { TipoObjetoMenu::PARED_LARGA, "Pared", 2, 0, 0, true },
    { TipoObjetoMenu::PLATAFORMA, "Plataforma", 2, 0, 0, true },
    { TipoObjetoMenu::PISTOLA, "Pistola", 2, 0, 0, true },
    { TipoObjetoMenu::RAMPA, "Rampa", 3, 0, 0, true },
    { TipoObjetoMenu::SOPORTE_TORQUE, "Torque", 3, 0, 0, true },
    { TipoObjetoMenu::TIJERA, "Tijera", 3, 0, 0, true },
    { TipoObjetoMenu::TRAMPOLIN, "Trampolin", 3, 0, 0, true },
    { TipoObjetoMenu::BARRIL_CHAVO, "Barril", 0, 0, 1, true },
    { TipoObjetoMenu::CAJA_SORPRESA, "CajaSorpresa", 0, 0, 1, true },
    { TipoObjetoMenu::CAMINADORA, "Caminadora", 0, 0, 1, true },
    { TipoObjetoMenu::GATO, "Gato", 0, 0, 1, true },
    { TipoObjetoMenu::GENERADOR_MOTOR, "Generador", 0, 0, 1, true },
    { TipoObjetoMenu::RATON, "Raton", 0, 0, 1, true },
    { TipoObjetoMenu::RUEDA_HAMSTER, "Rueda Hamster", 1, 0, 1, true },
    { TipoObjetoMenu::SEGUIDOR_BOOSTER, "Futbolista", 1, 0, 1, true },
    { TipoObjetoMenu::VENTILADOR, "Ventilador", 1, 0, 1, true },
    { TipoObjetoMenu::ZONA_META, "Zona Meta", 1, 0, 1, true },
    { TipoObjetoMenu::CINTA_TRANSPORTADORA, "Cinta Transp.", 0, 1, 0, true },
    { TipoObjetoMenu::CORREA, "Correa", 0, 1, 0, true },
    { TipoObjetoMenu::DINAMITA, "Dinamita", 0, 1, 0, true },
    { TipoObjetoMenu::DINAMITA_DETONADOR, "Dina Detonador", 0, 1, 0, true },
    { TipoObjetoMenu::LADRILLO_HORIZONTAL, "Ladrillo H", 0, 1, 0, true },
    { TipoObjetoMenu::LADRILLO_VERTICAL, "Ladrillo V", 0, 1, 0, true },
    { TipoObjetoMenu::PLATAFORMA_DECOR, "Deco", 1, 1, 0, true },
};

static const int CATALOGO_MENU_COUNT =
    static_cast<int>(sizeof(CATALOGO_MENU) / sizeof(CATALOGO_MENU[0]));

inline const char* tipo_objeto_menu_a_string(TipoObjetoMenu t) {
    switch (t) {
        case TipoObjetoMenu::BALANCIN: return "BALANCIN";
        case TipoObjetoMenu::BOLA_BOLOS: return "BOLA_BOLOS";
        case TipoObjetoMenu::BOLA_NORMAL: return "BOLA_NORMAL";
        case TipoObjetoMenu::BOLA_PLAYA: return "BOLA_PLAYA";
        case TipoObjetoMenu::BOLA_TENIS: return "BOLA_TENIS";
        case TipoObjetoMenu::BOLA_BEISBOL: return "BOLA_BEISBOL";
        case TipoObjetoMenu::BOLA_REBOTADORA: return "BOLA_REBOTADORA";
        case TipoObjetoMenu::CANON: return "CANON";
        case TipoObjetoMenu::CUBETA: return "CUBETA";
        case TipoObjetoMenu::CUERDA: return "CUERDA";
        case TipoObjetoMenu::ESCALON: return "ESCALON";
        case TipoObjetoMenu::FOCO: return "FOCO";
        case TipoObjetoMenu::GANCHO: return "GANCHO";
        case TipoObjetoMenu::GLOBO: return "GLOBO";
        case TipoObjetoMenu::LUPA: return "LUPA";
        case TipoObjetoMenu::PARED_LARGA: return "PARED_LARGA";
        case TipoObjetoMenu::PLATAFORMA: return "PLATAFORMA";
        case TipoObjetoMenu::PISTOLA: return "PISTOLA";
        case TipoObjetoMenu::RAMPA: return "RAMPA";
        case TipoObjetoMenu::SOPORTE_TORQUE: return "SOPORTE_TORQUE";
        case TipoObjetoMenu::TIJERA: return "TIJERA";
        case TipoObjetoMenu::TRAMPOLIN: return "TRAMPOLIN";
        case TipoObjetoMenu::BARRIL_CHAVO: return "BARRIL_CHAVO";
        case TipoObjetoMenu::CAJA_SORPRESA: return "CAJA_SORPRESA";
        case TipoObjetoMenu::CAMINADORA: return "CAMINADORA";
        case TipoObjetoMenu::GATO: return "GATO";
        case TipoObjetoMenu::GENERADOR_MOTOR: return "GENERADOR_MOTOR";
        case TipoObjetoMenu::RATON: return "RATON";
        case TipoObjetoMenu::RUEDA_HAMSTER: return "RUEDA_HAMSTER";
        case TipoObjetoMenu::SEGUIDOR_BOOSTER: return "SEGUIDOR_BOOSTER";
        case TipoObjetoMenu::VENTILADOR: return "VENTILADOR";
        case TipoObjetoMenu::ZONA_META: return "ZONA_META";
        case TipoObjetoMenu::CINTA_TRANSPORTADORA: return "CINTA_TRANSPORTADORA";
        case TipoObjetoMenu::CORREA: return "CORREA";
        case TipoObjetoMenu::DINAMITA: return "DINAMITA";
        case TipoObjetoMenu::DINAMITA_DETONADOR: return "DINAMITA_DETONADOR";
        case TipoObjetoMenu::LADRILLO_HORIZONTAL: return "LADRILLO_HORIZONTAL";
        case TipoObjetoMenu::LADRILLO_VERTICAL: return "LADRILLO_VERTICAL";
        case TipoObjetoMenu::PLATAFORMA_DECOR: return "PLATAFORMA_DECOR";
        default: return "NINGUNO";
    }
}

inline TipoObjetoMenu string_a_tipo_objeto_menu(const std::string& s) {
    static const std::unordered_map<std::string, TipoObjetoMenu> tabla = {
        {"BALANCIN", TipoObjetoMenu::BALANCIN},
        {"BOLA_BOLOS", TipoObjetoMenu::BOLA_BOLOS},
        {"BOLA_NORMAL", TipoObjetoMenu::BOLA_NORMAL},
        {"BOLA_PLAYA", TipoObjetoMenu::BOLA_PLAYA},
        {"BOLA_TENIS", TipoObjetoMenu::BOLA_TENIS},
        {"BOLA_BEISBOL", TipoObjetoMenu::BOLA_BEISBOL},
        {"BOLA_REBOTADORA", TipoObjetoMenu::BOLA_REBOTADORA},
        {"CANON", TipoObjetoMenu::CANON},
        {"CUBETA", TipoObjetoMenu::CUBETA},
        {"CUERDA", TipoObjetoMenu::CUERDA},
        {"ESCALON", TipoObjetoMenu::ESCALON},
        {"FOCO", TipoObjetoMenu::FOCO},
        {"GANCHO", TipoObjetoMenu::GANCHO},
        {"GLOBO", TipoObjetoMenu::GLOBO},
        {"LUPA", TipoObjetoMenu::LUPA},
        {"PARED_LARGA", TipoObjetoMenu::PARED_LARGA},
        {"PLATAFORMA", TipoObjetoMenu::PLATAFORMA},
        {"PISTOLA", TipoObjetoMenu::PISTOLA},
        {"RAMPA", TipoObjetoMenu::RAMPA},
        {"SOPORTE_TORQUE", TipoObjetoMenu::SOPORTE_TORQUE},
        {"TIJERA", TipoObjetoMenu::TIJERA},
        {"TRAMPOLIN", TipoObjetoMenu::TRAMPOLIN},
        {"BARRIL_CHAVO", TipoObjetoMenu::BARRIL_CHAVO},
        {"CAJA_SORPRESA", TipoObjetoMenu::CAJA_SORPRESA},
        {"CAMINADORA", TipoObjetoMenu::CAMINADORA},
        {"GATO", TipoObjetoMenu::GATO},
        {"GENERADOR_MOTOR", TipoObjetoMenu::GENERADOR_MOTOR},
        {"RATON", TipoObjetoMenu::RATON},
        {"RUEDA_HAMSTER", TipoObjetoMenu::RUEDA_HAMSTER},
        {"SEGUIDOR_BOOSTER", TipoObjetoMenu::SEGUIDOR_BOOSTER},
        {"VENTILADOR", TipoObjetoMenu::VENTILADOR},
        {"ZONA_META", TipoObjetoMenu::ZONA_META},
        {"CINTA_TRANSPORTADORA", TipoObjetoMenu::CINTA_TRANSPORTADORA},
        {"CORREA", TipoObjetoMenu::CORREA},
        {"DINAMITA", TipoObjetoMenu::DINAMITA},
        {"DINAMITA_DETONADOR", TipoObjetoMenu::DINAMITA_DETONADOR},
        {"LADRILLO_HORIZONTAL", TipoObjetoMenu::LADRILLO_HORIZONTAL},
        {"LADRILLO_VERTICAL", TipoObjetoMenu::LADRILLO_VERTICAL},
        {"PLATAFORMA_DECOR", TipoObjetoMenu::PLATAFORMA_DECOR},
    };
    auto it = tabla.find(s);
    if (it != tabla.end()) return it->second;
    return TipoObjetoMenu::NINGUNO;
}
