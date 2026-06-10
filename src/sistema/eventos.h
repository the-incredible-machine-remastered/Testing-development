#pragma once
#include "core/entidad_fisica.h"
#include "core/registro_eventos.h"
#include "objetos/zona_meta.h"
#include "objetos/bola.h"
#include "objetos/bola_rebotadora.h"
#include "objetos/trampolin.h"
#include "objetos/balancin.h"
#include "objetos/pared_rectangular.h"
#include "objetos/plano_inclinado.h"
#include "objetos/ventilador.h"
#include "objetos/seguidor_booster.h"
#include "objetos/barril_chavo.h"
#include "objetos/cubeta.h"
#include "objetos/soporte_torque.h"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

enum class TipoCondicion {
    CONTACTO_ENTIDADES,
    CONTACTO_TIPOS,
    CABEZAZO,
    PATADA,
    ENTIDAD_EN_ZONA,
    TIPO_EN_ZONA,
    BARRIL_ACTIVADO,
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
        default: return "???";
    }
}

inline TipoEntidadJuego clasificar_entidad(EntidadFisica* e) {
    if (dynamic_cast<Bola*>(e))              return TipoEntidadJuego::BOLA;
    if (dynamic_cast<BolaRebotadora*>(e))    return TipoEntidadJuego::BOLA_REBOTADORA;
    if (dynamic_cast<Trampolin*>(e))         return TipoEntidadJuego::TRAMPOLIN;
    if (dynamic_cast<Balancin*>(e))          return TipoEntidadJuego::BALANCIN;
    if (dynamic_cast<ParedRectangular*>(e))  return TipoEntidadJuego::PARED;
    if (dynamic_cast<PlanoInclinado*>(e))    return TipoEntidadJuego::RAMPA;
    if (dynamic_cast<Ventilador*>(e))        return TipoEntidadJuego::VENTILADOR;
    if (dynamic_cast<SeguidorBooster*>(e))   return TipoEntidadJuego::SEGUIDOR;
    if (dynamic_cast<BarrilChavo*>(e))       return TipoEntidadJuego::BARRIL;
    if (dynamic_cast<Cubeta*>(e))            return TipoEntidadJuego::CUBETA;
    if (dynamic_cast<SoporteTorque*>(e))     return TipoEntidadJuego::SOPORTE;
    if (dynamic_cast<ZonaMeta*>(e))          return TipoEntidadJuego::ZONA_META;
    return TipoEntidadJuego::CUALQUIERA;
}

struct CondicionEvento {
    TipoCondicion tipo;

    int id_entidad_a = -1;
    int id_entidad_b = -1;

    TipoEntidadJuego tipo_a = TipoEntidadJuego::CUALQUIERA;
    TipoEntidadJuego tipo_b = TipoEntidadJuego::CUALQUIERA;

    int id_seguidor = -1;
    int id_bola = -1;

    int id_zona_meta = -1;
    int id_entidad_zona = -1;
    TipoEntidadJuego tipo_zona = TipoEntidadJuego::BOLA;

    int id_barril = -1;

    std::string describir(const std::vector<EntidadFisica*>& entidades) const {
        switch (tipo) {
            case TipoCondicion::CONTACTO_ENTIDADES:
                return "Contacto #" + std::to_string(id_entidad_a) +
                       " y #" + std::to_string(id_entidad_b);
            case TipoCondicion::CONTACTO_TIPOS:
                return std::string("Contacto ") + nombre_tipo_entidad(tipo_a) +
                       " con " + nombre_tipo_entidad(tipo_b);
            case TipoCondicion::CABEZAZO:
                return "Cabezazo" + std::string(id_seguidor >= 0 ?
                       " (#" + std::to_string(id_seguidor) + ")" : "");
            case TipoCondicion::PATADA:
                return "Patada" + std::string(id_seguidor >= 0 ?
                       " (#" + std::to_string(id_seguidor) + ")" : "");
            case TipoCondicion::ENTIDAD_EN_ZONA:
                return "#" + std::to_string(id_entidad_zona) +
                       " en zona #" + std::to_string(id_zona_meta);
            case TipoCondicion::TIPO_EN_ZONA:
                return std::string(nombre_tipo_entidad(tipo_zona)) +
                       " en zona #" + std::to_string(id_zona_meta);
            case TipoCondicion::BARRIL_ACTIVADO:
                return "Barril #" + std::to_string(id_barril) + " activado";
            default: return "???";
        }
    }
};

enum class TipoAccion {
    VICTORIA
};

struct AccionEvento {
    TipoAccion tipo = TipoAccion::VICTORIA;

    std::string describir() const {
        switch (tipo) {
            case TipoAccion::VICTORIA: return "Victoria!";
            default: return "???";
        }
    }
};

struct EventoJuego {
    int id;
    CondicionEvento condicion;
    AccionEvento accion;
    bool disparado = false;
    bool una_sola_vez = true;
};

enum class TipoLogicaVictoria {
    CUALQUIERA,
    TODAS
};

class GestorEventos {
public:
    std::vector<EventoJuego> eventos;
    TipoLogicaVictoria logica_victoria = TipoLogicaVictoria::CUALQUIERA;
    bool victoria_alcanzada = false;
    float timer_victoria = 0.0f;
    int siguiente_id_evento = 1;

    void limpiar() {
        eventos.clear();
        victoria_alcanzada = false;
        timer_victoria = 0.0f;
        siguiente_id_evento = 1;
    }

    int agregar_evento(CondicionEvento cond, AccionEvento accion) {
        int id = siguiente_id_evento++;
        eventos.push_back({id, cond, accion, false, true});
        return id;
    }

    void remover_evento(int id_evento) {
        eventos.erase(
            std::remove_if(eventos.begin(), eventos.end(),
                [id_evento](const EventoJuego& e) { return e.id == id_evento; }),
            eventos.end()
        );
    }

    void reiniciar_estados() {
        for (auto& ev : eventos) {
            ev.disparado = false;
        }
        victoria_alcanzada = false;
        timer_victoria = 0.0f;
    }

    void evaluar(
        const std::vector<RegistroColision>& colisiones,
        const std::vector<RegistroEventoEspecial>& eventos_especiales,
        const std::vector<EntidadFisica*>& entidades,
        float dt
    ) {
        if (victoria_alcanzada) {
            timer_victoria += dt;
            return;
        }

        if (eventos.empty()) return;

        bool todos_cumplidos = true;
        bool alguno_cumplido_nuevo = false;

        for (auto& ev : eventos) {
            if (!ev.disparado || !ev.una_sola_vez) {
                bool cumplida = evaluar_condicion(
                    ev.condicion, colisiones, eventos_especiales, entidades
                );
                if (cumplida) {
                    ev.disparado = true;
                    alguno_cumplido_nuevo = true;
                }
            }
            if (!ev.disparado) {
                todos_cumplidos = false;
            }
        }

        if (logica_victoria == TipoLogicaVictoria::CUALQUIERA) {
            if (alguno_cumplido_nuevo) {
                ejecutar_accion(AccionEvento{TipoAccion::VICTORIA});
            }
        } else if (logica_victoria == TipoLogicaVictoria::TODAS) {
            if (todos_cumplidos) {
                ejecutar_accion(AccionEvento{TipoAccion::VICTORIA});
            }
        }
    }

private:
    bool evaluar_condicion(
        const CondicionEvento& cond,
        const std::vector<RegistroColision>& colisiones,
        const std::vector<RegistroEventoEspecial>& eventos_especiales,
        const std::vector<EntidadFisica*>& entidades
    ) {
        switch (cond.tipo) {

        case TipoCondicion::CONTACTO_ENTIDADES:
            for (const auto& col : colisiones) {
                if ((col.id_a == cond.id_entidad_a && col.id_b == cond.id_entidad_b) ||
                    (col.id_a == cond.id_entidad_b && col.id_b == cond.id_entidad_a)) {
                    return true;
                }
            }
            return false;

        case TipoCondicion::CONTACTO_TIPOS:
            for (const auto& col : colisiones) {
                EntidadFisica* ea = buscar_entidad(col.id_a, entidades);
                EntidadFisica* eb = buscar_entidad(col.id_b, entidades);
                if (!ea || !eb) continue;
                TipoEntidadJuego ta = clasificar_entidad(ea);
                TipoEntidadJuego tb = clasificar_entidad(eb);
                if ((ta == cond.tipo_a && tb == cond.tipo_b) ||
                    (ta == cond.tipo_b && tb == cond.tipo_a)) {
                    return true;
                }
            }
            return false;

        case TipoCondicion::CABEZAZO:
            for (const auto& ev : eventos_especiales) {
                if (ev.tipo != TipoEventoEspecial::CABEZAZO) continue;
                if (cond.id_seguidor >= 0 && ev.id_origen != cond.id_seguidor) continue;
                if (cond.id_bola >= 0 && ev.id_objetivo != cond.id_bola) continue;
                return true;
            }
            return false;

        case TipoCondicion::PATADA:
            for (const auto& ev : eventos_especiales) {
                if (ev.tipo != TipoEventoEspecial::PATADA) continue;
                if (cond.id_seguidor >= 0 && ev.id_origen != cond.id_seguidor) continue;
                if (cond.id_bola >= 0 && ev.id_objetivo != cond.id_bola) continue;
                return true;
            }
            return false;

        case TipoCondicion::ENTIDAD_EN_ZONA: {
            ZonaMeta* zona = nullptr;
            EntidadFisica* ent = nullptr;
            for (auto* e : entidades) {
                if (e->get_id() == cond.id_zona_meta)
                    zona = dynamic_cast<ZonaMeta*>(e);
                if (e->get_id() == cond.id_entidad_zona)
                    ent = e;
            }
            if (!zona || !ent) return false;
            return verificar_en_zona(zona, ent);
        }

        case TipoCondicion::TIPO_EN_ZONA: {
            ZonaMeta* zona = nullptr;
            for (auto* e : entidades) {
                if (e->get_id() == cond.id_zona_meta) {
                    zona = dynamic_cast<ZonaMeta*>(e);
                    break;
                }
            }
            if (!zona) return false;
            for (auto* e : entidades) {
                if (clasificar_entidad(e) == cond.tipo_zona) {
                    if (verificar_en_zona(zona, e)) return true;
                }
            }
            return false;
        }

        case TipoCondicion::BARRIL_ACTIVADO:
            for (const auto& ev : eventos_especiales) {
                if (ev.tipo != TipoEventoEspecial::BARRIL_LANZADO) continue;
                if (cond.id_barril >= 0 && ev.id_origen != cond.id_barril) continue;
                return true;
            }
            return false;
        }

        return false;
    }

    bool verificar_en_zona(ZonaMeta* zona, EntidadFisica* ent) {
        if (auto* bola = dynamic_cast<Bola*>(ent)) {
            return zona->intersecta_circulo(bola->get_posicion(), bola->get_radio());
        } else if (auto* br = dynamic_cast<BolaRebotadora*>(ent)) {
            return zona->intersecta_circulo(br->get_posicion(), br->get_radio());
        } else if (auto* cub = dynamic_cast<Cubeta*>(ent)) {
            return zona->intersecta_aabb(cub->get_posicion(), cub->get_ancho(), cub->get_alto());
        }
        return zona->contiene_punto(ent->get_posicion());
    }

    void ejecutar_accion(const AccionEvento& accion) {
        switch (accion.tipo) {
        case TipoAccion::VICTORIA:
            victoria_alcanzada = true;
            timer_victoria = 0.0f;
            break;
        }
    }

    EntidadFisica* buscar_entidad(int id, const std::vector<EntidadFisica*>& ents) {
        for (auto* e : ents) {
            if (e->get_id() == id) return e;
        }
        return nullptr;
    }
};

inline const char* tipo_condicion_a_str(TipoCondicion t) {
    switch (t) {
        case TipoCondicion::CONTACTO_ENTIDADES: return "CONTACTO_ENT";
        case TipoCondicion::CONTACTO_TIPOS:     return "CONTACTO_TIPO";
        case TipoCondicion::CABEZAZO:            return "CABEZAZO";
        case TipoCondicion::PATADA:              return "PATADA";
        case TipoCondicion::ENTIDAD_EN_ZONA:     return "ENT_EN_ZONA";
        case TipoCondicion::TIPO_EN_ZONA:        return "TIPO_EN_ZONA";
        case TipoCondicion::BARRIL_ACTIVADO:     return "BARRIL_ACT";
        default: return "DESCONOCIDO";
    }
}

inline TipoCondicion str_a_tipo_condicion(const std::string& s) {
    if (s == "CONTACTO_ENT")  return TipoCondicion::CONTACTO_ENTIDADES;
    if (s == "CONTACTO_TIPO") return TipoCondicion::CONTACTO_TIPOS;
    if (s == "CABEZAZO")      return TipoCondicion::CABEZAZO;
    if (s == "PATADA")        return TipoCondicion::PATADA;
    if (s == "ENT_EN_ZONA")   return TipoCondicion::ENTIDAD_EN_ZONA;
    if (s == "TIPO_EN_ZONA")  return TipoCondicion::TIPO_EN_ZONA;
    if (s == "BARRIL_ACT")    return TipoCondicion::BARRIL_ACTIVADO;
    return TipoCondicion::CONTACTO_ENTIDADES;
}

inline const char* tipo_accion_a_str(TipoAccion t) {
    switch (t) {
        case TipoAccion::VICTORIA: return "VICTORIA";
        default: return "VICTORIA";
    }
}

inline TipoAccion str_a_tipo_accion(const std::string& s) {
    if (s == "VICTORIA") return TipoAccion::VICTORIA;
    return TipoAccion::VICTORIA;
}

inline const char* tipo_entidad_juego_a_str(TipoEntidadJuego t) {
    switch (t) {
        case TipoEntidadJuego::CUALQUIERA:     return "CUALQUIERA";
        case TipoEntidadJuego::BOLA:           return "BOLA";
        case TipoEntidadJuego::BOLA_REBOTADORA:return "BOLA_REB";
        case TipoEntidadJuego::TRAMPOLIN:      return "TRAMPOLIN";
        case TipoEntidadJuego::BALANCIN:       return "BALANCIN";
        case TipoEntidadJuego::PARED:          return "PARED";
        case TipoEntidadJuego::RAMPA:          return "RAMPA";
        case TipoEntidadJuego::VENTILADOR:     return "VENTILADOR";
        case TipoEntidadJuego::SEGUIDOR:       return "SEGUIDOR";
        case TipoEntidadJuego::BARRIL:         return "BARRIL";
        case TipoEntidadJuego::CUBETA:         return "CUBETA";
        case TipoEntidadJuego::SOPORTE:        return "SOPORTE";
        case TipoEntidadJuego::ZONA_META:      return "ZONA_META";
        default: return "CUALQUIERA";
    }
}

inline TipoEntidadJuego str_a_tipo_entidad_juego(const std::string& s) {
    if (s == "BOLA")       return TipoEntidadJuego::BOLA;
    if (s == "BOLA_REB")   return TipoEntidadJuego::BOLA_REBOTADORA;
    if (s == "TRAMPOLIN")  return TipoEntidadJuego::TRAMPOLIN;
    if (s == "BALANCIN")   return TipoEntidadJuego::BALANCIN;
    if (s == "PARED")      return TipoEntidadJuego::PARED;
    if (s == "RAMPA")      return TipoEntidadJuego::RAMPA;
    if (s == "VENTILADOR") return TipoEntidadJuego::VENTILADOR;
    if (s == "SEGUIDOR")   return TipoEntidadJuego::SEGUIDOR;
    if (s == "BARRIL")     return TipoEntidadJuego::BARRIL;
    if (s == "CUBETA")     return TipoEntidadJuego::CUBETA;
    if (s == "SOPORTE")    return TipoEntidadJuego::SOPORTE;
    if (s == "ZONA_META")  return TipoEntidadJuego::ZONA_META;
    return TipoEntidadJuego::CUALQUIERA;
}

inline std::string serializar_evento(const EventoJuego& ev) {
    std::ostringstream oss;
    oss << "evt " << tipo_condicion_a_str(ev.condicion.tipo);
    oss << " id=" << ev.id;
    oss << " accion=" << tipo_accion_a_str(ev.accion.tipo);

    const auto& c = ev.condicion;
    switch (c.tipo) {
        case TipoCondicion::CONTACTO_ENTIDADES:
            oss << " ea=" << c.id_entidad_a << " eb=" << c.id_entidad_b;
            break;
        case TipoCondicion::CONTACTO_TIPOS:
            oss << " ta=" << tipo_entidad_juego_a_str(c.tipo_a)
                << " tb=" << tipo_entidad_juego_a_str(c.tipo_b);
            break;
        case TipoCondicion::CABEZAZO:
        case TipoCondicion::PATADA:
            oss << " es=" << c.id_seguidor << " eb=" << c.id_bola;
            break;
        case TipoCondicion::ENTIDAD_EN_ZONA:
            oss << " ez=" << c.id_zona_meta << " ee=" << c.id_entidad_zona;
            break;
        case TipoCondicion::TIPO_EN_ZONA:
            oss << " ez=" << c.id_zona_meta
                << " te=" << tipo_entidad_juego_a_str(c.tipo_zona);
            break;
        case TipoCondicion::BARRIL_ACTIVADO:
            oss << " eb=" << c.id_barril;
            break;
    }
    return oss.str();
}

inline EventoJuego deserializar_evento(const std::string& linea) {
    EventoJuego ev;
    auto extraer_str = [&](const std::string& clave) -> std::string {
        std::string buscar = clave + "=";
        size_t pos = linea.find(buscar);
        if (pos == std::string::npos) return "";
        pos += buscar.length();
        size_t fin = linea.find(' ', pos);
        if (fin == std::string::npos) fin = linea.length();
        return linea.substr(pos, fin - pos);
    };
    auto extraer_int = [&](const std::string& clave) -> int {
        std::string val = extraer_str(clave);
        if (val.empty()) return -1;
        try {
            return std::stoi(val);
        } catch (...) {
            return -1;
        }
    };

    size_t pos1 = linea.find(' ');
    size_t pos2 = linea.find(' ', pos1 + 1);
    if (pos2 == std::string::npos) pos2 = linea.length();
    std::string tipo_str = linea.substr(pos1 + 1, pos2 - pos1 - 1);

    ev.condicion.tipo = str_a_tipo_condicion(tipo_str);
    ev.id = extraer_int("id");
    ev.accion.tipo = str_a_tipo_accion(extraer_str("accion"));

    switch (ev.condicion.tipo) {
        case TipoCondicion::CONTACTO_ENTIDADES:
            ev.condicion.id_entidad_a = extraer_int("ea");
            ev.condicion.id_entidad_b = extraer_int("eb");
            break;
        case TipoCondicion::CONTACTO_TIPOS:
            ev.condicion.tipo_a = str_a_tipo_entidad_juego(extraer_str("ta"));
            ev.condicion.tipo_b = str_a_tipo_entidad_juego(extraer_str("tb"));
            break;
        case TipoCondicion::CABEZAZO:
        case TipoCondicion::PATADA:
            ev.condicion.id_seguidor = extraer_int("es");
            ev.condicion.id_bola = extraer_int("eb");
            break;
        case TipoCondicion::ENTIDAD_EN_ZONA:
            ev.condicion.id_zona_meta = extraer_int("ez");
            ev.condicion.id_entidad_zona = extraer_int("ee");
            break;
        case TipoCondicion::TIPO_EN_ZONA:
            ev.condicion.id_zona_meta = extraer_int("ez");
            ev.condicion.tipo_zona = str_a_tipo_entidad_juego(extraer_str("te"));
            break;
        case TipoCondicion::BARRIL_ACTIVADO:
            ev.condicion.id_barril = extraer_int("eb");
            break;
    }

    return ev;
}
