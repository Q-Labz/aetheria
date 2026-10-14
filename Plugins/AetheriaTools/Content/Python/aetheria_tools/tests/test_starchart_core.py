"""Engine-independent tests for starchart_core, run against the project's chart CSVs:
    python -m unittest discover -s Plugins/AetheriaTools/Content/Python -p "test_*_core.py" -t Plugins/AetheriaTools/Content/Python
"""
import csv
import os
import unittest

from aetheria_tools import starchart_core as core

BUILD_DATA = os.path.abspath(os.path.join(os.path.dirname(__file__), *[".."] * 6, "BuildData"))


def read_rows(path):
    with open(path, newline="", encoding="utf-8") as f:
        return [{"Name": r.pop("---"), **r} for r in csv.DictReader(f)]


def load_chart(nodes, routes):
    return core.build_chart(read_rows(os.path.join(BUILD_DATA, nodes)),
                            read_rows(os.path.join(BUILD_DATA, routes)))


class SliceChartTest(unittest.TestCase):
    def setUp(self):
        self.chart = load_chart("DT_StarNodes.csv", "DT_LeapRoutes.csv")

    def test_slice_chart_is_valid(self):
        self.assertEqual(core.validate(self.chart, "ArchWake"), [])

    def test_direct_leap_overflows_as_specified(self):
        cost, projected, after, ruptured, _ = core.leap(self.chart, "ArchWake", "EmberRelay", 25, set(), "SynodEnvoy")
        self.assertEqual((cost, projected, after, ruptured), (80, 105, 60, True))

    def test_strain_charting_and_guild_modifiers(self):
        strained = core.leap_cost(self.chart, "LanternDrift", "EmberRelay", 60, set(), "SynodEnvoy")
        charted = core.leap_cost(self.chart, "LanternDrift", "EmberRelay", 60, {"LanternDrift_EmberRelay"}, "SynodEnvoy")
        guild = core.leap_cost(self.chart, "ArchWake", "LanternDrift", 25, set(), "GuildEngineer")
        self.assertEqual((strained, charted, guild), (37.5, 18.75, 31.5))

    def test_plan_route_avoids_rupture_via_lantern_drift(self):
        plan = core.plan_route(self.chart, "ArchWake", "EmberRelay", 25, set(), "SynodEnvoy")
        self.assertEqual(plan["nodes"], ["ArchWake", "LanternDrift", "EmberRelay"])
        self.assertEqual((plan["ruptures"], plan["bleeds"], plan["final_instability"]), (0, 0, 97.5))

    def test_plan_route_counts_containment_bleed(self):
        plan = core.plan_route(self.chart, "ArchWake", "LanternDrift", 85, set(), "SynodEnvoy")
        self.assertEqual(plan["bleeds"], 1)

    def test_plan_route_raises_for_unknown_node(self):
        with self.assertRaises(KeyError):
            core.plan_route(self.chart, "ArchWake", "Nowhere", 25, set(), "SynodEnvoy")

    def test_plan_route_raises_for_same_start_and_goal(self):
        with self.assertRaises(ValueError):
            core.plan_route(self.chart, "ArchWake", "ArchWake", 25, set(), "SynodEnvoy")

    def test_leap_raises_without_lane(self):
        with self.assertRaises(KeyError):
            core.leap(self.chart, "LanternDrift", "LanternDrift", 25, set(), "SynodEnvoy")

    def test_leap_raises_for_unknown_background(self):
        with self.assertRaises(ValueError):
            core.leap_cost(self.chart, "ArchWake", "LanternDrift", 25, set(), "Pirate")


class ActSevenChartTest(unittest.TestCase):
    def setUp(self):
        self.chart = load_chart("ActVII/DT_StarNodes_ActVII.csv", "ActVII/DT_LeapRoutes_ActVII.csv")

    def test_act_vii_chart_is_valid(self):
        self.assertEqual(core.validate(self.chart, "EmberRelay"), [])

    def test_ash_storm_surcharge_unless_charted(self):
        stormy = core.leap_cost(self.chart, "AshfallVerge", "GlassSea", 0, set(), "SynodEnvoy")
        charted = core.leap_cost(self.chart, "AshfallVerge", "GlassSea", 0, {"AshfallVerge_GlassSea"}, "SynodEnvoy")
        calm = core.leap_cost(self.chart, "GlassSea", "AshfallVerge", 0, set(), "SynodEnvoy")
        self.assertEqual((stormy, charted, calm), (35, 12.5, 35))

    def test_vault_needs_management_from_slice_end_state(self):
        """From the worst slice end state (97.5), the Vault cannot be reached rupture-free
        without venting or holding, which is the intended pressure."""
        plan = core.plan_route(self.chart, "EmberRelay", "VaultOfRecords", 97.5, set(), "SynodEnvoy")
        self.assertGreater(plan["ruptures"], 0)

    def test_vault_forces_drive_management_even_from_zero(self):
        """Uncharted, every route to the Vault ruptures even from a cold drive."""
        plan = core.plan_route(self.chart, "EmberRelay", "VaultOfRecords", 0, set(), "SynodEnvoy")
        self.assertEqual(plan["ruptures"], 1)

    def test_charting_two_lanes_makes_the_vault_reachable_cleanly(self):
        charted = {"GlassSea_CinderArchive", "CinderArchive_PyreGate"}
        plan = core.plan_route(self.chart, "EmberRelay", "VaultOfRecords", 0, charted, "SynodEnvoy")
        self.assertEqual(plan["nodes"], ["EmberRelay", "GlassSea", "CinderArchive", "PyreGate", "VaultOfRecords"])
        self.assertEqual(plan["ruptures"], 0)
        self.assertAlmostEqual(plan["final_instability"], 94.375)


class ValidationTest(unittest.TestCase):
    def test_reports_structural_problems(self):
        nodes = [{"Name": "A", "NodeID": "A", "Encounter": "None"},
                 {"Name": "B", "NodeID": "B", "Encounter": "Ambush"},
                 {"Name": "C", "NodeID": "X", "Encounter": "E_Encounter::Combat"}]
        routes = [{"Name": "A_B", "FromNode": "A", "ToNode": "B", "BaseCost": "30"},
                  {"Name": "B_A", "FromNode": "B", "ToNode": "A", "BaseCost": "25"},
                  {"Name": "Wrong", "FromNode": "A", "ToNode": "Q", "BaseCost": "-5"},
                  {"Name": "B_C", "FromNode": "B", "ToNode": "C", "BaseCost": "abc"}]
        problems = "\n".join(core.validate(core.build_chart(nodes, routes), "A"))
        for expected in ("unknown encounter 'Ambush'", "NodeID 'X' does not match", "costs differ by direction",
                         "row name should be 'A_Q'", "unknown node 'Q'", "BaseCost must be positive",
                         "no return lane", "'abc' is not a number", "Node 'X': unreachable"):
            self.assertIn(expected, problems)

    def test_enum_prefix_is_stripped(self):
        self.assertEqual(core.enum_value("E_Encounter::Council"), "Council")

    def test_unknown_start_node(self):
        chart = core.build_chart([{"Name": "A", "NodeID": "A"}], [])
        self.assertIn("Start node 'Z' does not exist.", core.validate(chart, "Z"))


if __name__ == "__main__":
    unittest.main()
