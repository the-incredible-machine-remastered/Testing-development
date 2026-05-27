# Resumen del Proyecto: The Incredible Machine (TIM) Remastered

Este documento sirve como guía técnica y de arquitectura para que cualquier Inteligencia Artificial o desarrollador comprenda instantáneamente la estructura, el diseño de físicas y los objetos de este proyecto.

---

## 1. Arquitectura General y Tecnologías

El juego está desarrollado en **C++** utilizando la biblioteca multimedia **Raylib** para el renderizado 2D, manejo de texturas, fuentes e inputs. 

### Motor de Física Personalizado (`src/fisica/`)
El núcleo del juego es un motor de física de cuerpo rígido continuo desarrollado desde cero. Características clave:
* **Integración Numérica de Runge-Kutta de 4to Orden (RK4)**: En lugar de la integración de Euler simple (que diverge rápidamente en sistemas de resortes o colisiones consecutivas), se utiliza RK4 en `EntidadFisica::actualizar_fisica()`. Esto evalúa las aceleraciones en puntos intermedios del paso temporal para lograr una estabilidad rotacional y lineal extrema.
* **Resolución de Colisiones por Impulsos**: Se manejan rebotes con coeficientes de restitución y fricción en el punto de contacto. La fricción genera torque rotacional de forma natural, permitiendo a las bolas rodar y rotar según el contacto de superficie.
* **Fases de Detección (Broadphase + Narrowphase)**:
  * **Broadphase**: Detección de pares por fuerza bruta (O(n²)) optimizada para descartar objetos estáticos mutuos.
  * **Narrowphase**: Resoluciones específicas según la forma (`TipoForma::CIRCULO`, `TipoForma::AABB`, `TipoForma::POLIGONO`).

---

## 2. Estructura de Directorios

El código está organizado de la siguiente manera:

```text
TIM_Grafica/
├── Assets/                 # Recursos gráficos (sprites de Messi, fondos, HUD, etc.)
│   └── messi/              # Sprites para Messi (quieto, corriendo, cabezazo)
├── build/                  # Directorio de compilación de CMake
├── src/
│   ├── core/               # Clases base del motor y utilidades matemáticas
│   │   ├── entidad_fisica.h/cpp  # Clase madre de todos los cuerpos rígidos
│   │   ├── math_utils.h    # Constantes matemáticas (PI, EPSILON, clamp)
│   │   └── vector2d.h      # Vector 2D matemático con operaciones algebraicas
│   ├── fisica/             # Sistema de colisiones y motor
│   │   ├── colisiones.h    # Algoritmos estrechos (Círculo-Círculo, Círculo-AABB, Círculo-Polígono)
│   │   ├── fisica_ventilador.h # Lógica del viento e influencia neumática
│   │   └── motor_fisica.h  # Loop principal de integración, gravedad y dispatch
│   └── objetos/            # Clases derivadas para cada objeto del juego
│       ├── balancin.h      # Balancín oscilatorio rígido
│       ├── barril_chavo.h  # Obstáculo interactivo con El Chavo del 8
│       ├── bola.h          # Bola física clásica con colores/texturas
│       ├── bola_rebotadora.h # Bola elástica con alto rebote (superball)
│       ├── catalogo_menu.gen.h # [AUTO-GENERADO] Estructura del menú lateral
│       ├── obstaculo_estatico.h # Clase intermedia para cuerpos rígidos inmóviles
│       ├── pared_rectangular.h # Plataformas y paredes estáticas
│       ├── plano_inclinado.h # Rampa triangular (POLIGONO) con soporte de inversión (Flip)
│       ├── seguidor_booster.h # Futbolista interactivo (Messi) que corre y cabecea
│       ├── trampolin.h     # Trampolín elástico animado
│       └── ventilador.h    # Ventilador rotativo que sopla aire direccional
├── tools/
│   └── generar_catalogo_menu.py # Script de Python que escanea anotaciones y autogenera el menú
├── main.cpp                # Bucle de juego principal, renderizado Raylib, HUD y UI
└── CMakeLists.txt          # Configuración del sistema de construcción CMake
```

---

## 3. Catálogo y Catálogo de Objetos Interactivos

Los objetos del juego se declaran con una anotación especial de comentario en sus headers (ej. `// TIM_MENU_SPAWN id=RAMPA etiqueta="Rampa" tab=0 categoria=0`). El script en `tools/generar_catalogo_menu.py` escanea los headers y genera `src/objetos/catalogo_menu.gen.h`, poblando el menú dinámicamente.

### Objetos del Juego y sus Comportamientos:

1. **PlanoInclinado (Rampa)** (`src/objetos/plano_inclinado.h`):
   * **Clasificación**: `TipoForma::POLIGONO` (3 vértices).
   * **Rotación/Inversión**: Unificado en un solo objeto del menú. Al presionar **`F`** en modo diseño, se llama a `invertir()`, lo que conmuta el flag `es_invertido` y recalcula los vértices en orden antihorario para alternar la pendiente entre izquierda (`\`) y derecha (`/`).
   * **Físicas**: Utiliza el robusto resolvedor `circulo_vs_poligono` que soporta penetración profunda, empujando la bola radialmente hacia la arista más cercana para evitar que las bolas atraviesen la rampa a alta velocidad.

2. **Ventilador** (`src/objetos/ventilador.h`):
   * **Clasificación**: `TipoForma::AABB`.
   * **Rotación/Inversión**: Al presionar **`F`**, cambia su dirección entre derecha (`1, 0`) e izquierda (`-1, 0`).
   * **Físicas**: Soplido direccional en `FisicaVentilador::aplicar()`. Aplica una fuerza sobre las bolas en el área rectangular frente a sus aspas.
   * **Fórmula de Viento**: La fuerza se atenúa de forma no lineal inversamente proporcional a la distancia:
     $$\text{Fuerza} = \text{Potencia} \times \left(1.0 - \frac{\text{distancia}}{\text{rango}}\right) \times \left(\frac{50.0}{\text{distancia} + 15.0}\right) \times \text{factor\_centro}$$
     Esto genera un soplido increíblemente potente cuando la bola está extremadamente cerca de las aspas y decae suavemente hacia el límite de su rango (220px).

3. **SeguidorBooster (Futbolista Messi)** (`src/objetos/seguidor_booster.h`):
   * **Clasificación**: `TipoForma::AABB` dinámico (masa de 15.0, pesado).
   * **Mecánica 1: Patada Frontal**: Si detecta una bola a su nivel dentro de su rango de detección (450px), corre hacia ella y, al hacer contacto cercano, aplica un potente chute de fuerza `680.0` a 45 grados en su dirección de carrera.
   * **Mecánica 2: Cabezazo Suave**: Si una bola cae verticalmente sobre su cabeza (tope superior del AABB, verificado por `detectar_cabezazo()`), aplica un impulso suave de fuerza `350.0` a 45 grados hacia adelante-arriba. Activa un estado de cabezazo animado durante 0.6 segundos y, al igual que en la patada frontal, transiciona de inmediato al estado de retorno (`EstadoSeguidor::RETRAYENDO`) para regresar trotando a su base de inicio.
   * **Visuales**: Cambia dinámicamente de sprite a `messi-cabezazo.png` durante la acción del cabezazo.

4. **Trampolín** (`src/objetos/trampolin.h`):
   * **Clasificación**: `TipoForma::AABB` estático.
   * **Físicas**: Al colisionar con una bola por la parte superior, deforma su resorte animadamente y aplica un impulso elástico vertical restaurador proporcional a la velocidad de impacto de la bola.

5. **Balancín (Sube y Baja)** (`src/objetos/balancin.h`):
   * **Clasificación**: `TipoForma::POLIGONO` (OBB rotatoria).
   * **Físicas**: Célula pivotada fija en el centro que responde a torques cinemáticos. Si una bola cae en un extremo, el balancín rota, impulsando cualquier objeto en el extremo opuesto hacia arriba a través de la transferencia de momento angular.

6. **BarrilChavo** (`src/objetos/barril_chavo.h`):
   * **Clasificación**: `TipoForma::AABB` estático.
   * **Físicas**: Si una bola colisiona con él, "El Chavo" salta animadamente del barril, empujando la bola con un factor de impulso vertical.

7. **Bola y BolaRebotadora** (`src/objetos/bola.h`, `bola_rebotadora.h`):
   * **Clasificación**: `TipoForma::CIRCULO` dinámico.
   * **Físicas**: Cuerpos circulares que sufren la gravedad y colisiones elásticas continuas. La rebotadora posee un coeficiente de restitución cercano a `0.9` (alto rebote).

8. **ParedRectangular (Pared/Plataforma/Ladrillo)** (`src/objetos/pared_rectangular.h`):
   * **Clasificación**: `TipoForma::AABB` estático.
   * **Físicas**: Fronteras inmóviles contra las cuales colisionan e interactúan las bolas y otros objetos dinámicos.

---

## 4. Flujo del Ciclo de Vida del Motor (Game Loop)

En cada frame (`main.cpp`), mientras el motor no esté pausado:
1. **Gravedad**: Se aplica la fuerza de gravedad a todas las entidades móviles (`paso_fisico` -> `aplicar_gravedad`).
2. **Viento**: Se acumula la corriente de viento del Ventilador en las bolas circundantes (`FisicaVentilador::aplicar`).
3. **Controladores Inteligentes**: Messi actualiza su comportamiento (`actualizar_comportamiento`), buscando objetivos de patadas frontales o comprobando contactos para el cabezazo.
4. **Integración Numérica (RK4)**: Se computa el nuevo estado cinemático lineal y angular de todas las entidades para el paso temporal `dt` utilizando el resolvedor Runge-Kutta 4.
5. **Detección de Colisiones**: Se ejecuta el broadphase y narrowphase para encontrar intersecciones exactas.
6. **Resolución Física e Impulsos**: Se corrigen las posiciones (para evitar solapamientos) y se aplican impulsos lineales y angulares de restitución y fricción.

---

## 5. Notas sobre los Assets Visuales

Los assets visuales cargados en `main.cpp` son opcionales y actúan como "máscaras" estéticas de texturas sobre los cuerpos geométricos físicos reales subyacentes:
* Si las texturas PNG en `Assets/` están ausentes, el juego realiza automáticamente un renderizado geométrico vectorial básico como fallback (círculos y rectángulos planos de colores), manteniendo las físicas exactamente iguales e intactas.
* Los sprites de Messi (`messi-normal.png`, `mesirve.png` y `messi-cabezazo.png`) guían la escala del renderizado de la entidad `SeguidorBooster`, proporcionando una experiencia visual premium con oscilaciones de carrera y animaciones fluidas sin interferir en su hitbox AABB rígida.

---
*Este documento es auto-contenido y provee la ontología técnica completa necesaria para guiar cualquier desarrollo posterior o refactorización del motor.*
