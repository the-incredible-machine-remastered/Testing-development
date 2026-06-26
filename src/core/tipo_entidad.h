#pragma once

enum class TipoEntidadJuego {
    CUALQUIERA,
    BOLA,
    BOLA_REBOTADORA,
    BOLA_BEISBOL,
    TRAMPOLIN,
    BALANCIN,
    PARED,
    RAMPA,
    VENTILADOR,
    SEGUIDOR,
    BARRIL,
    CUBETA,
    SOPORTE,
    GLOBO,
    TIJERA,
    ZONA_META,
    CUERDA,
    GANCHO,
    PISTOLA
};

inline const char* nombre_tipo_entidad(TipoEntidadJuego t) {
    switch (t) {
        case TipoEntidadJuego::CUALQUIERA:      return "Cualquiera";
        case TipoEntidadJuego::BOLA:             return "Bola";
        case TipoEntidadJuego::BOLA_REBOTADORA:  return "Bola Reb.";
        case TipoEntidadJuego::BOLA_BEISBOL:     return "Beisbol";
        case TipoEntidadJuego::TRAMPOLIN:        return "Trampolin";
        case TipoEntidadJuego::BALANCIN:         return "Balancin";
        case TipoEntidadJuego::PARED:            return "Pared";
        case TipoEntidadJuego::RAMPA:            return "Rampa";
        case TipoEntidadJuego::VENTILADOR:       return "Ventilador";
        case TipoEntidadJuego::SEGUIDOR:         return "Futbolista";
        case TipoEntidadJuego::BARRIL:           return "Barril";
        case TipoEntidadJuego::CUBETA:           return "Cubeta";
        case TipoEntidadJuego::SOPORTE:          return "Soporte";
        case TipoEntidadJuego::GLOBO:            return "Globo";
        case TipoEntidadJuego::TIJERA:           return "Tijera";
        case TipoEntidadJuego::ZONA_META:        return "Zona Meta";
        case TipoEntidadJuego::CUERDA:           return "Cuerda";
        case TipoEntidadJuego::GANCHO:           return "Gancho";
        case TipoEntidadJuego::PISTOLA:          return "Pistola";
        default: return "???";
    }
}
