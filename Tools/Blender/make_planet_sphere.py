"""Smooth high-resolution sphere for planets and stars (diameter 100 uu, like /Engine/BasicShapes/Sphere,
so actor scale = diameter / 100). Run: blender --background --factory-startup --python make_planet_sphere.py -- <out_dir>"""
import os, sys
import bpy
args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
out = os.path.abspath(args[0] if args else ".")
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.mesh.primitive_uv_sphere_add(segments=192, ring_count=96, radius=50.0)
obj = bpy.context.active_object
obj.name = "SM_PlanetSphere"
bpy.ops.object.shade_smooth()
mat = bpy.data.materials.new("M_Planet")
obj.data.materials.append(mat)
bpy.ops.export_scene.fbx(filepath=os.path.join(out, "SM_PlanetSphere.fbx"), use_selection=True,
                         apply_scale_options="FBX_SCALE_UNITS", mesh_smooth_type="FACE", add_leaf_bones=False, bake_anim=False)
print("[planet] exported", obj.dimensions[:])
