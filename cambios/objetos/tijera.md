# Tijera — Mejoras

## Archivo
`src/objetos/tijera.h`

## Cambios realizados

### Visual ingame
- Aros ahora tienen **relleno rojo** (`Color{200, 40, 40, 255}`) con borde gris oscuro
- Antes solo eran `DrawCircleLines` (vacíos)

### Ícono menú
- Tijera horizontal: aros rojos a la izquierda, cuchillas abriéndose a la derecha
- Línea verde vertical de corte
- Pivote gris en el centro

## Lógica (sin cambios)
- Se activa por colisión con objeto dinámico (`permanentemente_activada`)
- Corta cuerdas UNA sola vez (`ya_corto_cuerdas`)
- Ver `cambios/cuerda/corte_cuerda_tijera.md` para lógica de corte
