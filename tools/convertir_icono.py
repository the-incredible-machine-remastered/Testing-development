#!/usr/bin/env python3
"""
Convierte icon.png a un archivo .ico de multiples resoluciones para Windows.
"""
import os
from PIL import Image

def main():
    base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    png_path = os.path.join(base_dir, "icon.png")
    ico_path = os.path.join(base_dir, "icon.ico")
    
    if not os.path.exists(png_path):
        print(f"Error: No se encontro el archivo {png_path}")
        return
        
    try:
        img = Image.open(png_path)
        # Resoluciones estandar de iconos en Windows
        sizes = [(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
        # Guardar como ICO
        img.save(ico_path, format="ICO", sizes=sizes)
        print(f"Exito: Icono generado en {ico_path}")
    except Exception as e:
        print(f"Error al convertir la imagen: {e}")

if __name__ == "__main__":
    main()
