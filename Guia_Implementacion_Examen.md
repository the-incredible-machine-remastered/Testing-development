# Guía de Respuestas para la Reunión con el Profesor: Implementaciones de KIWICHA

Esta guía resume de manera estructurada los fundamentos del motor físico y el apartado gráfico del proyecto **KIWICHA**. Úsala como material de apoyo para responder a las preguntas técnicas del docente sobre el código y las mecánicas implementadas.

---

## 1. El Motor de Física 2D (Conceptos Clave)

El motor físico fue desarrollado **desde cero** (sin dependencias como Box2D) y se basa en tres pilares: Integración, Detección de Colisiones y Resolución de Impulsos.

### A. Integración Temporal (Método de Runge-Kutta de 4to Orden - RK4)
Para actualizar la posición, velocidad, ángulo y velocidad angular de los objetos dinámicos, el motor no usa un método de Euler simple, sino **Runge-Kutta de 4to Orden (RK4)**:
* Para una ecuación diferencial de la forma $y' = f(t, y)$ donde el estado de la entidad es $y = [\text{posición}, \text{velocidad}]$:
  1. $k_1 = f(t, y_n)$ (derivada en el estado inicial).
  2. $k_2 = f(t + \frac{dt}{2}, y_n + \frac{dt}{2} k_1)$ (evaluación en la mitad del paso usando $k_1$).
  3. $k_3 = f(t + \frac{dt}{2}, y_n + \frac{dt}{2} k_2)$ (evaluación en la mitad del paso usando $k_2$).
  4. $k_4 = f(t + dt, y_n + dt \cdot k_3)$ (evaluación al final del paso usando $k_3$).
  5. Combinación final ponderada de las derivadas para avanzar el estado:
     $$y_{n+1} = y_n + \frac{dt}{6} (k_1 + 2k_2 + 2k_3 + k_4)$$
* **¿Por qué este método?** Es un integrador de orden $O(dt^4)$ (mucho más preciso y estable que Euler, que es de orden $O(dt)$). Es ideal para sistemas mecánicos de precisión como resortes, cañones, colisiones elásticas complejas y péndulos (balancines).

### B. Detección de Colisiones (Geometrías)
El motor evalúa tres tipos de colisiones geométricas en el plano 2D:
* **Círculo vs Círculo (Pelotas, Poleas):** Se calcula la distancia euclidiana entre los centros. Si es menor que la suma de sus radios, hay colisión.
  $$\text{distancia}^2 < (r_1 + r_2)^2$$
* **AABB vs AABB (Plataformas, Ladrillos):** Cajas alineadas con los ejes. Se verifica si se solapan en los ejes X e Y de forma simultánea.
* **Círculo vs AABB (Pelotas contra Rampas/Ladrillos):** Se encuentra el punto más cercano en el rectángulo AABB al centro del círculo. Si la distancia del centro a ese punto es menor al radio, colisionan.

### C. Resolución de Colisiones (Física de Impulsos)
Cuando ocurre una colisión, se calcula un **impulso instantáneo** ($J$) que modifica las velocidades lineales y angulares de los cuerpos para que reboten de forma realista:
* Se usa el **coeficiente de restitución** ($e$) para definir qué tan elástica es la colisión (de $0$ a $1$, donde $1$ es rebote perfecto).
* Se aplica una fricción tangente para frenar el deslizamiento lateral.

### D. Restricciones Físicas (Cuerdas y Correas)
* **Cuerdas:** Conectan dos objetos (ej. una cubeta a un contrapeso). El motor calcula la distancia entre ambos extremos; si supera la longitud máxima de la cuerda, aplica una fuerza correctiva de tensión en dirección del eje de la cuerda para mantenerlos unidos.
* **Correas de Transmisión:** Transmiten velocidad angular ($\omega$). Al conectar la rueda de hámster a un ventilador, el ventilador recibe el mismo torque multiplicando la velocidad por la relación de radios de los ejes:
  $$\omega_{\text{salida}} = \omega_{\text{entrada}} \cdot \left(\frac{r_{\text{entrada}}}{r_{\text{salida}}}\right)$$

---

## 2. El Apartado Gráfico en Raylib (Conceptos Gráficos)

Como implementador del arte, sprites y de la parte visual, debes explicar cómo viajan las imágenes del disco a la pantalla y cómo se aplican transformaciones 2D usando **Raylib**.

### A. Ciclo de Vida de los Assets (De Disco a VRAM)
1. **Lectura de Disco (`LoadImage`):** Raylib carga el archivo comprimido (PNG) del disco duro y lo decodifica en la memoria RAM del sistema como una estructura de píxeles planos (`Image`).
2. **Subida a VRAM (`LoadTextureFromImage`):** Transfiere esos píxeles crudos a la memoria de video (VRAM) de la tarjeta gráfica a través de comandos de OpenGL, creando un objeto `Texture2D` con un identificador de textura (`Texture.id`).
3. **Liberación de RAM:** La imagen en memoria RAM del sistema se destruye para liberar espacio. Solo la textura en VRAM permanece activa.
4. **Limpieza (`UnloadTexture`):** Al cerrar el juego, es vital llamar a `UnloadTexture(textura)` para liberar la memoria de video de la GPU y evitar fugas de memoria (memory leaks).

### B. El Bucle de Dibujo (Drawing Pipeline)
Toda llamada de dibujo ocurre obligatoriamente en el hilo principal entre:
* **`BeginDrawing()`**: Limpia el buffer de color de la pantalla (generalmente con un color de fondo) y prepara la matriz de proyección ortogonal 2D.
* **`EndDrawing()`**: Realiza el intercambio de buffers (Double Buffering) enviando todo el lote de comandos de dibujo acumulados (Batch Rendering) a la pantalla para evitar parpadeos (screen tearing).

### C. Animación de Sprites por Segmentación (Slicing)
Para que el gato o el hámster/rata caminen en lugar de mostrarse estáticos, usamos un **Spritesheet** (una sola imagen horizontal con varios fotogramas uno al lado del otro):
* **Cálculo del Frame Rect (`Rectangle src`):**
  Definimos el ancho de un solo fotograma dividiendo el ancho de la textura total entre el número de frames ($W_f = \text{AnchoTotal} / 8$).
  En base al tiempo transcurrido (`GetTime()`), incrementamos el índice del frame:
  $$\text{src.x} = \text{frame\_actual} \cdot W_f$$
  $$\text{src.y} = 0$$
* **Espejado (Flipping):**
  Para que la criatura mire a la izquierda o derecha usando la misma imagen, multiplicamos el ancho del rectángulo origen por $-1.0f$:
  $$\text{src.width} = W_f \cdot d \quad (d = 1.0f \text{ derecha, } -1.0f \text{ izquierda})$$
  Si el ancho enviado a la GPU es negativo, el rasterizador de OpenGL invierte horizontalmente las coordenadas UV al renderizar.

### D. Pivote y Rotación con `DrawTexturePro`
Para objetos que rotan sobre su propio eje (ruedas, engranajes, aspas de ventilador, cañones y la linterna):
* No podemos usar el dibujo tradicional porque rotaría desde la esquina superior izquierda $(0,0)$ del sprite.
* Usamos **`DrawTexturePro`**, que requiere:
  1. `src`: Qué región de la textura dibujar.
  2. `dst`: Dónde dibujarlo en la pantalla (coordenadas de destino).
  3. `origin`: El vector que define el **punto de pivote** relativo al tamaño de destino.
     Para rotar sobre el centro exacto del objeto:
     $$\text{origin} = \{ \text{ancho\_destino} / 2.0, \text{alto\_destino} / 2.0 \}$$
  4. `rotation`: El ángulo físico acumulado por el motor (en grados).

---

## 3. Preguntas Frecuentes del Profesor (y cómo responderlas)

1. **¿Cómo gestionan el Delta Time ($dt$) en el bucle?**
   * *Respuesta:* Pasamos el `GetFrameTime()` del motor a la actualización física. El motor físico usa un actualizador interno para asegurar estabilidad en la integración temporal de las colisiones.
2. **¿Por qué la rata dentro de la rueda no se sale si es dinámica?**
   * *Respuesta:* La rata dentro de la rueda no es un cuerpo dinámico independiente. Es un elemento visual renderizado con coordenadas polares relativas al centro de la rueda (`px`, `py`). Su posición angular se fija en el cuadrante inferior (aprox. $90^\circ \pm 20^\circ$) según el sentido de rotación.
3. **¿Cómo funciona el sistema de guardado y carga?**
   * *Respuesta:* Usamos una clase de serialización que escribe en un archivo binario/texto con formato `.tim` la posición, ángulo, tipo y estado de fijación (`es_fijo`) de todas las entidades en el canvas. Al cargar, borramos el canvas actual, instanciamos de nuevo los objetos con los datos leídos de disco, y reconstruimos el estado físico inicial.
4. **¿Por qué se usan texturas de potencia de 2 (Power of Two) en GPU?**
   * *Respuesta:* Aunque OpenGL moderno soporta texturas NPOT (Non-Power-of-Two), usar texturas proporcionales facilita el escalado de mipmaps y optimiza el acceso a la caché de texturas de la GPU al realizar el mapeado de spritesheets.
