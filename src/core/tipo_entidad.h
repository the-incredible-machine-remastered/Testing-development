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
    PISTOLA,
    CAJA_HAMSTER,
    BANDA,
    CAJA_SORPRESA,
    CAMINADORA,
    FOCO,
    LUPA,
    CANON,
    LADRILLO,
    DINAMITA,
    DINAMITA_DETONADOR,
    EXPLOSION,
    GATO,
    RATON
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
        case TipoEntidadJuego::CAJA_HAMSTER:     return "Hamster";
        case TipoEntidadJuego::BANDA:            return "Banda";
        case TipoEntidadJuego::CAJA_SORPRESA:   return "CajaSorpresa";
        case TipoEntidadJuego::CAMINADORA:        return "Caminadora";
        case TipoEntidadJuego::FOCO:              return "Foco";
        case TipoEntidadJuego::LUPA:              return "Lupa";
        case TipoEntidadJuego::CANON:            return "Canon";
        case TipoEntidadJuego::LADRILLO:         return "Ladrillo";
        case TipoEntidadJuego::DINAMITA:         return "Dinamita";
        case TipoEntidadJuego::DINAMITA_DETONADOR: return "Dina Detonador";
        case TipoEntidadJuego::EXPLOSION:        return "Explosion";
        case TipoEntidadJuego::GATO:             return "Gato";
        case TipoEntidadJuego::RATON:            return "Raton";
        default: return "???";
    }
}

extern EstadoJuego estado_actual;
