"""Celestial-body kit: a smooth sphere for planets and stars, flat ring bands, and the Tithe marker buoy.

Sphere: diameter 100 uu (like /Engine/BasicShapes/Sphere), so actor scale = diameter / 100.
Rings: outer radius 50 uu like the sphere, so a ring placed with the planet's scale x (outer ratio) fits;
double-sided so they read from above and below. Wide = inner 55% of outer (Verge 1.4-2.6, Grieve 1.6-2.8),
Mid = 80% (Glass Sea 1.5-1.9), Thin = 97% (the Tether's accretion line 1.6-1.65).
Run: blender --background --factory-startup --python make_planet_sphere.py -- <out_dir> [MeshName ...]
"""
import math
import os
import sys

import bmesh
import bpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
OUT = os.path.abspath(args[0] if args else ".")
ONLY = set(args[1:])


def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def export(obj, material="M_Planet"):
    mat = bpy.data.materials.new(material)
    obj.data.materials.append(mat)
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.fbx(filepath=os.path.join(OUT, obj.name + ".fbx"), use_selection=True,
                             apply_scale_options="FBX_SCALE_UNITS", mesh_smooth_type="FACE", add_leaf_bones=False, bake_anim=False)
    print("[planet] exported", obj.name, tuple(round(d, 2) for d in obj.dimensions))


def sphere():
    bpy.ops.mesh.primitive_uv_sphere_add(segments=192, ring_count=96, radius=50.0)
    obj = bpy.context.active_object
    obj.name = "SM_PlanetSphere"
    bpy.ops.object.shade_smooth()
    export(obj)


def ring(name, inner_ratio, segments=256):
    mesh = bpy.data.meshes.new(name)
    bm = bmesh.new()
    outer, inner = 50.0, 50.0 * inner_ratio
    def loop(r):
        return [bm.verts.new((r * math.cos(2 * math.pi * i / segments), r * math.sin(2 * math.pi * i / segments), 0)) for i in range(segments)]
    # two separate sheets, so the band reads from both sides without a two-sided material
    for flip in (False, True):
        vo, vi = loop(outer), loop(inner)
        for i in range(segments):
            j = (i + 1) % segments
            quad = (vi[i], vo[i], vo[j], vi[j])
            bm.faces.new(tuple(reversed(quad)) if flip else quad)
    bm.to_mesh(mesh)
    bm.free()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    export(obj, "M_Ring")


def marker_buoy():
    """Tithe Anchorage marker buoy: a gunmetal spar 160 uu tall with fins and a red strobe at the top."""
    from aetheria_hd import DARK, HULL, LIGHTS, Ship
    from mathutils import Matrix
    s = Ship("SM_TitheMarkerBuoy", seed=7)
    s.cylinder((0, 0, 0), 6, 160, HULL, axis="Z", segments=16)
    s.cylinder((0, 0, -88), 10, 16, DARK, axis="Z", segments=16, radius2=6)
    for a in range(4):
        yaw = 90 * a + 45
        s.box(None, (40, 2, 50), HULL, matrix=Matrix.Translation((0, 0, -50)) @ Matrix.Rotation(math.radians(yaw), 4, "Z")
              @ Matrix.Translation((20, 0, 0)))
    for z in (-20, 30):
        s.cylinder((0, 0, z), 9, 4, DARK, axis="Z", segments=16)
    s.cylinder((0, 0, 86), 5, 10, LIGHTS, axis="Z", segments=16)   # the strobe
    s.export(OUT, bevel=0.3)


BUILD = {
    "SM_PlanetSphere": sphere,
    "SM_Ring_Wide": lambda: ring("SM_Ring_Wide", 0.55),
    "SM_Ring_Mid": lambda: ring("SM_Ring_Mid", 0.80),
    "SM_Ring_Thin": lambda: ring("SM_Ring_Thin", 0.97),
    "SM_TitheMarkerBuoy": marker_buoy,
}

if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    for name, build in BUILD.items():
        if ONLY and name not in ONLY:
            continue
        reset()
        build()
