# Sistema de Anclajes de Cuerda

## Archivo principal
`src/objetos/cuerda.h` — enum `TipoAnclajeCuerda`

## Tipos de anclaje

| Tipo | Objeto | extremo_a | extremo_b | soporte intermedio |
|------|--------|-----------|-----------|-------------------|
| `Cubeta` | Cubeta | ✓ | ✓ | ✗ |
| `BalancinIzquierdo` | Balancín | ✓ | ✓ | ✗ |
| `BalancinDerecho` | Balancín | ✓ | ✓ | ✗ |
| `SoporteFijo` | SoporteTorque | ✗ | ✗ | ✓ solo |
| `Globo` | Globo | ✓ | ✓ | ✗ |
| `Gancho` | Gancho / Pistola | ✓ | ✓ | ✗ |

## Regla fundamental
> El torque (`SoporteFijo`) **solo puede ser soporte intermedio**, nunca `extremo_a` ni `extremo_b`.

Esto se refuerza en el editor: al crear una cuerda, el primer click en un torque es ignorado.

## Casos válidos de cuerda

```
[globo] --- [t0] --- [t1] --- [globo2]          N torques entre dos dinámicos
[globo] --- [gancho]                              directo sin torques
[balancin_izq] --- [t0] --- [pistola]            balancín a pistola vía torque
[balancin_der] --- [pistola]                      balancín a pistola directo
[gancho] --- [t0] --- [t1] --- [cubeta]          gancho como origen
```

## Serialización de tipo anclaje
Se guarda como entero (`tipo_anclaje_a_int`):
```
0 = Cubeta
1 = BalancinIzquierdo
2 = BalancinDerecho
3 = SoporteFijo
4 = Globo
5 = Gancho
```

## Posición del anclaje
- `Cubeta` → `cubeta->get_punto_cuerda()`
- `BalancinIzquierdo` → `bal->get_punto_extremo_izquierdo()`
- `BalancinDerecho` → `bal->get_punto_extremo_derecho()`
- `SoporteFijo`, `Globo`, `Gancho` → `e->get_posicion()`
