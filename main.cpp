// ============================================================================
// TIM: Motor de Física — Prototipo Interactivo con Raylib
//
// Controles:
//   [Click Izq]  Crear bola en la posición del mouse
//   [SPACE]      Pausar / Reanudar simulación
//   [D]          Activar / Desactivar modo debug (wireframes, vectores)
//   [R]          Reiniciar escena
//   [+/-]        Aumentar / Disminuir gravedad
//   [ESC]        Salir
// ============================================================================

#include "raylib.h"

#include "core/vector2d.h"
#include "core/math_utils.h"
#include "core/entidad_fisica.h"
#include "objetos/obstaculo_estatico.h"
#include "objetos/pared_rectangular.h"
#include "objetos/plano_inclinado.h"
#include "objetos/bola.h"
#include "fisica/colisiones.h"
#include "fisica/motor_fisica.h"

#include <cmath>

// ============================================================================
// Configuración global
// ============================================================================
const int ANCHO = 1024;
const int ALTO  = 768;

// Paleta de colores para bolas (inspirada en TIM, colores vivos)
Color PALETA_BOLAS[] = {
    {255, 107, 107, 255},   // Coral
    {78,  205, 196, 255},   // Turquesa
    {255, 230, 109, 255},   // Amarillo
    {199, 125, 255, 255},   // Violeta
    {69,  183, 209, 255},   // Cielo
    {255, 179, 71,  255},   // Naranja
    {95,  209, 111, 255},   // Verde
};
const int NUM_COLORES = 7;

// Colores del escenario
const Color COLOR_FONDO       = {22,  22,  42,  255};   // Navy oscuro
const Color COLOR_PARED       = {40,  42,  68,  255};   // Azul-gris oscuro
const Color COLOR_PARED_BORDE = {70,  75,  110, 255};   // Azul-gris claro
const Color COLOR_RAMPA       = {35,  75,  58,  255};   // Verde bosque oscuro
const Color COLOR_RAMPA_BORDE = {65,  140, 100, 255};   // Verde bosque claro
const Color COLOR_HUD         = {200, 200, 220, 255};   // Texto claro
const Color COLOR_CONTROLES   = {130, 130, 160, 200};   // Texto controles

// Estado del prototipo
bool modo_debug = false;
int contador_bolas = 0;

// Feedback visual de spawn fallido
float spawn_error_timer = 0.0f;
Vector2D spawn_error_pos;

// ============================================================================
// Crear escena inicial
// ============================================================================
void crear_escena(MotorFisica& motor) {
    int g = 20; // Grosor de paredes límite

    // ---- Paredes del nivel (4 bordes) ----
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(0, ALTO - g), ANCHO, g));          // Suelo
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(0, 0), g, ALTO));                  // Izquierda
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(ANCHO - g, 0), g, ALTO));         // Derecha
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(0, 0), ANCHO, g));                 // Techo

    // ---- Plataformas ----
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(80, 320), 220, 15));               // Plataforma superior izq
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(550, 260), 230, 15));              // Plataforma superior der
    motor.agregar_entidad(new ParedRectangular(
        motor.generar_id(), Vector2D(350, 550), 200, 15));              // Plataforma inferior centro

    // ---- Rampas ----
    // Rampa \ (pendiente de arriba-izq a abajo-der)
    // Conecta visualmente con la plataforma superior izquierda
    motor.agregar_entidad(new PlanoInclinado(
        motor.generar_id(), Vector2D(300, 320), 200, 200, false));

    // Rampa / (pendiente de abajo-izq a arriba-der) 
    motor.agregar_entidad(new PlanoInclinado(
        motor.generar_id(), Vector2D(700, 280), 200, 200, true));
}

// ============================================================================
// Validación de spawn — no crear objetos dentro de otros
// ============================================================================
bool posicion_valida_para_bola(const MotorFisica& motor, Vector2D pos, double radio) {
    for (const auto* e : motor.get_entidades()) {
        TipoForma forma = e->get_tipo_forma();

        if (forma == TipoForma::CIRCULO) {
            const Bola* b = dynamic_cast<const Bola*>(e);
            if (b) {
                InfoColision info = Colisiones::circulo_vs_circulo(
                    pos, radio, b->get_posicion(), b->get_radio());
                if (info.hay_colision) return false;
            }
        }
        else if (forma == TipoForma::AABB) {
            const ParedRectangular* p = dynamic_cast<const ParedRectangular*>(e);
            if (p) {
                InfoColision info = Colisiones::circulo_vs_aabb(
                    pos, radio, p->get_min(), p->get_max());
                if (info.hay_colision) return false;
            }
        }
        else if (forma == TipoForma::POLIGONO) {
            const PlanoInclinado* ramp = dynamic_cast<const PlanoInclinado*>(e);
            if (ramp) {
                InfoColision info = Colisiones::circulo_vs_poligono(
                    pos, radio, ramp->get_vertices());
                if (info.hay_colision) return false;
            }
        }
    }
    return true;
}

// ============================================================================
// Crear bola en posición del mouse (con validación)
// ============================================================================
bool crear_bola(MotorFisica& motor, Vector2D pos) {
    double radio = 8.0 + GetRandomValue(0, 8);   // Radio entre 8-16 px

    // Verificar que no colisiona con nada existente
    if (!posicion_valida_para_bola(motor, pos, radio)) {
        spawn_error_timer = 0.5f;  // Flash rojo de 0.5s
        spawn_error_pos = pos;
        return false;
    }

    double masa = radio * radio * 0.01;           // Proporcional al área
    Bola* b = new Bola(motor.generar_id(), pos, radio, masa);
    b->set_color_idx(contador_bolas % NUM_COLORES);
    contador_bolas++;
    motor.agregar_entidad(b);
    return true;
}

// ============================================================================
// Renderizado de entidades
// ============================================================================
void dibujar_entidad(const EntidadFisica* e) {
    TipoForma forma = e->get_tipo_forma();

    if (forma == TipoForma::CIRCULO) {
        const Bola* b = dynamic_cast<const Bola*>(e);
        if (!b) return;

        Vector2D pos = b->get_posicion();
        float r = static_cast<float>(b->get_radio());
        Color col = PALETA_BOLAS[b->get_color_idx()];

        // Círculo relleno + borde más oscuro
        DrawCircle(static_cast<int>(pos.x), static_cast<int>(pos.y), r, col);
        DrawCircleLines(static_cast<int>(pos.x), static_cast<int>(pos.y), r,
                        ColorBrightness(col, -0.3f));

        // Indicador de rotación (siempre visible)
        // Dos puntos en lados opuestos que giran con el ángulo de la bola
        double ang = b->get_angulo();
        float dot_dist = r * 0.55f;
        float dot_r = std::max(r * 0.2f, 2.0f);
        Color dot_col = ColorBrightness(col, -0.45f);

        // Punto 1
        int dx1 = static_cast<int>(pos.x + std::cos(ang) * dot_dist);
        int dy1 = static_cast<int>(pos.y + std::sin(ang) * dot_dist);
        DrawCircle(dx1, dy1, dot_r, dot_col);

        // Punto 2 (opuesto)
        int dx2 = static_cast<int>(pos.x - std::cos(ang) * dot_dist);
        int dy2 = static_cast<int>(pos.y - std::sin(ang) * dot_dist);
        DrawCircle(dx2, dy2, dot_r, dot_col);

        // Brillo especular (efecto de profundidad sutil)
        DrawCircle(static_cast<int>(pos.x - r * 0.25),
                   static_cast<int>(pos.y - r * 0.25),
                   r * 0.3f,
                   Color{255, 255, 255, 60});

        // Debug extra: línea de velocidad angular
        if (modo_debug) {
            Vector2D dir(std::cos(ang) * r, std::sin(ang) * r);
            DrawLine(static_cast<int>(pos.x), static_cast<int>(pos.y),
                     static_cast<int>(pos.x + dir.x),
                     static_cast<int>(pos.y + dir.y), WHITE);
        }
    }
    else if (forma == TipoForma::AABB) {
        const ParedRectangular* p = dynamic_cast<const ParedRectangular*>(e);
        if (!p) return;

        Vector2D pos = p->get_posicion();
        int px = static_cast<int>(pos.x);
        int py = static_cast<int>(pos.y);
        int pw = static_cast<int>(p->get_ancho());
        int ph = static_cast<int>(p->get_alto());

        DrawRectangle(px, py, pw, ph, COLOR_PARED);
        DrawRectangleLines(px, py, pw, ph, COLOR_PARED_BORDE);
    }
    else if (forma == TipoForma::POLIGONO) {
        const PlanoInclinado* ramp = dynamic_cast<const PlanoInclinado*>(e);
        if (!ramp) return;

        const auto& verts = ramp->get_vertices();
        if (verts.size() >= 3) {
            Vector2 v1 = {static_cast<float>(verts[0].x), static_cast<float>(verts[0].y)};
            Vector2 v2 = {static_cast<float>(verts[1].x), static_cast<float>(verts[1].y)};
            Vector2 v3 = {static_cast<float>(verts[2].x), static_cast<float>(verts[2].y)};

            DrawTriangle(v1, v2, v3, COLOR_RAMPA);
            DrawTriangleLines(v1, v2, v3, COLOR_RAMPA_BORDE);
        }
    }
}

// ============================================================================
// Renderizado de debug (vectores de velocidad, hitboxes)
// ============================================================================
void dibujar_debug(const EntidadFisica* e) {
    if (!modo_debug || e->get_es_estatico()) return;

    Vector2D pos = e->get_posicion();
    Vector2D vel = e->get_velocidad();

    // Vector de velocidad (verde, escalado)
    double vel_scale = 0.15;
    DrawLineEx(
        {static_cast<float>(pos.x), static_cast<float>(pos.y)},
        {static_cast<float>(pos.x + vel.x * vel_scale),
         static_cast<float>(pos.y + vel.y * vel_scale)},
        2.0f, GREEN);

    // Velocidad numérica
    double speed = vel.magnitud();
    if (speed > 10.0) {
        DrawText(TextFormat("%.0f", speed),
                 static_cast<int>(pos.x + 15),
                 static_cast<int>(pos.y - 10), 10, GREEN);
    }
}

// ============================================================================
// HUD (información en pantalla)
// ============================================================================
void dibujar_hud(const MotorFisica& motor) {
    int y = 30;
    int margin = 30;

    DrawText(TextFormat("FPS: %d", GetFPS()), margin, y, 16, COLOR_HUD);
    DrawText(TextFormat("Entidades: %d", static_cast<int>(motor.get_entidades().size())),
             margin, y + 22, 16, COLOR_HUD);
    DrawText(TextFormat("Bolas: %d", contador_bolas), margin, y + 44, 16, COLOR_HUD);
    DrawText(TextFormat("Gravedad: %.0f px/s2", motor.get_gravedad().y),
             margin, y + 66, 16, COLOR_HUD);

    // Indicador de pausa
    if (motor.get_pausado()) {
        int pause_w = MeasureText("|| PAUSADO", 30);
        DrawText("|| PAUSADO", ANCHO / 2 - pause_w / 2, ALTO / 2 - 15, 30,
                 Color{255, 255, 100, 200});
    }

    // Indicador de debug
    if (modo_debug) {
        DrawText("[DEBUG]", ANCHO - 100, y, 16, GREEN);
    }

    // Controles
    DrawText("[CLICK] Bola   [SPACE] Pausa   [D] Debug   [R] Reset   [+/-] Gravedad",
             margin, ALTO - 30, 14, COLOR_CONTROLES);
}

// ============================================================================
// Punto de entrada
// ============================================================================
int main() {
    // ---- Inicializar ventana ----
    SetConfigFlags(FLAG_MSAA_4X_HINT);  // Anti-aliasing
    InitWindow(ANCHO, ALTO, "TIM - Motor de Fisica | Prototipo RK4 + Raylib");
    SetTargetFPS(60);

    // ---- Inicializar motor de física ----
    // dt_fijo = 1/120s (120 pasos físicos por segundo)
    // Gravedad = 500 px/s² hacia abajo (Y+ en pantalla)
    MotorFisica motor(1.0 / 120.0, Vector2D(0, 500.0));
    crear_escena(motor);

    // Animación de título (fade-out)
    float titulo_alpha = 1.0f;

    // ---- Bucle principal ----
    while (!WindowShouldClose()) {
        // ======== INPUT ========

        // Click izquierdo: crear bola
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2D mouse_pos(GetMouseX(), GetMouseY());
            crear_bola(motor, mouse_pos);
        }

        // Spacebar: pausar/reanudar
        if (IsKeyPressed(KEY_SPACE)) {
            motor.set_pausado(!motor.get_pausado());
        }

        // D: toggle debug
        if (IsKeyPressed(KEY_D)) {
            modo_debug = !modo_debug;
        }

        // R: reiniciar escena
        if (IsKeyPressed(KEY_R)) {
            motor.limpiar();
            contador_bolas = 0;
            crear_escena(motor);
        }

        // +/-: ajustar gravedad
        if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) {
            Vector2D g = motor.get_gravedad();
            motor.set_gravedad(Vector2D(g.x, g.y + 100.0));
        }
        if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) {
            Vector2D g = motor.get_gravedad();
            motor.set_gravedad(Vector2D(g.x, g.y - 100.0));
        }

        // ======== UPDATE ========
        motor.actualizar(GetFrameTime());

        // Fade-out del título
        if (titulo_alpha > 0.0f) {
            titulo_alpha -= 0.008f;
            if (titulo_alpha < 0.0f) titulo_alpha = 0.0f;
        }

        // Fade-out del error de spawn
        if (spawn_error_timer > 0.0f) {
            spawn_error_timer -= GetFrameTime();
        }

        // ======== RENDER ========
        BeginDrawing();
        ClearBackground(COLOR_FONDO);

        // Dibujar todas las entidades
        for (const auto* e : motor.get_entidades()) {
            dibujar_entidad(e);
            dibujar_debug(e);
        }

        // Feedback visual de spawn fallido (X roja parpadeante)
        if (spawn_error_timer > 0.0f) {
            unsigned char alpha = static_cast<unsigned char>(spawn_error_timer * 2.0f * 255);
            Color err_col = {255, 50, 50, alpha};
            int ex = static_cast<int>(spawn_error_pos.x);
            int ey = static_cast<int>(spawn_error_pos.y);
            DrawLine(ex - 10, ey - 10, ex + 10, ey + 10, err_col);
            DrawLine(ex + 10, ey - 10, ex - 10, ey + 10, err_col);
            DrawCircleLines(ex, ey, 14, err_col);
        }

        // HUD
        dibujar_hud(motor);

        // Splash de título (desaparece gradualmente)
        if (titulo_alpha > 0.01f) {
            unsigned char a = static_cast<unsigned char>(titulo_alpha * 255);
            int tw = MeasureText("TIM: Motor de Fisica", 40);
            DrawText("TIM: Motor de Fisica", ANCHO / 2 - tw / 2, ALTO / 2 - 50, 40,
                     Color{255, 255, 255, a});
            int sw = MeasureText("Prototipo v0.1 | RK4 + Raylib", 20);
            DrawText("Prototipo v0.1 | RK4 + Raylib", ANCHO / 2 - sw / 2, ALTO / 2, 20,
                     Color{180, 180, 200, a});
        }

        EndDrawing();
    }

    // ---- Cleanup ----
    CloseWindow();
    return 0;
}
