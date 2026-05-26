#pragma once
// ============================================================================
// Animacion — Manejo de sprite sheets y animaciones
// ============================================================================

#include "raylib.h"
#include <cmath>

struct Animacion {
    Texture2D textura;
    int frames_totales;
    int frames_por_fila;
    int frame_actual;
    float timer;
    float tiempo_por_frame;
    float escala;

    Animacion(Texture2D tex, int total_frames, int fps_animacion, int frames_x_fila = 8)
        : textura(tex), frames_totales(total_frames), frames_por_fila(frames_x_fila),
          frame_actual(0), timer(0.0f), escala(1.0f) {
        tiempo_por_frame = 1.0f / fps_animacion;
    }

    void actualizar(float delta_time) {
        if (textura.id == 0) return;
        timer += delta_time;
        if (timer >= tiempo_por_frame) {
            frame_actual = (frame_actual + 1) % frames_totales;
            timer = 0.0f;
        }
    }

    void dibujar(Vector2 pos, float ancho, float alto) {
        if (textura.id == 0) return;

        int frame_width = textura.width / frames_por_fila;
        int frame_height = textura.height / ((frames_totales + frames_por_fila - 1) / frames_por_fila);

        int fila = frame_actual / frames_por_fila;
        int columna = frame_actual % frames_por_fila;

        Rectangle source = {
            (float)(columna * frame_width), (float)(fila * frame_height),
            (float)frame_width, (float)frame_height
        };

        Rectangle dest = {
            pos.x - ancho / 2.0f, pos.y - alto / 2.0f,
            ancho, alto
        };

        DrawTexturePro(textura, source, dest, {0, 0}, 0.0f, WHITE);
    }

    void dibujar_volteado(Vector2 pos, float ancho, float alto) {
        if (textura.id == 0) return;

        int frame_width = textura.width / frames_por_fila;
        int frame_height = textura.height / ((frames_totales + frames_por_fila - 1) / frames_por_fila);

        int fila = frame_actual / frames_por_fila;
        int columna = frame_actual % frames_por_fila;

        // Invertir columna para volteado horizontal
        Rectangle source = {
            (float)((columna + 1) * frame_width), (float)(fila * frame_height),
            (float)(-frame_width), (float)frame_height
        };

        Rectangle dest = {
            pos.x - ancho / 2.0f, pos.y - alto / 2.0f,
            ancho, alto
        };

        DrawTexturePro(textura, source, dest, {0, 0}, 0.0f, WHITE);
    }

    void reset() {
        frame_actual = 0;
        timer = 0.0f;
    }
};
