# Balancín — Mejoras

## Archivo
`src/objetos/balancin.h`

## Cambios realizados

### Ángulo inicial configurable
Nuevo campo `angulo_inicial` — permite colocar el balancín pre-inclinado en el editor.

**Tecla F** cicla entre 3 estados:
```
equilibrio (0°) → inclinado derecha (+límite) → inclinado izquierda (-límite) → equilibrio
```

Método: `ciclar_inclinacion()` — cambia `angulo` y `angulo_inicial`, resetea `velocidad_angular = 0`.

### Resistencia del pivote
- Antes: `1800.0`
- Ahora: `600.0`
- Por qué: con la resistencia alta el globo no podía rotarlo; ahora responde mejor a fuerzas externas

### Serialización actualizada
```
ent BALANCIN id=X x=Y y=Z largo=L esp=E ang0=A
```
`ang0` guarda el ángulo inicial para que al cargar el nivel quede pre-inclinado.

## Casos de uso
```
balancín en equilibrio  → globo choca extremo bajo → rota a equilibrio → tensión activa pistola
balancín inclinado izq  → extremo izq abajo, der arriba
balancín inclinado der  → extremo der abajo, izq arriba
```
