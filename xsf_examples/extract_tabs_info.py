#!/usr/bin/env python3
"""Extract molecule metadata and coordinates from xsf_examples/TABS.pdf.

Usage:
    python extract_tabs_info.py --molecule-number 758
"""

from __future__ import annotations

import argparse
import json
import re
import sys
import zlib
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterator, List, Optional, Sequence, Tuple, Union


MIN_MOLECULE = 1
MAX_MOLECULE = 1641
DEFAULT_PDF = Path("xsf_examples/TABS.pdf")
DEFAULT_GRID_DIR = Path("xsf_examples/TABS_data")

# Keep this symbol table aligned with webassembly/src/atoms/domain/element_database.cpp
# (chemical_symbols array) and file_io_manager.cpp.
CHEMICAL_SYMBOLS: Sequence[str] = (
    "",
    "H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne",
    "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca",
    "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
    "Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr",
    "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn",
    "Sb", "Te", "I", "Xe", "Cs", "Ba", "La", "Ce", "Pr", "Nd",
    "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb",
    "Lu", "Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg",
    "Tl", "Pb", "Bi", "Po", "At", "Rn", "Fr", "Ra", "Ac", "Th",
    "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm",
    "Md", "No", "Lr", "Rf", "Db", "Sg", "Bh", "Hs", "Mt", "Ds",
    "Rg", "Cn", "Nh", "Fl", "Mc", "Lv", "Ts", "Og",
)
SYMBOL_TO_ATOMIC_NUMBER = {sym: i for i, sym in enumerate(CHEMICAL_SYMBOLS) if sym}


@dataclass
class MoleculeData:
    molecule_number: int
    title: str
    point_group: str
    energy: float
    zpe: float
    lumo_eps: float
    homo_eps: float
    dipole: float
    atoms: List[Tuple[str, float, float, float]]


@dataclass
class GridEntry:
    label: str
    values: Any  # numpy.ndarray with shape (nx, ny, nz)


def log_step(message: str) -> None:
    print(f"[extract_tabs] {message}", file=sys.stderr, flush=True)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Extract one molecule entry from TABS.pdf and write JSON + XSF outputs."
    )
    parser.add_argument(
        "--molecule-number",
        type=int,
        required=True,
        help=f"Molecule number in [{MIN_MOLECULE}, {MAX_MOLECULE}]",
    )
    parser.add_argument(
        "--pdf-path",
        type=Path,
        default=DEFAULT_PDF,
        help=f"Path to TABS.pdf (default: {DEFAULT_PDF})",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("."),
        help="Directory for output files (default: current directory)",
    )
    parser.add_argument(
        "--format",
        choices=("crystal", "molecule"),
        default="crystal",
        help="XSF output format (default: crystal)",
    )
    parser.add_argument(
        "--add-grid",
        type=int,
        choices=(0, 1),
        default=0,
        help="Append DATAGRID_3D from xsf_examples/TABS_data/NNNN.h5 (default: 0)",
    )
    return parser.parse_args()


def unescape_pdf_literal(s: str) -> str:
    out: List[str] = []
    i = 0
    n = len(s)
    while i < n:
        ch = s[i]
        if ch != "\\":
            out.append(ch)
            i += 1
            continue

        i += 1
        if i >= n:
            break
        esc = s[i]

        if esc in ("n", "r", "t", "b", "f"):
            mapping = {"n": "\n", "r": "\r", "t": "\t", "b": "\b", "f": "\f"}
            out.append(mapping[esc])
            i += 1
            continue
        if esc in ("\\", "(", ")"):
            out.append(esc)
            i += 1
            continue
        if esc in ("\n", "\r"):
            # Line continuation.
            if esc == "\r" and i + 1 < n and s[i + 1] == "\n":
                i += 2
            else:
                i += 1
            continue
        if esc.isdigit():
            oct_digits = esc
            i += 1
            for _ in range(2):
                if i < n and s[i].isdigit():
                    oct_digits += s[i]
                    i += 1
                else:
                    break
            out.append(chr(int(oct_digits, 8) & 0xFF))
            continue

        # Unknown escape, keep the escaped char.
        out.append(esc)
        i += 1

    return "".join(out)


Token = Union[str, float]


def parse_tj_array(content: str) -> List[Token]:
    tokens: List[Token] = []
    i = 0
    n = len(content)

    while i < n:
        ch = content[i]
        if ch.isspace():
            i += 1
            continue

        if ch == "(":
            i += 1
            depth = 1
            buf: List[str] = []
            while i < n and depth > 0:
                c = content[i]
                if c == "\\":
                    if i + 1 < n:
                        buf.append(c)
                        buf.append(content[i + 1])
                        i += 2
                    else:
                        buf.append(c)
                        i += 1
                    continue
                if c == "(":
                    depth += 1
                    buf.append(c)
                    i += 1
                    continue
                if c == ")":
                    depth -= 1
                    if depth == 0:
                        i += 1
                        break
                    buf.append(c)
                    i += 1
                    continue
                buf.append(c)
                i += 1
            tokens.append(unescape_pdf_literal("".join(buf)))
            continue

        m = re.match(r"[+-]?(?:\d+\.\d+|\d+|\.\d+)", content[i:])
        if m:
            try:
                tokens.append(float(m.group(0)))
            except ValueError:
                pass
            i += len(m.group(0))
            continue

        i += 1

    return tokens


def tj_tokens_to_line(tokens: Sequence[Token], gap_threshold: float = 500.0) -> str:
    parts: List[str] = []
    prev_was_big_gap = False
    for token in tokens:
        if isinstance(token, str):
            if prev_was_big_gap and parts:
                parts.append(" ")
            parts.append(token)
            prev_was_big_gap = False
        else:
            prev_was_big_gap = abs(token) >= gap_threshold
    return "".join(parts).strip()


def extract_lines_from_content_stream(content_text: str) -> List[str]:
    lines: List[str] = []

    for arr in re.findall(r"\[(.*?)\]\s*TJ", content_text, flags=re.S):
        tokens = parse_tj_array(arr)
        line = tj_tokens_to_line(tokens)
        if line:
            lines.append(line)

    for lit in re.findall(r"\((.*?)(?<!\\)\)\s*Tj", content_text, flags=re.S):
        line = unescape_pdf_literal(lit).strip()
        if line:
            lines.append(line)

    return lines


def iter_decoded_streams(pdf_bytes: bytes) -> Iterator[str]:
    obj_re = re.compile(rb"(\d+)\s+(\d+)\s+obj\b(.*?)\bendobj\b", re.S)
    for m in obj_re.finditer(pdf_bytes):
        body = m.group(3)
        stream_match = re.search(rb"stream\r?\n", body)
        if not stream_match:
            continue
        stream_start = stream_match.end()
        stream_end = body.rfind(b"endstream")
        if stream_end <= stream_start:
            continue

        raw = body[stream_start:stream_end]
        if raw.endswith(b"\r\n"):
            raw = raw[:-2]
        elif raw.endswith(b"\n"):
            raw = raw[:-1]

        data = raw
        if b"/FlateDecode" in body:
            try:
                data = zlib.decompress(raw)
            except zlib.error:
                continue

        text = data.decode("latin-1", errors="ignore")
        if "BT" in text and ("TJ" in text or "Tj" in text):
            yield text


def find_molecule_lines(pdf_path: Path, molecule_number: int) -> List[str]:
    target = f"Molecule#{molecule_number:04d}"
    pdf_bytes = pdf_path.read_bytes()

    for stream_text in iter_decoded_streams(pdf_bytes):
        lines = extract_lines_from_content_stream(stream_text)
        for line in lines:
            if target in line.replace(" ", ""):
                return lines

    raise ValueError(f"Molecule#{molecule_number:04d} entry was not found in {pdf_path}")


def find_line(lines: Sequence[str], prefix: str) -> Optional[str]:
    for line in lines:
        if line.startswith(prefix):
            return line
    return None


def parse_value_from_line(line: str, key: str, unit: Optional[str] = None) -> float:
    pattern = rf"{re.escape(key)}\s*=\s*([+-]?\d+(?:\.\d+)?)"
    m = re.search(pattern, line)
    if not m:
        raise ValueError(f"Cannot parse numeric value for {key} from line: {line}")
    value = float(m.group(1))
    if unit and unit not in line:
        # Keep parsing strictness soft for forward compatibility.
        pass
    return value


def parse_atom_line(line: str) -> Tuple[str, float, float, float]:
    # Preferred: spaced columns generated by TJ gap parsing.
    m = re.match(
        r"^([A-Z][a-z]?)\s+([+-]?\d+\.\d+)\s+([+-]?\d+\.\d+)\s+([+-]?\d+\.\d+)$",
        line,
    )
    if m:
        return (m.group(1), float(m.group(2)), float(m.group(3)), float(m.group(4)))

    # Fallback for tightly concatenated text.
    m2 = re.match(
        r"^([A-Z][a-z]?)([+-]?\d+\.\d+)([+-]?\d+\.\d+)([+-]?\d+\.\d+)$",
        line,
    )
    if m2:
        return (m2.group(1), float(m2.group(2)), float(m2.group(3)), float(m2.group(4)))

    raise ValueError(f"Cannot parse atom line: {line}")


def symbol_to_atomic_number(symbol: str) -> int:
    atomic_number = SYMBOL_TO_ATOMIC_NUMBER.get(symbol)
    if atomic_number is None:
        raise ValueError(f"Unknown element symbol: {symbol}")
    return atomic_number


def sanitize_grid_label(label: str) -> str:
    clean = re.sub(r"[^A-Za-z0-9:_-]+", "_", label.strip("/"))
    return clean if clean else "grid"


def load_grid_entries(molecule_number: int, grid_dir: Path) -> Tuple[List[GridEntry], Path]:
    grid_path = grid_dir / f"{molecule_number:04d}.h5"
    if not grid_path.exists():
        raise ValueError(f"Grid file not found: {grid_path}")
    log_step(f"Loading grid source: {grid_path}")

    try:
        import h5py  # type: ignore
    except Exception as exc:
        raise RuntimeError(
            "h5py is required for --add-grid=1. "
            "Install it in the Python environment used to run this script."
        ) from exc

    entries: List[GridEntry] = []
    with h5py.File(grid_path, "r") as h5f:
        def visitor(name: str, obj: Any) -> None:
            if not isinstance(obj, h5py.Dataset):
                return
            shape = tuple(int(v) for v in obj.shape)
            base = sanitize_grid_label(name)
            log_step(f"Found grid dataset: {name} shape={shape}")
            if len(shape) == 3:
                entries.append(GridEntry(label=base, values=obj[...]))
                log_step(f"Prepared DATAGRID entry: {base} shape={shape}")
            elif len(shape) == 4:
                for idx in range(shape[0]):
                    label = f"{base}:ch_{idx}"
                    entries.append(
                        GridEntry(
                            label=label,
                            values=obj[idx, ...],
                        )
                    )
                    log_step(f"Prepared DATAGRID entry: {label} shape={shape[1:]}")

        h5f.visititems(visitor)

    if not entries:
        raise ValueError(
            f"No 3D/4D datasets found in {grid_path} "
            "(expected shapes: (nx,ny,nz) or (c,nx,ny,nz))."
        )
    log_step(f"Loaded {len(entries)} DATAGRID entries from {grid_path.name}")
    return entries, grid_path


def compute_bounding_box(mol: MoleculeData) -> Tuple[float, float, float, float, float, float]:
    xs = [x for _, x, _, _ in mol.atoms]
    ys = [y for _, _, y, _ in mol.atoms]
    zs = [z for _, _, _, z in mol.atoms]
    min_x, max_x = min(xs), max(xs)
    min_y, max_y = min(ys), max(ys)
    min_z, max_z = min(zs), max(zs)
    lx = (max_x - min_x) + 10.0
    ly = (max_y - min_y) + 10.0
    lz = (max_z - min_z) + 10.0
    return min_x, min_y, min_z, lx, ly, lz


def append_datagrid_block(
    lines: List[str],
    grid_entries: Sequence[GridEntry],
    grid_source: Optional[Path],
    origin: Tuple[float, float, float],
    vectors: Tuple[Tuple[float, float, float], Tuple[float, float, float], Tuple[float, float, float]],
) -> None:
    if not grid_entries:
        return

    log_step(
        f"Writing DATAGRID_3D block ({len(grid_entries)} entries, "
        f"source={grid_source.name if grid_source else 'unknown'})"
    )
    lines.append("BEGIN_BLOCK_DATAGRID_3D")
    if grid_source is not None:
        lines.append(f" DATA_from:{grid_source.name}")
    else:
        lines.append(" DATA_from:unknown")
    for idx, entry in enumerate(grid_entries, start=1):
        arr = entry.values
        if getattr(arr, "ndim", None) != 3:
            log_step(f"Skipping non-3D grid entry: {entry.label}")
            continue
        nx, ny, nz = int(arr.shape[0]), int(arr.shape[1]), int(arr.shape[2])
        log_step(
            f"Writing grid {idx}/{len(grid_entries)}: {entry.label} "
            f"shape=({nx}, {ny}, {nz}) values={nx * ny * nz}"
        )
        lines.append(f" BEGIN_DATAGRID_3D_{entry.label}")
        lines.append(f"   {nx}   {ny}   {nz}")
        lines.append(f"  {origin[0]:.7E}  {origin[1]:.7E}  {origin[2]:.7E}")
        lines.append(f"  {vectors[0][0]:.7E}  {vectors[0][1]:.7E}  {vectors[0][2]:.7E}")
        lines.append(f"  {vectors[1][0]:.7E}  {vectors[1][1]:.7E}  {vectors[1][2]:.7E}")
        lines.append(f"  {vectors[2][0]:.7E}  {vectors[2][1]:.7E}  {vectors[2][2]:.7E}")

        flat_values = arr.ravel(order="F")
        for i in range(0, len(flat_values), 6):
            chunk = flat_values[i : i + 6]
            lines.append("  " + "  ".join(f"{float(v):.5E}" for v in chunk))
        lines.append(" END_DATAGRID_3D")

    lines.append("END_BLOCK_DATAGRID_3D")
    log_step("Finished DATAGRID_3D block")


def write_xsf_datagrid_style(
    mol: MoleculeData,
    path: Path,
    xsf_format: str,
    grid_entries: Sequence[GridEntry],
    grid_source: Optional[Path] = None,
) -> None:
    # Match Si8_RHO.XSF style for DATAGRID files:
    # - ATOMS section first
    # - no CRYSTAL/PRIMVEC/PRIMCOORD headers
    # - BEGIN_BLOCK_DATAGRID_3D follows directly
    min_x, min_y, min_z, lx, ly, lz = compute_bounding_box(mol)
    if xsf_format == "crystal":
        shift_x = 5.0 - min_x
        shift_y = 5.0 - min_y
        shift_z = 5.0 - min_z
        atom_coords = [(sym, x + shift_x, y + shift_y, z + shift_z) for sym, x, y, z in mol.atoms]
        origin = (0.0, 0.0, 0.0)
        vectors = ((lx, 0.0, 0.0), (0.0, ly, 0.0), (0.0, 0.0, lz))
    else:
        atom_coords = list(mol.atoms)
        origin = (min_x - 5.0, min_y - 5.0, min_z - 5.0)
        vectors = ((lx, 0.0, 0.0), (0.0, ly, 0.0), (0.0, 0.0, lz))

    log_step(
        f"Writing XSF (datagrid-style) to {path} "
        f"format={xsf_format} atoms={len(atom_coords)} grids={len(grid_entries)}"
    )

    lines = ["ATOMS"]
    for idx, (sym, x, y, z) in enumerate(atom_coords, start=1):
        atomic_number = symbol_to_atomic_number(sym)
        lines.append(f"  {atomic_number:>2d} {x:20.8f} {y:20.8f} {z:20.8f}")
        log_step(
            f"  atom[{idx:03d}] {sym}(Z={atomic_number}) "
            f"x={x:.8f} y={y:.8f} z={z:.8f}"
        )

    append_datagrid_block(lines, grid_entries, grid_source, origin, vectors)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def parse_molecule_data(molecule_number: int, lines: Sequence[str]) -> MoleculeData:
    expected_molecule = f"Molecule#{molecule_number:04d}"
    mol_line_idx = None
    for i, line in enumerate(lines):
        if expected_molecule in line.replace(" ", ""):
            mol_line_idx = i
            break
    if mol_line_idx is None:
        raise ValueError(f"{expected_molecule} line not found")

    # Molecule title is typically the next line.
    if mol_line_idx + 1 >= len(lines):
        raise ValueError("Title line is missing")
    title = lines[mol_line_idx + 1].strip()

    point_group_line = find_line(lines, "PointGroup:")
    if not point_group_line:
        raise ValueError("PointGroup line not found")
    point_group = point_group_line.split(":", 1)[1].strip()

    e_line = find_line(lines, "E=")
    zpe_line = find_line(lines, "ZPE=")
    lumo_line = find_line(lines, "LUMO_eps=")
    homo_line = find_line(lines, "HOMO_eps=")
    dipole_line = find_line(lines, "Dipole=")
    if not all([e_line, zpe_line, lumo_line, homo_line, dipole_line]):
        raise ValueError("One or more metadata lines are missing")

    energy = parse_value_from_line(e_line, "E", unit="au")
    zpe = parse_value_from_line(zpe_line, "ZPE", unit="au")
    lumo_eps = parse_value_from_line(lumo_line, "LUMO_eps", unit="au")
    homo_eps = parse_value_from_line(homo_line, "HOMO_eps", unit="au")
    dipole = parse_value_from_line(dipole_line, "Dipole", unit="debye")

    natoms = None
    natoms_idx = None
    for i, line in enumerate(lines):
        if re.fullmatch(r"\d+", line.strip()):
            value = int(line.strip())
            if 1 <= value <= 1000:
                natoms = value
                natoms_idx = i
                break
    if natoms is None or natoms_idx is None:
        raise ValueError("Number of atoms line was not found")

    atoms: List[Tuple[str, float, float, float]] = []
    for line in lines[natoms_idx + 1 :]:
        try:
            atom = parse_atom_line(line)
            atoms.append(atom)
            if len(atoms) == natoms:
                break
        except ValueError:
            continue

    if len(atoms) != natoms:
        raise ValueError(
            f"Expected {natoms} atoms, but parsed {len(atoms)}. "
            "PDF text extraction pattern may have changed."
        )

    return MoleculeData(
        molecule_number=molecule_number,
        title=title,
        point_group=point_group,
        energy=energy,
        zpe=zpe,
        lumo_eps=lumo_eps,
        homo_eps=homo_eps,
        dipole=dipole,
        atoms=atoms,
    )


def write_json(mol: MoleculeData, path: Path) -> None:
    payload = {
        "title": mol.title,
        "PointGroup": mol.point_group,
        "E": mol.energy,
        "ZPE": mol.zpe,
        "LUMO_eps": mol.lumo_eps,
        "HOMO_eps": mol.homo_eps,
        "dipole": mol.dipole,
    }
    log_step(f"Writing JSON metadata to {path}")
    for key in ("title", "PointGroup", "E", "ZPE", "LUMO_eps", "HOMO_eps", "dipole"):
        log_step(f"  {key} = {payload[key]}")
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def write_xsf_molecule(
    mol: MoleculeData,
    path: Path,
    grid_entries: Optional[Sequence[GridEntry]] = None,
    grid_source: Optional[Path] = None,
) -> None:
    min_x, min_y, min_z, lx, ly, lz = compute_bounding_box(mol)
    log_step(f"Writing XSF (molecule) to {path}")
    log_step(
        f"Bounding box for molecule format: origin=({min_x - 5.0:.6f}, "
        f"{min_y - 5.0:.6f}, {min_z - 5.0:.6f}), size=({lx:.6f}, {ly:.6f}, {lz:.6f})"
    )
    lines = [
        "MOLECULE",
        f"# Molecule#{mol.molecule_number:04d} {mol.title}",
        "ATOMS",
    ]
    for idx, (sym, x, y, z) in enumerate(mol.atoms, start=1):
        atomic_number = symbol_to_atomic_number(sym)
        lines.append(f"{atomic_number} {x:.6f} {y:.6f} {z:.6f}")
        log_step(
            f"  atom[{idx:03d}] {sym}(Z={atomic_number}) "
            f"x={x:.6f} y={y:.6f} z={z:.6f}"
        )

    if grid_entries:
        origin = (min_x - 5.0, min_y - 5.0, min_z - 5.0)
        vectors = ((lx, 0.0, 0.0), (0.0, ly, 0.0), (0.0, 0.0, lz))
        append_datagrid_block(lines, grid_entries, grid_source, origin, vectors)

    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_xsf_crystal(
    mol: MoleculeData,
    path: Path,
    grid_entries: Optional[Sequence[GridEntry]] = None,
    grid_source: Optional[Path] = None,
) -> None:
    min_x, min_y, min_z, lx, ly, lz = compute_bounding_box(mol)
    log_step(f"Writing XSF (crystal) to {path}")
    log_step(
        f"Crystal PRIMVEC lengths: ({lx:.6f}, {ly:.6f}, {lz:.6f}); "
        f"shift=({5.0 - min_x:.6f}, {5.0 - min_y:.6f}, {5.0 - min_z:.6f})"
    )

    # Shift atoms so they have 5A margin from the lower face.
    shift_x = 5.0 - min_x
    shift_y = 5.0 - min_y
    shift_z = 5.0 - min_z

    lines = [
        "CRYSTAL",
        f"# Molecule#{mol.molecule_number:04d} {mol.title}",
        "PRIMVEC",
        f"{lx:.6f} 0.000000 0.000000",
        f"0.000000 {ly:.6f} 0.000000",
        f"0.000000 0.000000 {lz:.6f}",
        "PRIMCOORD",
        f"{len(mol.atoms)} 1",
    ]
    for idx, (sym, x, y, z) in enumerate(mol.atoms, start=1):
        atomic_number = symbol_to_atomic_number(sym)
        fx, fy, fz = x + shift_x, y + shift_y, z + shift_z
        lines.append(
            f"{atomic_number} {fx:.6f} {fy:.6f} {fz:.6f}"
        )
        log_step(
            f"  atom[{idx:03d}] {sym}(Z={atomic_number}) "
            f"x={fx:.6f} y={fy:.6f} z={fz:.6f}"
        )

    if grid_entries:
        origin = (0.0, 0.0, 0.0)
        vectors = ((lx, 0.0, 0.0), (0.0, ly, 0.0), (0.0, 0.0, lz))
        append_datagrid_block(lines, grid_entries, grid_source, origin, vectors)

    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_xsf(
    mol: MoleculeData,
    path: Path,
    xsf_format: str,
    grid_entries: Optional[Sequence[GridEntry]] = None,
    grid_source: Optional[Path] = None,
) -> None:
    if grid_entries:
        write_xsf_datagrid_style(
            mol,
            path,
            xsf_format,
            grid_entries=grid_entries,
            grid_source=grid_source,
        )
        return
    if xsf_format == "molecule":
        write_xsf_molecule(mol, path, grid_entries=grid_entries, grid_source=grid_source)
        return
    write_xsf_crystal(mol, path, grid_entries=grid_entries, grid_source=grid_source)


def main() -> int:
    args = parse_args()
    molecule_number = args.molecule_number
    pdf_path: Path = args.pdf_path
    output_dir: Path = args.output_dir
    xsf_format: str = args.format
    add_grid: bool = args.add_grid == 1

    if not (MIN_MOLECULE <= molecule_number <= MAX_MOLECULE):
        print(
            f"Error: --molecule-number must be in [{MIN_MOLECULE}, {MAX_MOLECULE}]",
            file=sys.stderr,
        )
        return 2
    if not pdf_path.exists():
        print(f"Error: PDF file not found: {pdf_path}", file=sys.stderr)
        return 2

    output_dir.mkdir(parents=True, exist_ok=True)
    stem = f"{molecule_number:04d}"
    json_path = output_dir / f"{stem}.json"
    xsf_path = output_dir / f"{stem}.XSF"
    log_step(
        f"Start extraction: molecule={stem}, format={xsf_format}, add_grid={int(add_grid)}"
    )
    log_step(f"Input PDF: {pdf_path}")
    log_step(f"Output files: {json_path.name}, {xsf_path.name} in {output_dir}")

    try:
        log_step("Scanning PDF content streams and locating requested molecule page block")
        lines = find_molecule_lines(pdf_path, molecule_number)
        log_step(f"Collected {len(lines)} text lines for Molecule#{stem}")
        log_step("Parsing molecule metadata and atomic coordinates")
        data = parse_molecule_data(molecule_number, lines)
        log_step(f"Parsed molecule title: {data.title}")
        log_step(f"Parsed atom count: {len(data.atoms)}")
    except Exception as exc:
        print(f"Error: failed to extract molecule {stem}: {exc}", file=sys.stderr)
        return 1

    grid_entries: Optional[List[GridEntry]] = None
    grid_source: Optional[Path] = None
    if add_grid:
        try:
            grid_entries, grid_source = load_grid_entries(molecule_number, DEFAULT_GRID_DIR)
        except Exception as exc:
            print(f"Error: failed to load grid data for {stem}: {exc}", file=sys.stderr)
            return 1
    else:
        log_step("Skipping grid load (--add-grid=0)")

    write_json(data, json_path)
    write_xsf(
        data,
        xsf_path,
        xsf_format,
        grid_entries=grid_entries,
        grid_source=grid_source,
    )

    log_step("Finished writing JSON and XSF outputs")
    print(f"Wrote {json_path}")
    print(f"Wrote {xsf_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
