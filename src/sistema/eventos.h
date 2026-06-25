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

inline TipoEntidadJuego clasificar_entidad(EntidadFisica* e) {
    if (!e) return TipoEntidadJuego::CUALQUIERA;
    return e->get_tipo_entidad();
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

enum class TipoNodo {
    CONDICION,
    OPERADOR_AND,
    OPERADOR_OR
};

struct NodoEvento {
    int id = -1;
    TipoNodo tipo = TipoNodo::CONDICION;
    CondicionEvento condicion;
    std::vector<NodoEvento> hijos;
    bool cumplido = false;

    void toggle_operador() {
        if (tipo == TipoNodo::OPERADOR_AND) tipo = TipoNodo::OPERADOR_OR;
        else if (tipo == TipoNodo::OPERADOR_OR) tipo = TipoNodo::OPERADOR_AND;
    }
};

enum class TipoLogicaVictoria {
    CUALQUIERA,
    TODAS
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

inline std::string describir_nodo_ecuacion(const NodoEvento& nodo, const std::vector<EntidadFisica*>& entidades) {
    if (nodo.tipo == TipoNodo::CONDICION) {
        return nodo.condicion.describir(entidades);
    }
    if (nodo.hijos.empty()) return "(vacio)";
    
    std::string op = (nodo.tipo == TipoNodo::OPERADOR_AND) ? " AND " : " OR ";
    std::string res = "(";
    for (size_t i = 0; i < nodo.hijos.size(); ++i) {
        if (i > 0) res += op;
        res += describir_nodo_ecuacion(nodo.hijos[i], entidades);
    }
    res += ")";
    return res;
}

class GestorEventos {
public:
    NodoEvento raiz;
    bool victoria_alcanzada = false;
    float timer_victoria = 0.0f;
    int siguiente_id_evento = 1;

    // Campo de retrocompatibilidad (para no romper otras partes si usan el viejo enum mientras refactorizamos)
    TipoLogicaVictoria logica_victoria = TipoLogicaVictoria::CUALQUIERA;

    GestorEventos() {
        limpiar();
    }

    void limpiar() {
        raiz = NodoEvento{-1, TipoNodo::OPERADOR_AND, {}, {}, false};
        victoria_alcanzada = false;
        timer_victoria = 0.0f;
        siguiente_id_evento = 1;
    }

    void reiniciar_estados() {
        reiniciar_estados_recursivo(raiz);
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

        actualizar_estados_nodos(raiz, colisiones, eventos_especiales, entidades);

        if (raiz.cumplido && !raiz.hijos.empty()) {
            victoria_alcanzada = true;
            timer_victoria = 0.0f;
        }
    }

    int agregar_condicion(int id_padre, CondicionEvento cond) {
        int nuevo_id = siguiente_id_evento++;
        NodoEvento* padre = buscar_nodo_por_id(raiz, id_padre);
        if (padre && padre->tipo != TipoNodo::CONDICION) {
            NodoEvento nuevo_nodo;
            nuevo_nodo.id = nuevo_id;
            nuevo_nodo.tipo = TipoNodo::CONDICION;
            nuevo_nodo.condicion = cond;
            nuevo_nodo.cumplido = false;
            padre->hijos.push_back(nuevo_nodo);
            return nuevo_id;
        }
        return -1;
    }

    int agregar_grupo(int id_padre, TipoNodo tipo_grupo) {
        int nuevo_id = siguiente_id_evento++;
        NodoEvento* padre = buscar_nodo_por_id(raiz, id_padre);
        if (padre && padre->tipo != TipoNodo::CONDICION) {
            NodoEvento nuevo_nodo;
            nuevo_nodo.id = nuevo_id;
            nuevo_nodo.tipo = tipo_grupo;
            nuevo_nodo.cumplido = false;
            padre->hijos.push_back(nuevo_nodo);
            return nuevo_id;
        }
        return -1;
    }

    void remover_nodo(int id_nodo) {
        if (raiz.id == id_nodo) {
            raiz.hijos.clear();
            raiz.cumplido = false;
        } else {
            remover_nodo_por_id(raiz, id_nodo);
        }
    }

    NodoEvento* buscar_nodo(int id_nodo) {
        return buscar_nodo_por_id(raiz, id_nodo);
    }

private:
    void reiniciar_estados_recursivo(NodoEvento& nodo) {
        nodo.cumplido = false;
        for (auto& h : nodo.hijos) {
            reiniciar_estados_recursivo(h);
        }
    }

    NodoEvento* buscar_nodo_por_id(NodoEvento& actual, int id_buscar) {
        if (actual.id == id_buscar) return &actual;
        for (auto& h : actual.hijos) {
            NodoEvento* res = buscar_nodo_por_id(h, id_buscar);
            if (res) return res;
        }
        return nullptr;
    }

    bool remover_nodo_por_id(NodoEvento& actual, int id_remover) {
        for (auto it = actual.hijos.begin(); it != actual.hijos.end(); ++it) {
            if (it->id == id_remover) {
                actual.hijos.erase(it);
                return true;
            }
            if (remover_nodo_por_id(*it, id_remover)) {
                return true;
            }
        }
        return false;
    }

    void actualizar_estados_nodos(
        NodoEvento& nodo,
        const std::vector<RegistroColision>& colisiones,
        const std::vector<RegistroEventoEspecial>& eventos_especiales,
        const std::vector<EntidadFisica*>& entidades
    ) {
        if (nodo.tipo == TipoNodo::CONDICION) {
            if (!nodo.cumplido) {
                nodo.cumplido = evaluar_condicion(nodo.condicion, colisiones, eventos_especiales, entidades);
            }
        } else {
            for (auto& h : nodo.hijos) {
                actualizar_estados_nodos(h, colisiones, eventos_especiales, entidades);
            }
            if (nodo.hijos.empty()) {
                nodo.cumplido = false;
            } else if (nodo.tipo == TipoNodo::OPERADOR_AND) {
                bool todo = true;
                for (const auto& h : nodo.hijos) {
                    if (!h.cumplido) {
                        todo = false;
                        break;
                    }
                }
                nodo.cumplido = todo;
            } else if (nodo.tipo == TipoNodo::OPERADOR_OR) {
                bool alguno = false;
                for (const auto& h : nodo.hijos) {
                    if (h.cumplido) {
                        alguno = true;
                        break;
                    }
                }
                nodo.cumplido = alguno;
            }
        }
    }

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

    EntidadFisica* buscar_entidad(int id, const std::vector<EntidadFisica*>& ents) {
        for (auto* e : ents) {
            if (e->get_id() == id) return e;
        }
        return nullptr;
    }
};

inline CondicionEvento deserializar_condicion_desde_linea(const std::string& linea) {
    CondicionEvento cond;
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

    std::string tipo_str = extraer_str("tipo");
    if (tipo_str.empty()) {
        size_t pos1 = linea.find(' ');
        if (pos1 != std::string::npos) {
            size_t pos2 = linea.find(' ', pos1 + 1);
            if (pos2 == std::string::npos) pos2 = linea.length();
            tipo_str = linea.substr(pos1 + 1, pos2 - pos1 - 1);
        }
    }

    cond.tipo = str_a_tipo_condicion(tipo_str);

    switch (cond.tipo) {
        case TipoCondicion::CONTACTO_ENTIDADES:
            cond.id_entidad_a = extraer_int("ea");
            cond.id_entidad_b = extraer_int("eb");
            break;
        case TipoCondicion::CONTACTO_TIPOS:
            cond.tipo_a = str_a_tipo_entidad_juego(extraer_str("ta"));
            cond.tipo_b = str_a_tipo_entidad_juego(extraer_str("tb"));
            break;
        case TipoCondicion::CABEZAZO:
        case TipoCondicion::PATADA:
            cond.id_seguidor = extraer_int("es");
            cond.id_bola = extraer_int("eb");
            break;
        case TipoCondicion::ENTIDAD_EN_ZONA:
            cond.id_zona_meta = extraer_int("ez");
            cond.id_entidad_zona = extraer_int("ee");
            break;
        case TipoCondicion::TIPO_EN_ZONA:
            cond.id_zona_meta = extraer_int("ez");
            cond.tipo_zona = str_a_tipo_entidad_juego(extraer_str("te"));
            break;
        case TipoCondicion::BARRIL_ACTIVADO:
            cond.id_barril = extraer_int("eb");
            break;
    }
    return cond;
}

inline void serializar_nodo_recursivo(const NodoEvento& nodo, std::ostream& out, int indent = 0) {
    std::string esp(indent * 2, ' ');
    if (nodo.tipo == TipoNodo::CONDICION) {
        out << esp << "evt_cond id=" << nodo.id << " tipo=" << tipo_condicion_a_str(nodo.condicion.tipo);
        const auto& c = nodo.condicion;
        switch (c.tipo) {
            case TipoCondicion::CONTACTO_ENTIDADES:
                out << " ea=" << c.id_entidad_a << " eb=" << c.id_entidad_b;
                break;
            case TipoCondicion::CONTACTO_TIPOS:
                out << " ta=" << tipo_entidad_juego_a_str(c.tipo_a)
                    << " tb=" << tipo_entidad_juego_a_str(c.tipo_b);
                break;
            case TipoCondicion::CABEZAZO:
            case TipoCondicion::PATADA:
                out << " es=" << c.id_seguidor << " eb=" << c.id_bola;
                break;
            case TipoCondicion::ENTIDAD_EN_ZONA:
                out << " ez=" << c.id_zona_meta << " ee=" << c.id_entidad_zona;
                break;
            case TipoCondicion::TIPO_EN_ZONA:
                out << " ez=" << c.id_zona_meta
                    << " te=" << tipo_entidad_juego_a_str(c.tipo_zona);
                break;
            case TipoCondicion::BARRIL_ACTIVADO:
                out << " eb=" << c.id_barril;
                break;
        }
        out << "\n";
    } else {
        out << esp << "evt_grupo id=" << nodo.id << " op=" << (nodo.tipo == TipoNodo::OPERADOR_AND ? "AND" : "OR") << "\n";
        for (const auto& h : nodo.hijos) {
            serializar_nodo_recursivo(h, out, indent + 1);
        }
        out << esp << "evt_grupo_fin\n";
    }
}
