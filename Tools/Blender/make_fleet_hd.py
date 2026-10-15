"""Aetheria high-detail fleet generator (Nanite hero meshes).

Each ship keeps its canon name, role and approximate size from make_fleet.py, rebuilt in a
used-future capital-ship style: stacked wedge hulls, stepped superstructure, trenches, turret
batteries, dense plating and machinery, lit windows and glowing engine banks.

Run:  blender --background --python make_fleet_hd.py -- <out_dir> [ShipName ...]
Import into Unreal at Uniform Scale 1.0 (1 Blender unit = 1 uu, as with the placeholders).
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from aetheria_hd import ACCENT, DARK, ENGINE, HULL, LIGHTS, Ship  # noqa: E402
import bpy  # noqa: E402

args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else sys.argv[1:]
OUT = os.path.abspath(args[0] if args else "Meshes")
ONLY = set(args[1:])
os.makedirs(OUT, exist_ok=True)


def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def lerp2(a, b, t):
    return (a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t)


def edge_turrets(ship, bow, stern, z, count, yaw, scale, inset=6.0, start=0.12, end=0.92):
    """A row of batteries along a hull edge from bow to stern, inset toward the centreline."""
    for i in range(count):
        t = start + (end - start) * i / max(1, count - 1)
        x, y = lerp2(bow, stern, t)
        y -= math.copysign(inset, y) if abs(y) > inset else 0
        ship.turret((x, y, z), yaw, scale)


def crystal_dreadnought():
    """The Crystal Dreadnought: 1,020 m flagship, Precursor crystal beam at the bow."""
    s = Ship("SM_CrystalDreadnought", seed=1020)

    # hull: two stacked wedges with sloped flanks
    lower = s.prism([(440, 0), (-490, 172), (-490, -172)], -46,
                    [(510, 0), (-510, 210), (-510, -210)], 0)
    upper = s.prism([(492, 0), (-505, 196), (-505, -196)], 0,
                    [(400, 0), (-496, 150), (-496, -150)], 32)

    # stepped superstructure
    tier1 = s.prism([(40, 0), (-120, 70), (-480, 95), (-480, -95), (-120, -70)], 32,
                    [(10, 0), (-120, 58), (-470, 80), (-470, -80), (-120, -58)], 58)
    tier2 = s.prism([(-160, 0), (-230, 55), (-460, 66), (-460, -66), (-230, -55)], 58,
                    [(-185, 0), (-235, 44), (-452, 54), (-452, -54), (-235, -44)], 84)

    # command tower and bridge
    neck = s.box((-405, 0, 105), (40, 30, 42), HULL)
    bridge = s.box((-400, 0, 134), (38, 124, 16), HULL)
    s.box((-400, 0, 144), (30, 90, 4), DARK)
    for y in (-46, 46):
        s.cylinder((-412, y, 158), 1.4, 32, DARK, axis="Z", segments=8)
        s.cylinder((-412, y, 175), 4.0, 1.2, LIGHTS, axis="Z", segments=10)
    s.cylinder((-420, 0, 165), 9, 6, HULL, axis="Z", segments=20, radius2=3)

    # dorsal trench along the spine
    s.box((180, 0, 32.6), (400, 30, 1.2), DARK)
    for y in (-16, 16):
        s.box((180, y, 36), (400, 4, 8), HULL)
    trench_floor = [f for f in s.box((180, 0, 33.4), (396, 26, 0.4), DARK)]

    # bow: prismatic crystal emitter and flank crystals
    s.crystal((506, 0, -10), (360, 0, -10), 18)
    s.crystal((420, -34, -24), (330, -30, -20), 8)
    s.crystal((420, 34, -24), (330, 30, -20), 8)
    s.box((372, 0, -10), (30, 52, 30), DARK)

    # ventral hangar and keel lights
    s.box((-120, 0, -46.5), (180, 70, 2), DARK)
    for i in range(18):
        x = -200 + i * 9.5
        for y in (-37, 37):
            s.box((x, y, -47.6), (3, 1, 0.4), LIGHTS)
    s.box((-200, 0, -60), (60, 40, 26), HULL)

    # stern engine bank
    for y in (-112, 0, 112):
        s.engine((-506, y, -6), 30, 22)
    for y, z in ((-62, 14), (62, 14), (-162, -14), (162, -14)):
        s.engine((-506, y, z), 14, 16)
    s.box((-497, 0, 24), (18, 300, 12), DARK)

    # batteries along the upper hull edges and the first tier
    edge_turrets(s, (400, 0), (-496, 150), 32, 11, 60, 1.6)
    edge_turrets(s, (400, 0), (-496, -150), 32, 11, -60, 1.6)
    edge_turrets(s, (10, 0), (-470, 80), 58, 5, 75, 1.2)
    edge_turrets(s, (10, 0), (-470, -80), 58, 5, -75, 1.2)

    # surface detail
    hull_faces = lower + upper + tier1 + tier2
    tops = s.faces_facing(hull_faces, (0, 0, 1), 0.3)
    sides = [f for f in hull_faces if f not in tops and f.is_valid]
    s.panel_grid(tops, cell=(22, 12), height=(0.4, 2.2))
    s.panel_grid(sides, cell=(16, 8), height=(0.3, 1.2), gap=0.6)
    s.greeble(s.faces_facing(tier1 + tier2, (0, 0, 1), 0.5), density=0.02)
    s.greeble(trench_floor, density=0.05, size=(1, 4), height=(1, 5))
    s.greeble(s.faces_facing(upper, (0, 0, 1), 0.3), density=0.004, size=(3, 10), height=(2, 8))
    for f in s.faces_facing(tier1 + tier2 + list(bridge) + list(neck), (0, 1, 0), 0.6) + \
            s.faces_facing(tier1 + tier2 + list(bridge) + list(neck), (0, -1, 0), 0.6):
        s.windows(f, rows=3, count=60)
    for f in s.faces_facing(list(bridge), (1, 0, 0), 0.8):
        s.windows(f, rows=2, count=50, size=(2.0, 0.8))

    s.export(OUT, bevel=0.2)


SHIPS = {
    "SM_CrystalDreadnought": crystal_dreadnought,
}

if __name__ == "__main__":
    for name, build in SHIPS.items():
        if ONLY and name not in ONLY:
            continue
        reset()
        build()
