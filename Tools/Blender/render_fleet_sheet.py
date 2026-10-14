"""Renders a labelled contact sheet of every known asset in a folder, each normalized to the same size.

Run:  blender --background --python render_fleet_sheet.py -- <meshes_dir> <out.png>
  or: python render_fleet_sheet.py <meshes_dir> <out.png>   (with the `bpy` pip module)
Uses Cycles on the CPU so it also works on machines without a GPU.
"""
import math
import os
import sys

import bpy
from mathutils import Vector

args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else sys.argv[1:]
MESHES, OUT = os.path.abspath(args[0]), os.path.abspath(args[1])

LABELS = [  # (asset, label); assets missing from the folder are skipped
    ("SM_CrystalDreadnought", "Crystal Dreadnought · 1,020 m"),
    ("SM_Wayward", "The Wayward"),
    ("SM_PhantomCruiser", "Phantom Battle-Cruiser"),
    ("SM_PhantomCarrier", "Phantom Heavy Carrier"),
    ("SM_CorsairGunship", "Corsair Salvage Gunship"),
    ("SM_BoardingPod", "Corsair Boarding Pod"),
    ("SM_TitheCruiser", "Iron Tithe Cruiser (Vance)"),
    ("SM_TitheInterceptor", "Iron Tithe Interceptor (Vance)"),
    ("SM_SeverantChoirSkiff", "Severant Choir-Skiff"),
    ("SM_SilenceEmitterFrigate", "Silence Emitter Frigate"),
    ("SM_TheLastSilence", "The Last Silence (Ithane)"),
    ("SM_KeshRefugeTender", "Kesh's Refuge Tender"),
    ("SM_OmegaArch", "The Omega Arch · 3 km"),
    ("SM_EmberRelay", "Ember Relay"),
    ("SM_LanternCluster", "Lantern Drift Cluster"),
    ("SM_ArchDebris_A", "Arch Wake Debris A"),
    ("SM_ArchDebris_B", "Arch Wake Debris B"),
    ("SM_SeverantBastion", "Pyre Gate Bastion"),
    ("SM_BastionPylon", "Bastion Shield Pylon"),
    ("SM_VaultOfRecords", "The Vault of Records"),
    ("SM_GlassShardField", "Glass Sea Shards"),
]
COLUMNS, CELL_W, CELL_H = 4, 140.0, 110.0
FIT_W, FIT_H = 110.0, 70.0  # on-screen box each asset is scaled into
DEFAULT_ROTATION = (20, 0, -30)  # degrees
VIEW_ROTATION = {"SM_OmegaArch": (0, 70, -20)}  # ring faces +X; tilt it toward the camera

bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene
label_mat = bpy.data.materials.new("Label")
label_mat.use_nodes = True
label_mat.node_tree.nodes["Principled BSDF"].inputs["Emission Color"].default_value = (0.8, 0.9, 1.0, 1)
label_mat.node_tree.nodes["Principled BSDF"].inputs["Emission Strength"].default_value = 3.0

present = [(a, l) for a, l in LABELS if os.path.isfile(os.path.join(MESHES, f"{a}.fbx"))]
for i, (asset, label) in enumerate(present):
    path = os.path.join(MESHES, f"{asset}.fbx")
    bpy.ops.import_scene.fbx(filepath=path)
    ship = bpy.context.selected_objects[0]
    col, row = i % COLUMNS, i // COLUMNS
    cx, cy = col * CELL_W, -row * CELL_H
    ship.rotation_euler = [math.radians(a) for a in VIEW_ROTATION.get(asset, DEFAULT_ROTATION)]
    ship.location = (0, 0, 0)
    bpy.context.view_layer.update()
    corners = [ship.matrix_world @ Vector(c) for c in ship.bound_box]
    width = max(c.x for c in corners) - min(c.x for c in corners)
    height = max(c.y for c in corners) - min(c.y for c in corners)
    scale = min(FIT_W / width, FIT_H / height)
    ship.scale = [s * scale for s in ship.scale]
    bpy.context.view_layer.update()
    corners = [ship.matrix_world @ Vector(c) for c in ship.bound_box]
    centre = sum(corners, Vector()) / len(corners)
    ship.location = (cx - centre.x, cy + 12 - centre.y, -centre.z)
    bpy.ops.object.text_add(location=(cx - 50, cy - 42, 0))
    text = bpy.context.active_object
    text.data.body, text.data.size = label, 7.5
    text.data.materials.append(label_mat)

scene.render.engine = "CYCLES"
scene.cycles.device, scene.cycles.samples, scene.cycles.use_denoising = "CPU", 32, False
scene.world = bpy.data.worlds.new("Space")
scene.world.use_nodes = True
scene.world.node_tree.nodes["Background"].inputs[0].default_value = (0.004, 0.007, 0.018, 1)
bpy.ops.object.light_add(type="SUN", rotation=(0.6, 0.2, 0.9))
bpy.context.active_object.data.energy = 4.0
rows = math.ceil(len(present) / COLUMNS)
bpy.ops.object.camera_add(location=((COLUMNS - 1) * CELL_W / 2, -(rows - 1) * CELL_H / 2, 600))
camera = bpy.context.active_object
camera.data.type, camera.data.ortho_scale = "ORTHO", COLUMNS * CELL_W + 20
camera.data.clip_end = 2000
scene.camera = camera
scene.render.resolution_x = 1800
scene.render.resolution_y = int(1800 * (rows * CELL_H + 20) / (COLUMNS * CELL_W + 20))
scene.render.filepath = OUT
bpy.ops.render.render(write_still=True)
print(f"[aetheria] contact sheet -> {OUT}")
