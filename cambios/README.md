# Cambios y nuevas funcionalidades

## Estructura

```
cambios/
├── README.md                        — este archivo, índice general
├── objetos/
│   ├── gancho.md                    — nuevo objeto Gancho
│   ├── pistola.md                   — nuevo objeto Pistola
│   ├── globo.md                     — mejoras al Globo
│   ├── balancin.md                  — mejoras al Balancín
│   └── tijera.md                    — mejoras a la Tijera
├── cuerda/
│   ├── corte_cuerda_tijera.md       — lógica de corte (original)
│   ├── corte_cuerda_tijera_codigo.h — snapshot del código de corte
│   └── anclajes.md                  — sistema de anclajes y casos válidos
├── motor_fisica_backup.h            — backup completo de motor_fisica.h
└── motor_fisica.md                  — cambios en el motor de física
```

## Resumen de cambios

### Nuevos objetos
- **Gancho** — punto fijo de anclaje para cuerdas (tornillo de ojo metálico)
- **Pistola** — dispara bola al activarse por tensión de cuerda o colisión

### Objetos mejorados
- **Globo** — flotación más fuerte, empuja balancines activamente, sin rebote
- **Balancín** — estado inicial configurable con `F` (equilibrio / izq / der)
- **Tijera** — aros rojos en ingame, ícono mejorado en menú

### Sistema de cuerdas
- Corte generalizado para N torques
- Torque no puede ser `extremo_a` (bloqueado en editor)
- Nuevo tipo de anclaje `Gancho` (usado por Gancho y Pistola)
- Pistola se activa por tensión de cuerda superando umbral

### Motor de física
- `disparar_pistolas()` — procesa pistolas activadas cada frame
- Activación de pistola por tensión en `aplicar_tensiones_cuerda()`
