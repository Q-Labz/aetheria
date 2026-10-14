import unreal
import toolset_registry

from aetheria_tools import starchart_core as core

NODE_COLUMNS = ("NodeID", "Encounter", "HazardSurcharge")
ROUTE_COLUMNS = ("FromNode", "ToNode", "BaseCost")


@unreal.ustruct()
class LeapCost(unreal.StructBase):
    """One lane's Stellar Leap cost from a given drive state."""

    route = unreal.uproperty(str, meta={"ToolTip": "Route row name, From_To."})
    base_cost = unreal.uproperty(float)
    cost = unreal.uproperty(float, meta={"ToolTip": "After hazard, charting, strain and background modifiers."})
    projected_instability = unreal.uproperty(float, meta={"ToolTip": "Instability on arrival before any rupture, 0-100 scale."})
    overflows = unreal.uproperty(bool, meta={"ToolTip": "True if the leap ruptures the Star-Drive (projected above 100)."})


@unreal.ustruct()
class LeapPlan(unreal.StructBase):
    """The safest path between two nodes and the instability along it."""

    nodes = unreal.uproperty(unreal.Array(str), meta={"ToolTip": "Node IDs from start to goal."})
    instability_after = unreal.uproperty(unreal.Array(float), meta={"ToolTip": "Instability after each leap, post-rupture reset applied."})
    ruptures = unreal.uproperty(int)
    containment_bleeds = unreal.uproperty(int, meta={"ToolTip": "Leaps started at Critical (80+), each injuring one crew member."})
    final_instability = unreal.uproperty(float)


@unreal.uclass()
class AetheriaStarChartToolset(unreal.ToolsetDefinition):
    """Checks and simulates Aetheria star charts: the star-node and leap-route DataTables the
    Stellar Leap screen reads. Validates chart structure and computes leap costs, Star-Drive
    ruptures and safest routes using the game's instability rules."""

    @toolset_registry.tool_call
    @staticmethod
    def validate_star_chart(nodes_table: unreal.DataTable, routes_table: unreal.DataTable,
                            start_node: str) -> list[str]:
        """Checks a chart for unknown or unreachable nodes, malformed or duplicate lanes, missing
        return lanes, direction-dependent costs and bad values.

        Args:
            nodes_table: Table of S_StarNode rows.
            routes_table: Table of S_LeapRoute rows.
            start_node: Node the act starts at; reachability is measured from it.

        Returns:
            Problems found. Empty when the chart is valid.
        """
        chart = AetheriaStarChartToolset._chart(nodes_table, routes_table)
        return core.validate(chart, start_node)

    @toolset_registry.tool_call
    @staticmethod
    def get_leap_costs(nodes_table: unreal.DataTable, routes_table: unreal.DataTable,
                       instability: float, charted_routes: list[str], background: str) -> list[LeapCost]:
        """Cost of every lane in the chart if leapt from the given drive state.

        Args:
            nodes_table: Table of S_StarNode rows.
            routes_table: Table of S_LeapRoute rows.
            instability: Current Star-Drive instability, 0-100.
            charted_routes: Route row names Lyra has charted.
            background: Captain background (E_Background value).

        Returns:
            One entry per lane, in table order.
        """
        chart = AetheriaStarChartToolset._chart(nodes_table, routes_table)
        AetheriaStarChartToolset._check_instability(instability)
        results = []
        for name, route in chart.routes.items():
            cost, projected, _, ruptured, _ = AetheriaStarChartToolset._leap(
                chart, route["FromNode"], route["ToNode"], instability, set(charted_routes), background)
            results.append(LeapCost(route=name, base_cost=route["BaseCost"], cost=cost,
                                    projected_instability=projected, overflows=ruptured))
        return results

    @toolset_registry.tool_call
    @staticmethod
    def plan_route(nodes_table: unreal.DataTable, routes_table: unreal.DataTable, start_node: str,
                   goal_node: str, instability: float, charted_routes: list[str],
                   background: str) -> LeapPlan:
        """Finds the safest path between two nodes: fewest ruptures, then fewest leaps started at
        Critical, then lowest final instability, then fewest leaps. No venting or holding station
        is assumed between leaps.

        Args:
            nodes_table: Table of S_StarNode rows.
            routes_table: Table of S_LeapRoute rows.
            start_node: Node ID to leap from.
            goal_node: Node ID to reach.
            instability: Star-Drive instability at the start, 0-100.
            charted_routes: Route row names Lyra has charted.
            background: Captain background (E_Background value).

        Returns:
            The chosen path and the drive state along it.
        """
        chart = AetheriaStarChartToolset._chart(nodes_table, routes_table)
        AetheriaStarChartToolset._check_instability(instability)
        try:
            plan = core.plan_route(chart, start_node, goal_node, instability, set(charted_routes), background)
        except KeyError as error:
            raise ValueError(error.args[0]) from None
        return LeapPlan(nodes=plan["nodes"], instability_after=plan["instability_after"],
                        ruptures=plan["ruptures"], containment_bleeds=plan["bleeds"],
                        final_instability=plan["final_instability"])

    @staticmethod
    def _chart(nodes_table: unreal.DataTable, routes_table: unreal.DataTable) -> core.Chart:
        node_rows = AetheriaStarChartToolset._rows(nodes_table, NODE_COLUMNS, optional=("HazardSurcharge",))
        route_rows = AetheriaStarChartToolset._rows(routes_table, ROUTE_COLUMNS, optional=())
        return core.build_chart(node_rows, route_rows)

    @staticmethod
    def _rows(table: unreal.DataTable, columns: tuple, optional: tuple) -> list:
        if table is None:
            raise ValueError("A DataTable is required.")
        names = [str(n) for n in unreal.DataTableFunctionLibrary.get_data_table_row_names(table)]
        rows = [{"Name": n} for n in names]
        for column in columns:
            values = list(unreal.DataTableFunctionLibrary.get_data_table_column_as_string(table, column))
            if len(values) == len(names) + 1:  # some engine versions prepend the column title
                values = values[1:]
            if len(values) != len(names):
                if column in optional:
                    continue
                raise ValueError(f"{table.get_path_name()} has no '{column}' column.")
            for row, value in zip(rows, values):
                row[column] = value
        return rows

    @staticmethod
    def _leap(chart, from_node, to_node, instability, charted, background):
        try:
            return core.leap(chart, from_node, to_node, instability, charted, background)
        except KeyError as error:
            raise ValueError(error.args[0]) from None

    @staticmethod
    def _check_instability(instability: float) -> None:
        if not 0.0 <= instability <= core.INSTABILITY_MAX:
            raise ValueError("instability must be between 0 and 100.")
