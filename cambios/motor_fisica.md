# Motor de Física — Cambios

## Archivo
`src/fisica/motor_fisica.h`

## Nuevas funciones

### `disparar_pistolas()`
Llamada cada frame después de `cortar_cuerdas_con_tijeras()`.
- Itera todas las entidades buscando `Pistola`
- Si `pistola->get_disparada()` → crea una `Bola` en la punta del cañón con velocidad en la dirección de disparo
- Resetea el flag `disparada` (pero `ya_disparo` queda true para siempre)

### Activación de pistola por tensión (en `aplicar_tensiones_cuerda()`)
Después de aplicar tensión de cada cuerda:
- Si `tension >= 80.0` y algún extremo es `Pistola` → llama `pistola->activar_por_tension()`
- Esto activa `disparada = true` y `ya_disparo = true`

## Orden de operaciones por frame
```
1. Aplicar fuerzas externas (gravedad, viento)
2. Aplicar tensiones de cuerdas → [activar pistolas por tensión]
3. Integrar RK4
4. Detectar y resolver colisiones
4.5. Cortar cuerdas con tijeras
4.6. Disparar pistolas
5. Recolectar eventos especiales
```

## Includes agregados
```cpp
#include "../objetos/gancho.h"
#include "../objetos/pistola.h"
```

## Parámetros de cuerda ajustados
En `aplicar_tension()` de `cuerda.h`:
- `k = 800.0` (rigidez — casi inextensible)
- `c = 280.0` (amortiguación — evita rebote)
