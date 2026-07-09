#pragma once
// ============================================================================
// GENERADO AUTOMATICAMENTE — NO EDITAR A MANO
// Ejecutar: python tools/generar_catalogo_menu.py
// Fuente: etiquetas // TIM_MENU_SPAWN en src/objetos/*.h
// ============================================================================

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
