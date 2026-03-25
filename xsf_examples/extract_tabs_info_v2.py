#!/usr/bin/env python3
"""Find the best-matching molecule/axis for an H5 grid and export JSON + XSF.

Usage:
    python extract_tabs_info_v2.py --grid-number 758
"""

from __future__ import annotations

import argparse
import itertools
import json
import math
import re
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Tuple

import numpy as np

from extract_tabs_info import (
    DEFAULT_GRID_DIR,
    DEFAULT_PDF,
    MAX_MOLECULE,
    MIN_MOLECULE,
    GridEntry,
    MoleculeData,
    extract_lines_from_content_stream,
    iter_decoded_streams,
    load_grid_entries,
    parse_molecule_data,
    write_json,
    write_xsf,
)


AXIS_PERMUTATIONS: Tuple[str, ...] = tuple("".join(p) for p in itertools.permutations("xyz"))
MOLECULE_RE = re.compile(r"Molecule#(\d{1,4})")
EPS = 1.0e-12


@dataclass
class Candidate:
    molecule_number: int
    title: str
    axis: str
    natoms: int
    atom_idx_coords: np.ndarray  # shape (N, 3), floating index-space coordinates
    dist_hist: np.ndarray
    inertia: np.ndarray


@dataclass
class RankedResult:
    molecule_number: int
    title: str
    axis: str
    natoms: int
    coarse_score: float
    final_score: float
    atom_score: float
    ncc_score: float
    sign: int
    best_shift: Tuple[int, int, int]


def log_step(message: str) -> None:
    print(f"[extract_tabs_v2] {message}", file=sys.stderr, flush=True)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Find the molecule/axis in TABS.pdf that best matches feature:ch_0 "
            "of xsf_examples/TABS_data/NNNN.h5, then write JSON + XSF."
        )
    )
    parser.add_argument(
        "--grid-number",
        type=int,
        required=True,
        help=(
            f"H5 file number in [{MIN_MOLECULE}, {MAX_MOLECULE}] "
            "(uses xsf_examples/TABS_data/NNNN.h5 as input query)"
        ),
    )
    parser.add_argument(
        "--pdf-path",
        type=Path,
        default=DEFAULT_PDF,
        help=f"Path to TABS.pdf (default: {DEFAULT_PDF})",
    )
    parser.add_argument(
        "--pdf-cache-path",
        type=Path,
        default=None,
        help=(
            "Path to cached parsed molecules JSON. "
            "Default: <pdf-path>.parsed.json. "
            "If this file exists, PDF parsing is skipped."
        ),
    )
    parser.add_argument(
        "--grid-dir",
        type=Path,
        default=DEFAULT_GRID_DIR,
        help=f"Directory containing NNNN.h5 files (default: {DEFAULT_GRID_DIR})",
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=Path("."),
        help="Output directory (default: current directory)",
    )
    parser.add_argument(
        "--format",
        choices=("crystal", "molecule"),
        default="crystal",
        help="XSF output format for the best match (default: crystal)",
    )
    parser.add_argument(
        "--coarse-top",
        type=int,
        default=80,
        help="Number of candidates kept after coarse stage (default: 80)",
    )
    parser.add_argument(
        "--top-k",
        type=int,
        default=10,
        help="Number of ranked results saved to match report (default: 10)",
    )
    parser.add_argument(
        "--shift-range",
        type=int,
        default=2,
        help="Shift search range in voxel units for fine stage (default: 2 => [-2,2])",
    )
    parser.add_argument(
        "--sigma",
        type=float,
        default=1.4,
        help="Gaussian sigma for fine-stage template NCC (default: 1.4)",
    )
    return parser.parse_args()


def default_cache_path(pdf_path: Path) -> Path:
    return pdf_path.with_suffix(".parsed.json")


def molecule_to_payload(mol: MoleculeData) -> Dict[str, object]:
    return {
        "molecule_number": mol.molecule_number,
        "title": mol.title,
        "point_group": mol.point_group,
        "energy": mol.energy,
        "zpe": mol.zpe,
        "lumo_eps": mol.lumo_eps,
        "homo_eps": mol.homo_eps,
        "dipole": mol.dipole,
        "atoms": [
            {"symbol": sym, "x": x, "y": y, "z": z}
            for sym, x, y, z in mol.atoms
        ],
    }


def payload_to_molecule(item: Dict[str, object]) -> MoleculeData:
    atoms_payload = item.get("atoms")
    if not isinstance(atoms_payload, list):
        raise ValueError("Invalid cache format: atoms must be a list")

    atoms: List[Tuple[str, float, float, float]] = []
    for atom in atoms_payload:
        if not isinstance(atom, dict):
            raise ValueError("Invalid cache format: atom must be an object")
        sym = str(atom["symbol"])
        x = float(atom["x"])
        y = float(atom["y"])
        z = float(atom["z"])
        atoms.append((sym, x, y, z))

    return MoleculeData(
        molecule_number=int(item["molecule_number"]),
        title=str(item["title"]),
        point_group=str(item["point_group"]),
        energy=float(item["energy"]),
        zpe=float(item["zpe"]),
        lumo_eps=float(item["lumo_eps"]),
        homo_eps=float(item["homo_eps"]),
        dipole=float(item["dipole"]),
        atoms=atoms,
    )


def save_molecule_cache(cache_path: Path, molecules: Dict[int, MoleculeData], pdf_path: Path) -> None:
    cache_path.parent.mkdir(parents=True, exist_ok=True)
    payload = {
        "meta": {
            "created_unix": int(time.time()),
            "pdf_path": str(pdf_path),
            "count": len(molecules),
            "range": [MIN_MOLECULE, MAX_MOLECULE],
        },
        "molecules": [molecule_to_payload(molecules[k]) for k in sorted(molecules.keys())],
    }
    cache_path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def load_molecule_cache(cache_path: Path) -> Dict[int, MoleculeData]:
    raw = json.loads(cache_path.read_text(encoding="utf-8"))
    items = raw.get("molecules")
    if not isinstance(items, list):
        raise ValueError("Invalid cache format: 'molecules' list is missing")

    result: Dict[int, MoleculeData] = {}
    for item in items:
        if not isinstance(item, dict):
            raise ValueError("Invalid cache format: molecule item must be an object")
        mol = payload_to_molecule(item)
        if MIN_MOLECULE <= mol.molecule_number <= MAX_MOLECULE:
            result[mol.molecule_number] = mol
    if not result:
        raise ValueError("Cache has no valid molecules")
    return result


def apply_axis(coords: np.ndarray, axis: str) -> np.ndarray:
    """axis='yzx' means new x<-old y, new y<-old z, new z<-old x."""
    mapping = {"x": 0, "y": 1, "z": 2}
    return coords[:, [mapping[c] for c in axis]]


def to_box_index_coords(coords: np.ndarray, grid_dims: Tuple[int, int, int]) -> np.ndarray:
    mins = np.min(coords, axis=0)
    maxs = np.max(coords, axis=0)
    lengths = (maxs - mins) + 10.0
    lengths = np.where(lengths < 1.0e-9, 1.0e-9, lengths)
    scale = np.array([grid_dims[0] - 1, grid_dims[1] - 1, grid_dims[2] - 1], dtype=float)
    return (coords - mins + 5.0) / lengths * scale.reshape(1, 3)


def cosine_similarity(a: np.ndarray, b: np.ndarray) -> float:
    denom = float(np.linalg.norm(a) * np.linalg.norm(b)) + EPS
    return float(np.dot(a, b) / denom)


def pairwise_distance_histogram(points: np.ndarray, bins: int, max_dist: float) -> np.ndarray:
    if points.shape[0] < 2:
        return np.zeros((bins,), dtype=float)
    diff = points[:, None, :] - points[None, :, :]
    dmat = np.sqrt(np.sum(diff * diff, axis=2))
    iu = np.triu_indices(points.shape[0], k=1)
    dists = dmat[iu]
    hist, _ = np.histogram(dists, bins=bins, range=(0.0, max_dist))
    hist = hist.astype(float)
    if hist.sum() > 0:
        hist /= hist.sum()
    return hist


def normalized_inertia(points: np.ndarray) -> np.ndarray:
    if points.shape[0] < 2:
        return np.zeros((3,), dtype=float)
    centered = points - np.mean(points, axis=0, keepdims=True)
    cov = centered.T @ centered / float(points.shape[0])
    eigvals = np.sort(np.linalg.eigvalsh(cov))
    total = float(np.sum(eigvals))
    if total > 0:
        eigvals = eigvals / total
    return eigvals.astype(float)


def robust_normalize(arr: np.ndarray) -> np.ndarray:
    median = float(np.median(arr))
    q1 = float(np.percentile(arr, 25.0))
    q3 = float(np.percentile(arr, 75.0))
    iqr = q3 - q1
    std = float(np.std(arr))
    mad = float(np.median(np.abs(arr - median)))
    p5 = float(np.percentile(arr, 5.0))
    p95 = float(np.percentile(arr, 95.0))

    # Many grids contain broad plateaus, so IQR may be near-zero.
    # Use the largest robust spread estimate to avoid exploding scores.
    scale = max(iqr, std, 1.4826 * mad, 0.5 * (p95 - p5), 1.0e-6)
    return (arr - median) / scale


def extract_local_peaks(arr_abs: np.ndarray, percentile: float = 99.7, max_peaks: int = 128) -> np.ndarray:
    threshold = float(np.percentile(arr_abs, percentile))
    mask = arr_abs >= threshold
    if not np.any(mask):
        flat_idx = np.argpartition(arr_abs.ravel(), -max_peaks)[-max_peaks:]
        return np.array(np.unravel_index(flat_idx, arr_abs.shape)).T.astype(float)

    nx, ny, nz = arr_abs.shape
    padded = np.pad(arr_abs, 1, mode="constant", constant_values=-np.inf)
    is_local = np.ones_like(mask, dtype=bool)
    for sx in (-1, 0, 1):
        for sy in (-1, 0, 1):
            for sz in (-1, 0, 1):
                if sx == 0 and sy == 0 and sz == 0:
                    continue
                neighbor = padded[
                    1 + sx : 1 + sx + nx,
                    1 + sy : 1 + sy + ny,
                    1 + sz : 1 + sz + nz,
                ]
                is_local &= arr_abs >= neighbor

    peaks_mask = mask & is_local
    peaks = np.argwhere(peaks_mask)
    if peaks.shape[0] == 0:
        peaks = np.argwhere(mask)

    if peaks.shape[0] > max_peaks:
        vals = arr_abs[peaks[:, 0], peaks[:, 1], peaks[:, 2]]
        order = np.argsort(vals)[::-1][:max_peaks]
        peaks = peaks[order]

    return peaks.astype(float)


def trilinear_sample(arr: np.ndarray, points: np.ndarray) -> np.ndarray:
    nx, ny, nz = arr.shape
    out = np.full((points.shape[0],), np.nan, dtype=float)
    for idx, (x, y, z) in enumerate(points):
        if not (0.0 <= x <= nx - 1 and 0.0 <= y <= ny - 1 and 0.0 <= z <= nz - 1):
            continue
        x0 = int(math.floor(x))
        y0 = int(math.floor(y))
        z0 = int(math.floor(z))
        x1 = min(x0 + 1, nx - 1)
        y1 = min(y0 + 1, ny - 1)
        z1 = min(z0 + 1, nz - 1)
        dx = x - x0
        dy = y - y0
        dz = z - z0

        c000 = arr[x0, y0, z0]
        c100 = arr[x1, y0, z0]
        c010 = arr[x0, y1, z0]
        c110 = arr[x1, y1, z0]
        c001 = arr[x0, y0, z1]
        c101 = arr[x1, y0, z1]
        c011 = arr[x0, y1, z1]
        c111 = arr[x1, y1, z1]

        c00 = c000 * (1.0 - dx) + c100 * dx
        c10 = c010 * (1.0 - dx) + c110 * dx
        c01 = c001 * (1.0 - dx) + c101 * dx
        c11 = c011 * (1.0 - dx) + c111 * dx
        c0 = c00 * (1.0 - dy) + c10 * dy
        c1 = c01 * (1.0 - dy) + c11 * dy
        out[idx] = c0 * (1.0 - dz) + c1 * dz
    return out


def build_gaussian_template(
    dims: Tuple[int, int, int], atom_idx_coords: np.ndarray, sigma: float
) -> np.ndarray:
    nx, ny, nz = dims
    out = np.zeros((nx, ny, nz), dtype=float)
    radius = max(1, int(math.ceil(3.0 * sigma)))
    denom = 2.0 * sigma * sigma
    for p in atom_idx_coords:
        cx, cy, cz = float(p[0]), float(p[1]), float(p[2])
        ix0 = max(0, int(math.floor(cx)) - radius)
        iy0 = max(0, int(math.floor(cy)) - radius)
        iz0 = max(0, int(math.floor(cz)) - radius)
        ix1 = min(nx - 1, int(math.floor(cx)) + radius)
        iy1 = min(ny - 1, int(math.floor(cy)) + radius)
        iz1 = min(nz - 1, int(math.floor(cz)) + radius)

        xs = np.arange(ix0, ix1 + 1, dtype=float) - cx
        ys = np.arange(iy0, iy1 + 1, dtype=float) - cy
        zs = np.arange(iz0, iz1 + 1, dtype=float) - cz
        gx, gy, gz = np.meshgrid(xs, ys, zs, indexing="ij")
        patch = np.exp(-(gx * gx + gy * gy + gz * gz) / denom)
        out[ix0 : ix1 + 1, iy0 : iy1 + 1, iz0 : iz1 + 1] += patch
    return out


def normalized_correlation(a: np.ndarray, b: np.ndarray) -> float:
    av = a.ravel()
    bv = b.ravel()
    ac = av - np.mean(av)
    bc = bv - np.mean(bv)
    denom = float(np.linalg.norm(ac) * np.linalg.norm(bc)) + EPS
    return float(np.dot(ac, bc) / denom)


def generate_shift_vectors(shift_range: int) -> List[Tuple[int, int, int]]:
    values = list(range(-shift_range, shift_range + 1))
    return [(sx, sy, sz) for sx in values for sy in values for sz in values]


def best_shifted_atom_score(
    arr: np.ndarray,
    atom_idx_coords: np.ndarray,
    shifts: Sequence[Tuple[int, int, int]],
) -> Tuple[float, Tuple[int, int, int]]:
    best_score = -float("inf")
    best_shift = (0, 0, 0)
    for sx, sy, sz in shifts:
        shifted = atom_idx_coords + np.array([sx, sy, sz], dtype=float).reshape(1, 3)
        samples = trilinear_sample(arr, shifted)
        valid = samples[np.isfinite(samples)]
        if valid.size == 0:
            continue
        score = float(np.mean(valid))
        if score > best_score:
            best_score = score
            best_shift = (sx, sy, sz)
    return best_score, best_shift


def load_feature_channel0(grid_path: Path) -> np.ndarray:
    try:
        import h5py  # type: ignore
    except Exception as exc:
        raise RuntimeError("h5py is required. Install it in the Python environment.") from exc

    with h5py.File(grid_path, "r") as f:
        if "feature" not in f:
            raise ValueError(f"'feature' dataset not found in {grid_path}")
        feature = f["feature"]
        if feature.ndim == 4:
            return np.asarray(feature[0, ...], dtype=float)
        if feature.ndim == 3:
            return np.asarray(feature[...], dtype=float)
        raise ValueError(f"Unsupported feature shape {feature.shape} in {grid_path}")


def parse_all_molecules_from_pdf(pdf_path: Path) -> Dict[int, MoleculeData]:
    pdf_bytes = pdf_path.read_bytes()
    result: Dict[int, MoleculeData] = {}

    for stream in iter_decoded_streams(pdf_bytes):
        lines = extract_lines_from_content_stream(stream)
        ids: List[int] = []
        for line in lines:
            compact = line.replace(" ", "")
            for m in MOLECULE_RE.finditer(compact):
                ids.append(int(m.group(1)))
        if not ids:
            continue

        for mol_id in ids:
            if mol_id in result:
                continue
            if not (MIN_MOLECULE <= mol_id <= MAX_MOLECULE):
                continue
            try:
                parsed = parse_molecule_data(mol_id, lines)
                result[mol_id] = parsed
            except Exception:
                continue

        if len(result) >= MAX_MOLECULE:
            break

    return result


def build_candidates(
    molecules: Iterable[MoleculeData],
    grid_dims: Tuple[int, int, int],
    bins: int,
    max_dist: float,
) -> List[Candidate]:
    candidates: List[Candidate] = []
    for mol in molecules:
        coords = np.array([(x, y, z) for _, x, y, z in mol.atoms], dtype=float)
        for axis in AXIS_PERMUTATIONS:
            rotated = apply_axis(coords, axis)
            idx_coords = to_box_index_coords(rotated, grid_dims)
            dist_hist = pairwise_distance_histogram(idx_coords, bins=bins, max_dist=max_dist)
            inertia = normalized_inertia(idx_coords)
            candidates.append(
                Candidate(
                    molecule_number=mol.molecule_number,
                    title=mol.title,
                    axis=axis,
                    natoms=len(mol.atoms),
                    atom_idx_coords=idx_coords,
                    dist_hist=dist_hist,
                    inertia=inertia,
                )
            )
    return candidates


def apply_axis_to_molecule(mol: MoleculeData, axis: str) -> MoleculeData:
    mapping = {"x": 0, "y": 1, "z": 2}
    idx = [mapping[c] for c in axis]
    new_atoms: List[Tuple[str, float, float, float]] = []
    for sym, x, y, z in mol.atoms:
        arr = np.array([x, y, z], dtype=float)
        nx, ny, nz = arr[idx[0]], arr[idx[1]], arr[idx[2]]
        new_atoms.append((sym, float(nx), float(ny), float(nz)))

    return MoleculeData(
        molecule_number=mol.molecule_number,
        title=mol.title,
        point_group=mol.point_group,
        energy=mol.energy,
        zpe=mol.zpe,
        lumo_eps=mol.lumo_eps,
        homo_eps=mol.homo_eps,
        dipole=mol.dipole,
        atoms=new_atoms,
    )


def main() -> int:
    args = parse_args()
    grid_number = args.grid_number
    pdf_path: Path = args.pdf_path
    pdf_cache_path: Path = args.pdf_cache_path if args.pdf_cache_path is not None else default_cache_path(pdf_path)
    grid_dir: Path = args.grid_dir
    output_dir: Path = args.output_dir

    if not (MIN_MOLECULE <= grid_number <= MAX_MOLECULE):
        print(
            f"Error: --grid-number must be in [{MIN_MOLECULE}, {MAX_MOLECULE}]",
            file=sys.stderr,
        )
        return 2
    if not pdf_path.exists():
        print(f"Error: PDF file not found: {pdf_path}", file=sys.stderr)
        return 2

    grid_path = grid_dir / f"{grid_number:04d}.h5"
    if not grid_path.exists():
        print(f"Error: H5 file not found: {grid_path}", file=sys.stderr)
        return 2

    output_dir.mkdir(parents=True, exist_ok=True)
    stem = f"{grid_number:04d}"
    report_path = output_dir / f"{stem}.match.json"

    log_step(f"Input H5: {grid_path}")
    log_step("Loading feature:ch_0 grid")
    feature_ch0 = load_feature_channel0(grid_path)
    grid_dims = feature_ch0.shape
    if len(grid_dims) != 3:
        print(f"Error: feature:ch_0 must be 3D, got {grid_dims}", file=sys.stderr)
        return 1
    log_step(f"Grid dimensions: {grid_dims}")

    grid_norm = robust_normalize(feature_ch0)
    grid_signs = [(+1, grid_norm), (-1, -grid_norm)]

    log_step("Extracting query peaks and descriptors")
    query_peaks = extract_local_peaks(np.abs(grid_norm), percentile=99.7, max_peaks=128)
    max_dist = float(np.linalg.norm(np.array([grid_dims[0] - 1, grid_dims[1] - 1, grid_dims[2] - 1])))
    query_hist = pairwise_distance_histogram(query_peaks, bins=24, max_dist=max_dist)
    query_inertia = normalized_inertia(query_peaks)
    query_peak_count = int(query_peaks.shape[0])
    log_step(f"Query peaks: {query_peak_count}")

    if pdf_cache_path.exists():
        try:
            log_step(f"Loading parsed molecule cache: {pdf_cache_path}")
            molecules_by_id = load_molecule_cache(pdf_cache_path)
            log_step(f"Loaded molecules from cache: {len(molecules_by_id)}")
        except Exception as exc:
            log_step(f"Cache load failed ({exc}); reparsing PDF")
            molecules_by_id = parse_all_molecules_from_pdf(pdf_path)
            if not molecules_by_id:
                print("Error: failed to parse any molecule from PDF", file=sys.stderr)
                return 1
            save_molecule_cache(pdf_cache_path, molecules_by_id, pdf_path)
            log_step(f"Saved parsed cache: {pdf_cache_path}")
    else:
        log_step("Parsing all molecules from TABS.pdf (single pass)")
        molecules_by_id = parse_all_molecules_from_pdf(pdf_path)
        if not molecules_by_id:
            print("Error: failed to parse any molecule from PDF", file=sys.stderr)
            return 1
        save_molecule_cache(pdf_cache_path, molecules_by_id, pdf_path)
        log_step(f"Saved parsed cache: {pdf_cache_path}")
    log_step(f"Available molecules: {len(molecules_by_id)}")

    log_step("Building candidate descriptors for 6 axis permutations")
    molecules = [molecules_by_id[k] for k in sorted(molecules_by_id.keys())]
    candidates = build_candidates(
        molecules=molecules,
        grid_dims=grid_dims,
        bins=24,
        max_dist=max_dist,
    )
    log_step(f"Total candidates: {len(candidates)}")

    # Stage 1: coarse ranking
    log_step("Stage 1/2: coarse ranking")
    coarse_scored: List[Tuple[float, Candidate]] = []
    for cand in candidates:
        sim_hist = cosine_similarity(query_hist, cand.dist_hist)
        sim_inertia = math.exp(-float(np.linalg.norm(query_inertia - cand.inertia)))
        sim_count = math.exp(-abs(query_peak_count - cand.natoms) / 18.0)
        coarse = 0.50 * sim_hist + 0.25 * sim_inertia + 0.25 * sim_count
        coarse_scored.append((coarse, cand))

    coarse_scored.sort(key=lambda x: x[0], reverse=True)
    coarse_top_n = max(1, min(int(args.coarse_top), len(coarse_scored)))
    coarse_top = coarse_scored[:coarse_top_n]
    log_step(f"Coarse top candidates: {coarse_top_n}")

    # Stage 2: fine ranking
    log_step("Stage 2/2: fine ranking")
    shifts = generate_shift_vectors(max(0, int(args.shift_range)))
    ranked: List[RankedResult] = []
    sigma = float(max(args.sigma, 0.2))

    for coarse_score, cand in coarse_top:
        template = build_gaussian_template(grid_dims, cand.atom_idx_coords, sigma=sigma)

        best_final = -float("inf")
        best_atom = -float("inf")
        best_ncc = -float("inf")
        best_sign = +1
        best_shift = (0, 0, 0)

        for sign, arr in grid_signs:
            atom_score, shift = best_shifted_atom_score(arr, cand.atom_idx_coords, shifts)
            ncc_score = normalized_correlation(arr, template)
            final_score = 0.80 * atom_score + 0.20 * ncc_score
            if final_score > best_final:
                best_final = final_score
                best_atom = atom_score
                best_ncc = ncc_score
                best_sign = sign
                best_shift = shift

        ranked.append(
            RankedResult(
                molecule_number=cand.molecule_number,
                title=cand.title,
                axis=cand.axis,
                natoms=cand.natoms,
                coarse_score=coarse_score,
                final_score=best_final,
                atom_score=best_atom,
                ncc_score=best_ncc,
                sign=best_sign,
                best_shift=best_shift,
            )
        )

    ranked.sort(key=lambda x: x.final_score, reverse=True)
    best = ranked[0]
    second = ranked[1] if len(ranked) > 1 else None
    gap = float(best.final_score - second.final_score) if second is not None else float("inf")

    log_step(
        f"Best match: Molecule#{best.molecule_number:04d}, axis={best.axis}, "
        f"score={best.final_score:.6f}, gap={gap:.6f}"
    )

    top_k = max(1, min(int(args.top_k), len(ranked)))
    report_payload = {
        "input_h5": str(grid_path),
        "grid_shape": list(grid_dims),
        "best": {
            "molecule_number": best.molecule_number,
            "title": best.title,
            "axis": best.axis,
            "sign": best.sign,
            "final_score": best.final_score,
            "coarse_score": best.coarse_score,
            "atom_score": best.atom_score,
            "ncc_score": best.ncc_score,
            "best_shift": list(best.best_shift),
            "score_gap_to_second": gap,
        },
        "top_k": [
            {
                "rank": i + 1,
                "molecule_number": row.molecule_number,
                "title": row.title,
                "axis": row.axis,
                "sign": row.sign,
                "final_score": row.final_score,
                "coarse_score": row.coarse_score,
                "atom_score": row.atom_score,
                "ncc_score": row.ncc_score,
                "best_shift": list(row.best_shift),
            }
            for i, row in enumerate(ranked[:top_k])
        ],
    }
    report_path.write_text(json.dumps(report_payload, indent=2) + "\n", encoding="utf-8")

    try:
        base_grid_entries, grid_source = load_grid_entries(grid_number, grid_dir)
    except Exception as exc:
        print(f"Error: failed to load grid entries for output: {exc}", file=sys.stderr)
        return 1

    signed_grid_cache: Dict[int, List[GridEntry]] = {+1: base_grid_entries}
    written_json_paths: List[Path] = []
    written_xsf_paths: List[Path] = []

    for row in report_payload["top_k"]:
        rank = int(row["rank"])
        molecule_number = int(row["molecule_number"])
        axis = str(row["axis"])
        sign = int(row["sign"])

        sign_token = str(sign)
        output_stem = f"{stem}_{molecule_number:04d}_{axis}_{sign_token}"
        json_path = output_dir / f"{output_stem}.json"
        xsf_path = output_dir / f"{output_stem}.XSF"

        matched_mol_src = molecules_by_id[molecule_number]
        matched_mol = apply_axis_to_molecule(matched_mol_src, axis)

        if sign not in signed_grid_cache:
            log_step("Preparing sign=-1 DATAGRID entries for matched outputs")
            signed_grid_cache[sign] = [
                GridEntry(label=entry.label, values=-np.asarray(entry.values, dtype=float))
                for entry in base_grid_entries
            ]
        grid_entries = signed_grid_cache[sign]

        log_step(
            f"Writing output rank={rank}/{top_k}: "
            f"molecule={molecule_number:04d}, axis={axis}, sign={sign:+d}"
        )
        write_json(matched_mol, json_path)
        write_xsf(
            matched_mol,
            xsf_path,
            args.format,
            grid_entries=grid_entries,
            grid_source=grid_source,
        )
        written_json_paths.append(json_path)
        written_xsf_paths.append(xsf_path)

    print(f"Best match: Molecule#{best.molecule_number:04d} axis={best.axis} sign={best.sign:+d}")
    print(f"Score: {best.final_score:.6f} (gap to 2nd: {gap:.6f})")
    print(f"Wrote {report_path}")
    for path in written_json_paths:
        print(f"Wrote {path}")
    for path in written_xsf_paths:
        print(f"Wrote {path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
