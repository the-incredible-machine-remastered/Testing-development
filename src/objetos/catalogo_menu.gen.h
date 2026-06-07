#pragma once
// ============================================================================
// GENERADO AUTOMATICAMENTE — NO EDITAR A MANO
// Ejecutar: python tools/generar_catalogo_menu.py
// Fuente: etiquetas // TIM_MENU_SPAWN en src/objetos/*.h
// ============================================================================

enum class TipoObjetoMenu {
    NINGUNO = -1,
    BALANCIN,
    BOLA,
    BOLA_REBOTADORA,
    CUBETA,
    CUERDA,
    PARED_LARGA,
    PLATAFORMA,
    RAMPA,
    SOPORTE_TORQUE,
    TRAMPOLIN,
    BARRIL_CHAVO,
    SEGUIDOR_BOOSTER,
    VENTILADOR,
    ZONA_META,
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
    { TipoObjetoMenu::BOLA, "Bola", 0, 0, 0, true },
    { TipoObjetoMenu::BOLA_REBOTADORA, "Bola Reb.", 0, 0, 0, true },
    { TipoObjetoMenu::CUBETA, "Cubeta", 0, 0, 0, true },
    { TipoObjetoMenu::CUERDA, "Cuerda", 0, 0, 0, true },
    { TipoObjetoMenu::PARED_LARGA, "Pared", 0, 0, 0, true },
    { TipoObjetoMenu::PLATAFORMA, "Plataforma", 1, 0, 0, true },
    { TipoObjetoMenu::RAMPA, "Rampa", 1, 0, 0, true },
    { TipoObjetoMenu::SOPORTE_TORQUE, "Torque", 1, 0, 0, true },
    { TipoObjetoMenu::TRAMPOLIN, "Trampolin", 1, 0, 0, true },
    { TipoObjetoMenu::BARRIL_CHAVO, "Barril", 0, 0, 1, true },
    { TipoObjetoMenu::SEGUIDOR_BOOSTER, "Futbolista", 0, 0, 1, true },
    { TipoObjetoMenu::VENTILADOR, "Ventilador", 0, 0, 1, true },
    { TipoObjetoMenu::ZONA_META, "Zona Meta", 0, 0, 1, true },
    { TipoObjetoMenu::PLATAFORMA_DECOR, "Ladrillo", 0, 1, 0, true },
};

static const int CATALOGO_MENU_COUNT =
    static_cast<int>(sizeof(CATALOGO_MENU) / sizeof(CATALOGO_MENU[0]));
