"""Engine-independent logic for the Aetheria narrative tables.

Kept free of `unreal` imports so it can be tested with plain Python.
"""
import json
import os

SPEAKERS = frozenset({
    "Narrator", "Captain", "TalaVane", "JaxMercer", "ArisThorne", "NixNull", "VexAI",
    "Lyra", "Cormaker", "CoreArchitect", "CommanderVance",
})
BACKGROUNDS = ("SynodEnvoy", "GuildEngineer", "CoalitionVanguard", "OutcastCorsair")

# Flags set by gameplay Blueprints (SLICE_SPEC §4, §9, §10) rather than by council options.
GAMEPLAY_FLAGS = frozenset({"AnomalousDamping", "BeamAnomaly", "UsedArchConduit", "LanternCleared"})

# Table name -> (row struct asset name, rows the slice requires).
TABLES = {
    "DT_Sequences": ("S_Sequence", (
        "Briefing", "CombatIntro", "PostCombat", "VanceIntercept", "ArchitectReveal",
        "Consequence_Custodian", "Consequence_Dominion", "Consequence_Undetermined")),
    "DT_Council_Relay": ("S_CouncilOption", ("Grant", "Sandbox", "Refuse", "Rite")),
    "DT_Council_Directive": ("S_CouncilOption", ("Passage", "Arch", "Command")),
}

CUSTODIAN_THRESHOLD = 20
DOMINION_THRESHOLD = -20

# BP_AetheriaGameInstance defaults and ApplyBackgroundStart (SLICE_SPEC §4).
START_LOYALTY = {"TalaVane": 50, "JaxMercer": 50, "ArisThorne": 50, "NixNull": 50, "Lyra": 50, "Cormaker": 40}
BACKGROUND_LOYALTY = {"GuildEngineer": ("JaxMercer", 15), "CoalitionVanguard": ("TalaVane", 15),
                      "OutcastCorsair": ("Cormaker", 20)}
START_STATE = {"instability": 25.0, "chrono_charge": 60.0, "phantom_ships_active": 200,
               "wayward_available": True, "corsair_ships_active": 12, "spire_recalibrations": 1,
               "architect_leaning": 0}


def load_source(source_dir):
    """Returns {table name: rows} read from <source_dir>/<table>.json."""
    data = {}
    for table in TABLES:
        path = os.path.join(source_dir, f"{table}.json")
        if not os.path.isfile(path):
            raise FileNotFoundError(f"Narrative source file not found: {path}")
        with open(path, encoding="utf-8") as f:
            rows = json.load(f)
        if not isinstance(rows, list):
            raise ValueError(f"{path} must contain a JSON array of rows.")
        data[table] = rows
    return data


def validate(data):
    """Returns human-readable problems in the source data. Empty means valid."""
    problems = []
    set_flags = set(GAMEPLAY_FLAGS)
    for table in ("DT_Council_Relay", "DT_Council_Directive"):
        set_flags.update(r.get("SetFlag", "None") for r in data.get(table, []))
    set_flags.discard("None")

    for table, (_, required) in TABLES.items():
        rows = data.get(table, [])
        names = [r.get("Name") for r in rows]
        for name in sorted({n for n in names if names.count(n) > 1}):
            problems.append(f"{table}: duplicate row '{name}'.")
        for name in required:
            if name not in names:
                problems.append(f"{table}: missing required row '{name}'.")
        for row in rows:
            where = f"{table}.{row.get('Name')}"
            lines = row.get("Lines") if table == "DT_Sequences" else row.get("Outcome")
            if not lines:
                problems.append(f"{where}: has no lines.")
            for i, line in enumerate(lines or []):
                problems.extend(_validate_line(line, f"{where}[{i}]", set_flags))
            if table != "DT_Sequences":
                problems.extend(_validate_option(row, where))
    return problems


def architect_outcomes(data):
    """Every reachable (background, relay option, directive option) path.

    Returns dicts with keys: background, relay, directive, leaning, disposition.
    """
    outcomes = []
    for background in BACKGROUNDS:
        for relay in _available(data["DT_Council_Relay"], background):
            for directive in _available(data["DT_Council_Directive"], background):
                leaning = _clamp(relay["LeaningDelta"] + directive["LeaningDelta"], -100, 100)
                outcomes.append({
                    "background": background, "relay": relay["Name"],
                    "directive": directive["Name"], "leaning": leaning,
                    "disposition": disposition(leaning),
                })
    return outcomes


def disposition(leaning):
    """Maps an ArchitectLeaning value to Custodian, Dominion or Undetermined."""
    if leaning >= CUSTODIAN_THRESHOLD:
        return "Custodian"
    if leaning <= DOMINION_THRESHOLD:
        return "Dominion"
    return "Undetermined"


def resolve_sequence(data, sequence, background, flags, captain_name):
    """Lines of a sequence as the player would see them: (speaker, text) pairs."""
    rows = {r["Name"]: r for r in data["DT_Sequences"]}
    if sequence not in rows:
        raise KeyError(f"Unknown sequence '{sequence}'. Known: {', '.join(sorted(rows))}.")
    _check_background(background)
    resolved = []
    for line in rows[sequence]["Lines"]:
        flag = line.get("RequiredFlag", "None")
        if flag != "None" and flag not in flags:
            continue
        text = line.get("BackgroundOverride", {}).get(background, line["Text"])
        resolved.append((line["Speaker"], text.replace("{Captain}", captain_name)))
    return resolved


def starting_state(background):
    """Campaign state after captain creation: Game Instance defaults plus the background's
    loyalty bonus. Keys: background, the START_STATE keys, crew_loyalty, story_flags."""
    _check_background(background)
    loyalty = dict(START_LOYALTY)
    if background in BACKGROUND_LOYALTY:
        officer, delta = BACKGROUND_LOYALTY[background]
        loyalty[officer] = _clamp(loyalty[officer] + delta, -100, 100)
    return {"background": background, **START_STATE, "crew_loyalty": loyalty, "story_flags": []}


def council_path(data, state, relay, directive):
    """State after choosing a Relay option, a Directive option and the Architect's consequence.

    Mirrors ApplyOption (SLICE_SPEC §4) and ApplyArchitectConsequence (§6). Returns a new state.
    """
    _check_background(state["background"])
    state = {**state, "crew_loyalty": dict(state["crew_loyalty"]), "story_flags": list(state["story_flags"])}
    for table, name in (("DT_Council_Relay", relay), ("DT_Council_Directive", directive)):
        _apply_option(state, _option(data, table, name, state["background"]))
    leaning = disposition(state["architect_leaning"])
    if leaning == "Custodian":
        state["instability"] = max(0.0, state["instability"] - 30)
        state["spire_recalibrations"] += 1
    elif leaning == "Dominion":
        state["phantom_ships_active"] -= 20
        _add_flag(state, "SquadronIIIAlienated")
    else:
        _add_flag(state, "ArchitectSilent")
    return state


def to_import_json(rows):
    """Serializes rows in the JSON layout DataTable import expects."""
    return json.dumps(rows, ensure_ascii=False)


def _available(options, background):
    return [o for o in options
            if not o.get("RequiresBackground") or o.get("RequiredBackground") == background]


def _option(data, table, name, background):
    rows = {r["Name"]: r for r in data[table]}
    if name not in rows:
        raise KeyError(f"Unknown {table} option '{name}'. Known: {', '.join(sorted(rows))}.")
    if rows[name] not in _available(data[table], background):
        raise ValueError(f"{table} option '{name}' is not offered to a {background}.")
    return rows[name]


def _apply_option(state, option):
    state["architect_leaning"] = _clamp(state["architect_leaning"] + option["LeaningDelta"], -100, 100)
    loyalty = state["crew_loyalty"]
    for officer, delta in option.get("LoyaltyDeltas", {}).items():
        loyalty[officer] = _clamp(loyalty.get(officer, 0) + delta, -100, 100)
    state["instability"] = _clamp(state["instability"] + option.get("InstabilityDelta", 0.0), 0.0, 100.0)
    state["chrono_charge"] = _clamp(state["chrono_charge"] + option.get("ChronoDelta", 0.0), 0.0, 100.0)
    if option.get("SetFlag", "None") != "None":
        _add_flag(state, option["SetFlag"])


def _add_flag(state, flag):
    if flag not in state["story_flags"]:
        state["story_flags"].append(flag)


def _clamp(value, low, high):
    return max(low, min(high, value))


def _check_background(background):
    if background not in BACKGROUNDS:
        raise ValueError(f"Unknown background '{background}'. Known: {', '.join(BACKGROUNDS)}.")


def _validate_line(line, where, set_flags):
    problems = []
    if line.get("Speaker") not in SPEAKERS:
        problems.append(f"{where}: unknown speaker '{line.get('Speaker')}'.")
    if not str(line.get("Text", "")).strip():
        problems.append(f"{where}: empty text.")
    for bg in line.get("BackgroundOverride", {}):
        if bg not in BACKGROUNDS:
            problems.append(f"{where}: override for unknown background '{bg}'.")
    flag = line.get("RequiredFlag", "None")
    if flag != "None" and flag not in set_flags:
        problems.append(f"{where}: requires flag '{flag}', which nothing sets.")
    return problems


def _validate_option(row, where):
    problems = []
    for key in ("Label", "ApprovalHint"):
        if not str(row.get(key, "")).strip():
            problems.append(f"{where}: empty {key}.")
    if not isinstance(row.get("LeaningDelta"), int):
        problems.append(f"{where}: LeaningDelta must be an integer.")
    for speaker in row.get("LoyaltyDeltas", {}):
        if speaker not in SPEAKERS:
            problems.append(f"{where}: loyalty change for unknown speaker '{speaker}'.")
    if row.get("RequiresBackground") and row.get("RequiredBackground") not in BACKGROUNDS:
        problems.append(f"{where}: unknown RequiredBackground '{row.get('RequiredBackground')}'.")
    return problems
