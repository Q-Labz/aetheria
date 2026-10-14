"""Renders a labelled contact sheet of every exported ship, each normalized to the same length.

Run:  blender --background --python render_fleet_sheet.py -- <meshes_dir> <out.png>
  or: python render_fleet_sheet.py <meshes_dir> <out.png>   (with the `bpy` pip module)
Uses Cycles on the CPU so it also works on machines without a GPU.
"""
import math
import os
import sys

import bpy

args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else sys.argv[1:]
MESHES, OUT = os.path.abspath(args[0]), os.path.abspath(args[1])

SHIPS = [  # (asset, label)
    ("SM_CrystalDreadnought", "Crystal Dreadnought · 1,020 m"),
    ("SM_Wayward", "The Wayward"),
    ("SM_PhantomCruiser", "Phantom Battle-Cruiser"),
    ("SM_PhantomCarrier", "Phantom Heavy Carrier"),
    ("SM_CorsairGunship", "Corsair Salvage Gunship"),
    ("SM_BoardingPod", "Corsair Boarding Pod"),
    ("SM_TitheCruiser", "Iron Tithe Cruiser (Vance)"),
    ("SM_TitheInterceptor", "Iron Tithe Interceptor (Vance)"),
]
COLUMNS, CELL_W, CELL_H, DISPLAY_LEN = 4, 140.0, 110.0, 100.0

bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene
label_mat = bpy.data.materials.new("Label")
label_mat.use_nodes = True
label_mat.node_tree.nodes["Principled BSDF"].inputs["Emission Color"].default_value = (0.8, 0.9, 1.0, 1)
label_mat.node_tree.nodes["Principled BSDF"].inputs["Emission Strength"].default_value = 3.0

for i, (asset, label) in enumerate(SHIPS):
    path = os.path.join(MESHES, f"{asset}.fbx")
    if not os.path.isfile(path):
        continue
    bpy.ops.import_scene.fbx(filepath=path)
    ship = bpy.context.selected_objects[0]
    scale = DISPLAY_LEN / max(ship.dimensions)
    col, row = i % COLUMNS, i // COLUMNS
    cx, cy = col * CELL_W, -row * CELL_H
    ship.scale = [s * scale for s in ship.scale]
    ship.rotation_euler = (math.radians(20), 0, math.radians(-30))
    ship.location = (cx, cy + 12, 0)
    bpy.ops.object.text_add(location=(cx - 50, cy - 42, 0))
    text = bpy.context.active_object
    text.data.body, text.data.size = label, 7.5
    text.data.materials.append(label_mat)

scene.render.engine = "CYCLES"
scene.cycles.device, scene.cycles.samples, scene.cycles.use_denoising = "CPU", 32, False
scene.render.resolution_x, scene.render.resolution_y = 1800, 960
scene.world = bpy.data.worlds.new("Space")
scene.world.use_nodes = True
scene.world.node_tree.nodes["Background"].inputs[0].default_value = (0.004, 0.007, 0.018, 1)
bpy.ops.object.light_add(type="SUN", rotation=(0.6, 0.2, 0.9))
bpy.context.active_object.data.energy = 4.0
rows = math.ceil(len(SHIPS) / COLUMNS)
bpy.ops.object.camera_add(location=((COLUMNS - 1) * CELL_W / 2, -(rows - 1) * CELL_H / 2, 600))
camera = bpy.context.active_object
camera.data.type, camera.data.ortho_scale = "ORTHO", COLUMNS * CELL_W + 20
camera.data.clip_end = 2000
scene.camera = camera
scene.render.filepath = OUT
bpy.ops.render.render(write_still=True)
print(f"[aetheria] contact sheet -> {OUT}")
