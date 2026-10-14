"""Shared mesh-building helpers for the Aetheria Blender generators.

Scale: 1 Blender unit = 1 m. Forward = +X. Meshes are faceted (flat-shaded) and Nanite-ready.
"""
import math
import os

import bpy


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


def finish_and_export(name, parts, out_dir, bevel=1.0):
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
    path = os.path.join(out_dir, f"{name}.fbx")
    bpy.ops.export_scene.fbx(filepath=path, use_selection=True,
                             apply_scale_options="FBX_SCALE_UNITS", use_mesh_modifiers=True,
                             mesh_smooth_type="FACE", add_leaf_bones=False, bake_anim=False)
    dims = tuple(round(d, 1) for d in o.dimensions)
    print(f"[aetheria] {name}: {len(o.data.polygons)} faces, dims(m)={dims} -> {path}")
