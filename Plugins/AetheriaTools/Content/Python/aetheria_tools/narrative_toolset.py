import os

import unreal
import toolset_registry

from aetheria_tools import narrative_core as core

STRUCT_FOLDER = "/Game/Aetheria/Data"


@unreal.ustruct()
class NarrativeTableInfo(unreal.StructBase):
    """A narrative DataTable after import."""

    table = unreal.uproperty(str, meta={"ToolTip": "Object path of the DataTable asset."})
    row_count = unreal.uproperty(int)


@unreal.ustruct()
class ArchitectOutcome(unreal.StructBase):
    """One path through the Ember Relay and Directive councils and where it leaves the Architect."""

    background = unreal.uproperty(str, meta={"ToolTip": "Captain background (E_Background value)."})
    relay = unreal.uproperty(str, meta={"ToolTip": "Row chosen in DT_Council_Relay."})
    directive = unreal.uproperty(str, meta={"ToolTip": "Row chosen in DT_Council_Directive."})
    leaning = unreal.uproperty(int, meta={"ToolTip": "ArchitectLeaning after both choices, -100 to 100."})
    disposition = unreal.uproperty(str, meta={"ToolTip": "Custodian, Dominion or Undetermined."})


@unreal.ustruct()
class NarrativeLine(unreal.StructBase):
    """A dialogue line as the player sees it."""

    speaker = unreal.uproperty(str, meta={"ToolTip": "E_Speaker value."})
    text = unreal.uproperty(str)


@unreal.ustruct()
class CampaignState(unreal.StructBase):
    """The BP_AetheriaGameInstance values the councils read or change."""

    background = unreal.uproperty(str, meta={"ToolTip": "Captain background (E_Background value)."})
    instability = unreal.uproperty(float, meta={"ToolTip": "Star-Drive instability, 0-100."})
    chrono_charge = unreal.uproperty(float, meta={"ToolTip": "Chrono-Core charge, 0-100."})
    phantom_ships_active = unreal.uproperty(int)
    spire_recalibrations = unreal.uproperty(int)
    architect_leaning = unreal.uproperty(int, meta={"ToolTip": "-100 (Dominion) to 100 (Custodian)."})
    crew_loyalty = unreal.uproperty(unreal.Map(str, int), meta={"ToolTip": "E_Speaker value to loyalty, -100 to 100."})
    story_flags = unreal.uproperty(unreal.Array(str))


@unreal.ustruct()
class CouncilPathPreview(unreal.StructBase):
    """Where a pair of council choices leaves the campaign."""

    disposition = unreal.uproperty(str, meta={"ToolTip": "Custodian, Dominion or Undetermined."})
    state = unreal.uproperty(CampaignState, meta={"ToolTip": "After both options and the Architect's consequence."})


@unreal.uclass()
class AetheriaNarrativeToolset(unreal.ToolsetDefinition):
    """Aetheria's story content: imports the dialogue and council DataTables from the
    project's BuildData/Narrative JSON, validates that content against the slice's rules,
    and previews dialogue and Core Architect outcomes."""

    @toolset_registry.tool_call
    @staticmethod
    def load_narrative_tables(destination: str = STRUCT_FOLDER) -> list[NarrativeTableInfo]:
        """Creates or overwrites DT_Sequences, DT_Council_Relay and DT_Council_Directive from
        BuildData/Narrative/*.json, then saves them. Nothing is written if the source has problems.

        Args:
            destination: Content folder for the tables. Row structs are always read from
                /Game/Aetheria/Data.

        Returns:
            The imported tables.
        """
        data = AetheriaNarrativeToolset._load_valid_source()
        results = []
        for table, (struct_name, _) in core.TABLES.items():
            data_table = AetheriaNarrativeToolset._get_or_create_table(destination, table, struct_name)
            json_rows = core.to_import_json(data[table])
            if not unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(data_table, json_rows):
                raise RuntimeError(f"Import into {data_table.get_path_name()} failed; see the Output Log.")
            if not unreal.EditorAssetLibrary.save_loaded_asset(data_table):
                raise RuntimeError(f"Could not save {data_table.get_path_name()}.")
            row_names = unreal.DataTableFunctionLibrary.get_data_table_row_names(data_table)
            results.append(NarrativeTableInfo(table=data_table.get_path_name(), row_count=len(row_names)))
        return results

    @toolset_registry.tool_call
    @staticmethod
    def validate_narrative(destination: str = STRUCT_FOLDER) -> list[str]:
        """Checks the narrative source and the imported tables.

        Covers unknown speakers or backgrounds, flags nothing sets, missing or duplicate rows,
        and required rows absent from the imported tables.

        Args:
            destination: Content folder holding the imported tables.

        Returns:
            Problems found. Empty when everything is valid.
        """
        data = core.load_source(AetheriaNarrativeToolset._source_dir())
        problems = core.validate(data)
        for table, (_, required) in core.TABLES.items():
            asset_path = f"{destination}/{table}"
            if not unreal.EditorAssetLibrary.does_asset_exist(asset_path):
                problems.append(f"{asset_path}: not imported yet.")
                continue
            data_table = unreal.EditorAssetLibrary.load_asset(asset_path)
            present = {str(n) for n in unreal.DataTableFunctionLibrary.get_data_table_row_names(data_table)}
            problems.extend(f"{asset_path}: missing row '{r}'." for r in required if r not in present)
        return problems

    @toolset_registry.tool_call
    @staticmethod
    def get_architect_outcomes() -> list[ArchitectOutcome]:
        """Lists every council path available to each background and the Architect disposition
        it produces, read from the narrative source.

        Returns:
            One entry per background, relay option and directive option combination.
        """
        data = AetheriaNarrativeToolset._load_valid_source()
        return [ArchitectOutcome(**o) for o in core.architect_outcomes(data)]

    @toolset_registry.tool_call
    @staticmethod
    def preview_sequence(sequence: str, background: str, flags: list[str],
                         captain_name: str) -> list[NarrativeLine]:
        """Resolves a dialogue sequence the way WBP_Dialogue shows it in game.

        Args:
            sequence: DT_Sequences row name.
            background: E_Background value of the Captain.
            flags: Story flags currently set; lines requiring any other flag are skipped.
            captain_name: Replaces {Captain} in the text.

        Returns:
            The lines that would be shown, in order.
        """
        data = AetheriaNarrativeToolset._load_valid_source()
        try:
            lines = core.resolve_sequence(data, sequence, background, set(flags), captain_name)
        except KeyError as error:
            raise ValueError(error.args[0]) from None
        return [NarrativeLine(speaker=s, text=t) for s, t in lines]

    @toolset_registry.tool_call
    @staticmethod
    def get_starting_campaign_state(background: str) -> CampaignState:
        """Campaign state right after captain creation, including the background's loyalty bonus.

        Args:
            background: E_Background value of the Captain.
        """
        return CampaignState(**core.starting_state(background))

    @toolset_registry.tool_call
    @staticmethod
    def preview_council_path(state: CampaignState, relay: str, directive: str) -> CouncilPathPreview:
        """Applies a Relay council choice, a Directive council choice and the resulting Architect
        consequence to a campaign state, as the game would, using the narrative source.

        Args:
            state: Campaign state on arrival at the Ember Relay council.
            relay: DT_Council_Relay row name. Must be offered to the state's background.
            directive: DT_Council_Directive row name.
        """
        data = AetheriaNarrativeToolset._load_valid_source()
        before = {
            "background": state.background, "instability": state.instability,
            "chrono_charge": state.chrono_charge, "phantom_ships_active": state.phantom_ships_active,
            "spire_recalibrations": state.spire_recalibrations, "architect_leaning": state.architect_leaning,
            "crew_loyalty": {str(k): v for k, v in state.crew_loyalty.items()},
            "story_flags": [str(f) for f in state.story_flags],
        }
        try:
            after = core.council_path(data, before, relay, directive)
        except (KeyError, ValueError) as error:
            raise ValueError(error.args[0]) from None
        return CouncilPathPreview(disposition=core.disposition(after["architect_leaning"]),
                                  state=CampaignState(**after))

    @staticmethod
    def _source_dir() -> str:
        project = unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())
        return os.path.join(project, "BuildData", "Narrative")

    @staticmethod
    def _load_valid_source() -> dict:
        data = core.load_source(AetheriaNarrativeToolset._source_dir())
        problems = core.validate(data)
        if problems:
            raise ValueError("Narrative source is invalid:\n" + "\n".join(problems))
        return data

    @staticmethod
    def _get_or_create_table(destination: str, table: str, struct_name: str) -> unreal.DataTable:
        asset_path = f"{destination}/{table}"
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
            return unreal.EditorAssetLibrary.load_asset(asset_path)
        struct_path = f"{STRUCT_FOLDER}/{struct_name}"
        row_struct = unreal.EditorAssetLibrary.load_asset(struct_path)
        if row_struct is None:
            raise ValueError(f"Row struct {struct_path} does not exist; create it first (CLAUDE.md phase 2).")
        factory = unreal.DataTableFactory()
        factory.set_editor_property("struct", row_struct)
        data_table = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            table, destination, unreal.DataTable, factory)
        if data_table is None:
            raise RuntimeError(f"Could not create {asset_path}.")
        return data_table
