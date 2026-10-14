import unreal
import toolset_registry

from aetheria_tools import combat_core as core
from aetheria_tools.narrative_toolset import CampaignState, campaign_state_dict


@unreal.ustruct()
class ShipStat(unreal.StructBase):
    """One tunable ship stat: its Blueprint default beside the spec's first-pass value."""

    stat = unreal.uproperty(str, meta={"ToolTip": "Blueprint variable name."})
    value = unreal.uproperty(float, meta={"ToolTip": "Class default, inherited values included."})
    spec_value = unreal.uproperty(float, meta={"ToolTip": "First-pass value from SLICE_SPEC §9."})


@unreal.ustruct()
class FleetClass(unreal.StructBase):
    """One canon ship class of the Key-Bearer's fleet."""

    ship_class = unreal.uproperty(str)
    canon_count = unreal.uproperty(int, meta={"ToolTip": "Ships of this class in the canon 214-ship fleet."})
    active_count = unreal.uproperty(int, meta={"ToolTip": "Ships still answering to the fleet in the given state."})
    mesh = unreal.uproperty(str, meta={"ToolTip": "Static mesh asset path."})
    blueprint = unreal.uproperty(str, meta={"ToolTip": "Ship Blueprint asset path. Empty if the slice has none."})


@unreal.uclass()
class AetheriaCombatToolset(unreal.ToolsetDefinition):
    """Aetheria's fleet and tactical combat slice: the canon fleet roster and its strength in a
    campaign state, checks of the ship Blueprints against fleet canon (classes, names, squadron
    and fleet sizes, abilities, meshes), their stats against first-pass values, and the combat
    level's spawn layout."""

    @toolset_registry.tool_call
    @staticmethod
    def get_fleet_roster(state: CampaignState) -> list[FleetClass]:
        """Lists the four canon ship classes and how many of each are active in a campaign state.

        Args:
            state: Campaign state to read Wayward, Phantom and Corsair strength from.
        """
        return [FleetClass(**entry) for entry in core.fleet_roster(campaign_state_dict(state))]

    @toolset_registry.tool_call
    @staticmethod
    def validate_fleet() -> list[str]:
        """Checks the ship Blueprints in /Game/Aetheria/Ships and BP_AetheriaGameInstance against
        fleet canon: missing Blueprints or variables, wrong parent class, canon names and counts,
        actor tags, hull meshes and roster meshes not imported. Stat values are not compared.

        Returns:
            Problems found. Empty when the fleet matches canon.
        """
        ships = {ship: AetheriaCombatToolset._read_ship(ship) for ship in core.SHIPS}
        game_instance = None
        blueprint = AetheriaCombatToolset._blueprint(core.GAME_INSTANCE)
        if blueprint is not None:
            game_instance = AetheriaCombatToolset._properties(blueprint, core.GAME_INSTANCE_CANON)
        imported = {e["mesh"] for e in core.ROSTER
                    if unreal.EditorAssetLibrary.does_asset_exist(core.mesh_path(e["mesh"]))}
        return core.validate_fleet(ships, game_instance, imported)

    @toolset_registry.tool_call
    @staticmethod
    def get_ship_stats(ship: str) -> list[ShipStat]:
        """Reads a ship Blueprint's tunable stats. Stats computed at BeginPlay are left out.

        Args:
            ship: Ship Blueprint name, e.g. BP_Flagship or BP_ShipBase.
        """
        try:
            names = core.read_names(ship)
        except KeyError as error:
            raise ValueError(error.args[0]) from None
        blueprint = AetheriaCombatToolset._blueprint(core.ship_path(ship))
        if blueprint is None:
            raise ValueError(f"{core.ship_path(ship)} does not exist.")
        stats = core.ship_stats(ship, AetheriaCombatToolset._properties(blueprint, names))
        return [ShipStat(stat=s, value=v, spec_value=e) for s, v, e in stats]

    @toolset_registry.tool_call
    @staticmethod
    def validate_combat_level() -> list[str]:
        """Checks the level open in the editor against the tactical combat layout: GameMode
        Override, BP_CombatManager, tagged spawn TargetPoints and their locations, PlayerStart,
        lighting and post-process.

        Returns:
            Problems found. Empty when the layout matches.
        """
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        if world is None:
            raise RuntimeError("No level is open in the editor.")
        game_mode = world.get_world_settings().get_editor_property("default_game_mode")
        game_mode_name = AetheriaCombatToolset._class_name(game_mode) if game_mode else None
        actors = [AetheriaCombatToolset._describe(a)
                  for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()]
        return core.validate_combat_level(game_mode_name, actors)

    @staticmethod
    def _blueprint(asset_path: str):
        if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            return None
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    @staticmethod
    def _properties(blueprint, names) -> dict:
        defaults = unreal.get_default_object(blueprint.generated_class())
        found = {}
        for name in names:
            try:
                value = defaults.get_editor_property(name)
            except Exception:  # the variable does not exist on this class
                continue
            found[name] = str(value) if isinstance(value, (unreal.Text, unreal.Name)) else value
        return found

    @staticmethod
    def _read_ship(ship: str):
        blueprint = AetheriaCombatToolset._blueprint(core.ship_path(ship))
        if blueprint is None:
            return None
        cls = blueprint.generated_class()
        base = unreal.load_class(None, f"{core.ship_path(core.BASE_SHIP)}.{core.BASE_SHIP}_C")
        return {
            "is_ship": base is not None and unreal.MathLibrary.class_is_child_of(cls, base),
            "properties": AetheriaCombatToolset._properties(blueprint, core.read_names(ship)),
            "tags": [str(t) for t in unreal.get_default_object(cls).get_editor_property("tags")],
            "meshes": AetheriaCombatToolset._component_meshes(blueprint),
        }

    @staticmethod
    def _component_meshes(blueprint) -> dict:
        subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
        library = unreal.SubobjectDataBlueprintFunctionLibrary
        meshes = {}
        for handle in subsystem.k2_gather_subobject_data_for_blueprint(blueprint):
            data = library.get_data(handle)
            component = library.get_object_for_blueprint(data, blueprint)
            if isinstance(component, unreal.StaticMeshComponent):
                mesh = component.get_editor_property("static_mesh")
                meshes[str(library.get_variable_name(data))] = mesh.get_name() if mesh else None
        return meshes

    @staticmethod
    def _describe(actor) -> dict:
        kinds = ((unreal.TargetPoint, core.TARGET_POINT), (unreal.PlayerStart, core.PLAYER_START_KIND),
                 (unreal.DirectionalLight, core.DIRECTIONAL_LIGHT), (unreal.PostProcessVolume, core.POST_PROCESS))
        class_name = AetheriaCombatToolset._class_name(actor.get_class())
        kind = next((k for cls, k in kinds if isinstance(actor, cls)), class_name)
        if class_name == "BP_CombatManager":
            kind = core.COMBAT_MANAGER
        location = actor.get_actor_location()
        described = {"kind": kind, "label": actor.get_actor_label(), "tags": [str(t) for t in actor.get_editor_property("tags")],
                     "location": (location.x, location.y, location.z)}
        if kind == core.POST_PROCESS:
            described["unbound"] = actor.get_editor_property("unbound")
        return described

    @staticmethod
    def _class_name(cls) -> str:
        name = cls.get_name()
        return name[:-2] if name.endswith("_C") else name
