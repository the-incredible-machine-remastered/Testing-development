#!/usr/bin/env python3
"""
Genera src/objetos/catalogo_menu.gen.h escaneando TIM_MENU_SPAWN en src/objetos/*.h

Uso: python tools/generar_catalogo_menu.py
"""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
OBJ_DIR = ROOT / "src" / "objetos"
OUT = OBJ_DIR / "catalogo_menu.gen.h"
SKIP = {"obstaculo_estatico.h"}

LINE_RE = re.compile(
    r"TIM_MENU_SPAWN\s+"
    r"(?:id=)?(?P<id>[A-Za-z0-9_]+)?\s*"
    r'etiqueta="(?P<etiqueta>[^"]+)"\s*'
    r"tab=(?P<tab>\d+)\s*"
    r"categoria=(?P<cat>\d+)"
    r"(?:\s+disponible=(?P<disp>0|1))?"
    r"(?:\s+variante=(?P<var>[a-z_]+))?"
)


def snake_to_enum(name: str) -> str:
    return name.upper()


def default_id_from_file(path: Path) -> str:
    stem = path.stem
    return snake_to_enum(stem)


def parse_header(path: Path) -> list[dict]:
    entries = []
    text = path.read_text(encoding="utf-8", errors="replace")
    for line in text.splitlines():
        if "TIM_MENU_SPAWN" not in line:
            continue
        m = LINE_RE.search(line)
        if not m:
            print(f"WARN: linea no parseada en {path.name}: {line.strip()}")
            continue
        explicit_id = m.group("id")
        entry_id = explicit_id or default_id_from_file(path)
        if m.group("var") and not explicit_id:
            entry_id = f"{entry_id}_{m.group('var').upper()}"
        disp = m.group("disp")
        entries.append({
            "id": snake_to_enum(entry_id),
            "etiqueta": m.group("etiqueta"),
            "tab": int(m.group("tab")),
            "categoria": int(m.group("cat")),
            "disponible": disp != "0",
            "archivo": path.name,
        })
    return entries


def assign_pages(entries: list[dict], per_page: int = 6) -> None:
    groups: dict[tuple[int, int], list[dict]] = {}
    for e in entries:
        key = (e["tab"], e["categoria"])
        groups.setdefault(key, []).append(e)
    for group in groups.values():
        for i, e in enumerate(group):
            e["pagina"] = i // per_page


def emit_header(entries: list[dict]) -> str:
    ids = ["NINGUNO = -1"] + [e["id"] for e in entries] + ["COUNT"]
    enum_lines = ",\n    ".join(ids)

    rows = []
    for e in entries:
        disp = "true" if e["disponible"] else "false"
        rows.append(
            f'    {{ TipoObjetoMenu::{e["id"]}, "{e["etiqueta"]}", '
            f'{e["pagina"]}, {e["tab"]}, {e["categoria"]}, {disp} }},'
        )
    table = "\n".join(rows)

    return f"""#pragma once
// ============================================================================
// GENERADO AUTOMATICAMENTE — NO EDITAR A MANO
// Ejecutar: python tools/generar_catalogo_menu.py
// Fuente: etiquetas // TIM_MENU_SPAWN en src/objetos/*.h
// ============================================================================

enum class TipoObjetoMenu {{
    {enum_lines}
}};

struct ItemCatalogo {{
    TipoObjetoMenu tipo;
    const char* etiqueta;
    int pagina;
    int tab;
    int categoria;
    bool disponible;
}};

static const ItemCatalogo CATALOGO_MENU[] = {{
{table}
}};

static const int CATALOGO_MENU_COUNT =
    static_cast<int>(sizeof(CATALOGO_MENU) / sizeof(CATALOGO_MENU[0]));
"""


def main() -> None:
    all_entries: list[dict] = []
    for path in sorted(OBJ_DIR.glob("*.h")):
        if path.name in SKIP or path.name.endswith(".gen.h"):
            continue
        all_entries.extend(parse_header(path))

    if not all_entries:
        raise SystemExit("No se encontraron entradas TIM_MENU_SPAWN")

    # Orden estable: tab, categoria, archivo, etiqueta
    all_entries.sort(key=lambda e: (e["tab"], e["categoria"], e["archivo"], e["etiqueta"]))
    assign_pages(all_entries)

    OUT.write_text(emit_header(all_entries), encoding="utf-8")
    print(f"OK: {len(all_entries)} objetos -> {OUT.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
