"""In-editor tests for AetheriaStarChartToolset, run against the slice tables imported in phase 2."""
import unreal
from toolset_registry.tests.tool_call_test_case import ToolCallTestCase  # confirm path against engine tests

from aetheria_tools.starchart_toolset import AetheriaStarChartToolset

NODES = "/Game/Aetheria/Data/DT_StarNodes"
ROUTES = "/Game/Aetheria/Data/DT_LeapRoutes"


class AetheriaStarChartToolsetTestCase(ToolCallTestCase):
    """Test AetheriaStarChartToolset."""

    def setUp(self):
        super().setUp()
        self.nodes = unreal.EditorAssetLibrary.load_asset(NODES)
        self.routes = unreal.EditorAssetLibrary.load_asset(ROUTES)

    def test_validate_star_chart_clean_for_slice(self):
        """The imported slice chart has no problems."""
        self.assertEqual(AetheriaStarChartToolset.validate_star_chart(self.nodes, self.routes, "ArchWake"), [])

    def test_validate_star_chart_reports_unknown_start(self):
        """Reports a start node that is not in the chart."""
        problems = AetheriaStarChartToolset.validate_star_chart(self.nodes, self.routes, "Nowhere")
        self.assertIn("Start node 'Nowhere' does not exist.", problems)

    def test_get_leap_costs_flags_direct_overflow(self):
        """Every lane is costed and the direct Ember Relay leap overflows at 25."""
        costs = {c.route: c for c in AetheriaStarChartToolset.get_leap_costs(
            self.nodes, self.routes, 25.0, [], "SynodEnvoy")}
        self.assertEqual(len(costs), 6)
        self.assertTrue(costs["ArchWake_EmberRelay"].overflows)
        self.assertFalse(costs["ArchWake_LanternDrift"].overflows)

    def test_plan_route_goes_through_lantern_drift(self):
        """The safest slice route avoids the rupture."""
        plan = AetheriaStarChartToolset.plan_route(
            self.nodes, self.routes, "ArchWake", "EmberRelay", 25.0, [], "SynodEnvoy")
        self.assertEqual(list(plan.nodes), ["ArchWake", "LanternDrift", "EmberRelay"])
        self.assertEqual(plan.ruptures, 0)

    def test_plan_route_raises_for_unknown_node(self):
        """Raises for a goal outside the chart."""
        with self.assertToolRaisesRuntimeError():
            AetheriaStarChartToolset.plan_route(self.nodes, self.routes, "ArchWake", "Nowhere", 25.0, [], "SynodEnvoy")

    def test_get_leap_costs_raises_for_out_of_range_instability(self):
        """Raises when instability is outside 0-100."""
        with self.assertToolRaisesRuntimeError():
            AetheriaStarChartToolset.get_leap_costs(self.nodes, self.routes, 140.0, [], "SynodEnvoy")

    def test_get_leap_costs_raises_for_unknown_background(self):
        """Raises for a background that does not exist."""
        with self.assertToolRaisesRuntimeError():
            AetheriaStarChartToolset.get_leap_costs(self.nodes, self.routes, 25.0, [], "Pirate")

    def test_validate_star_chart_raises_without_table(self):
        """Raises when a table is missing."""
        with self.assertToolRaisesRuntimeError():
            AetheriaStarChartToolset.validate_star_chart(None, self.routes, "ArchWake")
