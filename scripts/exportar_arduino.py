#!/usr/bin/env python3
"""Exporta las fuentes actuales a una carpeta compatible con Arduino IDE."""

import argparse
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DESTINO = ROOT / "build" / "arduino" / "supervision_ambiental"

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--con-secretos", action="store_true",
                        help="copiar include/secrets.h para uso privado local")
    args = parser.parse_args()
    if DESTINO.exists():
        parser.error(f"Ya existe {DESTINO}. Retiralo antes de volver a exportar.")
    secreto = ROOT / "include" / "secrets.h"
    if args.con_secretos and not secreto.is_file():
        parser.error("No existe include/secrets.h")
    archivos = [
        ROOT / "src" / "main.cpp",
        ROOT / "include" / "ComandoRele.h",
        ROOT / "include" / "secrets.example.h",
    ]
    archivos.extend(sorted((ROOT / "lib").glob("*/*.h")))
    archivos.extend(sorted((ROOT / "lib").glob("*/*.cpp")))
    if args.con_secretos:
        archivos.append(secreto)
    nombres = [archivo.name for archivo in archivos]
    if len(nombres) != len(set(nombres)):
        parser.error("Hay archivos con nombres repetidos; no se puede exportar.")
    src = DESTINO / "src"
    src.mkdir(parents=True)
    shutil.copy2(ROOT / "supervision_ambiental.ino", DESTINO)
    for archivo in archivos:
        shutil.copy2(archivo, src / archivo.name)
    print(f"Sketch generado en {DESTINO}")
    if args.con_secretos:
        print("Contiene credenciales privadas: no compartir esta carpeta.")

if __name__ == "__main__":
    main()
