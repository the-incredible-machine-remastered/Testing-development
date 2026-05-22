#pragma once
// ============================================================================
// EntidadFisica — Clase base para todos los objetos del motor de física
// Integración numérica con Runge-Kutta 4 (RK4) para traslación y rotación.
// ============================================================================

#include "vector2d.h"
#include "math_utils.h"

// Tipo de forma para dispatch de colisiones en el motor
enum class TipoForma {
    NINGUNA,
    CIRCULO,
    AABB,       // Axis-Aligned Bounding Box (rectángulos alineados a ejes)
    POLIGONO    // Polígono convexo genérico (triángulos, rampas, etc.)
};

class EntidadFisica {
protected:
    int id_objeto;

    // ---- Cinemática lineal ----
    Vector2D posicion;
    Vector2D velocidad;
    Vector2D aceleracion;
    double masa;
    Vector2D fuerza_neta;

    // ---- Cinemática angular ----
    double angulo;
    double velocidad_angular;
    double torque_neto;
    double inercia;

    // ---- Propiedades físicas ----
    bool es_estatico;
    double coef_restitucion;        // 0.0 = plástico, 1.0 = elástico perfecto
    double coef_friccion;           // Coeficiente de fricción cinética
    double amortiguamiento_lineal;  // Damping lineal (simula resistencia del aire)
    double amortiguamiento_angular; // Damping angular

    TipoForma tipo_forma;

    // ========================================================================
    // Integración Runge-Kutta de 4to Orden (RK4)
    //
    // Para un sistema y' = f(t, y) donde y = [posición, velocidad]:
    //   k1 = f(t, y)
    //   k2 = f(t + dt/2, y + dt/2 * k1)
    //   k3 = f(t + dt/2, y + dt/2 * k2)
    //   k4 = f(t + dt, y + dt * k3)
    //   y_{n+1} = y_n + dt/6 * (k1 + 2*k2 + 2*k3 + k4)
    //
    // Ventaja sobre Euler: O(dt^4) vs O(dt) — mucho más preciso para
    // fuerzas dependientes de posición (resortes, péndulos, etc.)
    // ========================================================================
    void integrar_rk4(double dt) {
        // ---- Integración de Traslación ----
        
        // k1: derivada en el estado actual
        Vector2D a1 = calcular_aceleracion_en(posicion, velocidad);
        Vector2D v1 = velocidad;

        // k2: derivada en el punto medio usando k1
        Vector2D pos2 = posicion + v1 * (dt * 0.5);
        Vector2D vel2 = velocidad + a1 * (dt * 0.5);
        Vector2D a2 = calcular_aceleracion_en(pos2, vel2);
        Vector2D v2 = vel2;

        // k3: derivada en el punto medio usando k2
        Vector2D pos3 = posicion + v2 * (dt * 0.5);
        Vector2D vel3 = velocidad + a2 * (dt * 0.5);
        Vector2D a3 = calcular_aceleracion_en(pos3, vel3);
        Vector2D v3 = vel3;

        // k4: derivada al final del paso usando k3
        Vector2D pos4 = posicion + v3 * dt;
        Vector2D vel4 = velocidad + a3 * dt;
        Vector2D a4 = calcular_aceleracion_en(pos4, vel4);
        Vector2D v4 = vel4;

        // Combinación ponderada final
        posicion += (v1 + v2 * 2.0 + v3 * 2.0 + v4) * (dt / 6.0);
        velocidad += (a1 + a2 * 2.0 + a3 * 2.0 + a4) * (dt / 6.0);

        // ---- Integración de Rotación ----
        double alpha1 = calcular_aceleracion_angular_en(angulo, velocidad_angular);
        double omega1 = velocidad_angular;

        double theta2 = angulo + omega1 * (dt * 0.5);
        double omega2_v = velocidad_angular + alpha1 * (dt * 0.5);
        double alpha2 = calcular_aceleracion_angular_en(theta2, omega2_v);
        double omega2 = omega2_v;

        double theta3 = angulo + omega2 * (dt * 0.5);
        double omega3_v = velocidad_angular + alpha2 * (dt * 0.5);
        double alpha3 = calcular_aceleracion_angular_en(theta3, omega3_v);
        double omega3 = omega3_v;

        double omega4_v = velocidad_angular + alpha3 * dt;
        double alpha4 = calcular_aceleracion_angular_en(angulo + omega3 * dt, omega4_v);
        double omega4 = omega4_v;

        angulo += (omega1 + 2.0 * omega2 + 2.0 * omega3 + omega4) * (dt / 6.0);
        velocidad_angular += (alpha1 + 2.0 * alpha2 + 2.0 * alpha3 + alpha4) * (dt / 6.0);
    }

public:
    EntidadFisica(int id, Vector2D pos_inicial, double m, 
                  TipoForma forma = TipoForma::NINGUNA, bool estatico = false)
        : id_objeto(id), posicion(pos_inicial), velocidad(0, 0), aceleracion(0, 0),
          masa(m), fuerza_neta(0, 0), angulo(0.0), velocidad_angular(0.0),
          torque_neto(0.0), inercia(1.0), es_estatico(estatico),
          coef_restitucion(0.5), coef_friccion(0.3),
          amortiguamiento_lineal(0.002), amortiguamiento_angular(0.002),
          tipo_forma(forma) {}

    virtual ~EntidadFisica() = default;

    // ---- Acumulación de fuerzas (se resetean cada frame) ----
    void aplicar_fuerza(const Vector2D& fuerza) {
        if (!es_estatico) fuerza_neta += fuerza;
    }

    void aplicar_torque(double torque) {
        if (!es_estatico) torque_neto += torque;
    }

    // ---- Métodos virtuales para fuerzas dependientes del estado ----
    // Las subclases (resorte, péndulo, balancín) sobreescriben estos para 
    // agregar fuerzas que dependen de la posición/velocidad actual.
    // Esto permite que RK4 las evalúe correctamente en los puntos intermedios.
    virtual Vector2D calcular_aceleracion_en(const Vector2D& pos, const Vector2D& vel) const {
        if (masa > MathUtils::EPSILON) return fuerza_neta / masa;
        return Vector2D(0, 0);
    }

    virtual double calcular_aceleracion_angular_en(double theta, double omega) const {
        if (inercia > MathUtils::EPSILON) return torque_neto / inercia;
        return 0.0;
    }

    // ---- Paso de simulación ----
    virtual void actualizar_fisica(double dt) {
        if (es_estatico) return;

        // 1. Integrar con RK4
        integrar_rk4(dt);

        // 2. Aplicar amortiguamiento (simula aire/fricción ambiental)
        velocidad *= (1.0 - amortiguamiento_lineal);
        velocidad_angular *= (1.0 - amortiguamiento_angular);

        // 3. Reiniciar acumuladores para el siguiente paso
        fuerza_neta = Vector2D(0, 0);
        torque_neto = 0.0;
    }

    // ---- Getters ----
    int get_id() const { return id_objeto; }
    Vector2D get_posicion() const { return posicion; }
    Vector2D get_velocidad() const { return velocidad; }
    double get_masa() const { return masa; }
    double get_angulo() const { return angulo; }
    double get_velocidad_angular() const { return velocidad_angular; }
    bool get_es_estatico() const { return es_estatico; }
    double get_restitucion() const { return coef_restitucion; }
    double get_friccion() const { return coef_friccion; }
    TipoForma get_tipo_forma() const { return tipo_forma; }
    double get_inercia() const { return inercia; }

    // ---- Setters ----
    void set_posicion(const Vector2D& pos) { posicion = pos; }
    void set_velocidad(const Vector2D& vel) { velocidad = vel; }
    void set_velocidad_angular(double omega) { velocidad_angular = omega; }
    void set_restitucion(double r) { coef_restitucion = r; }
    void set_friccion(double f) { coef_friccion = f; }
    void set_amortiguamiento(double d) { amortiguamiento_lineal = d; }
    void set_inercia(double i) { inercia = i; }
};
