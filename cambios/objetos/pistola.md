# Pistola

## Archivo
`src/objetos/pistola.h`

## Qué es
Objeto estático que dispara una bola al activarse. Apunta en una dirección fija configurable con `F`.

## Activación
Tiene dos modos:
1. **Por tensión de cuerda** — cuando la tensión de la cuerda conectada supera 80 unidades
2. **Por colisión directa** — solo si la golpea una `Bola`, `BolaBeisbol` o `Cubeta`

Solo dispara **una vez** (`ya_disparo = true` permanente).

## Tecla F
Invierte la dirección 180° (izquierda ↔ derecha).

## Como anclaje de cuerda
Usa `TipoAnclajeCuerda::Gancho`. Puede ser `extremo_a` o `extremo_b`.

```
[pistola] --- [torque] --- [balancin_izq]   ✓
[balancin_der] --- [pistola]                ✓
```

## Flujo típico (The Incredible Machine)
```
globo sube → choca balancín → balancín rota → tensión en cuerda → pistola dispara → bala viaja
```

## Parámetros de la bala
- Radio: 8.0
- Masa: 0.5
- Velocidad: 600 en dirección del cañón
- Restitución: 0.3, amortiguamiento: 0.01

## Serialización
```
ent PISTOLA id=X x=Y y=Z ang=A
```
Donde `ang` es el ángulo en grados (0 = derecha, 180 = izquierda).

## Visual
- Cuerpo gris oscuro horizontal
- Cañón metálico apuntando según dirección
- Mango marrón inclinado abajo
- Flash amarillo al disparar
