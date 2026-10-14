"""Aetheria environment set-piece generator for Acts VI and VII.

Builds faceted, Nanite-ready placeholder set pieces and exports one FBX each.
Run:  blender --background --python make_environments.py -- <out_dir>
  or: python make_environments.py <out_dir>          (with the `bpy` pip module)
Scale: 1 Blender unit = 1 m; import into Unreal with Import Uniform Scale 0.01 like the ships.
Random layouts use fixed seeds, so every run produces the same meshes.
"""
import math
import os
import random
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from aetheria_mesh import finish_and_export as _export, mat, part, reset, spike  # noqa: E402

args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else sys.argv[1:]
OUT = os.path.abspath(args[0] if args else "Environments")
os.makedirs(OUT, exist_ok=True)


def export(name, parts, bevel=1.0):
    _export(name, parts, OUT, bevel)


def crystal(name, loc, length, radius, rot, material, verts=6):
    """Double-ended crystal: two cones base to base along local Z."""
    up = part("primitive_cone_add", f"{name}Up", loc, (radius, radius, length * 0.6), rot, material,
              vertices=verts, radius1=1, radius2=0, depth=2)
    rx, ry, rz = (math.radians(r) for r in rot)
    # Push the lower cone along the crystal's own -Z so the two halves meet at their bases.
    dx = -math.sin(ry) * length * 0.35
    dy = math.sin(rx) * math.cos(ry) * length * 0.35
    dz = -math.cos(rx) * math.cos(ry) * length * 0.35
    down = part("primitive_cone_add", f"{name}Down", (loc[0] + dx, loc[1] + dy, loc[2] + dz),
                (radius, radius, length * 0.4), (rot[0] + 180, rot[1], rot[2]), material,
                vertices=verts, radius1=1, radius2=0, depth=2)
    return [up, down]


# ---- Act VI: The Far Shore ---------------------------------------------------------------

def omega_arch():
    """The Omega Arch seen from the far side: a 3 km ring gate held by crystal pylons."""
    reset()
    stone = mat("M_ArchStone", (0.18, 0.20, 0.26), 0.3)
    crystal_mat = mat("M_PrecursorCrystal", (0.10, 0.35, 0.60), 1.5)
    horizon = mat("M_ArchHorizon", (0.65, 0.85, 1.00), 12.0)
    parts = [
        part("primitive_torus_add", "Ring", (0, 0, 0), (1500, 1500, 1500), (0, 90, 0), stone,
             major_radius=1, minor_radius=0.06, major_segments=48, minor_segments=6),
        part("primitive_torus_add", "EventHorizonEdge", (0, 0, 0), (1420, 1420, 1420), (0, 90, 0), horizon,
             major_radius=1, minor_radius=0.008, major_segments=48, minor_segments=4),
    ]
    for i in range(8):
        a = math.radians(i * 45)
        y, z = 1500 * math.cos(a), 1500 * math.sin(a)
        parts.append(part("primitive_cube_add", f"Clamp{i}", (0, y, z), (160, 70, 70),
                          (math.degrees(a), 0, 0), stone, size=2))
        parts += crystal(f"Pylon{i}", (0, y * 1.12, z * 1.12), 360, 45, (math.degrees(a) - 90, 0, 0), crystal_mat)
    export("SM_OmegaArch", parts, bevel=4.0)


def ember_relay():
    """Dormant Precursor relay on the Zero Beacon carrier: a spire with three arms and a halo."""
    reset()
    stone = mat("M_ArchStone", (0.18, 0.20, 0.26), 0.3)
    ember = mat("M_EmberGlow", (1.00, 0.45, 0.15), 9.0)
    crystal_mat = mat("M_PrecursorCrystal", (0.10, 0.35, 0.60), 1.5)
    parts = crystal("Spire", (0, 0, 0), 600, 45, (0, 0, 0), crystal_mat, verts=8)
    parts.append(part("primitive_torus_add", "Halo", (0, 0, 120), (200, 200, 200), material=ember,
                      major_radius=1, minor_radius=0.05, major_segments=24, minor_segments=4))
    for i in range(3):
        a = math.radians(i * 120)
        parts.append(spike(f"Arm{i}", (140 * math.cos(a), 140 * math.sin(a), -40), 18, 130, stone, verts=4))
        parts[-1].rotation_euler = (0, math.radians(90), a)
        parts.append(part("primitive_ico_sphere_add", f"Node{i}", (280 * math.cos(a), 280 * math.sin(a), -40),
                          (24, 24, 24), material=ember, subdivisions=1, radius=1))
    export("SM_EmberRelay", parts, bevel=2.0)


def lantern_cluster():
    """Bioluminescent crystal cluster that gives the Lantern Drift its name."""
    reset()
    lantern = mat("M_LanternGlow", (0.25, 1.00, 0.75), 10.0)
    shell = mat("M_LanternShell", (0.10, 0.30, 0.28), 0.8)
    rng = random.Random(7)
    parts = [part("primitive_ico_sphere_add", "Core", (0, 0, 0), (40, 34, 30), material=shell,
                  subdivisions=1, radius=1)]
    for i in range(9):
        rot = (rng.uniform(-60, 60), rng.uniform(-60, 60), rng.uniform(0, 360))
        parts += crystal(f"Lantern{i}", (rng.uniform(-30, 30), rng.uniform(-30, 30), rng.uniform(-20, 30)),
                         rng.uniform(70, 150), rng.uniform(8, 16), rot, lantern if i % 3 else shell)
    export("SM_LanternCluster", parts, bevel=0.8)


def arch_debris(name, seed, count):
    """Shattered hull plating from the Arch transit, scattered through the Arch Wake."""
    reset()
    hull = mat("M_DebrisHull", (0.22, 0.22, 0.25), 0.1)
    burn = mat("M_DebrisBurn", (1.00, 0.40, 0.10), 3.0)
    rng = random.Random(seed)
    parts = []
    for i in range(count):
        size = (rng.uniform(15, 60), rng.uniform(4, 20), rng.uniform(2, 8))
        parts.append(part("primitive_cube_add", f"Plate{i}",
                          (rng.uniform(-80, 80), rng.uniform(-80, 80), rng.uniform(-40, 40)),
                          size, (rng.uniform(0, 360), rng.uniform(0, 360), rng.uniform(0, 360)),
                          burn if i == 0 else hull, size=2))
    export(name, parts, bevel=0.6)


# ---- Act VII: The Cinder Archive ---------------------------------------------------------

def severant_bastion():
    """Pyre Gate fortress core. Battle E2: its shield holds until all three pylons fall."""
    reset()
    ash = mat("M_SeverantAsh", (0.55, 0.55, 0.60), 0.2)
    silence = mat("M_SilenceGlow", (0.30, 0.50, 1.00), 7.0)
    parts = [
        part("primitive_cylinder_add", "Keep", (0, 0, 0), (220, 220, 90), material=ash,
             vertices=8, radius=1, depth=2),
        part("primitive_cone_add", "Crown", (0, 0, 170), (150, 150, 80), material=ash,
             vertices=8, radius1=1, radius2=0.2, depth=2),
        part("primitive_ico_sphere_add", "CoreLight", (0, 0, 260), (45, 45, 45), material=silence,
             subdivisions=1, radius=1),
        part("primitive_cylinder_add", "Underkeep", (0, 0, -130), (150, 150, 40), material=ash,
             vertices=8, radius=1, depth=2),
    ]
    for i in range(8):
        a = math.radians(i * 45 + 22.5)
        parts.append(part("primitive_cone_add", f"Spike{i}", (250 * math.cos(a), 250 * math.sin(a), 30),
                          (18, 18, 70), (0, 0, 0), ash, vertices=4, radius1=1, radius2=0, depth=2))
    export("SM_SeverantBastion", parts, bevel=2.0)


def bastion_pylon():
    """Shield pylon of the Pyre Gate bastion; three surround the core."""
    reset()
    ash = mat("M_SeverantAsh", (0.55, 0.55, 0.60), 0.2)
    silence = mat("M_SilenceGlow", (0.30, 0.50, 1.00), 7.0)
    parts = [
        part("primitive_cylinder_add", "Base", (0, 0, 0), (40, 40, 12), material=ash,
             vertices=6, radius=1, depth=2),
        part("primitive_cone_add", "Obelisk", (0, 0, 70), (22, 22, 60), material=ash,
             vertices=4, radius1=1, radius2=0.25, depth=2),
        part("primitive_torus_add", "Crown", (0, 0, 135), (24, 24, 24), material=silence,
             major_radius=1, minor_radius=0.12, major_segments=12, minor_segments=4),
        part("primitive_ico_sphere_add", "Focus", (0, 0, 135), (10, 10, 10), material=silence,
             subdivisions=1, radius=1),
    ]
    export("SM_BastionPylon", parts, bevel=0.6)


def vault_of_records():
    """The Cinder Archive's core: stepped glass slabs around the record well."""
    reset()
    ash = mat("M_SeverantAsh", (0.55, 0.55, 0.60), 0.2)
    glass = mat("M_AshGlass", (0.70, 0.75, 0.85), 1.5)
    record = mat("M_RecordLight", (1.00, 0.90, 0.60), 9.0)
    parts = []
    for tier, (half, height) in enumerate(((260, 40), (200, 40), (140, 40), (90, 40))):
        z = tier * 80
        parts.append(part("primitive_cube_add", f"Tier{tier}", (0, 0, z), (half, half, height / 2 + 10),
                          (0, 0, tier * 12), ash if tier % 2 == 0 else glass, size=2))
    parts.append(part("primitive_cylinder_add", "RecordWell", (0, 0, 330), (30, 30, 70), material=record,
                      vertices=8, radius=1, depth=2))
    parts.append(part("primitive_torus_add", "WellHalo", (0, 0, 420), (110, 110, 110), material=record,
                      major_radius=1, minor_radius=0.05, major_segments=20, minor_segments=4))
    for i in range(4):
        a = math.radians(i * 90 + 45)
        parts += crystal(f"Ward{i}", (320 * math.cos(a), 320 * math.sin(a), 60), 200, 20, (0, 0, 0), glass, verts=4)
    export("SM_VaultOfRecords", parts, bevel=1.5)


def glass_shard_field():
    """Vitrified shards thrown up from the Glass Sea, drifting in orbit."""
    reset()
    glass = mat("M_AshGlass", (0.70, 0.75, 0.85), 1.5)
    flare = mat("M_GlassFlare", (0.60, 0.85, 1.00), 6.0)
    rng = random.Random(11)
    parts = []
    for i in range(14):
        rot = (rng.uniform(0, 360), rng.uniform(0, 360), rng.uniform(0, 360))
        parts += crystal(f"Shard{i}", (rng.uniform(-150, 150), rng.uniform(-150, 150), rng.uniform(-60, 60)),
                         rng.uniform(40, 120), rng.uniform(6, 18), rot, flare if i % 5 == 0 else glass, verts=4)
    export("SM_GlassShardField", parts, bevel=0.5)


if __name__ == "__main__":
    # Act VI: The Far Shore.
    omega_arch()
    ember_relay()
    lantern_cluster()
    arch_debris("SM_ArchDebris_A", seed=3, count=6)
    arch_debris("SM_ArchDebris_B", seed=5, count=9)
    # Act VII: The Cinder Archive.
    severant_bastion()
    bastion_pylon()
    vault_of_records()
    glass_shard_field()
