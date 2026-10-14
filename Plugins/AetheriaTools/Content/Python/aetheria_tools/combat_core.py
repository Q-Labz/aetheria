"""Engine-independent checks for the tactical combat slice: ship Blueprints against fleet canon
(SLICE_SPEC §9) and the combat level's layout (§7, §10).

Kept free of `unreal` imports so it can be tested with plain Python.
"""
SHIPS_FOLDER = "/Game/Aetheria/Ships"
GAME_INSTANCE = "/Game/Aetheria/Core/BP_AetheriaGameInstance"
BASE_SHIP = "BP_ShipBase"

# Of the canon 214 ships (1 Dreadnought + 1 Wayward + 200 Phantom + 12 Corsair), the slice tracks
# the Phantom Fleet, in squadrons of 20.
PHANTOM_FLEET = 200
PHANTOM_SQUADRON_SIZE = 20

# First-pass stat defaults from §9. Numbers are tuning targets, not canon.
BASE_STATS = {
    "MaxHull": 500, "MaxShield": 150, "ShieldRegenPerSec": 10, "ShieldRegenDelay": 3, "MoveSpeed": 900,
    "TurnSpeed": 60, "WeaponRange": 3500, "WeaponDamage": 20, "FireInterval": 1.0, "AggroRange": 7000,
    "DamageMultiplier": 1.0,
}

# Ship Blueprint -> its own spec. "stats": first-pass defaults it overrides or adds; "runtime": base
# stats it computes at BeginPlay, so its defaults are not compared; "required": variables that must
# exist whatever their value; "canon": exact defaults; "tags": actor tags; "meshes": component -> mesh.
SHIPS = {
    BASE_SHIP: {"required": ("DisplayName", "Faction", "bTargetsFlagship")},
    "BP_Flagship": {
        "stats": {"MaxHull": 4000, "MaxShield": 2500, "ShieldRegenPerSec": 60, "MoveSpeed": 450, "TurnSpeed": 25,
                  "WeaponRange": 4000, "WeaponDamage": 35, "FireInterval": 0.8, "BeamDamage": 900,
                  "BeamLength": 9000, "BeamRadius": 250, "BeamChargeTime": 2.0, "BeamCooldown": 15,
                  "ArchitectBoost": 1.12},
        "required": ("BeamKills", "BeamShots"),
        "canon": {"DisplayName": "The Crystal Dreadnought"},
        "tags": ("Flagship",),
        "meshes": {"HullMesh": "SM_CrystalDreadnought"},
    },
    "BP_PhantomSquadron": {
        "stats": {"HullPerShip": 150, "DmgPerShip": 3, "MoveSpeed": 1000, "TurnSpeed": 70, "MaxShield": 400},
        "runtime": ("MaxHull", "WeaponDamage"),
        "canon": {"ShipsInSquadron": PHANTOM_SQUADRON_SIZE},
        "meshes": {"Formation": "SM_PhantomCruiser"},
    },
    "BP_CorsairGunship": {
        "stats": {"MaxHull": 1200, "MaxShield": 300},
        "meshes": {"HullMesh": "SM_CorsairGunship"},
    },
    "BP_TitheInterceptor": {
        "stats": {"MaxHull": 500, "MaxShield": 150, "MoveSpeed": 1300, "TurnSpeed": 120, "WeaponDamage": 20,
                  "FireInterval": 0.6, "WeaponRange": 2500},
        "meshes": {"HullMesh": "SM_TitheInterceptor"},
    },
    "BP_TitheCruiser": {
        "stats": {"MaxHull": 4000, "MaxShield": 1500, "MoveSpeed": 500, "WeaponDamage": 90, "FireInterval": 1.5,
                  "WeaponRange": 4500},
        "canon": {"DisplayName": "Picket Cruiser 'Tithe of Sector 7'", "bTargetsFlagship": True},
        "meshes": {"HullMesh": "SM_TitheCruiser"},
    },
}
GAME_INSTANCE_CANON = {"PhantomShipsActive": PHANTOM_FLEET}

# Combat level layout (§7). Locations in uu.
GAME_MODE = "GM_Tactical"
SPAWN_POINTS = {
    "PlayerSpawn_Flagship": (-4000, 0, 0), "PlayerSpawn_Sq1": (-3500, -1500, 0),
    "PlayerSpawn_Sq2": (-3500, 1500, 0), "PlayerSpawn_Corsair": (-3000, 0, 0),
    "EnemySpawn_Cruiser": (5000, 0, 0),
}
ENEMY_SPAWN_TAG = "EnemySpawn"
ENEMY_SPAWNS = ((4000, 1200, 0), (4000, -1200, 0), (4500, 2400, 0), (4500, -2400, 0),
                (3500, 600, 0), (3500, -600, 0))
MAX_INTERCEPTORS = 4 + 2  # BP_CombatManager: 4, plus 2 once PursuitClock >= 2
PLAYER_START = (-4000, 0, 0)
LOCATION_TOLERANCE = 1.0

# Actor kinds the level check reads; the engine wrapper maps classes onto these.
TARGET_POINT, PLAYER_START_KIND, COMBAT_MANAGER = "TargetPoint", "PlayerStart", "CombatManager"
DIRECTIONAL_LIGHT, POST_PROCESS = "DirectionalLight", "PostProcessVolume"


def ship_path(ship):
    return f"{SHIPS_FOLDER}/{ship}"


def read_names(ship):
    """Variable names to read from a ship's class defaults."""
    stats = expected_stats(ship)
    spec = SHIPS[ship]
    return sorted(set(stats) | set(spec.get("required", ())) | set(spec.get("canon", {})))


def expected_stats(ship):
    """First-pass stat defaults for a ship, inherited base values included."""
    if ship not in SHIPS:
        raise KeyError(f"Unknown ship '{ship}'. Known: {', '.join(SHIPS)}.")
    spec = SHIPS[ship]
    stats = {**BASE_STATS, **spec.get("stats", {})}
    for name in spec.get("runtime", ()):
        stats.pop(name, None)
    return stats


def ship_stats(ship, properties):
    """(stat, value, spec value) for each first-pass stat, from the ship's read defaults."""
    expected = expected_stats(ship)
    missing = [name for name in expected if name not in properties]
    if missing:
        raise ValueError(f"{ship} has no variable {', '.join(missing)}.")
    return [(name, float(properties[name]), float(spec)) for name, spec in expected.items()]


def validate_fleet(ships, game_instance):
    """Problems with the ship Blueprints and Game Instance against fleet canon. Empty means valid.

    ships: ship name -> None if the asset is missing, else a dict with "is_ship" (derives from
    BP_ShipBase), "properties" (variable -> default; absent if the variable does not exist),
    "tags" and "meshes" (component name -> mesh asset name or None).
    game_instance: None if missing, else variable -> default.
    """
    problems = []
    for ship, spec in SHIPS.items():
        info = ships.get(ship)
        if info is None:
            problems.append(f"{ship_path(ship)}: not created yet.")
            continue
        if ship != BASE_SHIP and not info["is_ship"]:
            problems.append(f"{ship}: does not derive from {BASE_SHIP}.")
        properties = info["properties"]
        own = set(spec.get("stats", {})) | set(spec.get("required", ())) | set(spec.get("canon", {}))
        if ship == BASE_SHIP:
            own |= set(BASE_STATS)
        problems.extend(f"{ship}: missing variable '{n}'." for n in sorted(own - set(properties)))
        problems.extend(_canon_problems(ship, spec.get("canon", {}), properties))
        problems.extend(f"{ship}: missing actor tag '{t}'." for t in spec.get("tags", ()) if t not in info["tags"])
        for component, mesh in spec.get("meshes", {}).items():
            if component not in info["meshes"]:
                problems.append(f"{ship}: no mesh component '{component}'.")
            elif info["meshes"][component] != mesh:
                problems.append(f"{ship}: {component} uses '{info['meshes'][component]}', expected '{mesh}'.")

    if game_instance is None:
        problems.append(f"{GAME_INSTANCE}: not created yet.")
    else:
        name = GAME_INSTANCE.rsplit("/", 1)[-1]
        problems.extend(f"{name}: missing variable '{n}'." for n in GAME_INSTANCE_CANON if n not in game_instance)
        problems.extend(_canon_problems(name, GAME_INSTANCE_CANON, game_instance))
    return problems


def validate_combat_level(game_mode, actors):
    """Problems with a combat level's setup. Empty means valid.

    game_mode: GameMode Override class name without the _C suffix, or None.
    actors: dicts with "kind" (one of the kinds above, else the class name), "label", "tags",
    "location" (x, y, z) and, for post-process volumes, "unbound".
    """
    problems = []
    if game_mode != GAME_MODE:
        problems.append(f"GameMode Override is '{game_mode}', expected '{GAME_MODE}'.")

    managers = _of_kind(actors, COMBAT_MANAGER)
    if len(managers) != 1:
        problems.append(f"Expected one BP_CombatManager, found {len(managers)}.")
    problems.extend(_misplaced(m, (0, 0, 0)) for m in managers if not _near(m["location"], (0, 0, 0)))

    points = _of_kind(actors, TARGET_POINT)
    for tag, location in SPAWN_POINTS.items():
        tagged = [p for p in points if tag in p["tags"]]
        if len(tagged) != 1:
            problems.append(f"Expected one TargetPoint tagged '{tag}', found {len(tagged)}.")
        problems.extend(_misplaced(p, location) for p in tagged if not _near(p["location"], location))

    enemy = [p for p in points if ENEMY_SPAWN_TAG in p["tags"]]
    if len(enemy) < MAX_INTERCEPTORS:
        problems.append(f"Found {len(enemy)} TargetPoints tagged '{ENEMY_SPAWN_TAG}'; "
                        f"the battle spawns up to {MAX_INTERCEPTORS} interceptors.")
    unmatched = [p["location"] for p in enemy]
    for location in ENEMY_SPAWNS:
        match = next((u for u in unmatched if _near(u, location)), None)
        if match is None:
            problems.append(f"No TargetPoint tagged '{ENEMY_SPAWN_TAG}' at {_format(location)}.")
        else:
            unmatched.remove(match)

    starts = _of_kind(actors, PLAYER_START_KIND)
    if len(starts) != 1:
        problems.append(f"Expected one PlayerStart, found {len(starts)}.")
    problems.extend(_misplaced(s, PLAYER_START) for s in starts if not _near(s["location"], PLAYER_START))

    if not _of_kind(actors, DIRECTIONAL_LIGHT):
        problems.append("No DirectionalLight.")
    if not any(v.get("unbound") for v in _of_kind(actors, POST_PROCESS)):
        problems.append("No PostProcessVolume with Infinite Extent (Unbound).")
    return problems


def _canon_problems(owner, canon, properties):
    return [f"{owner}: {name} is {properties[name]!r}, canon is {value!r}."
            for name, value in canon.items() if name in properties and properties[name] != value]


def _of_kind(actors, kind):
    return [a for a in actors if a["kind"] == kind]


def _near(a, b):
    return all(abs(x - y) <= LOCATION_TOLERANCE for x, y in zip(a, b))


def _misplaced(actor, expected):
    return f"{actor['label']} is at {_format(actor['location'])}, expected {_format(expected)}."


def _format(location):
    return "(" + ", ".join(f"{v:g}" for v in location) + ")"
