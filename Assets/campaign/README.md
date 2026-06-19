# Campaña Oficial - Carpeta de Niveles

Esta carpeta está destinada a los archivos de niveles de la campaña oficial del juego.

Si eres el desarrollador y deseas diseñar niveles en el **Modo Creativo** para la campaña oficial:
1. Diseña el nivel y guárdalo en el juego.
2. Ve a la carpeta de partidas guardadas del usuario (`saves/`).
3. Copia el archivo `.tim` generado a esta carpeta (`Assets/campaign/`).
4. Renombra el archivo según el nivel correspondiente:
   * Nivel 1: `1_primer_impacto.tim`
   * Nivel 2: `2_rebote_perfecto.tim`
   * Nivel 3: `3_el_soplido.tim`
   * Nivel 4: `4_reaccion_fisica.tim`

El juego detectará automáticamente la presencia de estos archivos en esta carpeta. Si existen, los cargará dinámicamente; de lo contrario, utilizará los niveles predefinidos por defecto directamente en el código de memoria.
