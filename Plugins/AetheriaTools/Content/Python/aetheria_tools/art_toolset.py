"""Art pipeline helpers: reimports Blender-built meshes over their existing assets.

The MCP mesh importer refuses to overwrite an asset, but the bridge and ship kits are rebuilt from
Tools/Blender and must replace their meshes in place so level actors keep their references.
Asset setup only: nothing here runs in the game.
"""
import os

import unreal
import toolset_registry


def _fbx_options():
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_materials = False
    options.import_textures = False
    options.import_as_skeletal = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
    data = options.static_mesh_import_data
    data.combine_meshes = True
    data.auto_generate_collision = False
    data.import_uniform_scale = 1.0
    data.build_nanite = True
    return options


@unreal.uclass()
class AetheriaArtToolset(unreal.ToolsetDefinition):
    """Aetheria art pipeline: reimports the Blender-built meshes (Tools/Blender -> Source_Art) over their assets."""

    @toolset_registry.tool_call
    @staticmethod
    def reimport_static_meshes(folder_path: str, source_files: list[str]) -> list[str]:
        """Reimports FBX files over existing static meshes of the same name, in place.

        Keeps the assets (and every reference to them) and their stored import settings; the mesh data,
        material slots and UVs come from the files. Only for meshes that already exist: make new ones with
        StaticMeshTools.import_file, whose first import sets the scale the kit needs (Interchange ignores the
        legacy FBX options for a first import and would bring the centimetre kits in 100 times too large).

        Args:
            folder_path: Content folder, e.g. /Game/Aetheria/Bridge/Meshes
            source_files: Absolute paths of the .fbx files; each asset is named after its file

        Returns:
            The object paths of the imported meshes.
        """
        tasks = []
        for source in source_files:
            if not os.path.isfile(source):
                raise ValueError(f"No such file: {source}")
            name = os.path.splitext(os.path.basename(source))[0]
            if not unreal.EditorAssetLibrary.does_asset_exist(f"{folder_path}/{name}"):
                raise ValueError(f"{folder_path}/{name} does not exist yet: import it first with StaticMeshTools.import_file")
            task = unreal.AssetImportTask()
            task.filename = source
            task.destination_path = folder_path
            task.destination_name = os.path.splitext(os.path.basename(source))[0]
            task.replace_existing = True
            task.replace_existing_settings = False   # keep the stored import settings (scale)
            task.automated = True
            task.save = True
            task.options = _fbx_options()
            tasks.append(task)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
        imported = []
        for task in tasks:
            paths = list(task.imported_object_paths)
            if not paths:
                raise RuntimeError(f"Import failed: {task.filename}")
            imported.extend(paths)
        return imported
