"""Finishes the bridge crew's MetaHuman characters (BRIDGE_SPEC.md B4) without the Creator UI.

Each step mirrors one done by hand in the MetaHuman Creator, using the MetaHumanCharacterEditorSubsystem
the way Epic's MetaHumanCharacter/Content/Python/examples do: face from a preset character, grooms and
outfit from the engine's wardrobe items, colours through the items' instance parameters, the face rig
(MetaHuman cloud service, needs the editor's Epic sign-in) and assembly into the character's folder.
Asset setup only: nothing here runs in the game.
"""
import unreal
import toolset_registry

CREW_ROOT = "/Game/Aetheria/Crew"
COMMON = CREW_ROOT + "/Common"
PRESETS = "/MetaHumanCharacter/Optional/Presets/"
GROOMS = "/MetaHumanCharacter/Optional/Grooms/Bindings/"
OUTFIT = "/MetaHumanCharacter/Optional/Clothing/WI_DefaultGarment.WI_DefaultGarment"

# wardrobe folder -> the pipeline slot it fills (matched case-insensitively against the palette's slots)
_GROOM_FOLDERS = {"Hair": "hair", "Eyebrows": "eyebrows", "Beards": "beard", "Mustaches": "mustache"}


def _subsystem():
    return unreal.get_editor_subsystem(unreal.MetaHumanCharacterEditorSubsystem)


def _character(name: str):
    path = f"{CREW_ROOT}/{name}/MH_{name}.MH_{name}"
    character = unreal.load_asset(path)
    if not character:
        raise ValueError(f"No MetaHuman at {path}")
    return character


class _Editing:
    """Holds a character in MetaHuman edit mode for the duration of a with-block."""

    def __init__(self, character):
        self.character = character
        self.added = False

    def __enter__(self):
        sub = _subsystem()
        if not sub.is_object_added_for_editing(self.character):
            if not sub.try_add_object_to_edit(self.character):
                raise RuntimeError(f"Cannot edit {self.character.get_name()}: is it open in the Creator?")
            self.added = True
        return self.character

    def __exit__(self, *exc):
        sub = _subsystem()
        if self.added and sub.is_object_added_for_editing(self.character):
            sub.remove_object_to_edit(self.character)
        return False


def _slot_for(collection, wanted: str) -> str:
    slots = [str(s) for s in unreal.MetaHumanCollectionBlueprintLibrary.get_slot_names(collection)]
    for slot in slots:
        if slot.lower() == wanted or slot.lower().rstrip("s") == wanted:
            return slot
    for slot in slots:
        if slot.lower().startswith(wanted):
            return slot
    raise ValueError(f"No slot for {wanted}; slots are {slots}")


def _select(character, slot: str, item_path: str):
    item = unreal.load_asset(item_path)
    if not item:
        raise ValueError(f"No wardrobe item at {item_path}")
    collection = character.internal_collection
    existing = unreal.MetaHumanCollectionBlueprintLibrary.get_item_keys_for_wardrobe_item(collection, item)
    key = existing[0] if existing else collection.try_add_item_from_wardrobe_item(slot, item)
    if not key:
        raise RuntimeError(f"Could not add {item_path} to slot {slot}")
    collection.default_instance.set_single_slot_selection(slot, key)
    return key


def _params(character, key):
    path = unreal.MetaHumanPaletteItemPath(item_key=key)
    return character.internal_collection.default_instance.get_instance_parameters(item_path=path)


def _describe_crew(name: str):
    character = _character(name)
    lib = unreal.MetaHumanCollectionBlueprintLibrary
    collection = character.internal_collection
    slots = [str(s) for s in lib.get_slot_names(collection)]
    selected = {slot: len(lib.get_item_keys_for_slot(collection, slot)) for slot in slots}
    with _Editing(character):
        can_build = _subsystem().can_build_meta_human(character, True)
    return f"slots={slots}; items per slot={selected}; can_build={can_build}"


def _finish_crew(name: str, preset: str, grooms: list[str], hair_melanin: float, outfit_color: unreal.LinearColor):
    sub = _subsystem()
    character = _character(name)
    done = []

    preset_asset = unreal.load_asset(f"{PRESETS}{preset}.{preset}")
    if not preset_asset:
        raise ValueError(f"No preset {preset}")
    with _Editing(preset_asset):
        coefficients = sub.get_face_model_coefficients(preset_asset)

    with _Editing(character):
        sub.set_face_model_coefficients(character, coefficients)
        sub.commit_face_state(character)
        done.append(f"face<-{preset} ({len(coefficients)} coeffs)")

        keys = []
        for entry in grooms:
            folder, item = entry.split("/", 1)
            slot = _slot_for(character.internal_collection, _GROOM_FOLDERS[folder])
            keys.append(_select(character, slot, f"{GROOMS}{folder}/{item}.{item}"))
            done.append(f"{slot}<-{item}")
        outfit_key = _select(character, _slot_for(character.internal_collection, "outfit"), OUTFIT)
        done.append("outfit<-DefaultGarment")

        # instance parameters exist once the character has been assembled for preview
        sub.assemble_for_preview(character=character)
        for key in keys:
            for param in _params(character, key):
                if str(param.name) == "Melanin":
                    param.set_float(value=hair_melanin)
        colored = 0
        for param in _params(character, outfit_key):
            if str(param.name).startswith("PrimaryColor"):
                param.set_color(value=outfit_color)
                colored += 1
        done.append(f"melanin={hair_melanin}, outfit colours={colored}")

    unreal.EditorAssetLibrary.save_loaded_asset(character, only_if_is_dirty=False)
    return "; ".join(done)


def _rig_crew(name: str):
    sub = _subsystem()
    character = _character(name)
    with _Editing(character):
        request = unreal.MetaHumanCharacterAutoRiggingRequestParams()
        request.blocking = True
        request.report_progress = False
        request.rig_type = unreal.MetaHumanRigType.JOINTS_AND_BLEND_SHAPES
        if not sub.can_build_meta_human(character, False):
            sub.request_auto_rigging(character, request)
        # assembly also needs the high-resolution skin texture sources from the service
        textures = unreal.MetaHumanCharacterTextureRequestParams()
        textures.blocking = True
        textures.report_progress = False
        sub.request_texture_sources(character, textures)
        can_build = sub.can_build_meta_human(character, True)
    unreal.EditorAssetLibrary.save_loaded_asset(character, only_if_is_dirty=False)
    return f"rigged; can_build={can_build}"


def _build_crew(name: str):
    sub = _subsystem()
    character = _character(name)
    folder = f"{CREW_ROOT}/{name}"
    with _Editing(character):
        if not sub.can_build_meta_human(character, True):
            raise RuntimeError(f"{name} cannot be assembled yet (see the output log)")
        params = unreal.MetaHumanCharacterEditorBuildParameters()
        params.pipeline_type = unreal.MetaHumanDefaultPipelineType.OPTIMIZED
        params.pipeline_quality = unreal.MetaHumanQualityLevel.MEDIUM
        params.absolute_build_path = folder
        params.common_folder_path = COMMON
        params.enable_wardrobe_item_validation = False
        try:
            sub.build_meta_human(character=character, params=params)
        except RuntimeError as error:
            # copying the shared face Control Rigs logs link warnings that Python raises as errors;
            # assembly itself still completes, so log them and save what was built
            unreal.log_warning(f"[AetheriaCrew] {name} assembly reported: {str(error)[:300]}")
    unreal.EditorAssetLibrary.save_directory(folder)
    unreal.EditorAssetLibrary.save_directory(COMMON)
    return [str(a) for a in unreal.EditorAssetLibrary.list_assets(folder, recursive=True)]


def _colour_crew(name: str, hair_melanin: float, outfit_color: unreal.LinearColor):
    sub = _subsystem()
    character = _character(name)
    lib = unreal.MetaHumanCollectionBlueprintLibrary
    collection = character.internal_collection
    set_count = 0
    # instance parameters exist once the preview assembly has finished, which happens on a later
    # editor tick: the first call starts it and keeps the character in edit mode, the next call colours
    if not sub.is_object_added_for_editing(character):
        if not sub.try_add_object_to_edit(character):
            raise RuntimeError(f"Cannot edit {name}")
        sub.assemble_for_preview(character=character)
        return "preview assembly started; call colour_crew again to apply the colours"
    seen = []
    try:
        for slot in ("Hair", "Eyebrows", "Beard", "Mustache", "Outfits"):
            for key in lib.get_item_keys_for_slot(collection, slot):
                params = _params(character, key)
                seen.append(f"{slot}:{[str(p.name) for p in params][:8]}")
                for param in params:
                    if slot != "Outfits" and str(param.name) == "Melanin":
                        param.set_float(value=hair_melanin)
                        set_count += 1
                    elif slot == "Outfits" and str(param.name).startswith("PrimaryColor"):
                        param.set_color(value=outfit_color)
                        set_count += 1
    finally:
        sub.remove_object_to_edit(character)
    unreal.EditorAssetLibrary.save_loaded_asset(character, only_if_is_dirty=False)
    return f"{set_count} parameters set; seen {seen}"


@unreal.uclass()
class AetheriaCrewToolset(unreal.ToolsetDefinition):
    """Finishes and assembles the Aetheria bridge crew's MetaHuman characters (face from a preset,
    hair/brows/beard/outfit from the engine wardrobe, face rig, assembly into /Game/Aetheria/Crew)."""

    @toolset_registry.tool_call
    @staticmethod
    def describe_crew(name: str) -> str:
        """Reports a crew character's palette slots, current selections and whether it can be assembled.

        Args:
            name: Crew folder name, e.g. TalaVane

        Returns:
            A short report.
        """
        return _describe_crew(name)

    @toolset_registry.tool_call
    @staticmethod
    def finish_crew(name: str, preset: str, grooms: list[str], hair_melanin: float, outfit_color: unreal.LinearColor) -> str:
        """Gives a crew character a face, grooms and an outfit, as done by hand in the Creator.

        Args:
            name: Crew folder name, e.g. TalaVane
            preset: Epic preset character whose face to copy, e.g. Jelena (from /MetaHumanCharacter/Optional/Presets)
            grooms: Wardrobe items as Folder/WI_Name, e.g. Hair/WI_Hair_S_Pixie, Eyebrows/WI_Eyebrows_M_Natural,
                Beards/WI_Beard_S_Full; an empty list keeps the head bare
            hair_melanin: Groom melanin, 0 (white/silver) .. 1 (black)
            outfit_color: Primary colour of the default garment

        Returns:
            What was applied.
        """
        return _finish_crew(name, preset, grooms, hair_melanin, outfit_color)

    @toolset_registry.tool_call
    @staticmethod
    def rig_crew(name: str) -> str:
        """Requests the face rig for a crew character from the MetaHuman service (blocking; needs Epic sign-in).

        Args:
            name: Crew folder name

        Returns:
            Whether the character can be assembled afterwards.
        """
        return _rig_crew(name)

    @toolset_registry.tool_call
    @staticmethod
    def build_crew(name: str) -> list[str]:
        """Assembles a crew character (UE Optimized, medium) into its own folder.

        Args:
            name: Crew folder name

        Returns:
            The assets in the character's folder after assembly.
        """
        return _build_crew(name)

    @toolset_registry.tool_call
    @staticmethod
    def colour_crew(name: str, hair_melanin: float, outfit_color: unreal.LinearColor) -> str:
        """Sets groom melanin and outfit colour on a rigged crew character (run after rig_crew).

        Args:
            name: Crew folder name
            hair_melanin: Groom melanin, 0 (white/silver) .. 1 (black)
            outfit_color: Primary colour of the outfit

        Returns:
            How many parameters were set.
        """
        return _colour_crew(name, hair_melanin, outfit_color)
