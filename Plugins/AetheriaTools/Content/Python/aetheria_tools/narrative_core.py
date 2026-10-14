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
                leaning = max(-100, min(100, relay["LeaningDelta"] + directive["LeaningDelta"]))
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
    if background not in BACKGROUNDS:
        raise ValueError(f"Unknown background '{background}'. Known: {', '.join(BACKGROUNDS)}.")
    resolved = []
    for line in rows[sequence]["Lines"]:
        flag = line.get("RequiredFlag", "None")
        if flag != "None" and flag not in flags:
            continue
        text = line.get("BackgroundOverride", {}).get(background, line["Text"])
        resolved.append((line["Speaker"], text.replace("{Captain}", captain_name)))
    return resolved


def to_import_json(rows):
    """Serializes rows in the JSON layout DataTable import expects."""
    return json.dumps(rows, ensure_ascii=False)


def _available(options, background):
    return [o for o in options
            if not o.get("RequiresBackground") or o.get("RequiredBackground") == background]


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
