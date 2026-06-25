#pragma once

enum class EstadoJuego {
    MENU_PRINCIPAL,
    MENU_OPCIONES,
    SELECCION_NIVELES,
    JUEGO_CREATIVO,
    JUEGO_NIVEL
};

enum class TipoEntidadJuego {
    CUALQUIERA,
    BOLA,
    BOLA_REBOTADORA,
    TRAMPOLIN,
    BALANCIN,
    PARED,
    RAMPA,
    VENTILADOR,
    SEGUIDOR,
    BARRIL,
    CUBETA,
    SOPORTE,
    ZONA_META,
    CUERDA
};

inline const char* nombre_tipo_entidad(TipoEntidadJuego t) {
    switch (t) {
        case TipoEntidadJuego::CUALQUIERA:      return "Cualquiera";
        case TipoEntidadJuego::BOLA:             return "Bola";
        case TipoEntidadJuego::BOLA_REBOTADORA:  return "Bola Reb.";
        case TipoEntidadJuego::TRAMPOLIN:        return "Trampolin";
        case TipoEntidadJuego::BALANCIN:         return "Balancin";
        case TipoEntidadJuego::PARED:            return "Pared";
        case TipoEntidadJuego::RAMPA:            return "Rampa";
        case TipoEntidadJuego::VENTILADOR:       return "Ventilador";
        case TipoEntidadJuego::SEGUIDOR:         return "Futbolista";
        case TipoEntidadJuego::BARRIL:           return "Barril";
        case TipoEntidadJuego::CUBETA:           return "Cubeta";
        case TipoEntidadJuego::SOPORTE:          return "Soporte";
        case TipoEntidadJuego::ZONA_META:        return "Zona Meta";
        case TipoEntidadJuego::CUERDA:           return "Cuerda";
        default: return "???";
    }
}

extern EstadoJuego estado_actual;
