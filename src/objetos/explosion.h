#pragma once
// ============================================================================
// Explosion — efecto visual efímero "boom" que la dinamita crea al explotar.
// No colisiona ni tiene física; solo se anima y se auto-marca para eliminación.
// El motor la crea en procesar_explosiones y la remueve cuando termina().
// ============================================================================

#include "../core/entidad_fisica.h"
#include "../sistema/assets_extern.h"
#include <cmath>
#include <sstream>

class Explosion : public EntidadFisica {
private:
    double radio_max;
    double tiempo;
    double duracion;

public:
    Explosion(int id, Vector2D centro, double r_max = 120.0)
        : EntidadFisica(id, centro, 0.0, TipoForma::NINGUNA, true),
          radio_max(r_max), tiempo(0.0), duracion(0.45) {}

    bool termino() const { return tiempo >= duracion; }
    double get_progreso() const { return duracion > 0.0 ? MathUtils::clamp(tiempo / duracion, 0.0, 1.0) : 1.0; }

    void actualizar_fisica(double dt) override { tiempo += dt; }

    TipoEntidadJuego get_tipo_entidad() const override { return TipoEntidadJuego::EXPLOSION; }
    std::string serializar() const override { return ""; } // no se guarda

    void dibujar(bool debug) const override {
        (void)debug;
        double t = get_progreso();
        float cx = static_cast<float>(posicion.x);
        float cy = static_cast<float>(posicion.y);

        // Onda expansiva: crece y se desvanece.
        float r = static_cast<float>(radio_max * (0.3 + 0.7 * t));
        unsigned char alpha = static_cast<unsigned char>(220 * (1.0 - t));

        // Núcleo (blanco→amarillo→naranja→rojo según avanza)
        DrawCircle((int)cx, (int)cy, r,        Color{255, 90, 30,  (unsigned char)(alpha * 0.5)});
        DrawCircle((int)cx, (int)cy, r * 0.7f, Color{255, 170, 40, (unsigned char)(alpha * 0.7)});
        DrawCircle((int)cx, (int)cy, r * 0.4f, Color{255, 240, 180, alpha});
        DrawCircleLines((int)cx, (int)cy, r, Color{255, 200, 80, alpha});

        // Rayos/chispas radiales
        int n = 10;
        for (int i = 0; i < n; ++i) {
            double ang = (2.0 * MathUtils::TIM_PI * i) / n;
            float x0 = cx + std::cos(ang) * r * 0.5f;
            float y0 = cy + std::sin(ang) * r * 0.5f;
            float x1 = cx + std::cos(ang) * r * 1.05f;
            float y1 = cy + std::sin(ang) * r * 1.05f;
            DrawLineEx({x0, y0}, {x1, y1}, 3.0f, Color{255, 220, 120, alpha});
        }
    }
};
