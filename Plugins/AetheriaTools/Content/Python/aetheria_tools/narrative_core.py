"""Engine-independent logic for the Aetheria narrative tables.

Kept free of `unreal` imports so it can be tested with plain Python.
"""
import json
import os

SPEAKERS = frozenset({
    "Narrator", "Captain", "TalaVane", "JaxMercer", "ArisThorne", "NixNull", "VexAI",
    "Lyra", "Cormaker", "CoreArchitect", "CommanderVance", "WardenKesh", "ExarchIthane",
})
BACKGROUNDS = ("SynodEnvoy", "GuildEngineer", "CoalitionVanguard", "OutcastCorsair")

# Flags set by gameplay Blueprints (SLICE_SPEC §4, §9, §10) rather than by council options.
GAMEPLAY_FLAGS = frozenset({"AnomalousDamping", "BeamAnomaly", "UsedArchConduit", "LanternCleared",
                            # slice consequences and Act VII rules in UAetheriaGameInstance / PC_Command
                            "SquadronIIIAlienated", "ArchitectSilent", "CureSynthesising", "CureReady", "SeedsAboard",
                            "AmbushSprung", "ActVIIComplete",
                            # Act VIII (ACT_VIII_IRON_TITHE.md)
                            "AmbushSprung_ActVIII", "ActVIIIComplete",
                            # set by the Anchorage vault sequence (§4.5), Synod Envoy and Guild Engineer only
                            "CoalitionKeyRecovered"})

# Flags the game sets by pattern: per node, per council, per battle, per Weave anchor, per shipment order
GAMEPLAY_FLAG_PREFIXES = ("Visited_", "Council_", "Won_", "Anchor_", "Ordered_")

# Act VII option tables (DT_Council_<Name>), one per council in DT_Councils
ACT_VII_COUNCILS = ("Lifeline", "Kesh", "KeshCure", "AshChoir", "Salvage", "Echo", "Canopy",
                    "GlassSeaHail", "PyreGate", "TitheMarker", "Records", "ArchiveFate")
ACT_VII_NODES = ("EmberRelay", "AshfallVerge", "GlassSea", "TetherPoint", "CinderArchive", "QuietHollow",
                 "PyreGate", "TitheMarker", "VaultOfRecords")

# Act VIII option tables and nodes (ACT_VIII_IRON_TITHE.md)
ACT_VIII_COUNCILS = ("Terms", "FairShares", "TollLine", "ChoirCalls", "ExarchCell", "Choir", "Crucible",
                     "GraftedWounded", "Approach", "TithePaid", "Sentence")
ACT_VIII_NODES = ("TitheMarker", "Rustmarch", "TollLine", "SerenVael", "GraftForge", "Grieve")
ACT_VIII_BATTLES = ("E1_TollLine", "E1_TollLineStalled", "E2_ForgeDestroy", "E2_ForgeHold", "E3_Siege",
                    "E3_SiegeAllied", "E3_Infiltration", "E3_InfiltrationAllied", "Ambush_ActVIII")

# Extra source files merged into a table on load
TABLE_EXTRA_SOURCES = {"DT_Sequences": ("DT_Sequences_ActVII", "DT_Sequences_ActVIII"),
                       "DT_Councils": ("DT_Councils_ActVIII",),
                       "DT_Battles": ("DT_Battles_ActVIII",)}

# Tables imported from CSV (BuildData/<path>) instead of JSON
CSV_TABLES = {
    "DT_StarNodes_ActVII": ("S_StarNode", "ActVII/DT_StarNodes_ActVII.csv"),
    "DT_LeapRoutes_ActVII": ("S_LeapRoute", "ActVII/DT_LeapRoutes_ActVII.csv"),
    "DT_StarNodes_ActVIII": ("S_StarNode", "ActVIII/DT_StarNodes_ActVIII.csv"),
    "DT_LeapRoutes_ActVIII": ("S_LeapRoute", "ActVIII/DT_LeapRoutes_ActVIII.csv"),
}

# Table name -> (row struct asset name, rows the slice requires).
TABLES = {
    "DT_Sequences": ("S_Sequence", (
        "Briefing", "CombatIntro", "PostCombat", "VanceIntercept", "ArchitectReveal",
        "Consequence_Custodian", "Consequence_Dominion", "Consequence_Undetermined")),
    "DT_Council_Relay": ("S_CouncilOption", ("Grant", "Sandbox", "Refuse", "Rite")),
    "DT_Council_Directive": ("S_CouncilOption", ("Passage", "Arch", "Command")),
    # Act VII (ACT_VII_CINDER_ARCHIVE.md)
    "DT_Councils": ("S_CouncilDef", ACT_VII_COUNCILS + ACT_VIII_COUNCILS),
    **{f"DT_Council_{name}": ("S_CouncilOption", ()) for name in ACT_VII_COUNCILS + ACT_VIII_COUNCILS},
    "DT_NodeEvents_ActVII": ("S_NodeEvent", ACT_VII_NODES),
    "DT_NodeEvents_ActVIII": ("S_NodeEvent", ACT_VIII_NODES),
    "DT_Battles": ("S_Battle", ("E1", "E2", "E3", "E4_Defend", "E4_DefendAllied", "E4_Assault", "E4_Vance", "Ambush")
                   + ACT_VIII_BATTLES),
}

# What each table holds, for validation
TABLE_KIND = {table: ("sequence" if struct == "S_Sequence" else "option" if struct == "S_CouncilOption"
                      else "council" if struct == "S_CouncilDef" else "node" if struct == "S_NodeEvent" else "battle")
              for table, (struct, _) in TABLES.items()}

CUSTODIAN_THRESHOLD = 20
DOMINION_THRESHOLD = -20

# BP_AetheriaGameInstance defaults and ApplyBackgroundStart (SLICE_SPEC §4).
START_LOYALTY = {"TalaVane": 50, "JaxMercer": 50, "ArisThorne": 50, "NixNull": 50, "Lyra": 50, "Cormaker": 40}
BACKGROUND_LOYALTY = {"GuildEngineer": ("JaxMercer", 15), "CoalitionVanguard": ("TalaVane", 15),
                      "OutcastCorsair": ("Cormaker", 20)}
START_STATE = {"instability": 25.0, "chrono_charge": 60.0, "phantom_ships_active": 200,
               "spire_recalibrations": 1, "architect_leaning": 0}


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
        for extra in TABLE_EXTRA_SOURCES.get(table, ()):
            extra_path = os.path.join(source_dir, f"{extra}.json")
            if os.path.isfile(extra_path):
                with open(extra_path, encoding="utf-8") as f:
                    rows = rows + json.load(f)
        data[table] = rows
    return data


def validate(data):
    """Returns human-readable problems in the source data. Empty means valid."""
    problems = []
    set_flags = set(GAMEPLAY_FLAGS)
    for table, kind in TABLE_KIND.items():
        for r in data.get(table, []):
            if kind == "sequence":
                set_flags.update(r.get("SetFlags", []))
            elif kind == "option":
                set_flags.add(r.get("SetFlag", "None"))
                set_flags.update(r.get("SetFlags", []))
            elif kind == "node":
                set_flags.update(r.get("ArrivalFlags", []))
            elif kind == "battle":
                set_flags.update(r.get("WinFlags", []))
    set_flags.discard("None")
    councils = {r.get("Name") for r in data.get("DT_Councils", [])}
    battles = {r.get("Name") for r in data.get("DT_Battles", [])}
    sequences = {r.get("Name") for r in data.get("DT_Sequences", [])}

    for table, (_, required) in TABLES.items():
        rows = data.get(table, [])
        names = [r.get("Name") for r in rows]
        for name in sorted({n for n in names if names.count(n) > 1}):
            problems.append(f"{table}: duplicate row '{name}'.")
        for name in required:
            if name not in names:
                problems.append(f"{table}: missing required row '{name}'.")
        kind = TABLE_KIND[table]
        for row in rows:
            where = f"{table}.{row.get('Name')}"
            if kind in ("sequence", "option"):
                lines = row.get("Lines") if kind == "sequence" else row.get("Outcome")
                if not lines:
                    problems.append(f"{where}: has no lines.")
                for i, line in enumerate(lines or []):
                    problems.extend(_validate_line(line, f"{where}[{i}]", set_flags))
            if kind == "option":
                problems.extend(_validate_option(row, where))
                for flag in row.get("RequiredFlags", []) + row.get("ForbiddenFlags", []):
                    if not _is_set_somewhere(flag, set_flags):
                        problems.append(f"{where}: requires flag '{flag}', which nothing sets.")
                if row.get("NextCouncil", "None") not in ("None", *councils):
                    problems.append(f"{where}: NextCouncil '{row.get('NextCouncil')}' is not in DT_Councils.")
                if row.get("StartBattle", "None") not in ("None", *battles):
                    problems.append(f"{where}: StartBattle '{row.get('StartBattle')}' is not in DT_Battles.")
            elif kind == "council":
                if f"DT_Council_{row.get('Name')}" not in data:
                    problems.append(f"{where}: no option table DT_Council_{row.get('Name')}.")
                if row.get("SituationSpeaker") not in SPEAKERS:
                    problems.append(f"{where}: unknown situation speaker '{row.get('SituationSpeaker')}'.")
                for i, entry in enumerate(row.get("Counsel", [])):
                    if entry.get("Officer") not in SPEAKERS:
                        problems.append(f"{where}.Counsel[{i}]: unknown officer '{entry.get('Officer')}'.")
                    for key in ("RequiredFlag", "ForbiddenFlag"):
                        if entry.get(key, "None") != "None" and not _is_set_somewhere(entry[key], set_flags):
                            problems.append(f"{where}.Counsel[{i}]: {key} '{entry[key]}' is never set.")
                if row.get("Intro", "None") not in ("None", *sequences):
                    problems.append(f"{where}: intro sequence '{row.get('Intro')}' is not in DT_Sequences.")
            elif kind == "node":
                for council in row.get("Councils", []):
                    if council not in councils:
                        problems.append(f"{where}: council '{council}' is not in DT_Councils.")
                if row.get("ArrivalSequence", "None") not in ("None", *sequences):
                    problems.append(f"{where}: arrival sequence '{row.get('ArrivalSequence')}' is not in DT_Sequences.")
            elif kind == "battle":
                if not row.get("Groups"):
                    problems.append(f"{where}: has no ship groups.")
                if row.get("Debrief", "None") not in ("None", *sequences):
                    problems.append(f"{where}: debrief '{row.get('Debrief')}' is not in DT_Sequences.")
                for bg, sequence in row.get("BackgroundAftermath", {}).items():
                    if bg not in BACKGROUNDS:
                        problems.append(f"{where}: aftermath for unknown background '{bg}'.")
                    if sequence not in sequences:
                        problems.append(f"{where}: aftermath '{sequence}' is not in DT_Sequences.")
    return problems


def _is_set_somewhere(flag, set_flags):
    return flag in set_flags or flag.startswith(GAMEPLAY_FLAG_PREFIXES)


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
        if not text:
            continue
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


# Defaults for the council-option fields added in Act VII, so older rows import without missing-field prompts
OPTION_DEFAULTS = {
    "RequiredFlags": [], "ForbiddenFlags": [], "MinStanding": -1000, "MaxStanding": 1000, "MinLoyaltyAny": {},
    "RequiredStates": {}, "StandingDelta": 0, "PursuitDelta": 0, "ExposureDelta": 0, "HullDelta": 0.0,
    "SetFlags": [], "SetStates": {}, "NextCouncil": "None", "StartBattle": "None",
}


# Defaults for sequence and battle fields added in Act VIII
SEQUENCE_DEFAULTS = {"SetFlags": []}
BATTLE_DEFAULTS = {"BackgroundAftermath": {}}


def to_import_json(rows, table=None):
    """Serializes rows in the JSON layout DataTable import expects, filling fields older rows lack."""
    defaults = {"option": OPTION_DEFAULTS, "sequence": SEQUENCE_DEFAULTS, "battle": BATTLE_DEFAULTS}
    if table is not None and TABLE_KIND.get(table) in defaults:
        rows = [{**defaults[TABLE_KIND[table]], **row} for row in rows]
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
    # Empty text with overrides: a line only those backgrounds see
    if not str(line.get("Text", "")).strip() and not line.get("BackgroundOverride"):
        problems.append(f"{where}: empty text.")
    for bg in line.get("BackgroundOverride", {}):
        if bg not in BACKGROUNDS:
            problems.append(f"{where}: override for unknown background '{bg}'.")
    flag = line.get("RequiredFlag", "None")
    if flag != "None" and not _is_set_somewhere(flag, set_flags):
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
