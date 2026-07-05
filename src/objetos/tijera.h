#pragma once
#include "obstaculo_estatico.h"
#include "../sistema/assets_extern.h"
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
        // Al cerrarse, la tijera actúa como RAMPA cuya cara superior SIGUE la hoja
        // superior de la tijera dibujada: baja del aro superior-izquierdo a la punta
        // derecha-centro. Así el hitbox coincide con lo que se ve (no una cuña que
        // llena el rectángulo).
        //
        //   A (sobre el aro sup-izq)
        //    \____
        //         \___ hipotenusa = hoja superior (baja hacia la derecha)
        //   C----------T (punta der-centro)
        //
        // La bola cae sobre A→T y resbala hacia T (derecha).
        double r  = get_radio_aro();
        double ax = posicion.x + r + 1.0;                 // x de los aros (borde izq)
        double mid_y = posicion.y + alto * 0.5;           // centro vertical
        double ay_sup = mid_y - r;                        // centro aro superior

        Vector2D A(ax, ay_sup - r);                       // cresta: tope del aro superior
        Vector2D T(posicion.x + ancho, mid_y);            // punta derecha (donde se juntan las hojas)
        Vector2D C(ax, mid_y + r * 0.4);                  // base bajo el arranque, da grosor a la cuña

        vertices_filo = { A, T, C };
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

    // Al cerrarse fija la forma de colisión como el triángulo-rampa (POLIGONO) para
    // que el motor deje de usar el AABB rectangular grande y la bola resbale.
    void cerrar() {
        if (!permanentemente_activada) {
            fue_activada = true;
            permanentemente_activada = true;
            recalcular_vertices_filo();
            tipo_forma = TipoForma::POLIGONO; // el motor usará vertices_filo como rampa
        }
    }

    void on_collision(EntidadFisica* otro, const InfoColision& info) override {
        if (!otro || otro->get_es_estatico()) return;

        bool estaba_abierta = !permanentemente_activada;
        cerrar();

        // Empujón inicial abajo-derecha SOLO en el instante del cierre (una vez por
        // entidad) para arrancar el deslizamiento; ya cerrada, la rampa física lo hace.
        int id = otro->get_id();
        if (estaba_abierta && !ids_desviados.count(id)) {
            ids_desviados.insert(id);
            Vector2D vel = otro->get_velocidad();
            double speed = vel.magnitud();
            if (speed < 80.0) speed = 80.0;
            otro->set_velocidad(Vector2D(speed * 0.85, speed * 0.5));
        }
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
        Texture2D tex = cerrada ? tex_tijera_cerrada : tex_tijera_abierta;

        
        // Obtenemos los centros reales de los aros (usados para debug y dibujo procedural)
        Vector2D cs, ci; double ra;
        get_aro_superior(cs, ra);
        get_aro_inferior(ci, ra);
        float r = static_cast<float>(ra);
        float ax = static_cast<float>(cs.x); // x compartida de ambos aros
        float ay_sup = static_cast<float>(cs.y);
        float ay_inf = static_cast<float>(ci.y);

        if (tex.id > 0) {
            Rectangle src = {0.0f, 0.0f, (float)tex.width, (float)tex.height};
            Rectangle dst = {px + w / 2.0f, py + h / 2.0f, w, h};
            Vector2 origin = {w / 2.0f, h / 2.0f};
            DrawTexturePro(tex, src, dst, origin, 0.0f, WHITE);
        } else {
            Color col  = cerrada ? Color{255, 80, 80, 255} : Color{175, 182, 195, 255};
            Color col2 = cerrada ? Color{200, 30, 30, 255} : Color{130, 138, 150, 255};
            Color col_aro = cerrada ? Color{255, 80, 80, 255} : Color{200, 40, 40, 255};

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
        }
        if (debug) {
            if (cerrada && vertices_filo.size() == 3) {
                // Hitbox real cerrada: el triángulo-rampa (lo que colisiona de verdad).
                for (int i = 0; i < 3; ++i) {
                    const Vector2D& v0 = vertices_filo[i];
                    const Vector2D& v1 = vertices_filo[(i + 1) % 3];
                    DrawLineEx({(float)v0.x, (float)v0.y}, {(float)v1.x, (float)v1.y}, 1.5f, GREEN);
                }
            } else {
                // Abierta: AABB (hitbox de colisión).
                DrawRectangleLines(static_cast<int>(px), static_cast<int>(py),
                                   static_cast<int>(w),  static_cast<int>(h), GREEN);
            }
            // Círculos de los aros (activación / corte de cuerda) siempre.
            DrawCircleLines(static_cast<int>(ax), static_cast<int>(ay_sup), r, GREEN);
            DrawCircleLines(static_cast<int>(ax), static_cast<int>(ay_inf), r, GREEN);
        }
    }
};

// TIM_MENU_SPAWN etiqueta="Tijera" tab=0 categoria=0
