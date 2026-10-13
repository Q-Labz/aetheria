"""Aetheria placeholder fleet generator.

Builds faceted, Nanite-ready hero placeholders and exports one FBX per ship.
Run:  blender --background --python make_fleet.py -- <out_dir>
  or: python make_fleet.py <out_dir>          (with the `bpy` pip module)
Scale: 1 Blender unit = 1 m. Import into Unreal with Import Uniform Scale 0.01
(tactical scale: the 1,020 m Crystal Dreadnought becomes 1,020 uu long). Forward = +X.
"""
import math
import os
import sys

import bpy

args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else sys.argv[1:]
OUT = os.path.abspath(args[0] if args else "Meshes")
os.makedirs(OUT, exist_ok=True)


def reset():
    bpy.ops.wm.read_factory_settings(use_empty=True)


def mat(name, rgb, strength):
    m = bpy.data.materials.get(name) or bpy.data.materials.new(name)
    m.use_nodes = True
    bsdf = m.node_tree.nodes["Principled BSDF"]
    bsdf.inputs["Base Color"].default_value = (*rgb, 1)
    bsdf.inputs["Emission Color"].default_value = (*rgb, 1)
    bsdf.inputs["Emission Strength"].default_value = strength
    bsdf.inputs["Roughness"].default_value = 0.15
    return m


def part(op, name, loc, scale, rot=(0, 0, 0), material=None, **kw):
    getattr(bpy.ops.mesh, op)(location=loc, rotation=[math.radians(r) for r in rot], **kw)
    o = bpy.context.active_object
    o.name, o.scale = name, scale
    o.data.materials.append(material)
    return o


def spike(name, loc, radius_yz, half_len, material, verts=6, z_squash=1.0):
    """Cone lying along +X (tip forward)."""
    return part("primitive_cone_add", name, loc, (radius_yz * z_squash, radius_yz, half_len),
                (0, 90, 0), material, vertices=verts, radius1=1, radius2=0, depth=2)


def finish_and_export(name, parts, bevel=1.0):
    bpy.ops.object.select_all(action="DESELECT")
    for p in parts:
        p.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
    o = bpy.context.active_object
    o.name = o.data.name = name
    b = o.modifiers.new("Bevel", "BEVEL")
    b.width, b.segments, b.limit_method = bevel, 1, "ANGLE"
    bpy.ops.object.shade_flat()
    path = os.path.join(OUT, f"{name}.fbx")
    bpy.ops.export_scene.fbx(filepath=path, use_selection=True,
                             apply_scale_options="FBX_SCALE_UNITS", use_mesh_modifiers=True,
                             mesh_smooth_type="FACE", add_leaf_bones=False, bake_anim=False)
    dims = tuple(round(d, 1) for d in o.dimensions)
    print(f"[aetheria] {name}: {len(o.data.polygons)} faces, dims(m)={dims} -> {path}")


def crystal_dreadnought():
    reset()
    hull = mat("M_PrecursorCrystal", (0.10, 0.35, 0.60), 0.5)
    core = mat("M_PrismaticCore", (0.80, 0.95, 1.00), 8.0)
    drive = mat("M_StarDrive", (0.55, 0.30, 1.00), 4.0)
    parts = [
        spike("Keel", (0, 0, 0), 48, 510, hull, verts=8, z_squash=0.8),        # 1,020 m keel
        spike("Crest", (-120, 0, 36), 26, 300, hull, verts=4, z_squash=0.35),
        spike("Ventral", (-60, 0, -30), 20, 260, hull, verts=4, z_squash=0.35),
        spike("PrismL", (-220, -70, 0), 18, 170, hull),
        spike("PrismR", (-220, 70, 0), 18, 170, hull),
        spike("ScoutBayL", (40, -40, -6), 10, 90, hull, verts=4),
        spike("ScoutBayR", (40, 40, -6), 10, 90, hull, verts=4),
        part("primitive_torus_add", "StarDriveRing", (-470, 0, 0), (72, 72, 72), (0, 90, 0), drive,
             major_radius=1, minor_radius=0.12, major_segments=12, minor_segments=6),
        part("primitive_ico_sphere_add", "ChronoCore", (-470, 0, 0), (26, 26, 26), material=drive,
             subdivisions=1, radius=1),
        part("primitive_ico_sphere_add", "BeamLens", (400, 0, 0), (20, 20, 20), material=core,
             subdivisions=1, radius=1),
    ]
    finish_and_export("SM_CrystalDreadnought", parts, bevel=1.5)


def phantom_cruiser():
    reset()
    hull = mat("M_PhantomCrystal", (0.20, 0.55, 0.70), 1.0)
    core = mat("M_PrismaticCore", (0.80, 0.95, 1.00), 6.0)
    parts = [
        spike("Hull", (0, 0, 0), 14, 120, hull, verts=6, z_squash=0.6),
        spike("FinL", (-40, -20, 0), 6, 60, hull, verts=4, z_squash=0.3),
        spike("FinR", (-40, 20, 0), 6, 60, hull, verts=4, z_squash=0.3),
        part("primitive_ico_sphere_add", "Core", (-20, 0, 6), (6, 6, 6), material=core,
             subdivisions=1, radius=1),
    ]
    finish_and_export("SM_PhantomCruiser", parts, bevel=0.4)


def tithe_interceptor():
    reset()
    hull = mat("M_TitheHull", (0.25, 0.04, 0.02), 0.2)
    glow = mat("M_TitheEngine", (1.00, 0.25, 0.05), 6.0)
    parts = [
        part("primitive_cube_add", "Fuselage", (0, 0, 0), (45, 8, 5), material=hull, size=2),
        part("primitive_cube_add", "Wings", (-15, 0, 0), (12, 34, 1.5), (0, 0, 0), hull, size=2),
        spike("Nose", (55, 0, 0), 8, 12, hull, verts=4),
        part("primitive_cylinder_add", "Engine", (-48, 0, 0), (5, 5, 4), (0, 90, 0), glow,
             vertices=8, radius=1, depth=2),
    ]
    finish_and_export("SM_TitheInterceptor", parts, bevel=0.3)


def tithe_cruiser():
    reset()
    hull = mat("M_TitheHull", (0.25, 0.04, 0.02), 0.2)
    glow = mat("M_TitheEngine", (1.00, 0.25, 0.05), 6.0)
    parts = [
        part("primitive_cube_add", "Spine", (0, 0, 0), (180, 30, 22), material=hull, size=2),
        part("primitive_cube_add", "Bridge", (-110, 0, 34), (30, 14, 14), material=hull, size=2),
        part("primitive_cube_add", "Sponsons", (20, 0, 0), (60, 52, 10), material=hull, size=2),
        spike("Ram", (210, 0, 0), 26, 40, hull, verts=4),
        part("primitive_cylinder_add", "EngineL", (-190, -18, 0), (12, 12, 8), (0, 90, 0), glow,
             vertices=8, radius=1, depth=2),
        part("primitive_cylinder_add", "EngineR", (-190, 18, 0), (12, 12, 8), (0, 90, 0), glow,
             vertices=8, radius=1, depth=2),
    ]
    finish_and_export("SM_TitheCruiser", parts, bevel=0.6)


if __name__ == "__main__":
    crystal_dreadnought()
    phantom_cruiser()
    tithe_interceptor()
    tithe_cruiser()
