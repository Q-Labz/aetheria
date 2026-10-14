"""In-editor tests for AetheriaNarrativeToolset. Requires S_Sequence and S_CouncilOption in
/Game/Aetheria/Data. Imported tables go to a scratch folder that is deleted afterwards."""
import unreal
from toolset_registry.tests.tool_call_test_case import ToolCallTestCase  # confirm path against engine tests

from aetheria_tools.narrative_toolset import AetheriaNarrativeToolset

SCRATCH = "/Game/Aetheria/Tests/Narrative"


class AetheriaNarrativeToolsetTestCase(ToolCallTestCase):
    """Test AetheriaNarrativeToolset."""

    def tearDown(self):
        if unreal.EditorAssetLibrary.does_directory_exist(SCRATCH):
            unreal.EditorAssetLibrary.delete_directory(SCRATCH)
        super().tearDown()

    def test_load_narrative_tables_imports_all_rows(self):
        """Creates the three tables with the rows the slice needs."""
        infos = AetheriaNarrativeToolset.load_narrative_tables(SCRATCH)
        counts = {info.table.rsplit(".", 1)[-1]: info.row_count for info in infos}
        self.assertEqual(counts, {"DT_Sequences": 8, "DT_Council_Relay": 4, "DT_Council_Directive": 3})

    def test_load_narrative_tables_overwrites_existing(self):
        """Re-importing replaces rows rather than duplicating them."""
        AetheriaNarrativeToolset.load_narrative_tables(SCRATCH)
        infos = AetheriaNarrativeToolset.load_narrative_tables(SCRATCH)
        self.assertEqual(sum(info.row_count for info in infos), 15)

    def test_imported_line_keeps_enum_and_override(self):
        """Speaker enum values and background overrides survive the JSON import."""
        AetheriaNarrativeToolset.load_narrative_tables(SCRATCH)
        table = unreal.EditorAssetLibrary.load_asset(f"{SCRATCH}/DT_Sequences")
        speakers = unreal.DataTableFunctionLibrary.get_data_table_column_as_string(table, "Lines")
        self.assertIn("TalaVane", speakers[0])
        self.assertIn("CoalitionVanguard", speakers[0])

    def test_validate_narrative_clean_after_import(self):
        """Reports no problems once the tables are imported."""
        AetheriaNarrativeToolset.load_narrative_tables(SCRATCH)
        self.assertEqual(AetheriaNarrativeToolset.validate_narrative(SCRATCH), [])

    def test_validate_narrative_reports_missing_tables(self):
        """Reports tables that have not been imported."""
        problems = AetheriaNarrativeToolset.validate_narrative(SCRATCH)
        self.assertEqual(len(problems), 3)
        self.assertTrue(all("not imported yet" in p for p in problems))

    def test_get_architect_outcomes_reaches_every_disposition(self):
        """Both branches of the Architect are reachable."""
        dispositions = {o.disposition for o in AetheriaNarrativeToolset.get_architect_outcomes()}
        self.assertEqual(dispositions, {"Custodian", "Dominion", "Undetermined"})

    def test_preview_sequence_resolves_lines(self):
        """Applies flags, background overrides and the Captain's name."""
        lines = AetheriaNarrativeToolset.preview_sequence(
            "ArchitectReveal", "SynodEnvoy", ["BeamAnomaly"], "Rhen")
        self.assertEqual(len(lines), 4)
        self.assertEqual(lines[0].speaker, "CoreArchitect")

    def test_preview_sequence_raises_on_unknown_sequence(self):
        """Raises for a sequence that does not exist."""
        with self.assertToolRaisesRuntimeError():
            AetheriaNarrativeToolset.preview_sequence("Nope", "SynodEnvoy", [], "Rhen")

    def test_preview_sequence_raises_on_unknown_background(self):
        """Raises for a background that does not exist."""
        with self.assertToolRaisesRuntimeError():
            AetheriaNarrativeToolset.preview_sequence("Briefing", "Pirate", [], "Rhen")
