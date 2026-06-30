#pragma once
#include "obstaculo_estatico.h"
#include <vector>
#include <unordered_set>

class Tijera : public ObstaculoEstatico {
private:
    double ancho;
    double alto;
    bool fue_activada;
    bool permanentemente_activada;
    bool ya_corto_cuerdas;
    std::unordered_set<int> ids_desviados; // entidades ya redirigidas, no volver a tocar

    // Vértices del filo derecho (triángulo) cuando la tijera está cerrada.
    // Se recalculan al activarse. Orden: sentido horario para que la normal
    // superior apunte hacia arriba-derecha y guíe la bola hacia la derecha.
    std::vector<Vector2D> vertices_filo;

    void recalcular_vertices_filo() {
        // Triángulo rampa: punta izquierda en el centro-izquierdo de la tijera,
        // punta derecha en la esquina derecha.
        // La arista superior va de izquierda-abajo a derecha-arriba (rampa /)
        // → normal apunta arriba-derecha → bola que cae sobre ella sale a la DERECHA.
        //
        //              T (der-arriba)
        //             /
        //   P (izq-centro)
        //             \
        //              B (der-abajo)
        double cx = posicion.x + ancho * 0.5;
        double cy = posicion.y + alto  * 0.5;

        Vector2D P(posicion.x,         cy);                 // pivote izq-centro
        Vector2D T(posicion.x + ancho, posicion.y);         // punta der-arriba
        Vector2D B(posicion.x + ancho, posicion.y + alto);  // punta der-abajo

        // Orden antihorario en Y-down (= horario en Y-up) para que las normales
        // de circulo_vs_poligono apunten hacia afuera (hacia el círculo que cae).
        // Arista P→T: va de izq-centro a der-arriba (sube hacia la derecha).
        // perp(T-P) = perp(+dx, -dy) = (dy, dx) → apunta abajo-derecha.
        // Pero la bola viene de arriba y está FUERA → la normal real es pos_circ-closest,
        // que apunta arriba. La componente horizontal del filo P→T empuja a la derecha.
        vertices_filo = { P, T, B };
    }

public:
    Tijera(int id, Vector2D pos, double w = 110.0, double h = 51.0)
        : ObstaculoEstatico(id, pos, TipoForma::AABB),
          ancho(w), alto(h), fue_activada(false), permanentemente_activada(false), ya_corto_cuerdas(false) {
        set_restitucion(0.05);
        set_friccion(0.15); // poco roce para que la bola resbale
    }

    double get_ancho() const { return ancho; }
    double get_alto() const { return alto; }
    bool get_fue_activada() const { return fue_activada; }
    void resetear_activacion() { fue_activada = false; }

    // Hitbox cubre todo el AABB (aros + hojas)
    Vector2D get_min() const override { return posicion; }
    Vector2D get_max() const override { return Vector2D(posicion.x + ancho, posicion.y + alto); }

    // Radio de los aros
    double get_radio_aro() const { return alto * 0.27; }

    // Aro superior: izquierda de la tijera, arriba
    // cx_aro = posicion.x + r_aro (pegado al borde izq con margen)
    void get_aro_superior(Vector2D& centro, double& radio_aro) const {
        double r  = get_radio_aro();
        double cx = posicion.x + r + 1.0;           // borde izq + radio
        double cy = posicion.y + alto * 0.5 - r;    // mitad - separación
        centro    = Vector2D(cx, cy);
        radio_aro = r;
    }
    void get_aro_inferior(Vector2D& centro, double& radio_aro) const {
        double r  = get_radio_aro();
        double cx = posicion.x + r + 1.0;
        double cy = posicion.y + alto * 0.5 + r;    // mitad + separación
        centro    = Vector2D(cx, cy);
        radio_aro = r;
    }

    // Comprueba si un círculo toca alguno de los aros
    bool circulo_toca_aros(const Vector2D& pos_circ, double radio_circ) const {
        Vector2D c; double r;
        get_aro_superior(c, r);
        if ((pos_circ - c).magnitud() <= r + radio_circ) return true;
        get_aro_inferior(c, r);
        if ((pos_circ - c).magnitud() <= r + radio_circ) return true;
        return false;
    }

    const std::vector<Vector2D>& get_vertices_filo() const { return vertices_filo; }

    bool get_permanentemente_activada() const { return permanentemente_activada; }
    bool get_ya_corto_cuerdas() const { return ya_corto_cuerdas; }
    void set_ya_corto_cuerdas() { ya_corto_cuerdas = true; }

    void on_collision(EntidadFisica* otro, const InfoColision& info) override {
        if (!otro || otro->get_es_estatico()) return;

        if (!permanentemente_activada) {
            fue_activada = true;
            permanentemente_activada = true;
        }

        // Solo desviar una vez por entidad
        int id = otro->get_id();
        if (ids_desviados.count(id)) return;
        ids_desviados.insert(id);

        Vector2D vel = otro->get_velocidad();
        double speed = vel.magnitud();
        if (speed < 80.0) speed = 80.0;
        // Dirección: abajo-derecha, conservando algo de la velocidad original
        otro->set_velocidad(Vector2D(speed * 0.85, speed * 0.5));
    }

    bool contiene_punto(const Vector2D& p) const override {
        return p.x >= posicion.x - 8 && p.x <= posicion.x + ancho + 8 && p.y >= posicion.y - 8 && p.y <= posicion.y + alto + 8;
    }

    TipoEntidadJuego get_tipo_entidad() const override {
        return TipoEntidadJuego::TIJERA;
    }

    std::string serializar() const override {
        std::stringstream ss;
        ss << "ent TIJERA id=" << get_id() << serializar_base() << " w=" << ancho << " h=" << alto;
        return ss.str();
    }

    void dibujar(bool debug) const override {
        float px = static_cast<float>(posicion.x);
        float py = static_cast<float>(posicion.y);
        float w  = static_cast<float>(ancho);
        float h  = static_cast<float>(alto);
        float tip_x = px + w;          // punta derecha de las hojas
        float mid_y = py + h * 0.5f;

        bool cerrada = permanentemente_activada;
        Color col  = cerrada ? Color{255, 80, 80, 255} : Color{175, 182, 195, 255};
        Color col2 = cerrada ? Color{200, 30, 30, 255} : Color{130, 138, 150, 255};
        Color col_aro = cerrada ? Color{255, 80, 80, 255} : Color{200, 40, 40, 255};

        // Obtenemos los centros reales de los aros (misma fuente que el hitbox)
        Vector2D cs, ci; double ra;
        get_aro_superior(cs, ra);
        get_aro_inferior(ci, ra);
        float r = static_cast<float>(ra);
        float ax = static_cast<float>(cs.x); // x compartida de ambos aros
        float ay_sup = static_cast<float>(cs.y);
        float ay_inf = static_cast<float>(ci.y);

        if (!cerrada) {
            // Hojas en X: parten de los centros de los aros y se cruzan en la punta
            DrawLineEx({ax, ay_sup}, {tip_x, mid_y + h * 0.3f}, 2.8f, col);
            DrawLineEx({ax, ay_inf}, {tip_x, mid_y - h * 0.3f}, 2.8f, col);
        } else {
            // Hojas cerradas: casi paralelas apuntando a la derecha
            DrawLineEx({ax, ay_sup}, {tip_x, mid_y - 2.0f}, 3.0f, col);
            DrawLineEx({ax, ay_inf}, {tip_x, mid_y + 2.0f}, 3.0f, col);
        }

        // Aros (siempre en la posición del hitbox)
        DrawCircle(static_cast<int>(ax), static_cast<int>(ay_sup), r, col_aro);
        DrawCircleLines(static_cast<int>(ax), static_cast<int>(ay_sup), r, col2);
        DrawCircle(static_cast<int>(ax), static_cast<int>(ay_inf), r, col_aro);
        DrawCircleLines(static_cast<int>(ax), static_cast<int>(ay_inf), r, col2);

        // Pivote en la punta donde se juntan las hojas
        DrawCircle(static_cast<int>(tip_x - w * 0.15f), static_cast<int>(mid_y), 3.5f, col2);

        if (debug) {
            // AABB completo (verde) — es también la hitbox activa
            DrawRectangleLines(static_cast<int>(px), static_cast<int>(py),
                               static_cast<int>(w),  static_cast<int>(h), GREEN);
            // Círculos de los aros
            DrawCircleLines(static_cast<int>(ax), static_cast<int>(ay_sup), r, GREEN);
            DrawCircleLines(static_cast<int>(ax), static_cast<int>(ay_inf), r, GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Tijera" tab=0 categoria=0
