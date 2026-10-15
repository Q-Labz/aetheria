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


def mirror(plan):
    """The plan reflected across the centreline, kept counter-clockwise."""
    return [(x, -y) for x, y in reversed(plan)]


def detail(s, hull_faces, k, greeble_faces=(), window_faces=(), window_rows=2, window_count=30):
    """Panelling, machinery and windows scaled by k (1.0 = Dreadnought scale)."""
    tops = s.faces_facing(hull_faces, (0, 0, 1), 0.3)
    sides = [f for f in hull_faces if f not in tops and f.is_valid]
    s.panel_grid(tops, cell=(22 * k, 12 * k), height=(0.4 * k, 2.2 * k), gap=0.7 * k)
    s.panel_grid(sides, cell=(16 * k, 8 * k), height=(0.3 * k, 1.2 * k), gap=0.6 * k)
    if greeble_faces:
        s.greeble(s.faces_facing(greeble_faces, (0, 0, 1), 0.4), density=0.02 / (k * k),
                  size=(1.5 * k, 8 * k), height=(1 * k, 6 * k))
    for f in s.faces_facing(window_faces, (0, 1, 0), 0.6) + s.faces_facing(window_faces, (0, -1, 0), 0.6):
        s.windows(f, rows=window_rows, count=window_count, size=(1.2 * max(k, 0.4), 0.5 * max(k, 0.4)))


def phantom_cruiser():
    """Phantom battle-cruiser, 226 m: the Dreadnought's sleek little sister, twin outrigger nacelles."""
    s = Ship("SM_PhantomCruiser", seed=226)
    lower = s.prism([(98, 0), (-108, 36), (-108, -36)], -10, [(113, 0), (-113, 44), (-113, -44)], 0)
    upper = s.prism([(110, 0), (-112, 42), (-112, -42)], 0, [(88, 0), (-108, 30), (-108, -30)], 8)
    tier = s.prism([(20, 0), (-30, 16), (-100, 20), (-100, -20), (-30, -16)], 8,
                   [(10, 0), (-30, 12), (-96, 16), (-96, -16), (-30, -12)], 15)
    bridge = s.box((-86, 0, 19), (14, 24, 7), HULL)
    s.box((-86, 0, 23), (10, 18, 1.5), DARK)

    # outriggers and engine nacelles
    for side in (-1, 1):
        s.box((-78, side * 40, -2), (34, 16, 4), HULL)
        s.cylinder((-72, side * 54, -2), 7.5, 64, HULL, segments=20)
        s.cylinder((-72, side * 54, -2), 8.0, 10, DARK, segments=20)
        s.engine((-104, side * 54, -2), 6.5, 6)
        s.crystal((-36, side * 54, -2), (-48, side * 54, -2), 3.5)
    s.engine((-113, 0, -3), 9, 8)
    for y in (-20, 20):
        s.engine((-112, y, -2), 4, 5)

    # crystal core and bow emitter
    s.crystal((12, 0, 25), (-44, 0, 15), 5)
    s.crystal((114, 0, -4), (80, 0, -4), 4)

    edge_turrets(s, (88, 0), (-108, 30), 8, 5, 60, 0.45, inset=3)
    edge_turrets(s, (88, 0), (-108, -30), 8, 5, -60, 0.45, inset=3)

    detail(s, lower + upper + tier, 0.35, greeble_faces=tier + upper,
           window_faces=tier + list(bridge), window_count=20)
    s.export(OUT, bevel=0.06)


def corsair_gunship():
    """Corsair salvage gunship, 150 m: patched-together and asymmetric, flak batteries everywhere."""
    s = Ship("SM_CorsairGunship", seed=150)
    plan_lo = [(68, 0), (55, 12), (-66, 15), (-66, -15), (55, -12)]
    plan_mid = [(75, 0), (60, 14), (-70, 18), (-70, -18), (60, -14)]
    plan_hi = [(62, 0), (52, 10), (-66, 13), (-66, -13), (52, -10)]
    lower = s.prism(plan_lo, -12, plan_mid, 0)
    upper = s.prism(plan_mid, 0, plan_hi, 10)

    # offset cockpit pod on a pylon
    s.box((40, 16, 8), (10, 8, 6), DARK)
    pod = s.cylinder((44, 24, 10), 6.5, 30, HULL, segments=18)
    s.cylinder((60, 24, 10), 6.5, 2, DARK, segments=18, radius2=4)
    s.box((60.6, 24, 11.5), (0.4, 6, 1.4), LIGHTS)   # canopy slit

    # bolted-on cargo module and containers
    module = s.box((-18, -26, 2), (54, 14, 18), HULL)
    s.box((-18, -19, 2), (40, 2, 10), DARK)
    for i in range(7):
        x = 30 - i * 13 + s.rng.uniform(-2, 2)
        y, h = s.rng.uniform(-6, 6), s.rng.uniform(4, 7)
        w, d = s.rng.uniform(8, 12), s.rng.uniform(5, 9)
        s.box((x, y, 10 + h / 2), (w, d, h), s.rng.choice((HULL, DARK)))
        s.box((x, y - d / 2 - 0.1, 10 + h * 0.75), (w * 0.8, 0.2, 0.5), ACCENT)   # hazard stripe

    # exposed pipe runs and a sensor mast
    for z in (-3, 3):
        s.cylinder((0, 17.5, z), 1.0, 120, DARK, segments=10)
        s.cylinder((0, -17.5 if z > 0 else 17.5, z - 6), 0.8, 90, DARK, segments=10)
    s.cylinder((-40, 6, 22), 0.7, 24, DARK, axis="Z", segments=8)
    s.cylinder((-40, 6, 34), 3.0, 0.8, LIGHTS, axis="Z", segments=10)

    # flak batteries
    for x, y in ((40, -6), (10, 6), (-20, -4), (-50, 6), (-10, -33), (-38, -33)):
        z = 11 if abs(y) < 15 else 11.5
        s.turret((x, y, z), s.rng.uniform(-90, 90), 0.5)

    # mismatched engines
    s.engine((-72, 6, 0), 9, 8)
    s.engine((-72, -10, 3), 6, 6)
    s.engine((-46, -26, 2), 5.5, 6)
    s.engine((-46, 24, 10), 4, 5)

    detail(s, lower + upper + list(module), 0.25, greeble_faces=upper + list(module),
           window_faces=list(pod) + list(module), window_count=14)
    s.export(OUT, bevel=0.05)


def tithe_cruiser():
    """Picket Cruiser 'Tithe of Sector 7', 445 m: hammerhead bow, armoured spine, heavy batteries."""
    s = Ship("SM_TitheCruiser", seed=445)
    spine = s.prism([(120, -32), (120, 32), (-222, 32), (-222, -32)], -20,
                    [(120, -24), (120, 24), (-216, 24), (-216, -24)], 22)
    hammer = s.prism([(222, -70), (222, 70), (190, 95), (120, 95), (120, -95), (190, -95)], -26,
                     [(214, -62), (214, 62), (186, 85), (128, 85), (128, -85), (186, -85)], 26)
    fin = s.prism([(40, -3), (40, 3), (-170, 3), (-170, -3)], 22,
                  [(-30, -2), (-30, 2), (-160, 2), (-160, -2)], 72)
    keel = s.prism([(-40, -2), (-40, 2), (-150, 2), (-150, -2)], -58,
                   [(30, -3), (30, 3), (-160, 3), (-160, -3)], -20)
    bridge = s.box((160, 0, 34), (34, 56, 16), HULL)
    s.box((176, 0, 34), (4, 50, 6), LIGHTS)

    # stern engine block
    s.box((-228, 0, 0), (16, 84, 52), DARK)
    for y in (-22, 22):
        for z in (-12, 12):
            s.engine((-238, y, z), 11, 10)

    # warning strips along the hammer and spine
    for y in (-96, 96):
        s.box((170, y, 0), (90, 1.2, 4), ACCENT)
    s.box((223, 0, 14), (1.2, 120, 3), ACCENT)
    for y in (-33, 33):
        s.box((-50, y, 10), (300, 1.0, 2.5), ACCENT)

    # heavy batteries
    for i in range(8):
        x = 100 - i * 38
        for y in (-16, 16):
            s.turret((x, y, 22), 90 if y > 0 else -90, 1.1)
    for x, y in ((200, -50), (200, 50), (150, -72), (150, 72), (135, -40), (135, 40)):
        s.turret((x, y, 26), 0, 1.3)

    detail(s, spine + hammer + fin + keel, 0.45, greeble_faces=spine + hammer,
           window_faces=hammer + spine + list(bridge), window_rows=3, window_count=40)
    for f in s.faces_facing(hammer, (1, 0, 0), 0.8):
        s.windows(f, rows=3, count=40, size=(1.6, 0.6), slot=LIGHTS)
    s.export(OUT, bevel=0.1)


def tithe_interceptor():
    """Tithe interceptor, 118 m: angular fuselage, forward-swept blade wings, one heavy engine."""
    s = Ship("SM_TitheInterceptor", seed=118)
    lower = s.prism([(52, 0), (10, 8), (-48, 7), (-48, -7), (10, -8)], -6,
                    [(59, 0), (10, 10), (-50, 8), (-50, -8), (10, -10)], 0)
    upper = s.prism([(59, 0), (10, 10), (-50, 8), (-50, -8), (10, -10)], 0,
                    [(48, 0), (10, 7), (-48, 6), (-48, -6), (10, -7)], 6)
    canopy = s.box((26, 0, 7.5), (16, 6, 3), DARK)
    s.box((32, 0, 8.6), (6, 4.4, 1.0), LIGHTS)

    right = [(-40, 8), (-18, 8), (8, 46), (-6, 46)]
    wings = []
    for plan in (right, mirror(right)):
        wings += s.prism(plan, -1.2, plan, 1.2)
    for side in (-1, 1):
        s.cylinder((10, side * 46, 0), 0.9, 20, DARK, segments=10)
        s.box((20.5, side * 46, 0), (1.5, 1.4, 1.4), ACCENT)
        s.box((-10, side * 26, 1.3), (14, 1.0, 0.3), ACCENT)
        s.box((-30, side * 9, 4), (16, 3, 3), HULL)

    s.engine((-50, 0, 0), 6, 6)
    for y in (-6, 6):
        s.engine((-49, y, 3.5), 2.5, 4)

    detail(s, lower + upper + wings, 0.2, greeble_faces=upper, window_faces=[], window_count=0)
    s.export(OUT, bevel=0.04)


SHIPS = {
    "SM_CrystalDreadnought": crystal_dreadnought,
    "SM_PhantomCruiser": phantom_cruiser,
    "SM_CorsairGunship": corsair_gunship,
    "SM_TitheCruiser": tithe_cruiser,
    "SM_TitheInterceptor": tithe_interceptor,
}

if __name__ == "__main__":
    for name, build in SHIPS.items():
        if ONLY and name not in ONLY:
            continue
        reset()
        build()
