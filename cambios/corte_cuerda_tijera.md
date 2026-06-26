# Corte de Cuerda con Tijera

## Archivo relevante
`src/fisica/motor_fisica.h` — función `cortar_cuerdas_con_tijeras()`

## Objetos involucrados
- **Tijera**: obstáculo estático. Se activa al ser golpeada por un objeto dinámico (bola, etc). Una vez activada, corta cuerdas que pasen por su zona. Solo corta UNA vez (`ya_corto_cuerdas`).
- **Cuerda**: constraint entre dos extremos (`extremo_a`, `extremo_b`) pasando por soportes intermedios (`soportes_id`). Estructura interna: `[extremo_a, sop0, sop1, ..., extremo_b]`
- **SoporteTorque**: punto fijo por el que pasa la cuerda.
- **Globo**: extremo dinámico de la cuerda. Flota hacia arriba cuando queda libre.
- **Bola** (pequeña, r=2, m=0.3): se crea al cortar para simular la punta suelta de la cuerda cayendo.

---

## Estructura de ejemplo

```
[GLOBO1] --- [torque_izq] --- [torque_der] --- [GLOBO2]
  extremo_a     sops[0]          sops[1]        extremo_b
  (tipo Globo)                               (tipo Globo)
```

`puntos[]` al llamar `obtener_puntos()`:
```
puntos[0] = GLOBO1
puntos[1] = torque_izq
puntos[2] = torque_der
puntos[3] = GLOBO2
n = 4
```

---

## Caso 1: Corte entre [torque_der] y [GLOBO2]

**Tijera ubicada entre torque_der y GLOBO2.**

```
seg_cortado = 3  (segmento puntos[2]→puntos[3])
num_sops_a  = 2
num_sops_b  = 0
```

### Antes:
```
[GLOBO1] --- [torque_izq] --- [torque_der] --- [GLOBO2]
                                         ✂ aquí
```

### Después:
```
[GLOBO1] --- [torque_izq]  ~~~bolita suelta cae
[torque_der]  (solo, desconectado)
[GLOBO2] ↑  sube libre
```

### Código (bloque LADO A, `num_sops_a >= 2`):
- Cuerda nueva: `extremo_a=GLOBO1`, soportes=`[]`, `extremo_b=torque_izq` (SoporteFijo)
- Bolita suelta anclada a `torque_izq`, longitud = distancia `torque_izq→torque_der`

---

## Caso 2: Corte entre [GLOBO1] y [torque_izq]

**Tijera ubicada entre GLOBO1 y torque_izq.**

```
seg_cortado = 1  (segmento puntos[0]→puntos[1])
num_sops_a  = 0
num_sops_b  = 2
```

### Antes:
```
[GLOBO1] --- [torque_izq] --- [torque_der] --- [GLOBO2]
       ✂ aquí
```

### Después:
```
[GLOBO1] ↑  sube libre
[torque_izq]  (solo, desconectado)
[torque_der] --- [GLOBO2]  (intacta)
~~~bolita suelta cae desde torque_der
```

### Código (bloque LADO B, `num_sops_b >= 2`):
- Cuerda nueva: `extremo_a=torque_der` (SoporteFijo), soportes=`[]`, `extremo_b=GLOBO2`
- Bolita suelta anclada a `torque_der`, longitud = distancia `torque_der→GLOBO2`
- `torque_izq` (primer soporte del lado B) se descarta — queda solo

---

## Lógica general de índices

```
sops[i]  corresponde a  puntos[i+1]

Lado A: sops[0 .. num_sops_a-1]   donde num_sops_a = seg_cortado - 1
Lado B: sops[num_sops_a .. num_sops-1]
```

- **LADO A** activa cuando `num_sops_a >= 2`: el penúltimo soporte (`sops[num_sops_a-2]`) es el nuevo `extremo_b`. El último soporte (`sops[num_sops_a-1]`) se descarta. Bolita cuelga del penúltimo.
- **LADO B** activa cuando `num_sops_b >= 2`: el segundo soporte del lado B (`sops[num_sops_a+1]`) es el nuevo `extremo_a`. El primero (`sops[num_sops_a]`) se descarta. Bolita cuelga del segundo.

---

## Parámetros de la bolita suelta
```cpp
Bola(id, pos, radio=2.0, masa=0.3)
set_amortiguamiento(0.08)  // frena rápido, no oscila
set_restitucion(0.05)      // casi sin rebote
```
Posición inicial: soporte + Vector2D(0, longitud * 0.3) — empieza cerca del soporte y cae.
