"""Act VIII Iron Tithe meshes (ACT_VIII_IRON_TITHE.md 12; WORLD_BIBLE.md 2.2, 4.5, 4.7).

Ships: the Tithe Carrack (twin armoured booms around a spinal ram), the Tithe Collector (a tug with grapple arms)
and the Writ of Sector 7 (a lengthened cruiser with three armour belts and a graft spike as tall as its tower).
Set pieces: Tithe Anchorage (Grieve's ring gap), the Graft Forge (Cauter), the Toll Line's derelict toll
platforms and mines, and the forge's cable pylons.

Same kit and scale as make_fleet_hd.py (1 Blender unit = 1 uu, forward = +X); gunmetal slab armour, red warning
strips (ACCENT), red-orange engines. Ship lengths match their collision spheres in AetheriaTitheShips.cpp.

Run:  blender --background --factory-startup --python make_tithe_hd.py -- <out_dir> [MeshName ...]
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


def mirror(plan):
    """The plan reflected across the centreline, kept counter-clockwise."""
    return [(x, -y) for x, y in reversed(plan)]


def detail(s, hull_faces, k, greeble_faces=(), window_faces=(), window_rows=2, window_count=30):
    """Panelling, machinery and windows scaled by k (1.0 = Dreadnought scale), as make_fleet_hd.detail."""
    tops = s.faces_facing(hull_faces, (0, 0, 1), 0.3)
    sides = [f for f in hull_faces if f not in tops and f.is_valid]
    s.panel_grid(tops, cell=(22 * k, 12 * k), height=(0.4 * k, 2.2 * k), gap=0.7 * k)
    s.panel_grid(sides, cell=(16 * k, 8 * k), height=(0.3 * k, 1.2 * k), gap=0.6 * k)
    if greeble_faces:
        s.greeble(s.faces_facing(greeble_faces, (0, 0, 1), 0.4), density=0.02 / (k * k),
                  size=(1.5 * k, 8 * k), height=(1 * k, 6 * k))
    for f in s.faces_facing(window_faces, (0, 1, 0), 0.6) + s.faces_facing(window_faces, (0, -1, 0), 0.6):
        s.windows(f, rows=window_rows, count=window_count, size=(1.2 * max(k, 0.4), 0.5 * max(k, 0.4)))


def slab(s, x0, x1, half_w, z0, z1, chamfer=0.15, slot=HULL):
    """A chamfered armour slab from x0 (stern) to x1 (bow)."""
    c = half_w * chamfer
    bottom = [(x1, -half_w + c), (x1, half_w - c), (x0, half_w), (x0, -half_w)]
    top = [(x1 - c, -half_w + 2 * c), (x1 - c, half_w - 2 * c), (x0 + c, half_w - c), (x0 + c, -half_w + c)]
    return s.prism(bottom, z0, top, z1, slot)


def belt(s, x, length, half_w, z0, z1):
    """A riveted armour belt around a hull: a raised dark band with a warning strip on each flank."""
    s.box((x, 0, (z0 + z1) / 2), (length, half_w * 2 + 6, z1 - z0 + 6), DARK)
    for y in (-(half_w + 3.2), half_w + 3.2):
        s.box((x, y, z1 - 2), (length * 0.9, 0.8, 1.6), ACCENT)


def stencil(s, x, y, z, w, h):
    """Faded hull numbers: a few short light bars on a flank (WORLD_BIBLE 2.2)."""
    for i in range(3):
        s.box((x + i * w * 0.4, y, z), (w * 0.25, 0.3, h), LIGHTS)


# ---- ships ------------------------------------------------------------------------------------

def tithe_carrack():
    """Tithe Carrack, ~560 m: heavy siege ship, twin armoured booms around a spinal ram."""
    s = Ship("SM_TitheCarrack", seed=560)
    spine = slab(s, -250, 170, 40, -30, 34)
    ram = s.prism([(300, 0), (170, 24), (170, -24)], -18, [(290, 0), (170, 16), (170, -16)], 18)
    s.box((300, 0, 0), (12, 8, 26), DARK)                          # the ram's striking face
    booms = []
    for side in (-1, 1):
        plan = [(230, 64), (230, 92), (-210, 96), (-210, 60)]
        plan = plan if side > 0 else mirror(plan)
        booms += s.prism(plan, -26, plan, 22)
        s.box((-40, side * 52, 0), (220, 26, 30), DARK)             # struts tying the boom to the spine
        s.box((232, side * 78, 0), (6, 30, 46), DARK)
        s.box((150, side * 97, 0), (140, 1.2, 4), ACCENT)
    tower = s.box((-150, 0, 52), (60, 48, 36), HULL)
    s.box((-130, 0, 62), (4, 40, 6), LIGHTS)
    belt(s, -40, 60, 40, -30, 34)
    s.box((-262, 0, 0), (24, 110, 60), DARK)
    for y in (-30, 0, 30):
        for z in (-14, 14):
            s.engine((-272, y, z), 11, 10)
    for i in range(5):
        s.turret((110 - i * 60, 0, 34), 0, 1.3)
    s.box((175, 0, 20), (1.2, 46, 3), ACCENT)
    stencil(s, -60, 41, 10, 20, 8)
    detail(s, spine + booms + ram, 0.45, greeble_faces=spine + booms,
           window_faces=spine + list(tower), window_rows=2, window_count=30)
    s.export(OUT, bevel=0.1)


def tithe_collector():
    """Tithe Collector, ~220 m: tug-like boarding ship with grapple arms, built to take the Dreadnought."""
    s = Ship("SM_TitheCollector", seed=220)
    body = slab(s, -90, 70, 34, -22, 22, chamfer=0.25)
    cab = s.box((40, 0, 28), (36, 30, 14), HULL)
    s.box((56, 0, 30), (2, 24, 5), LIGHTS)
    arms = []
    for side in (-1, 1):
        for z in (-12, 12):
            # an arm reaching forward and in, ending in a grapple claw
            root = Vector((60, side * 30, z))
            tip = Vector((128, side * 18, z * 0.6))
            mid = (root + tip) / 2
            yaw = math.degrees(math.atan2(tip.y - root.y, tip.x - root.x))
            arms += s.box(mid, ((tip - root).length, 6, 6), DARK, yaw=yaw)
            s.box(tip + Vector((6, 0, 0)), (10, 10, 3), HULL)
            s.box(tip + Vector((10, -side * 4, 0)), (8, 2, 8), ACCENT)
    s.box((-100, 0, 0), (20, 60, 40), DARK)
    for y in (-14, 14):
        s.engine((-110, y, 0), 10, 9)
    s.box((0, 35, 0), (120, 1.0, 3), ACCENT)
    s.box((0, -35, 0), (120, 1.0, 3), ACCENT)
    stencil(s, -40, 35, 10, 12, 5)
    detail(s, body, 0.25, greeble_faces=body, window_faces=list(cab), window_rows=1, window_count=8)
    s.export(OUT, bevel=0.05)


def writ_of_sector_7():
    """Writ of Sector 7, ~650 m: Vance's lengthened cruiser, three armour belts, a graft spike as tall as its tower."""
    s = Ship("SM_WritOfSector7", seed=707)
    spine = slab(s, -320, 200, 40, -26, 28)
    hammer = s.prism([(320, -84), (320, 84), (280, 112), (200, 112), (200, -112), (280, -112)], -32,
                     [(310, -74), (310, 74), (276, 100), (210, 100), (210, -100), (276, -100)], 32)
    keel = s.prism([(-60, -3), (-60, 3), (-220, 3), (-220, -3)], -80, [(40, -4), (40, 4), (-240, 4), (-240, -4)], -26)
    for x in (120, -40, -200):
        belt(s, x, 46, 40, -26, 28)
    tower = slab(s, -230, -120, 34, 28, 120, chamfer=0.25)
    s.box((-118, 0, 104), (6, 56, 8), LIGHTS)
    # the graft spike: Severant ceramic bolted through the armour, as tall as the tower, glowing seams
    s.cylinder((-40, 0, 110), 16, 170, DARK, axis="Z", segments=10, radius2=2.5)
    for a in range(5):
        ang = math.radians(72 * a)
        s.box((-40 + 9 * math.cos(ang), 9 * math.sin(ang), 110), (1.2, 1.2, 150), LIGHTS)
    s.cylinder((-40, 0, 30), 24, 10, HULL, axis="Z", segments=10)    # the clamp collar
    s.box((-336, 0, 0), (30, 110, 64), DARK)
    for y in (-30, 0, 30):
        for z in (-14, 14):
            s.engine((-348, y, z), 12, 11)
    for i in range(10):
        x = 180 - i * 46
        for y in (-20, 20):
            s.turret((x, y, 28), 90 if y > 0 else -90, 1.3)
    for y in (-113, 113):
        s.box((260, y, 0), (110, 1.2, 4), ACCENT)
    stencil(s, 0, 41, 8, 26, 9)
    detail(s, spine + hammer + keel + tower, 0.5, greeble_faces=spine + hammer,
           window_faces=hammer + spine + tower, window_rows=3, window_count=46)
    s.export(OUT, bevel=0.12)


# ---- set pieces -------------------------------------------------------------------------------

def tithe_anchorage():
    """Tithe Anchorage, ~3,500 m: a fortified spine across Grieve's ring gap, hangars hung beneath like teeth,
    a tower at each end, a ring of turrets and an empty dock cut to the Dreadnought's dimensions."""
    s = Ship("SM_TitheAnchorage", seed=3500)
    spine = slab(s, -1750, 1750, 120, -60, 60, chamfer=0.2)
    hangars = []
    for i in range(8):
        x = -1400 + i * 400
        hangars += s.box((x, 0, -170), (260, 200, 220), HULL)
        s.box((x + 131, 0, -190), (4, 150, 120), DARK)              # hangar mouth
        s.box((x + 133, 0, -120), (2, 160, 6), ACCENT)
        s.box((x + 133, 0, -255), (2, 160, 6), ACCENT)
        s.box((x + 133, 0, -190), (1, 140, 110), ENGINE)            # red-orange hangar light
    towers = []
    for x in (-1650, 1650):
        for k, (w, h) in enumerate(((220, 260), (170, 220), (120, 180))):
            towers += s.box((x, 0, 60 + 130 + k * 220 - 40), (w, w, h), HULL)
        s.box((x, 0, 760), (40, 40, 60), LIGHTS)
    for i in range(24):
        x = -1500 + i * 130
        s.turret((x, -70 if i % 2 else 70, 60), 90 if i % 2 == 0 else -90, 6.0)
    # docking booms projecting forward
    for y in (-90, 90):
        s.box((1950, y, 0), (400, 24, 24), DARK)
        for k in range(4):
            s.box((1800 + k * 100, y, 0), (8, 30, 40), HULL)
    # the empty dock: an open frame the Dreadnought's size, slung on the stern side
    frame = []
    for z in (-140, 140):
        for y in (-230, 230):
            frame += s.box((-200, y, z - 400), (1100, 20, 20), DARK)
    for x in (-740, -380, -20, 340):
        for y in (-230, 230):
            s.box((x, y, -400), (20, 20, 300), DARK)
        s.box((x, 0, -260), (20, 480, 20), DARK)
        s.box((x, 0, -260), (6, 460, 4), LIGHTS)
    for x in (-1100, 0, 1100):
        stencil(s, x, 121, 0, 120, 40)
    detail(s, spine + towers + hangars, 1.6, greeble_faces=spine + towers,
           window_faces=spine + towers, window_rows=4, window_count=80)
    s.export(OUT, bevel=0.4)


def graft_forge():
    """The Graft Forge, ~1,800 m: a gunmetal foundry anchored to Cauter by three huge cables, with an ash-white
    Severant crucible at its heart where the grafts are grown."""
    s = Ship("SM_GraftForge", seed=1800)
    decks = []
    for k, (x, w, z) in enumerate(((0, 900, 0), (0, 700, 140), (0, 520, -140))):
        decks += slab(s, x - w / 2, x + w / 2, w * 0.32, z - 40, z + 40, chamfer=0.2)
    # three truss pylons running down to the surface
    for a in range(3):
        ang = math.radians(120 * a + 30)
        x, y = 420 * math.cos(ang), 420 * math.sin(ang)
        s.box((x, y, -700), (40, 40, 1200), DARK)
        for k in range(8):
            s.box((x, y, -200 - k * 140), (80, 8, 8), HULL, yaw=45)
        s.box((x, y, -1310), (120, 120, 30), HULL)
    # the crucible: a torus ring with spikes and a glowing vertical core
    for i in range(24):
        ang = 2 * math.pi * i / 24
        m = Matrix.Translation(Vector((300 * math.cos(ang), 300 * math.sin(ang), 260))) @ Matrix.Rotation(ang, 4, "Z")
        s.box(None, (40, 82, 40), HULL, matrix=m)
        if i % 3 == 0:
            tip = Vector((360 * math.cos(ang), 360 * math.sin(ang), 400))
            s.crystal(tip, Vector((300 * math.cos(ang), 300 * math.sin(ang), 270)), 14, slot=HULL, sides=5)
    s.cylinder((0, 0, 260), 90, 360, HULL, axis="Z", segments=16)
    s.cylinder((0, 0, 260), 70, 380, LIGHTS, axis="Z", segments=16)  # Silence-blue glow (seen through slots)
    for a in range(6):
        ang = math.radians(60 * a)
        s.box((92 * math.cos(ang), 92 * math.sin(ang), 260), (6, 6, 300), LIGHTS)
    for i in range(10):
        s.turret((-400 + i * 90, 290 if i % 2 else -290, 40), 0, 4.0)
    s.box((0, 0, 40), (900, 2, 6), ACCENT)
    detail(s, decks, 1.2, greeble_faces=decks, window_faces=decks, window_rows=3, window_count=40)
    s.export(OUT, bevel=0.3)


def forge_pylon():
    """A cable pylon of the Graft Forge (E2): a truss tower clamping one of the forge's cables, ~260 m."""
    s = Ship("SM_ForgePylon", seed=261)
    s.box((0, 0, 0), (40, 40, 260), DARK)
    for k in range(6):
        s.box((0, 0, -110 + k * 44), (70, 6, 6), HULL, yaw=45)
        s.box((0, 0, -110 + k * 44), (70, 6, 6), HULL, yaw=-45)
    s.cylinder((0, 0, 140), 34, 30, HULL, axis="Z", segments=12)
    s.cylinder((0, 0, 140), 26, 34, LIGHTS, axis="Z", segments=12)
    s.cylinder((0, 0, 200), 6, 140, DARK, axis="Z", segments=8)     # the cable stub
    s.box((0, 0, -140), (90, 90, 20), HULL)
    for y in (-21, 21):
        s.box((0, y, 60), (30, 1.0, 3), ACCENT)
    s.export(OUT, bevel=0.06)


def toll_platform():
    """A derelict toll platform of the Toll Line, ~400 m: an octagonal deck, a gate frame and broken spars."""
    s = Ship("SM_TollPlatform", seed=400)
    plan = [(150 * math.cos(math.radians(22.5 + 45 * i)), 150 * math.sin(math.radians(22.5 + 45 * i))) for i in range(8)]
    inner = [(x * 0.9, y * 0.9) for x, y in plan]
    deck = s.prism(plan, -20, inner, 16)
    gate = []
    for y in (-150, 150):
        gate += s.box((0, y, 120), (30, 30, 240), DARK)
    gate += s.box((0, 0, 250), (30, 330, 26), DARK)
    s.box((0, 0, 236), (6, 300, 4), ACCENT)
    s.box((-60, 0, 40), (80, 70, 50), HULL)                          # the toll office
    s.box((-19, 0, 46), (2, 50, 6), LIGHTS)
    for ang, length in ((30, 160), (150, 120), (260, 190), (320, 90)):
        a = math.radians(ang)
        c = Vector((150 * math.cos(a) + length / 2 * math.cos(a), 150 * math.sin(a) + length / 2 * math.sin(a), 0))
        s.box(c, (length, 10, 10), DARK, yaw=ang)
    for i in range(6):
        s.turret((90 * math.cos(i), 90 * math.sin(i), 16), math.degrees(i), 2.0)
    detail(s, deck + gate, 0.6, greeble_faces=deck, window_faces=[], window_count=0)
    s.export(OUT, bevel=0.08)


def toll_mine():
    """A Toll Line mine, ~30 m: a spiked gunmetal sphere with a red strobe."""
    s = Ship("SM_TollMine", seed=30)
    s.cylinder((0, 0, 0), 10, 16, HULL, axis="Z", segments=10)
    s.cylinder((0, 0, 9), 10, 4, HULL, axis="Z", segments=10, radius2=5)
    s.cylinder((0, 0, -9), 5, 4, HULL, axis="Z", segments=10, radius2=10)
    for i in range(8):
        a = 2 * math.pi * i / 8
        s.crystal((16 * math.cos(a), 16 * math.sin(a), (-1) ** i * 4), (8 * math.cos(a), 8 * math.sin(a), 0), 1.6,
                  slot=DARK, sides=4)
    s.box((0, 0, 12), (3, 3, 2), ACCENT)
    s.box((0, 10, 0), (4, 0.4, 2), LIGHTS)
    s.export(OUT, bevel=0.02)


MESHES = {
    "SM_TitheCarrack": tithe_carrack,
    "SM_TitheCollector": tithe_collector,
    "SM_WritOfSector7": writ_of_sector_7,
    "SM_TitheAnchorage": tithe_anchorage,
    "SM_GraftForge": graft_forge,
    "SM_ForgePylon": forge_pylon,
    "SM_TollPlatform": toll_platform,
    "SM_TollMine": toll_mine,
}

if __name__ == "__main__":
    for name, build in MESHES.items():
        if ONLY and name not in ONLY:
            continue
        reset()
        build()
