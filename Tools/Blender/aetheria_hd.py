"""Hard-surface kit for Aetheria's high-detail (Nanite) ship meshes.

Builds each ship as a single bmesh: hull slabs with sloped sides, then layered plating, greebles,
trenches, turret batteries, engine banks and lit window strips scattered over the surfaces.
Dense geometry is intended: Unreal renders it with Nanite, and Lumen picks up the emissive
engine and window materials.

Scale and axes match aetheria_mesh: 1 Blender unit = 1 m (imported at 1 uu per unit), forward = +X.

Material slots (names are kept so Unreal can map them):
    0 M_Hull      main hull plating
    1 M_HullDark  recesses, trenches, greebles, nozzles
    2 M_Engine    engine glow (emissive)
    3 M_Lights    window strips and running lights (emissive)
    4 M_Accent    faction accent: prismatic crystal (fleet) or warning red (hostile)
"""
import math
import os
import random

import bmesh
import bpy
from mathutils import Matrix, Vector

HULL, DARK, ENGINE, LIGHTS, ACCENT = range(5)
SLOT_NAMES = ("M_Hull", "M_HullDark", "M_Engine", "M_Lights", "M_Accent")
SLOT_PREVIEW = (  # Blender preview colours only; Unreal assigns its own materials per slot
    ((0.55, 0.57, 0.60), 0.0), ((0.12, 0.13, 0.15), 0.0), ((0.45, 0.75, 1.0), 25.0),
    ((1.0, 0.85, 0.6), 6.0), ((0.4, 0.8, 1.0), 4.0))


class Ship:
    """One ship under construction."""

    def __init__(self, name, seed):
        self.name = name
        self.bm = bmesh.new()
        self.rng = random.Random(seed)


    # ---- raw geometry (bmesh.ops scale with mesh size, so primitives are built vertex by vertex) --

    def _cube(self, m, slot):
        corners = [m @ Vector((x * 0.5, y * 0.5, z * 0.5)) for x in (-1, 1) for y in (-1, 1) for z in (-1, 1)]
        v = [self.bm.verts.new(c) for c in corners]
        quads = ((0, 1, 3, 2), (4, 6, 7, 5), (0, 4, 5, 1), (2, 3, 7, 6), (0, 2, 6, 4), (1, 5, 7, 3))
        faces = [self.bm.faces.new([v[i] for i in q]) for q in quads]
        self._tag(faces, slot)
        return faces

    def _cone(self, m, radius1, radius2, depth, segments, slot, cap=True):
        """Along local Z, centred on the origin of m."""
        def ring(r, z):
            return [self.bm.verts.new(m @ Vector((r * math.cos(a), r * math.sin(a), z)))
                    for a in (2 * math.pi * i / segments for i in range(segments))]
        bottom, top = ring(radius1, -depth * 0.5), ring(max(radius2, 1e-4), depth * 0.5)
        faces = [self.bm.faces.new((bottom[i], bottom[(i + 1) % segments], top[(i + 1) % segments], top[i]))
                 for i in range(segments)]
        if cap:
            faces.append(self.bm.faces.new(list(reversed(bottom))))
            faces.append(self.bm.faces.new(top))
        self._tag(faces, slot)
        return faces

    # ---- primitives ---------------------------------------------------------------------------

    def _tag(self, faces, slot):
        # new bmesh faces have no normal until asked; scatter and orientation helpers rely on it
        for f in faces:
            f.material_index = slot
            f.normal_update()

    def box(self, center, size, slot=HULL, yaw=0.0, matrix=None):
        """Axis box of size (x, y, z) at center, optionally yawed (degrees) or placed by matrix."""
        m = matrix if matrix is not None else (
            Matrix.Translation(Vector(center)) @ Matrix.Rotation(math.radians(yaw), 4, "Z"))
        return self._cube(m @ Matrix.Diagonal((*size, 1.0)), slot)

    def cylinder(self, center, radius, length, slot=DARK, axis="X", segments=16, radius2=None, cap=True):
        """Cylinder (or cone when radius2 is given) along an axis, centred on center."""
        rot = {"X": Matrix.Rotation(math.radians(90), 4, "Y"), "Y": Matrix.Rotation(math.radians(90), 4, "X"),
               "Z": Matrix.Identity(4)}[axis]
        m = Matrix.Translation(Vector(center)) @ rot
        return self._cone(m, radius, radius if radius2 is None else radius2, length, segments, slot, cap)

    def prism(self, plan_bottom, z0, plan_top, z1, slot=HULL):
        """Slab between two horizontal polygons (counter-clockwise from above, same vertex count)."""
        bottom = [self.bm.verts.new((x, y, z0)) for x, y in plan_bottom]
        top = [self.bm.verts.new((x, y, z1)) for x, y in plan_top]
        faces = [self.bm.faces.new(list(reversed(bottom))), self.bm.faces.new(top)]
        n = len(bottom)
        for i in range(n):
            j = (i + 1) % n
            # two triangles, not a quad: when the top outline isn't a scaled copy of the bottom
            # the side quad is twisted, and plating aligned to its average plane would stick out
            faces.append(self.bm.faces.new((bottom[i], bottom[j], top[j])))
            faces.append(self.bm.faces.new((bottom[i], top[j], top[i])))
        self._tag(faces, slot)
        return faces

    def crystal(self, tip, back, radius, slot=ACCENT, sides=6):
        """Elongated bipyramid crystal from back to tip."""
        tip, back = Vector(tip), Vector(back)
        axis = (tip - back)
        mid = back + axis * 0.35
        frame = axis.normalized().to_track_quat("Z", "Y").to_matrix().to_4x4()
        ring = [self.bm.verts.new(mid + frame @ Vector((radius * math.cos(a), radius * math.sin(a), 0)))
                for a in (2 * math.pi * i / sides for i in range(sides))]
        vt, vb = self.bm.verts.new(tip), self.bm.verts.new(back)
        faces = []
        for i in range(sides):
            j = (i + 1) % sides
            faces.append(self.bm.faces.new((ring[i], ring[j], vt)))
            faces.append(self.bm.faces.new((ring[j], ring[i], vb)))
        self._tag(faces, slot)
        return faces

    # ---- composite details ---------------------------------------------------------------------

    def face_frame(self, face):
        """Matrix whose Z is the face normal and X runs along the ship (world X) projected onto the
        face, so panels line up with the hull; faces facing fore/aft use their longest edge instead."""
        n = face.normal.normalized()
        t = Vector((1, 0, 0))
        t = t - n * t.dot(n)
        if t.length < 0.3:
            edge = max(face.edges, key=lambda e: e.calc_length())
            t = (edge.verts[1].co - edge.verts[0].co)
            t = t - n * t.dot(n)
        t = t.normalized()
        b = n.cross(t)
        return Matrix((t, b, n)).transposed().to_4x4()

    def random_point(self, face):
        """Uniform random point on a (convex) face."""
        vs = [v.co for v in face.verts]
        tris = [(vs[0], vs[i], vs[i + 1]) for i in range(1, len(vs) - 1)]
        areas = [((b - a).cross(c - a)).length for a, b, c in tris]
        a, b, c = self.rng.choices(tris, weights=areas)[0]
        r1, r2 = self.rng.random(), self.rng.random()
        if r1 + r2 > 1:
            r1, r2 = 1 - r1, 1 - r2
        return a + (b - a) * r1 + (c - a) * r2

    def face_polygon_2d(self, face, frame):
        inv = frame.inverted()
        return [(inv @ v.co).to_2d() for v in face.verts]

    @staticmethod
    def inside(poly, x, y):
        """Point in polygon (even-odd rule)."""
        hit = False
        n = len(poly)
        for i in range(n):
            (x1, y1), (x2, y2) = poly[i], poly[(i + 1) % n]
            if (y1 > y) != (y2 > y) and x < (x2 - x1) * (y - y1) / (y2 - y1) + x1:
                hit = not hit
        return hit

    def rect_inside(self, poly, cx, cy, hx, hy):
        return all(self.inside(poly, cx + sx * hx, cy + sy * hy) for sx in (-1, 1) for sy in (-1, 1))

    def panel_grid(self, faces, cell=(18.0, 10.0), height=(0.3, 1.4), gap=0.7, keep=0.9,
                   split=0.35, slots=(HULL,) * 9 + (DARK,)):
        """Orderly hull panelling: a grid of raised plates aligned with the ship, separated by
        panel-line gaps, clipped to each face. Some cells split into four smaller plates."""
        for face in list(faces):
            if not face.is_valid or face.calc_area() < cell[0] * cell[1]:
                continue
            frame = self.face_frame(face)
            poly = self.face_polygon_2d(face, frame)
            plane = (frame.inverted() @ face.verts[0].co).z   # face's offset along its normal
            xs, ys = [p.x for p in poly], [p.y for p in poly]
            # offset the grid per face so neighbouring faces don't line up exactly
            ox, oy = self.rng.uniform(0, cell[0]), self.rng.uniform(0, cell[1])
            x = min(xs) - ox
            while x < max(xs):
                y = min(ys) - oy
                while y < max(ys):
                    cx, cy = x + cell[0] / 2, y + cell[1] / 2
                    if self.rng.random() < keep:
                        if self.rng.random() < split:
                            subs = [(cx + sx * cell[0] / 4, cy + sy * cell[1] / 4, cell[0] / 2, cell[1] / 2)
                                    for sx in (-1, 1) for sy in (-1, 1)]
                        else:
                            subs = [(cx, cy, cell[0], cell[1])]
                        for px, py, w, l in subs:
                            hx, hy = (w - gap) / 2, (l - gap) / 2
                            if hx <= 0 or hy <= 0 or not self.rect_inside(poly, px, py, hx, hy):
                                continue
                            h = self.rng.uniform(*height)
                            m = frame @ Matrix.Translation((px, py, plane + h * 0.5))
                            self.box(None, (hx * 2, hy * 2, h), self.rng.choice(slots), matrix=m)
                    y += cell[1]
                x += cell[0]

    def plate(self, faces, density, size=(6, 30), height=(0.4, 2.0), slots=(HULL, HULL, DARK), max_count=6000):
        """Layered hull plating: thin boxes lying on the faces, aligned to each face."""
        for face in list(faces):
            if not face.is_valid:
                continue
            count = min(max_count, int(face.calc_area() * density))
            frame = self.face_frame(face)
            for _ in range(count):
                p = self.random_point(face)
                w, l = self.rng.uniform(*size), self.rng.uniform(*size)
                h = self.rng.uniform(*height)
                m = Matrix.Translation(p + face.normal * (h * 0.5)) @ frame
                self.box(None, (l, w * 0.6, h), self.rng.choice(slots), matrix=m)

    def greeble(self, faces, density, size=(1.5, 8), height=(1, 6), max_count=4000):
        """Small machinery: blocks, vents, pipes and dishes standing on the faces."""
        for face in list(faces):
            if not face.is_valid:
                continue
            count = min(max_count, int(face.calc_area() * density))
            frame = self.face_frame(face)
            poly = self.face_polygon_2d(face, frame)
            inv = frame.inverted()
            for _ in range(count):
                p = self.random_point(face)
                s = self.rng.uniform(*size)
                h = self.rng.uniform(*height)
                local = inv @ p
                if not self.rect_inside(poly, local.x, local.y, s * 1.6, s * 1.6):
                    continue
                kind = self.rng.random()
                base = Matrix.Translation(p) @ frame
                if kind < 0.6:
                    self.box(None, (s, s * self.rng.uniform(0.4, 1.0), h), DARK,
                             matrix=base @ Matrix.Translation((0, 0, h * 0.5)))
                elif kind < 0.85:
                    pipe = base @ Matrix.Translation((0, 0, s * 0.25)) @ Matrix.Rotation(math.radians(90), 4, "Y")
                    self._cone(pipe, s * 0.18, s * 0.18, s * 3, 8, DARK)
                else:
                    self._cone(base @ Matrix.Translation((0, 0, h * 0.2)), s * 0.6, s * 0.2, h * 0.4, 12, HULL)

    def windows(self, face, rows, count, slot=LIGHTS, size=(1.2, 0.5)):
        """Rows of lit window strips across a (vertical) face."""
        frame = self.face_frame(face)
        vs = [frame.inverted() @ v.co for v in face.verts]
        xs, ys = [v.x for v in vs], [v.y for v in vs]
        plane = vs[0].z   # face's offset along its normal
        poly = [v.to_2d() for v in vs]
        for r in range(rows):
            y = min(ys) + (max(ys) - min(ys)) * (r + 1) / (rows + 1)
            for _ in range(count):
                x = self.rng.uniform(min(xs), max(xs))
                if self.rng.random() < 0.3:
                    continue
                w = size[0] * self.rng.uniform(1, 4)
                if not self.rect_inside(poly, x, y, w / 2, size[1] / 2):
                    continue
                m = frame @ Matrix.Translation((x, y, plane + 0.15))
                self.box(None, (w, size[1], 0.3), slot, matrix=m)

    def turret(self, at, yaw, scale=1.0, slot=DARK):
        """Twin-barrel battery on a round base."""
        x, y, z = at
        self.cylinder((x, y, z + 1.5 * scale), 4 * scale, 3 * scale, slot, axis="Z", segments=12)
        self.box((x, y, z + 4 * scale), (6 * scale, 5 * scale, 3 * scale), HULL, yaw=yaw)
        d = Vector((math.cos(math.radians(yaw)), math.sin(math.radians(yaw)), 0))
        side = Vector((-d.y, d.x, 0))
        for s in (-1, 1):
            c = Vector((x, y, z + 4 * scale)) + d * 7 * scale + side * s * 1.3 * scale
            m = Matrix.Translation(c) @ Matrix.Rotation(math.radians(yaw), 4, "Z") @ Matrix.Rotation(math.radians(90), 4, "Y")
            self._cone(m, 0.5 * scale, 0.4 * scale, 9 * scale, 8, slot)

    def engine(self, at, radius, depth, slot_glow=ENGINE):
        """Rear-facing engine: housing ring, recessed glowing core, and a faint outer glow ring."""
        x, y, z = at
        self.cylinder((x + depth * 0.5, y, z), radius * 1.15, depth, DARK, segments=24)
        self.cylinder((x - 0.4, y, z), radius * 1.05, 1.0, HULL, segments=24, radius2=radius * 0.98)
        self.cylinder((x + 0.6, y, z), radius * 0.86, 0.6, slot_glow, segments=24)
        for i in range(6):
            a = math.radians(60 * i + 30)
            self.box((x - 0.5, y + math.cos(a) * radius * 1.1, z + math.sin(a) * radius * 1.1),
                     (depth * 0.8, radius * 0.18, radius * 0.18), DARK)

    def faces_facing(self, faces, direction, threshold=0.5):
        d = Vector(direction).normalized()
        return [f for f in faces if f.is_valid and f.normal.normalized().dot(d) > threshold]

    # ---- output --------------------------------------------------------------------------------

    def export(self, out_dir, bevel=0.15):
        bmesh.ops.recalc_face_normals(self.bm, faces=self.bm.faces[:])
        mesh = bpy.data.meshes.new(self.name)
        self.bm.to_mesh(mesh)
        self.bm.free()

        for name, (rgb, strength) in zip(SLOT_NAMES, SLOT_PREVIEW):
            m = bpy.data.materials.get(name) or bpy.data.materials.new(name)
            m.use_nodes = True
            bsdf = m.node_tree.nodes.get("Principled BSDF")
            if bsdf:
                bsdf.inputs["Base Color"].default_value = (*rgb, 1)
                bsdf.inputs["Emission Color"].default_value = (*rgb, 1)
                bsdf.inputs["Emission Strength"].default_value = strength
            mesh.materials.append(m)

        obj = bpy.data.objects.new(self.name, mesh)
        bpy.context.collection.objects.link(obj)
        bpy.ops.object.select_all(action="DESELECT")
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        if bevel > 0:
            b = obj.modifiers.new("Bevel", "BEVEL")
            b.width, b.segments, b.limit_method = bevel, 1, "ANGLE"
            b.angle_limit = math.radians(40)
        bpy.ops.object.shade_flat()

        path = os.path.join(out_dir, f"{self.name}.fbx")
        bpy.ops.export_scene.fbx(filepath=path, use_selection=True, apply_scale_options="FBX_SCALE_UNITS",
                                 use_mesh_modifiers=True, mesh_smooth_type="FACE", add_leaf_bones=False,
                                 bake_anim=False)
        depsgraph = bpy.context.evaluated_depsgraph_get()
        evaluated = obj.evaluated_get(depsgraph).to_mesh()
        tris = sum(len(p.vertices) - 2 for p in evaluated.polygons)
        dims = tuple(round(d, 1) for d in obj.dimensions)
        print(f"[aetheria-hd] {self.name}: {tris:,} triangles, dims(m)={dims} -> {path}")
        return path
