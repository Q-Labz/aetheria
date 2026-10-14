"""In-editor tests for AetheriaCombatToolset, run against the slice's ship Blueprints and combat
level (CLAUDE.md phases 5-6). The combat-level tests open L_Combat_LanternDrift."""
import unreal
from toolset_registry.tests.tool_call_test_case import ToolCallTestCase  # confirm path against engine tests

from aetheria_tools.combat_toolset import AetheriaCombatToolset

COMBAT_LEVEL = "/Game/Aetheria/Maps/L_Combat_LanternDrift"
COMMAND_LEVEL = "/Game/Aetheria/Maps/L_Command"


class AetheriaCombatToolsetTestCase(ToolCallTestCase):
    """Test AetheriaCombatToolset."""

    def test_validate_fleet_clean_for_slice(self):
        """The slice's ship Blueprints match fleet canon."""
        self.assertEqual(AetheriaCombatToolset.validate_fleet(), [])

    def test_get_ship_stats_reads_flagship(self):
        """Reads the flagship's stats, beam stats included."""
        stats = {s.stat: s for s in AetheriaCombatToolset.get_ship_stats("BP_Flagship")}
        self.assertIn("BeamDamage", stats)
        self.assertEqual(stats["BeamDamage"].spec_value, 900.0)

    def test_get_ship_stats_raises_for_unknown_ship(self):
        """Raises for a ship outside the slice."""
        with self.assertToolRaisesRuntimeError():
            AetheriaCombatToolset.get_ship_stats("BP_Wayward")

    def test_validate_combat_level_clean_for_slice(self):
        """The combat map's layout matches the spec."""
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(COMBAT_LEVEL)
        self.assertEqual(AetheriaCombatToolset.validate_combat_level(), [])

    def test_validate_combat_level_reports_non_combat_level(self):
        """Reports problems for a level that is not set up for combat."""
        unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(COMMAND_LEVEL)
        problems = AetheriaCombatToolset.validate_combat_level()
        self.assertIn("GameMode Override is 'GM_Command', expected 'GM_Tactical'.", problems)
