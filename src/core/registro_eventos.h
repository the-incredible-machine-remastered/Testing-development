#pragma once
#include "core/vector2d.h"

// Registro de una colision detectada en el frame actual
struct RegistroColision {
    int id_a;
    int id_b;
    Vector2D punto_contacto;
    Vector2D normal;
    double profundidad;
};

enum class TipoEventoEspecial {
    CABEZAZO,
    PATADA,
    BARRIL_LANZADO
};

struct RegistroEventoEspecial {
    TipoEventoEspecial tipo;
    int id_origen;
    int id_objetivo;
};
