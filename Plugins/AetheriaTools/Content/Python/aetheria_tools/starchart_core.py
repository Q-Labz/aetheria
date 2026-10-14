"""Engine-independent star chart rules: leap costs, ruptures, validation and route planning.

Leap rules mirror BP_AetheriaGameInstance (SLICE_SPEC §4). Rows are plain dicts keyed by the
DataTable column names plus "Name" for the row name.
"""
from dataclasses import dataclass, field

from aetheria_tools.narrative_core import BACKGROUNDS

ENCOUNTERS = frozenset({"None", "Combat", "Council", "Event"})
INSTABILITY_MAX = 100.0
STRAIN_THRESHOLD = 50.0
CRITICAL_THRESHOLD = 80.0
POST_RUPTURE_INSTABILITY = 60.0
CHARTED_MULTIPLIER = 0.5
STRAIN_MULTIPLIER = 1.25
GUILD_MULTIPLIER = 0.9


@dataclass
class Chart:
    nodes: dict          # NodeID -> {"Encounter": str, "HazardSurcharge": float}
    routes: dict         # route name -> {"FromNode", "ToNode", "BaseCost"}
    problems: list = field(default_factory=list)  # parse problems found while building

    def route(self, from_node, to_node):
        return self.routes.get(route_name(from_node, to_node))

    def neighbours(self, node):
        return [r["ToNode"] for r in self.routes.values() if r["FromNode"] == node]


def route_name(from_node, to_node):
    return f"{from_node}_{to_node}"


def build_chart(node_rows, route_rows):
    """Builds a Chart from raw rows, recording unparseable values as problems."""
    problems = []
    nodes = {}
    for row in node_rows:
        node_id = str(row.get("NodeID") or row["Name"])
        if node_id != row["Name"]:
            problems.append(f"Node row '{row['Name']}': NodeID '{node_id}' does not match the row name.")
        encounter = enum_value(row.get("Encounter", "None"))
        if encounter not in ENCOUNTERS:
            problems.append(f"Node '{node_id}': unknown encounter '{encounter}'.")
        hazard = _number(row.get("HazardSurcharge", 0), f"Node '{node_id}' HazardSurcharge", problems)
        if hazard < 0:
            problems.append(f"Node '{node_id}': HazardSurcharge must not be negative.")
        nodes[node_id] = {"Encounter": encounter, "HazardSurcharge": max(hazard, 0.0)}

    routes = {}
    for row in route_rows:
        name = row["Name"]
        route = {"FromNode": str(row["FromNode"]), "ToNode": str(row["ToNode"]),
                 "BaseCost": _number(row.get("BaseCost"), f"Route '{name}' BaseCost", problems)}
        routes[name] = route
    return Chart(nodes, routes, problems)


def validate(chart, start_node):
    """Returns human-readable problems with the chart. Empty means valid."""
    problems = list(chart.problems)
    if start_node not in chart.nodes:
        problems.append(f"Start node '{start_node}' does not exist.")

    seen_pairs = set()
    for name, r in chart.routes.items():
        a, b, cost = r["FromNode"], r["ToNode"], r["BaseCost"]
        if name != route_name(a, b):
            problems.append(f"Route '{name}': row name should be '{route_name(a, b)}'.")
        for end in (a, b):
            if end not in chart.nodes:
                problems.append(f"Route '{name}': unknown node '{end}'.")
        if a == b:
            problems.append(f"Route '{name}': leads back to its own node.")
        if cost <= 0:
            problems.append(f"Route '{name}': BaseCost must be positive.")
        if (a, b) in seen_pairs:
            problems.append(f"Route '{name}': duplicate lane {a} → {b}.")
        seen_pairs.add((a, b))
        reverse = chart.route(b, a)
        if reverse is None:
            problems.append(f"Route '{name}': no return lane {b} → {a}.")
        elif reverse["BaseCost"] != cost and a < b:
            problems.append(f"Lane {a} ↔ {b}: costs differ by direction ({cost} vs {reverse['BaseCost']}).")

    if start_node in chart.nodes:
        reachable = _reachable(chart, start_node)
        for node in sorted(set(chart.nodes) - reachable):
            problems.append(f"Node '{node}': unreachable from '{start_node}'.")
    return problems


def leap_cost(chart, from_node, to_node, instability, charted, background):
    """Effective cost of one leap, applying hazard, charting, strain and background modifiers."""
    route = chart.route(from_node, to_node)
    if route is None:
        raise KeyError(f"No lane from '{from_node}' to '{to_node}'.")
    if background not in BACKGROUNDS:
        raise ValueError(f"Unknown background '{background}'. Known: {', '.join(BACKGROUNDS)}.")
    cost = route["BaseCost"]
    if route_name(from_node, to_node) in charted:
        cost *= CHARTED_MULTIPLIER
    else:
        cost += chart.nodes.get(from_node, {}).get("HazardSurcharge", 0.0)
    if instability >= STRAIN_THRESHOLD:
        cost *= STRAIN_MULTIPLIER
    if background == "GuildEngineer":
        cost *= GUILD_MULTIPLIER
    return cost


def leap(chart, from_node, to_node, instability, charted, background):
    """Simulates one leap. Returns (cost, projected, instability_after, ruptured, containment_bleed)."""
    cost = leap_cost(chart, from_node, to_node, instability, charted, background)
    projected = instability + cost
    ruptured = projected > INSTABILITY_MAX
    after = POST_RUPTURE_INSTABILITY if ruptured else projected
    return cost, projected, after, ruptured, instability >= CRITICAL_THRESHOLD


def plan_route(chart, start, goal, instability, charted, background):
    """Safest simple path from start to goal.

    Ranks by fewest ruptures, then fewest containment-bleed leaps, then lowest final
    instability, then fewest leaps. Returns a dict with nodes, instability_after,
    ruptures, bleeds and final_instability.
    """
    for node in (start, goal):
        if node not in chart.nodes:
            raise KeyError(f"Unknown node '{node}'.")
    if background not in BACKGROUNDS:
        raise ValueError(f"Unknown background '{background}'. Known: {', '.join(BACKGROUNDS)}.")
    if start == goal:
        raise ValueError("Start and goal are the same node.")

    best = None
    stack = [(start, [start], [], instability, 0, 0)]
    while stack:
        node, path, history, level, ruptures, bleeds = stack.pop()
        if node == goal:
            plan = {"nodes": path, "instability_after": history, "ruptures": ruptures,
                    "bleeds": bleeds, "final_instability": level}
            if best is None or _rank(plan) < _rank(best):
                best = plan
            continue
        for nxt in chart.neighbours(node):
            if nxt in path or nxt not in chart.nodes:
                continue
            _, _, after, ruptured, bleed = leap(chart, node, nxt, level, charted, background)
            stack.append((nxt, path + [nxt], history + [after], after,
                          ruptures + ruptured, bleeds + bleed))
    if best is None:
        raise ValueError(f"No route from '{start}' to '{goal}'.")
    return best


def enum_value(value):
    """Strips an 'E_Type::' prefix from an exported enum value."""
    text = str(value)
    return text.split("::", 1)[1] if "::" in text else text


def _rank(plan):
    return (plan["ruptures"], plan["bleeds"], plan["final_instability"], len(plan["nodes"]))


def _reachable(chart, start):
    seen, frontier = {start}, [start]
    while frontier:
        for nxt in chart.neighbours(frontier.pop()):
            if nxt in chart.nodes and nxt not in seen:
                seen.add(nxt)
                frontier.append(nxt)
    return seen


def _number(value, label, problems):
    try:
        return float(value)
    except (TypeError, ValueError):
        problems.append(f"{label}: '{value}' is not a number.")
        return 0.0
