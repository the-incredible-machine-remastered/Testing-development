# Globo — Mejoras

## Archivo
`src/objetos/globo.h`

## Cambios realizados

### Fuerza de flotación
- Antes: `1200.0`
- Ahora: `4500.0`
- Por qué: necesita fuerza suficiente para empujar el balancín y llevarlo a equilibrio

### Física
- Restitución: `0.6` → `0.02` — casi sin rebote, presiona en lugar de botar
- Fricción: `0.4` → `0.1`
- Amortiguamiento: `0.015` → `0.004` — sube más rápido

### on_collision nuevo
Cuando choca con algo arriba (normal.y > 0.3): cancela velocidad vertical para que no rebote y siga presionando.

Cuando choca con un **Balancín**: aplica torque extra en el extremo más cercano del balancín (máximo brazo de palanca) con fuerza `FUERZA_FLOTACION * 3.0` hacia arriba.

```cpp
Vector2D punto_extremo = (dist_izq < dist_der) ? ext_izq : ext_der;
Vector2D fuerza_up(0.0, -FUERZA_FLOTACION * 3.0);
bal->aplicar_fuerza_en_punto(punto_extremo, fuerza_up);
```

## Comportamiento esperado
- Globo libre sube rápido
- Al tocar el extremo bajo de un balancín, lo empuja hacia arriba con fuerza suficiente para llevarlo a equilibrio
- No rebota — presiona continuamente
- Al quedar libre (cuerda cortada) sube sin detenerse
