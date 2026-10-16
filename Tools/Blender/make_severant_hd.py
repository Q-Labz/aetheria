"""Severant ships for Act VII (WORLD_BIBLE.md 2.4; ACT_VII_CINDER_ARCHIVE.md 13).

Ash-white ceramic and clouded glass, long tapered spikes, splinter fins and "choir ring" collars that carry the
Silence Pulse (the ACCENT slot glows Silence blue in Unreal). Same kit and scale as make_fleet_hd.py:
1 Blender unit = 1 uu, forward = +X.

Run:  blender --background --factory-startup --python make_severant_hd.py -- <out_dir> [ShipName ...]
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from aetheria_hd import ACCENT, DARK, ENGINE, HULL, LIGHTS, Ship  # noqa: E402
import bpy  # noqa: E402
from mathutils import Matrix, Vector  # noqa: E402

args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else sys.argv[1:]
OUT = os.path.abspath(args[0] if args else "Meshes")
ONLY = set(args[1:])
os.makedirs(OUT, exist_ok=True)


def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def needle(s, length, radius, x_tail, sides=8, slot=HULL, z=0.0, taper_tail=0.45):
    """A long tapered spindle along X: a sharp bow cone, a body and a shorter tail cone."""
    bow_len = length * 0.42
    tail_len = length * 0.18
    body_len = length - bow_len - tail_len
    x_body = x_tail + tail_len + body_len / 2
    faces = []
    faces += s.cylinder((x_body, 0, z), radius, body_len, slot, segments=sides)
    faces += s.cylinder((x_tail + tail_len + body_len + bow_len / 2, 0, z), radius, bow_len, slot, segments=sides, radius2=radius * 0.04)
    faces += s.cylinder((x_tail + tail_len / 2, 0, z), radius * taper_tail, tail_len, DARK, segments=sides, radius2=radius)
    return faces


def ring(s, x, radius, thickness, width, segments=28, slot=ACCENT, z=0.0, frame=True):
    """A choir ring: a collar around the X axis built from tangent segments, with a ceramic frame either side."""
    for i in range(segments):
        a = 2 * math.pi * (i + 0.5) / segments
        y, zz = radius * math.cos(a), radius * math.sin(a)
        seg = 2 * math.pi * radius / segments * 1.04
        m = Matrix.Translation(Vector((x, y, z + zz))) @ Matrix.Rotation(a, 4, "X")
        s.box(None, (width, seg, thickness), slot, matrix=m)
        if frame:
            for dx in (-width * 0.75, width * 0.75):
                s.box(None, (width * 0.3, seg, thickness * 1.6), HULL, matrix=Matrix.Translation(Vector((x + dx, y, z + zz))) @ Matrix.Rotation(a, 4, "X"))


def spokes(s, x, inner, outer, count, thickness, z=0.0, phase=0.0):
    for i in range(count):
        a = 2 * math.pi * i / count + phase
        mid = (inner + outer) / 2
        m = Matrix.Translation(Vector((x, mid * math.cos(a), z + mid * math.sin(a)))) @ Matrix.Rotation(a, 4, "X")
        s.box(None, (thickness, thickness, outer - inner), HULL, matrix=m @ Matrix.Rotation(math.radians(90), 4, "X"))


def spike_cluster(s, x, radius, count, length, z=0.0, rake=0.35, phase=0.0, slot=HULL):
    """Long tapered spikes raking aft from a ring of points around the hull."""
    for i in range(count):
        a = 2 * math.pi * i / count + phase
        base = Vector((x, radius * math.cos(a), z + radius * math.sin(a)))
        out = Vector((0, math.cos(a), math.sin(a)))
        tip = base + out * length + Vector((-length * rake, 0, 0))
        s.crystal(tip, base - out * radius * 0.3, max(length * 0.05, 0.6), slot, sides=5)


def splinter_fins(s, x0, x1, root, span, count, z=0.0, phase=0.0):
    """Thin splinter fins: flattened crystals radiating from the hull aft."""
    for i in range(count):
        a = 2 * math.pi * i / count + phase
        out = Vector((0, math.cos(a), math.sin(a)))
        base = Vector((x0, 0, z)) + out * root
        tip = Vector((x1, 0, z)) + out * (root + span)
        s.crystal(tip, base, max(span * 0.06, 0.5), DARK, sides=4)


def windows_along(s, x0, x1, radius, rows, slot=LIGHTS, z=0.0, count=12):
    """Pale portholes around the body (Severant interiors glow dim blue-white)."""
    for r in range(rows):
        a = 2 * math.pi * r / rows + 0.3
        for i in range(count):
            x = x0 + (x1 - x0) * (i + 0.5) / count
            m = Matrix.Translation(Vector((x, radius * 1.01 * math.cos(a), z + radius * 1.01 * math.sin(a)))) @ Matrix.Rotation(a, 4, "X")
            s.box(None, (max(radius * 0.25, 0.8), max(radius * 0.08, 0.3), max(radius * 0.1, 0.3)), slot, matrix=m)


def choir_skiff():
    """Choir-skiff, 112 m: a fast needle with one choir ring and four splinter fins."""
    s = Ship("SM_SeverantChoirSkiff", seed=112)
    hull = needle(s, 112, 6.5, -56)
    ring(s, -8, 11, 1.2, 4, segments=20)
    spokes(s, -8, 6.5, 10.5, 4, 1.2, phase=math.radians(45))
    splinter_fins(s, -40, -62, 6, 18, 4, phase=math.radians(45))
    spike_cluster(s, 28, 5.5, 3, 14, rake=0.6, phase=math.radians(90))
    windows_along(s, -30, 20, 6.5, 2, count=8)
    s.engine((-58, 0, 0), 3.2, 3, slot_glow=ENGINE)
    s.panel_grid(hull, cell=(9.0, 3.0), height=(0.2, 0.6), gap=0.35)
    s.export(OUT, bevel=0.04)


def emitter_frigate():
    """Silence Emitter Frigate, 262 m: a heavier needle carrying a great emitter ring at the bow and two choir rings."""
    s = Ship("SM_SilenceEmitterFrigate", seed=262)
    hull = needle(s, 262, 15, -131, sides=10)
    ring(s, 62, 34, 2.6, 9, segments=36)                   # the emitter
    spokes(s, 62, 15, 32, 6, 3.0)
    ring(s, -10, 22, 1.8, 6, segments=28)
    ring(s, -60, 20, 1.8, 6, segments=28)
    for x in (-10, -60):
        spokes(s, x, 14, 20.5, 4, 2.0, phase=math.radians(45))
    splinter_fins(s, -90, -140, 13, 34, 6)
    spike_cluster(s, 100, 11, 4, 30, rake=0.55, phase=math.radians(45))
    spike_cluster(s, -30, 14, 6, 22, rake=0.4)
    windows_along(s, -110, 40, 15, 3, count=16)
    for y in (-8, 8):
        s.engine((-133, y, 0), 5.5, 5, slot_glow=ENGINE)
    s.panel_grid(hull, cell=(16.0, 5.0), height=(0.3, 1.0), gap=0.5)
    s.export(OUT, bevel=0.06)


def last_silence():
    """The Last Silence, Exarch Ithane's flagship, 720 m: a cathedral spire of stacked choir rings and spike crowns."""
    s = Ship("SM_TheLastSilence", seed=720)
    hull = needle(s, 720, 40, -360, sides=12)
    upper = needle(s, 420, 18, -260, sides=10, z=48)          # dorsal spire
    s.box((-60, 0, 26), (260, 14, 24), HULL)                   # spine joining the spires
    for i, x in enumerate((150, 60, -30, -120, -210)):
        r = 70 - i * 6
        ring(s, x, r, 4.0, 16, segments=44)
        spokes(s, x, 40, r - 2, 6, 6.0, phase=math.radians(30 * i))
    ring(s, 40, 32, 3.0, 10, segments=32, z=48)
    spike_cluster(s, 250, 30, 6, 90, rake=0.5)
    spike_cluster(s, 10, 38, 8, 70, rake=0.35, phase=math.radians(22.5))
    spike_cluster(s, -200, 36, 6, 60, rake=0.25)
    splinter_fins(s, -260, -380, 36, 110, 6, phase=math.radians(30))
    windows_along(s, -300, 120, 40, 5, count=30)
    windows_along(s, -220, 100, 18, 3, z=48, count=18)
    for y, z in ((-18, -10), (18, -10), (0, 16)):
        s.engine((-362, y, z), 12, 10, slot_glow=ENGINE)
    s.panel_grid(hull + upper, cell=(40.0, 12.0), height=(0.6, 2.4), gap=1.2)
    s.export(OUT, bevel=0.1)


SHIPS = {
    "SM_SeverantChoirSkiff": choir_skiff,
    "SM_SilenceEmitterFrigate": emitter_frigate,
    "SM_TheLastSilence": last_silence,
}

if __name__ == "__main__":
    for name, build in SHIPS.items():
        if ONLY and name not in ONLY:
            continue
        reset()
        build()
