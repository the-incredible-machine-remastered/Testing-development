# Gancho

## Archivo
`src/objetos/gancho.h`

## Qué es
Punto fijo de anclaje para cuerdas. Visualmente es un tornillo de ojo (eye bolt) metálico — cuerpo con rosca abajo y aro circular arriba.

## Para qué sirve
Permite terminar una cuerda en un punto fijo sin necesitar un objeto dinámico. Reemplaza el caso incorrecto de usar un torque como extremo de cuerda.

## Casos válidos
```
[globo] --- [gancho]                      ✓ directo sin torques
[gancho] --- [torque] --- [globo]         ✓ gancho como extremo_a
[balancin_izq] --- [torque] --- [gancho]  ✓ gancho como extremo_b
[pistola] --- [gancho]                    ✓
```

## Tipo de anclaje
`TipoAnclajeCuerda::Gancho` — mismo comportamiento que `Globo` en posición (usa `e->get_posicion()`).

## Restricciones
- No puede ser soporte intermedio (solo torques pueden serlo)
- Puede ser `extremo_a` o `extremo_b`
- Es estático (no se mueve)

## Serialización
```
ent GANCHO id=X x=Y y=Z
```

## Ícono menú
Tornillo de ojo metálico (aro + cuerpo con rosca), gris plateado.

## Corte con tijera
Si la cuerda es `[globo]---[gancho]` sin torques (`num_sops=0`), al cortarla desaparece entera y el globo sube libre.
