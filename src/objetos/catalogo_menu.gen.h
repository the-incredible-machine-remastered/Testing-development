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
    BOLA_REBOTADORA,
    CUBETA,
    CUERDA,
    PARED_LARGA,
    PLATAFORMA,
    RAMPA,
    SOPORTE_TORQUE,
    TRAMPOLIN,
    BARRIL_CHAVO,
    GENERADOR_MOTOR,
    RUEDA_HAMSTER,
    SEGUIDOR_BOOSTER,
    VENTILADOR,
    ZONA_META,
    CINTA_TRANSPORTADORA,
    CORREA,
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
    { TipoObjetoMenu::BOLA_REBOTADORA, "Bola Reb.", 0, 0, 0, true },
    { TipoObjetoMenu::CUBETA, "Cubeta", 1, 0, 0, true },
    { TipoObjetoMenu::CUERDA, "Cuerda", 1, 0, 0, true },
    { TipoObjetoMenu::PARED_LARGA, "Pared", 1, 0, 0, true },
    { TipoObjetoMenu::PLATAFORMA, "Plataforma", 1, 0, 0, true },
    { TipoObjetoMenu::RAMPA, "Rampa", 1, 0, 0, true },
    { TipoObjetoMenu::SOPORTE_TORQUE, "Torque", 1, 0, 0, true },
    { TipoObjetoMenu::TRAMPOLIN, "Trampolin", 2, 0, 0, true },
    { TipoObjetoMenu::BARRIL_CHAVO, "Barril", 0, 0, 1, true },
    { TipoObjetoMenu::GENERADOR_MOTOR, "Generador", 0, 0, 1, true },
    { TipoObjetoMenu::RUEDA_HAMSTER, "Rueda Hamster", 0, 0, 1, true },
    { TipoObjetoMenu::SEGUIDOR_BOOSTER, "Futbolista", 0, 0, 1, true },
    { TipoObjetoMenu::VENTILADOR, "Ventilador", 0, 0, 1, true },
    { TipoObjetoMenu::ZONA_META, "Zona Meta", 0, 0, 1, true },
    { TipoObjetoMenu::CINTA_TRANSPORTADORA, "Cinta Transp.", 0, 1, 0, true },
    { TipoObjetoMenu::CORREA, "Correa", 0, 1, 0, true },
    { TipoObjetoMenu::PLATAFORMA_DECOR, "Ladrillo", 0, 1, 0, true },
};

static const int CATALOGO_MENU_COUNT =
    static_cast<int>(sizeof(CATALOGO_MENU) / sizeof(CATALOGO_MENU[0]));
