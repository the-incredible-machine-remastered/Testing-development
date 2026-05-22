#pragma once
// ============================================================================
// Vector2D — Álgebra vectorial 2D completa
// Base matemática para todo el motor de física.
// ============================================================================

#include <cmath>
#include <iostream>

struct Vector2D {
    double x, y;

    Vector2D(double x = 0.0, double y = 0.0) : x(x), y(y) {}

    // --- Operadores aritméticos ---
    Vector2D operator+(const Vector2D& v) const { return {x + v.x, y + v.y}; }
    Vector2D operator-(const Vector2D& v) const { return {x - v.x, y - v.y}; }
    Vector2D operator*(double s) const { return {x * s, y * s}; }
    Vector2D operator/(double s) const { return {x / s, y / s}; }
    Vector2D operator-() const { return {-x, -y}; }

    Vector2D& operator+=(const Vector2D& v) { x += v.x; y += v.y; return *this; }
    Vector2D& operator-=(const Vector2D& v) { x -= v.x; y -= v.y; return *this; }
    Vector2D& operator*=(double s) { x *= s; y *= s; return *this; }

    // --- Producto punto ---
    // Proyección de un vector sobre otro. Esencial para SAT y cálculos de impulso.
    static double dot(const Vector2D& a, const Vector2D& b) {
        return a.x * b.x + a.y * b.y;
    }

    // --- Producto cruz 2D (devuelve escalar) ---
    // Equivale al componente Z del producto cruz 3D.
    // Útil para calcular torques y determinar orientación (CW/CCW).
    static double cross(const Vector2D& a, const Vector2D& b) {
        return a.x * b.y - a.y * b.x;
    }

    // --- Magnitud ---
    double magnitud() const {
        return std::sqrt(x * x + y * y);
    }

    // Magnitud al cuadrado — evita sqrt, útil para comparaciones de distancia
    double magnitud_cuadrada() const {
        return x * x + y * y;
    }

    // --- Normalización ---
    // Retorna vector unitario (magnitud 1). Si el vector es ~cero, retorna (0,0).
    Vector2D normalizar() const {
        double m = magnitud();
        if (m < 1e-10) return {0.0, 0.0};
        return {x / m, y / m};
    }

    // --- Perpendicular ---
    // Rotación 90° antihorario. Necesario para calcular normales de aristas.
    Vector2D perpendicular() const {
        return {-y, x};
    }

    // --- Rotación por ángulo arbitrario (radianes) ---
    // Usa sin/cos exactos, sin aproximaciones de ángulos pequeños.
    Vector2D rotar(double angulo) const {
        double c = std::cos(angulo);
        double s = std::sin(angulo);
        return {x * c - y * s, x * s + y * c};
    }

    // --- Distancia entre dos puntos ---
    static double distancia(const Vector2D& a, const Vector2D& b) {
        return (a - b).magnitud();
    }

    // --- Debug output ---
    friend std::ostream& operator<<(std::ostream& os, const Vector2D& v) {
        os << "(" << v.x << ", " << v.y << ")";
        return os;
    }
};

// Permite escribir: double * Vector2D (conmutatividad con escalar)
inline Vector2D operator*(double s, const Vector2D& v) {
    return {s * v.x, s * v.y};
}
