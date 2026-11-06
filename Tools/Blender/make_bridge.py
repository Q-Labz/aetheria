"""Crystal Dreadnought bridge kit (BRIDGE_SPEC.md B1).

Interior scale: 1 Blender unit = 1 cm = 1 uu (import at Uniform Scale 1.0). Forward (the viewport) = +X,
port = +Y, floor of the holotable deck at Z = 0. Every prop's origin is its floor contact point, centred,
facing +X, so the level can place it straight from the plan in WORLD_BIBLE.md 5.2.

Run:  blender --background --factory-startup --python make_bridge.py -- <out_dir> [MeshName ...]
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import aetheria_hd  # noqa: E402
from aetheria_hd import Ship  # noqa: E402
import bpy  # noqa: E402
from mathutils import Matrix, Vector  # noqa: E402

# bridge material slots (Unreal maps them by name)
CERAMIC, TRIM, INLAY, STRIP, CRYSTAL, GLASS, FLOOR, SCREEN = range(8)
aetheria_hd.SLOT_NAMES = ("M_Ceramic", "M_DarkTrim", "M_Inlay", "M_LightStrip", "M_Crystal", "M_Glass",
                          "M_Floor", "M_Screen")
aetheria_hd.SLOT_PREVIEW = (
    ((0.89, 0.91, 0.92), 0.0), ((0.08, 0.11, 0.13), 0.0), ((0.22, 0.88, 1.0), 20.0),
    ((1.0, 0.97, 0.92), 8.0), ((0.5, 0.85, 1.0), 3.0), ((0.6, 0.85, 0.95), 0.0),
    ((0.03, 0.03, 0.035), 0.0), ((0.2, 0.7, 1.0), 6.0))

# Display content, drawn by M_BridgeScreen from UV channel 1: x = (mode + variation) / 6, y = (palette + 0.5) / 4.
# The display face itself gets 0-1 UVs in channel 0.
TEXT, RADAR, BARS, WAVE, SCHEMATIC, LIGHTS = range(6)
BLUE, AMBER, RED, GREEN = range(4)

args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else sys.argv[1:]
OUT = os.path.abspath(args[0] if args else "Bridge")
ONLY = set(args[1:])
os.makedirs(OUT, exist_ok=True)

W, D = 2800.0, 2400.0          # shell width (Y) and depth (X)
HY, HX = W / 2, D / 2
WALK, PIT = 120.0, -30.0       # command walk and forward crew pit levels
VP_X, VP_HALF = 1150.0, 1100.0  # viewport chord position and half-width
VP_BULGE = 160.0                # how far the curved pane bows forward at the centre


def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def rot(yaw=0.0, pitch=0.0, roll=0.0):
    return (Matrix.Rotation(math.radians(yaw), 4, "Z") @ Matrix.Rotation(math.radians(pitch), 4, "Y")
            @ Matrix.Rotation(math.radians(roll), 4, "X"))


def at(x, y, z, yaw=0.0, pitch=0.0, roll=0.0):
    return Matrix.Translation(Vector((x, y, z))) @ rot(yaw, pitch, roll)


def slab(s, x0, x1, y0, y1, top, thick=40.0, slot=CERAMIC):
    """Floor or ceiling slab from (x0, y0) to (x1, y1) whose top is at `top`."""
    return s.box(((x0 + x1) / 2, (y0 + y1) / 2, top - thick / 2), (x1 - x0, y1 - y0, thick), slot)


def strip(s, p0, p1, z, width=4.0, slot=INLAY, height=0.6):
    """Thin flush light line on a floor or ceiling between two plan points."""
    (x0, y0), (x1, y1) = p0, p1
    length = math.hypot(x1 - x0, y1 - y0)
    yaw = math.degrees(math.atan2(y1 - y0, x1 - x0))
    s.box(None, (length, width, height), slot, matrix=at((x0 + x1) / 2, (y0 + y1) / 2, z + height / 2, yaw))


def ring_strip(s, cx, cy, radius, z, segments=48, width=4.0, slot=INLAY):
    for i in range(segments):
        a0, a1 = 2 * math.pi * i / segments, 2 * math.pi * (i + 1) / segments
        strip(s, (cx + radius * math.cos(a0), cy + radius * math.sin(a0)),
              (cx + radius * math.cos(a1), cy + radius * math.sin(a1)), z, width, slot)


def screen(s, m, w, h, mode=None, palette=None):
    """Display panel: dark bezel with a live display face on local +X of matrix m. Large panels get
    radar, schematic or waveform displays and small ones data or bar readouts unless mode is given."""
    s.box(None, (4, w + 6, h + 6), TRIM, matrix=m)
    if mode is None:
        mode = s.rng.choice((RADAR, SCHEMATIC, WAVE, SCHEMATIC) if w * h > 4000 else (TEXT, TEXT, BARS, WAVE))
    display(s, m @ Matrix.Translation((2.5, 0, 0)), w, h, mode, palette)


def display(s, m, w, h, mode, palette=None, depth=1.0):
    """A display face (SCREEN slot) of w x h on local +X of m, recorded for its UVs."""
    if palette is None:
        palette = s.rng.choice((BLUE, BLUE, BLUE, AMBER, GREEN, RED) if mode != LIGHTS else (BLUE,))
    box_m = m @ Matrix.Diagonal((depth, w, h, 1.0))
    faces = s._cube(box_m, SCREEN)
    if not hasattr(s, "displays"):
        s.displays = []
    # faces[1] is the cube's +X face
    s.displays.append((faces[1], box_m.inverted(), (mode + s.rng.uniform(0.05, 0.95)) / 6.0, (palette + 0.5) / 4.0))


def indicator_strip(s, m, w, h):
    """A row of blinking status lights (red / green / amber) on a dark plate."""
    s.box(None, (3, w + 4, h + 4), TRIM, matrix=m)
    display(s, m @ Matrix.Translation((2.0, 0, 0)), w, h, LIGHTS)


def apply_display_uvs(s):
    """Channel 0: each display face spans 0-1; channel 1: the display's content seed and palette."""
    bm = s.bm
    uv0 = bm.loops.layers.uv.new("UVMap")
    uv1 = bm.loops.layers.uv.new("Display")
    for face, inv, seed, palette in getattr(s, "displays", []):
        if not face.is_valid:
            continue
        for loop in face.loops:
            local = inv @ loop.vert.co
            loop[uv0].uv = (0.5 - local.y, 0.5 - local.z)
            loop[uv1].uv = (seed, palette)


def viewport_points(n):
    """Points along the curved viewport (plan), port to starboard."""
    pts = []
    for i in range(n + 1):
        t = -1 + 2 * i / n
        y = t * VP_HALF
        x = VP_X + VP_BULGE * (1 - t * t)
        pts.append((x, y))
    return pts


# ---- shell -------------------------------------------------------------------------------------

def make_shell():
    s = Ship("SM_Bridge_Shell", 501)
    # decks: aft holotable floor, raised command walk, forward side ledges and the sunken helm pit
    floors = []
    floors += slab(s, -HX, 200, -HY, HY, 0, slot=FLOOR)
    floors += slab(s, 200, 800, -HY, HY, WALK, WALK + 40, slot=FLOOR)
    floors += slab(s, 800, VP_X + VP_BULGE, -HY, -900, WALK, WALK + 40, slot=FLOOR)
    floors += slab(s, 800, VP_X + VP_BULGE, 900, HY, WALK, WALK + 40, slot=FLOOR)
    floors += slab(s, 800, VP_X + VP_BULGE, -900, 900, PIT, 40, slot=FLOOR)
    # deck plating: a grid of seams aft, grating down the command walk and across the pit
    for x in range(int(-HX) + 150, 200, 150):
        s.box((x, 0, 0.3), (3, W, 0.6), TRIM)
    for y in range(int(-HY) + 150, int(HY), 150):
        s.box(((-HX + 200) / 2, y, 0.3), (HX + 200, 3, 0.6), TRIM)
    for x in range(220, 800, 30):
        s.box((x, 0, WALK + 0.3), (8, 360, 0.6), TRIM)
    for x in range(820, 1300, 30):
        s.box((x, 0, PIT + 0.3), (8, 1800, 0.6), TRIM)
    # steps: holotable floor -> walk (centre and both sides), walk -> pit
    for yc in (0, -1000, 1000):
        for k, h in enumerate((40, 80)):
            s.box((200 - 30 - k * 30 + 15, yc, h / 2), (30 * (2 - k), 360, h), CERAMIC)
    for k, h in enumerate((WALK - 50, WALK - 100)):
        s.box((800 + 15 + k * 30, 0, (PIT + h) / 2), (30, 360, h - PIT), CERAMIC)
    # walk and pit edge trim with light
    for y0, y1 in ((-HY, -200), (200, HY)):
        s.box((200, (y0 + y1) / 2, WALK - 20), (6, y1 - y0, 40), TRIM)
        strip(s, (203, y0), (203, y1), WALK - 6, 3, STRIP)
    for y0, y1 in ((-900, -200), (200, 900)):
        s.box((800, (y0 + y1) / 2, (WALK + PIT) / 2), (6, y1 - y0, WALK - PIT), TRIM)
    s.box((990, -900, (WALK + PIT) / 2), (380, 6, WALK - PIT), TRIM)
    s.box((990, 900, (WALK + PIT) / 2), (380, 6, WALK - PIT), TRIM)
    # railing along the walk's forward edge (either side of the pit stair)
    for y0, y1 in ((-900, -200), (200, 900)):
        for y in range(int(y0), int(y1) + 1, 140):
            s.box((805, y, WALK + 50), (6, 6, 100), TRIM)
        s.box((805, (y0 + y1) / 2, WALK + 100), (8, y1 - y0, 6), CRYSTAL)

    # floor inlay: rings around the holotable, radial spokes, lines down the walk
    hx, hy = -350.0, 0.0
    for r in (300, 420, 560):
        ring_strip(s, hx, hy, r, 0.0, 64)
    for k in range(8):
        a = math.radians(22.5 + 45 * k)
        strip(s, (hx + 560 * math.cos(a), hy + 560 * math.sin(a)), (hx + 900 * math.cos(a), hy + 900 * math.sin(a)), 0.0)
    for y in (-700, 700):
        strip(s, (-HX + 60, y), (180, y), 0.0, 3)
        strip(s, (220, y), (780, y), WALK, 3)

    # ceiling: steps from 7 m at the viewport to 4.5 m aft, with light strips
    ceil = []
    ceil += slab(s, 600, VP_X + VP_BULGE, -HY, HY, 740, 40)
    ceil += slab(s, 0, 600, -HY, HY, 640, 40)
    ceil += slab(s, -HX, 0, -HY, HY, 490, 40)
    s.box((600, 0, 670), (6, W, 100), TRIM)
    s.box((0, 0, 545), (6, W, 130), TRIM)
    for y in range(-1200, 1201, 400):
        strip(s, (620, y), (1150, y), 700 - 1.0, 14, STRIP)
        strip(s, (20, y), (580, y), 600 - 1.0, 14, STRIP)
        strip(s, (-HX + 40, y), (-20, y), 450 - 1.0, 14, STRIP)
    # structural ribs across the ceiling and conduits along its edges
    for x0, x1, z in ((600, VP_X + VP_BULGE, 700), (0, 600, 600), (-HX, 0, 450)):
        for x in range(int(x0) + 100, int(x1), 200):
            s.box((x, 0, z - 22), (26, W - 20, 44), TRIM)
        for sgn in (-1, 1):
            for k, dz in enumerate((-18, -42)):
                s.cylinder(((x0 + x1) / 2, sgn * (HY - 50 - 26 * k), z + dz), 9, x1 - x0 - 20, TRIM,
                           axis="X", segments=10)
    # flush light rings in the ceiling over the holotable (mirrors the floor inlay)
    ring_strip(s, hx, hy, 380, 450 - 1.5, 64, 10, STRIP)
    ring_strip(s, hx, hy, 260, 450 - 1.5, 48, 4, INLAY)

    # walls: aft (with lift doors and ready-room door), port (with the Loom alcove), starboard (holo pads)
    walls = []
    walls += s.box((-HX - 20, 0, 260), (40, W + 80, 520), CERAMIC)
    for y, w in ((-650, 260), (650, 260), (1150, 140)):   # lift doors (stbd/port) and ready room (port aft)
        s.box((-HX + 4, y, 160), (10, w + 40, 340), TRIM)
        s.box((-HX + 10, y, 155), (4, w, 310), CERAMIC)
        s.box((-HX + 12, y, 155), (2, 6, 310), STRIP)
        strip(s, (-HX + 12, y - w / 2 - 14), (-HX + 12, y + w / 2 + 14), 330, 4, STRIP)
    # starboard wall (Y = -HY): full length
    walls += s.box((0, -HY - 20, 380), (D, 40, 760), CERAMIC)
    # port wall (Y = +HY) with an alcove opening from X -300 to 200
    walls += s.box(((-HX - 300) / 2, HY + 20, 380), (HX - 300, 40, 760), CERAMIC)
    walls += s.box(((200 + VP_X + VP_BULGE) / 2, HY + 20, 380), (VP_X + VP_BULGE - 200, 40, 760), CERAMIC)
    s.box((-50, HY + 20, 680), (500, 40, 200), CERAMIC)       # lintel over the alcove
    # the alcove: a curved recess 3 m deep
    for k in range(9):
        a = math.radians(-80 + 20 * k)
        cx, cy = -50 + 230 * math.sin(a), HY + 40 + 230 * math.cos(a) * 0.9
        s.box(None, (90, 20, 580), CERAMIC, matrix=at(cx, cy + 40, 290, math.degrees(-a)))
    s.box((-50, HY + 330, 320), (600, 40, 680), CERAMIC)       # backing wall: no gaps to space behind the panels
    slab(s, -320, 220, HY, HY + 320, 0)
    slab(s, -320, 220, HY, HY + 320, 600, 40)
    s.box((-50, HY + 2, 1), (500, 4, 2), STRIP)
    # forward wall pieces beside the viewport
    for sgn in (-1, 1):
        y0 = sgn * VP_HALF
        walls += s.box((VP_X + 10, sgn * (VP_HALF + HY) / 2, 380), (60, HY - VP_HALF, 760), CERAMIC)
    # wall panelling and pilasters
    for x in range(-1000, 1101, 360):
        for sgn in (-1, 1):
            if sgn > 0 and -340 < x < 240:
                continue
            s.box((x, sgn * (HY - 6), 300), (40, 12, 600), TRIM)
            s.box((x, sgn * (HY - 13), 300), (6, 2, 560), STRIP)
    s.panel_grid([f for f in walls if f.is_valid and abs(f.normal.z) < 0.2], cell=(120.0, 90.0),
                 height=(1.0, 3.0), gap=4.0, keep=0.85, split=0.25, slots=(CERAMIC, CERAMIC, CERAMIC, TRIM))
    # a dado of dark vent grilles along the foot of the side walls
    for sgn in (-1, 1):
        for x in range(int(-HX) + 80, int(VP_X), 160):
            if sgn > 0 and -340 < x < 240:
                continue
            z0 = 0 if x < 200 else WALK
            s.box((x, sgn * (HY - 4), z0 + 22), (120, 8, 30), TRIM)
            for k in range(4):
                s.box((x, sgn * (HY - 9), z0 + 12 + 6 * k), (110, 2, 2), CERAMIC)
    return s


# ---- viewport ----------------------------------------------------------------------------------

def make_viewport():
    s = Ship("SM_Bridge_Viewport", 502)
    pts = viewport_points(6)          # 7 mullions, ~3.6 m apart along the arc
    z0, z1 = 40.0, 680.0
    for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
        length = math.hypot(x1 - x0, y1 - y0)
        yaw = math.degrees(math.atan2(y1 - y0, x1 - x0))
        mx, my = (x0 + x1) / 2, (y0 + y1) / 2
        s.box(None, (length, 3, z1 - z0), GLASS, matrix=at(mx, my, (z0 + z1) / 2, yaw))
        s.box(None, (length, 40, 60), TRIM, matrix=at(mx, my, z0 - 30, yaw))       # sill
        s.box(None, (length, 40, 50), TRIM, matrix=at(mx, my, z1 + 25, yaw))       # head
        s.box(None, (length, 6, 6), CRYSTAL, matrix=at(mx - 0, my, z0 + 4, yaw))  # crystal edge
    for x, y in pts:
        nyaw = math.degrees(math.atan2(-2 * VP_BULGE * (y / VP_HALF) / VP_HALF, 1))  # arc normal
        s.box(None, (40, 24, z1 - z0 + 100), TRIM, matrix=at(x, y, (z0 + z1) / 2, nyaw))
        s.crystal((x - 10, y, z1 + 40), (x - 10, y, z0 - 40), 9, CRYSTAL)
    # sill console ledge under the glass (pit side)
    for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
        length = math.hypot(x1 - x0, y1 - y0)
        yaw = math.degrees(math.atan2(y1 - y0, x1 - x0))
        s.box(None, (length, 70, 20), CERAMIC, matrix=at((x0 + x1) / 2 - 40, (y0 + y1) / 2, 0, yaw))
    return s


# ---- props -------------------------------------------------------------------------------------

def make_chair():
    s = Ship("SM_CaptainsChair", 510)
    s.cylinder((0, 0, 8), 45, 16, TRIM, axis="Z", segments=32)
    s.cylinder((0, 0, 30), 14, 40, TRIM, axis="Z", segments=16)
    s.box((5, 0, 55), (60, 62, 14), CERAMIC)                      # seat
    s.box(None, (14, 64, 110), CERAMIC, matrix=at(-28, 0, 115, pitch=-10))    # back
    s.box(None, (10, 40, 30), CERAMIC, matrix=at(-38, 0, 175, pitch=-10))     # headrest
    s.box(None, (2, 6, 100), CRYSTAL, matrix=at(-20, 0, 115, pitch=-10))
    for sgn in (-1, 1):
        s.box((10, sgn * 38, 75), (60, 12, 8), CERAMIC)          # armrest
        s.box((0, sgn * 38, 63), (40, 10, 18), TRIM)
        screen(s, at(28, sgn * 38, 82, pitch=-60), 22, 14)
        s.box((10, sgn * 32, 70), (50, 2, 2), CRYSTAL)
    return s


def make_holotable():
    s = Ship("SM_Holotable", 511)
    s.cylinder((0, 0, 10), 200, 20, TRIM, axis="Z", segments=64)
    s.cylinder((0, 0, 60), 210, 80, TRIM, axis="Z", segments=64, radius2=225)
    s.cylinder((0, 0, 101), 225, 2, INLAY, axis="Z", segments=64, radius2=226)
    s.cylinder((0, 0, 105), 218, 6, CRYSTAL, axis="Z", segments=64)
    s.cylinder((0, 0, 109), 190, 2, STRIP, axis="Z", segments=64)   # projection plate
    for k in range(16):
        a = 2 * math.pi * k / 16
        s.box(None, (30, 8, 70), TRIM, matrix=at(222 * math.cos(a), 222 * math.sin(a), 55, math.degrees(a)))
        s.box(None, (2, 3, 50), INLAY, matrix=at(229 * math.cos(a), 229 * math.sin(a), 55, math.degrees(a)))
    return s


def standing_console(name, seed, width, wall=True):
    s = Ship(name, seed)
    s.box((0, 0, 45), (50, width, 90), TRIM)
    s.box(None, (60, width + 10, 8), CERAMIC, matrix=at(10, 0, 100, pitch=-25))
    for k in range(3):
        y = (k - 1) * width / 3
        screen(s, at(14, y, 106, pitch=-65), width / 3 - 12, 30)
    s.box((24, 0, 90), (4, width, 3), INLAY)
    indicator_strip(s, at(26, 0, 70), width - 30, 10)
    if wall:
        s.box((-40, 0, 170), (10, width + 40, 200), CERAMIC)
        for k in range(4):
            screen(s, at(-34, (k - 1.5) * (width + 20) / 4, 200), (width + 20) / 4 - 14, 80)
    return s


def make_tactical():
    s = standing_console("SM_Console_Tactical", 520, 180)
    s.box((-30, 70, 230), (2, 8, 10), CRYSTAL)          # Coalition unit badge clipped to the wall
    return s


def make_pit():
    s = Ship("SM_Console_Pit", 521)
    s.box((0, 0, 35), (70, 160, 70), TRIM)
    s.box(None, (70, 165, 6), CERAMIC, matrix=at(8, 0, 74, pitch=-20))
    for k in (-1, 1):
        screen(s, at(14, k * 40, 82, pitch=-70), 66, 30)
    indicator_strip(s, at(36, 0, 55), 140, 8)
    s.box((-50, 0, 30), (40, 50, 8), CERAMIC)            # rating's stool
    s.cylinder((-50, 0, 13), 6, 26, TRIM, axis="Z")
    return s


def make_vex():
    s = Ship("SM_VexColumn", 522)
    s.cylinder((0, 0, 6), 60, 12, TRIM, axis="Z", segments=48)
    s.cylinder((0, 0, 14), 44, 4, INLAY, axis="Z", segments=48)
    s.cylinder((0, 0, 180), 22, 330, INLAY, axis="Z", segments=32)
    for z in (60, 180, 300):
        s.cylinder((0, 0, z), 30, 4, CRYSTAL, axis="Z", segments=32)
    s.cylinder((0, 0, 350), 40, 10, TRIM, axis="Z", segments=48)
    return s


def make_cradle():
    s = Ship("SM_NeuralCradle", 523)
    s.cylinder((0, 0, 6), 80, 12, TRIM, axis="Z", segments=40)
    s.box((0, 0, 30), (40, 30, 40), TRIM)
    s.box(None, (110, 60, 12), CERAMIC, matrix=at(5, 0, 60, pitch=-15))       # reclined seat
    s.box(None, (12, 60, 80), CERAMIC, matrix=at(-55, 0, 95, pitch=-55))      # back, steeply reclined
    s.box(None, (10, 36, 22), CERAMIC, matrix=at(-80, 0, 130, pitch=-55))
    screen(s, at(55, 0, 120, pitch=-40), 50, 30)
    # cage of crystal rods
    for k in range(10):
        a = 2 * math.pi * k / 10
        x, y = 75 * math.cos(a), 75 * math.sin(a)
        s.crystal((x * 0.55, y * 0.55, 205), (x, y, 10), 4, CRYSTAL)
    s.cylinder((0, 0, 205), 45, 6, TRIM, axis="Z", segments=24)
    # cables to the deck
    for k in range(5):
        m = at(-40 - 10 * k, -20 + 10 * k, 3, yaw=200 + 8 * k) @ Matrix.Rotation(math.radians(90), 4, "Y")
        s._cone(m @ Matrix.Translation((0, 0, 60)), 2.5, 2.5, 120, 8, TRIM)
    s.box((60, 50, 62), (14, 8, 3), INLAY)              # Nix's modded handheld, stickered
    return s


def make_engineering():
    s = standing_console("SM_Console_Engineering", 524, 200)
    for k in range(3):                                   # drive / chrono / instability gauges
        y = (k - 1) * 60
        s.cylinder((-33, y, 290), 22, 4, TRIM, axis="X", segments=24)
        s.cylinder((-31, y, 290), 18, 1, STRIP if k < 2 else INLAY, axis="X", segments=24)
    return s


def make_medical():
    return standing_console("SM_Console_Medical", 525, 140)


def make_loom():
    s = Ship("SM_LoomFrame", 526)
    # curved frame (arc in plan, concave toward +X), with light threads strung across it
    n = 10
    pts = [(-40 * math.cos(math.radians(-60 + 12 * k)), 130 * math.sin(math.radians(-60 + 12 * k))) for k in range(n + 1)]
    for (x0, y0), (x1, y1) in zip(pts, pts[1:]):
        length = math.hypot(x1 - x0, y1 - y0)
        yaw = math.degrees(math.atan2(y1 - y0, x1 - x0))
        s.box(None, (length, 8, 8), CRYSTAL, matrix=at((x0 + x1) / 2, (y0 + y1) / 2, 15, yaw))
        s.box(None, (length, 8, 8), CRYSTAL, matrix=at((x0 + x1) / 2, (y0 + y1) / 2, 255, yaw))
    for x, y in (pts[0], pts[-1]):
        s.box((x, y, 135), (10, 10, 250), CRYSTAL)
    rng = s.rng
    for _ in range(26):
        i, j = rng.randrange(n + 1), rng.randrange(n + 1)
        (xa, ya), (xb, yb) = pts[i], pts[j]
        za, zb = 15, 255
        p0, p1 = Vector((xa, ya, za)), Vector((xb, yb, zb))
        d = p1 - p0
        frame = d.normalized().to_track_quat("Z", "Y").to_matrix().to_4x4()
        s._cone(Matrix.Translation((p0 + p1) / 2) @ frame, 0.6, 0.6, d.length, 6, INLAY)
    # a low cushioned bench: the only soft fabric on the bridge
    s.box((60, 0, 22), (50, 120, 44), CERAMIC)
    return s


def make_holopad():
    s = Ship("SM_HoloPad", 527)
    s.cylinder((0, 0, 8), 90, 16, TRIM, axis="Z", segments=48)
    s.cylinder((0, 0, 17), 82, 2, INLAY, axis="Z", segments=48)
    for k in range(12):
        a = 2 * math.pi * k / 12
        s.box(None, (10, 4, 4), STRIP, matrix=at(86 * math.cos(a), 86 * math.sin(a), 16, math.degrees(a)))
    return s


def make_wall_bank():
    """Wall-mounted computer bank (280 wide, 420 tall, 50 deep) between the wall pilasters: a stack of readouts,
    a status-light grid, data drums, vents and conduits. Its back is against the wall at -X."""
    s = Ship("SM_Bridge_WallBank", 530)
    s.box((-30, 0, 210), (40, 280, 420), TRIM)                       # cabinet against the wall
    s.box((-6, 0, 20), (12, 270, 40), TRIM)                          # kick plinth
    s.box((-4, 0, 410), (16, 280, 20), CERAMIC)                      # cornice
    for z in (330, 262):
        for k in range(3):
            screen(s, at(-8, (k - 1) * 88, z), 76, 52)
    screen(s, at(-8, 0, 182), 250, 70, mode=s.rng.choice((RADAR, SCHEMATIC, WAVE)))
    s.box((-8, 0, 128), (4, 254, 34), TRIM)
    display(s, at(-5.5, 0, 128), 244, 26, LIGHTS)
    for k in range(4):                                               # data drums
        y = (k - 1.5) * 62
        s.cylinder((-8, y, 78), 20, 6, TRIM, axis="X", segments=20)
        s.cylinder((-5, y, 78), 14, 2, CERAMIC, axis="X", segments=20)
    for k in range(5):                                               # vent slots
        s.box((-8, 0, 40 + 6 * k), (3, 240, 2), CERAMIC)
    for y in (132, -132):                                            # conduits
        s.cylinder((-14, y, 210), 6, 400, CERAMIC, axis="Z", segments=10)
    return s


def make_overhead_monitor():
    """Ceiling-hung monitor cluster: a strut 160 down from its origin at the ceiling, three angled screens
    facing aft (-X) and three facing forward."""
    s = Ship("SM_Bridge_OverheadMonitor", 531)
    s.box((0, 0, -15), (60, 60, 30), TRIM)                           # ceiling mount
    s.cylinder((0, 0, -95), 7, 160, TRIM, axis="Z", segments=12)
    s.box((0, 0, -175), (40, 260, 14), TRIM)                         # crossbar
    for side, yaw in ((-1, 180), (1, 0)):
        for k in range(3):
            y = (k - 1) * 82
            tilt = 18 if k == 1 else 12
            screen(s, at(side * 14, y, -225, yaw=yaw + (k - 1) * -12, pitch=tilt), 74, 52)
    return s


def make_pit_bank():
    """Long crew-pit console (320 wide) with three slanted readouts, a status strip, a rear riser of small
    readouts and two stools: the rows of ratings in the sunken pit."""
    s = Ship("SM_Console_PitBank", 532)
    s.box((0, 0, 38), (70, 320, 76), TRIM)
    s.box(None, (76, 326, 6), CERAMIC, matrix=at(6, 0, 80, pitch=-18))
    for k in range(3):
        screen(s, at(16, (k - 1) * 102, 88, pitch=-68), 88, 32)
    indicator_strip(s, at(36, 0, 58), 290, 10)
    s.box((-8, 0, 118), (24, 320, 50), TRIM)                         # rear riser with a readout row
    for k in range(4):
        screen(s, at(4, (k - 1.5) * 78, 120, pitch=-10), 66, 30)
    for y in (-80, 80):                                              # the ratings sit facing the screens (+X)
        s.box((80, y, 30), (40, 46, 8), CERAMIC)
        s.cylinder((80, y, 13), 6, 26, TRIM, axis="Z")
    return s


BUILDERS = [make_shell, make_viewport, make_chair, make_holotable, make_tactical, make_pit, make_vex, make_cradle,
            make_engineering, make_medical, make_loom, make_holopad, make_wall_bank, make_overhead_monitor,
            make_pit_bank]

if __name__ == "__main__":
    for build in BUILDERS:
        name = build.__name__
        reset()
        s = build()
        if ONLY and s.name not in ONLY:
            continue
        apply_display_uvs(s)
        s.export(OUT, bevel=1.0)
