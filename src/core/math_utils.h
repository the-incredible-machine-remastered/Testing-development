#pragma once
// ============================================================================
// MathUtils — Constantes y funciones matemáticas de utilidad
// ============================================================================

#include <cmath>
#include <algorithm>

namespace MathUtils {

    // --- Constantes ---
    // Nota: usamos TIM_PI en vez de PI para evitar conflicto con el macro
    // #define PI que define raylib.h
    constexpr double TIM_PI = 3.14159265358979323846;
    constexpr double EPSILON = 1e-8;

    // Gravedad en pixeles/s² (equivale a ~9.8 m/s² con escala de 50 px/m)
    constexpr double GRAVEDAD_PX = 500.0;

    // --- Funciones de utilidad ---

    // Limita un valor al rango [minimo, maximo]
    inline double clamp(double valor, double minimo, double maximo) {
        return std::max(minimo, std::min(maximo, valor));
    }

    // Interpolación lineal entre a y b con factor t ∈ [0, 1]
    inline double lerp(double a, double b, double t) {
        return a + (b - a) * t;
    }

    // Conversiones de ángulo
    inline double grados_a_radianes(double grados) {
        return grados * TIM_PI / 180.0;
    }

    inline double radianes_a_grados(double radianes) {
        return radianes * 180.0 / TIM_PI;
    }

    // Función signo: retorna +1, -1, o 0
    // Útil para dirección de fricción y fuerzas condicionales.
    inline double signo(double valor) {
        if (valor > EPSILON) return 1.0;
        if (valor < -EPSILON) return -1.0;
        return 0.0;
    }

} // namespace MathUtils
