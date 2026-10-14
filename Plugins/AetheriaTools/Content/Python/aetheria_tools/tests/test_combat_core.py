"""Engine-independent tests for combat_core. Run outside Unreal:
    python -m unittest discover -s Plugins/AetheriaTools/Content/Python -p "test_combat_core.py" -t Plugins/AetheriaTools/Content/Python
"""
import copy
import unittest

from aetheria_tools import combat_core as core


def built_fleet():
    """Ship and Game Instance readings for a fleet built exactly to spec."""
    ships = {}
    for ship, spec in core.SHIPS.items():
        properties = {name: 0 for name in core.read_names(ship)}
        properties.update(core.expected_stats(ship))
        properties.update(spec.get("canon", {}))
        ships[ship] = {"is_ship": True, "properties": properties, "tags": list(spec.get("tags", ())),
                       "meshes": dict(spec.get("meshes", {}))}
    return ships, dict(core.GAME_INSTANCE_CANON), {e["mesh"] for e in core.ROSTER}


def actor(kind, location, tags=(), label=None, **extra):
    return {"kind": kind, "label": label or kind, "tags": list(tags), "location": location, **extra}


def built_level():
    actors = [actor(core.COMBAT_MANAGER, (0, 0, 0), label="BP_CombatManager"),
              actor(core.PLAYER_START_KIND, core.PLAYER_START),
              actor(core.DIRECTIONAL_LIGHT, (0, 0, 1000)),
              actor(core.POST_PROCESS, (0, 0, 0), unbound=True)]
    actors += [actor(core.TARGET_POINT, loc, [tag], label=tag) for tag, loc in core.SPAWN_POINTS.items()]
    actors += [actor(core.TARGET_POINT, loc, [core.ENEMY_SPAWN_TAG]) for loc in core.ENEMY_SPAWNS]
    return actors


class FleetTest(unittest.TestCase):
    def setUp(self):
        self.ships, self.game_instance, self.meshes = built_fleet()

    def problems(self):
        return "\n".join(core.validate_fleet(self.ships, self.game_instance, self.meshes))

    def test_fleet_built_to_spec_is_valid(self):
        self.assertEqual(core.validate_fleet(self.ships, self.game_instance, self.meshes), [])

    def test_canon_counts_add_up(self):
        self.assertEqual(sum(e["canon_count"] for e in core.ROSTER), 214)
        self.assertEqual(core.PHANTOM_FLEET % core.PHANTOM_SQUADRON_SIZE, 0)

    def test_reports_roster_mesh_not_imported(self):
        self.meshes.discard("SM_Wayward")
        self.assertIn("/Game/Aetheria/Meshes/SM_Wayward: not imported yet.", self.problems())

    def test_reports_wayward_and_corsair_defaults(self):
        self.game_instance["WaywardAvailable"] = False
        del self.game_instance["CorsairShipsActive"]
        problems = self.problems()
        self.assertIn("WaywardAvailable is False, canon is True", problems)
        self.assertIn("missing variable 'CorsairShipsActive'", problems)

    def test_reports_missing_assets(self):
        self.ships["BP_TitheCruiser"] = None
        problems = core.validate_fleet(self.ships, None, self.meshes)
        self.assertIn("/Game/Aetheria/Ships/BP_TitheCruiser: not created yet.", problems)
        self.assertIn(f"{core.GAME_INSTANCE}: not created yet.", problems)

    def test_reports_canon_violations(self):
        self.ships["BP_PhantomSquadron"]["properties"]["ShipsInSquadron"] = 25
        self.ships["BP_TitheCruiser"]["properties"]["bTargetsFlagship"] = False
        self.ships["BP_Flagship"]["properties"]["DisplayName"] = "Dreadnought"
        self.game_instance["PhantomShipsActive"] = 180
        problems = self.problems()
        for expected in ("ShipsInSquadron is 25, canon is 20", "bTargetsFlagship is False, canon is True",
                         "DisplayName is 'Dreadnought', canon is 'The Crystal Dreadnought'",
                         "PhantomShipsActive is 180, canon is 200"):
            self.assertIn(expected, problems)

    def test_reports_missing_ability_variable_once(self):
        del self.ships["BP_Flagship"]["properties"]["BeamDamage"]
        del self.ships["BP_ShipBase"]["properties"]["MaxHull"]
        del self.ships["BP_Flagship"]["properties"]["MaxHull"]
        problems = core.validate_fleet(self.ships, self.game_instance, self.meshes)
        self.assertIn("BP_Flagship: missing variable 'BeamDamage'.", problems)
        self.assertIn("BP_ShipBase: missing variable 'MaxHull'.", problems)
        self.assertEqual(sum("'MaxHull'" in p for p in problems), 2)  # base, plus the flagship's override

    def test_reports_parent_tag_and_mesh_problems(self):
        self.ships["BP_CorsairGunship"]["is_ship"] = False
        self.ships["BP_Flagship"]["tags"] = []
        self.ships["BP_TitheInterceptor"]["meshes"]["HullMesh"] = "SM_TitheCruiser"
        del self.ships["BP_PhantomSquadron"]["meshes"]["Formation"]
        problems = self.problems()
        for expected in ("BP_CorsairGunship: does not derive from BP_ShipBase",
                         "BP_Flagship: missing actor tag 'Flagship'",
                         "HullMesh uses 'SM_TitheCruiser', expected 'SM_TitheInterceptor'",
                         "BP_PhantomSquadron: no mesh component 'Formation'"):
            self.assertIn(expected, problems)

    def test_stat_drift_is_not_a_canon_problem(self):
        self.ships["BP_TitheCruiser"]["properties"]["MaxHull"] = 5200
        self.assertEqual(core.validate_fleet(self.ships, self.game_instance, self.meshes), [])


class FleetRosterTest(unittest.TestCase):
    STATE = {"wayward_available": True, "phantom_ships_active": 200, "corsair_ships_active": 12}

    def test_roster_at_full_strength(self):
        roster = {e["ship_class"]: e for e in core.fleet_roster(self.STATE)}
        self.assertEqual(list(roster), ["Crystal Dreadnought", "Wayward", "Phantom Fleet", "Corsair Armada"])
        self.assertEqual(sum(e["active_count"] for e in roster.values()), 214)
        self.assertEqual(roster["Wayward"]["blueprint"], "")
        self.assertEqual(roster["Corsair Armada"]["blueprint"], "/Game/Aetheria/Ships/BP_CorsairGunship")
        self.assertEqual(roster["Wayward"]["mesh"], "/Game/Aetheria/Meshes/SM_Wayward")

    def test_roster_reflects_losses(self):
        state = dict(self.STATE, wayward_available=False, phantom_ships_active=180, corsair_ships_active=9)
        active = {e["ship_class"]: e["active_count"] for e in core.fleet_roster(state)}
        self.assertEqual(active, {"Crystal Dreadnought": 1, "Wayward": 0, "Phantom Fleet": 180,
                                  "Corsair Armada": 9})

    def test_roster_raises_for_impossible_strength(self):
        for key, value in (("corsair_ships_active", 13), ("phantom_ships_active", -5)):
            with self.assertRaises(ValueError):
                core.fleet_roster(dict(self.STATE, **{key: value}))


class ShipStatsTest(unittest.TestCase):
    def test_child_inherits_base_stats(self):
        stats = core.expected_stats("BP_CorsairGunship")
        self.assertEqual((stats["MaxHull"], stats["WeaponRange"]), (1200, 3500))

    def test_runtime_stats_are_left_out(self):
        stats = core.expected_stats("BP_PhantomSquadron")
        self.assertNotIn("MaxHull", stats)
        self.assertNotIn("WeaponDamage", stats)

    def test_ship_stats_pairs_value_with_spec(self):
        properties = dict(core.expected_stats("BP_Flagship"), BeamCooldown=12)
        stats = {name: (value, spec) for name, value, spec in core.ship_stats("BP_Flagship", properties)}
        self.assertEqual(stats["BeamCooldown"], (12.0, 15.0))
        self.assertEqual(stats["ArchitectBoost"], (1.12, 1.12))

    def test_ship_stats_raises_for_missing_variable(self):
        with self.assertRaises(ValueError):
            core.ship_stats("BP_Flagship", {"MaxHull": 4000})

    def test_unknown_ship_raises(self):
        for lookup in (core.expected_stats, core.read_names):
            with self.assertRaisesRegex(KeyError, "Unknown ship 'BP_Wayward'"):
                lookup("BP_Wayward")


class CombatLevelTest(unittest.TestCase):
    def setUp(self):
        self.actors = built_level()

    def problems(self, game_mode="GM_Tactical"):
        return "\n".join(core.validate_combat_level(game_mode, self.actors))

    def test_level_built_to_spec_is_valid(self):
        self.assertEqual(core.validate_combat_level("GM_Tactical", self.actors), [])

    def test_reports_wrong_game_mode(self):
        self.assertIn("GameMode Override is 'None', expected 'GM_Tactical'", self.problems(None))

    def test_reports_missing_and_duplicate_spawns(self):
        self.actors = [a for a in self.actors if "PlayerSpawn_Corsair" not in a["tags"]]
        self.actors.append(actor(core.TARGET_POINT, (5000, 0, 0), ["EnemySpawn_Cruiser"]))
        problems = self.problems()
        self.assertIn("Expected one TargetPoint tagged 'PlayerSpawn_Corsair', found 0.", problems)
        self.assertIn("Expected one TargetPoint tagged 'EnemySpawn_Cruiser', found 2.", problems)

    def test_reports_misplaced_actors(self):
        next(a for a in self.actors if "PlayerSpawn_Sq1" in a["tags"])["location"] = (-3500, 1500, 0)
        next(a for a in self.actors if a["kind"] == core.COMBAT_MANAGER)["location"] = (10, 0, 0)
        problems = self.problems()
        self.assertIn("PlayerSpawn_Sq1 is at (-3500, 1500, 0), expected (-3500, -1500, 0).", problems)
        self.assertIn("BP_CombatManager is at (10, 0, 0), expected (0, 0, 0).", problems)

    def test_reports_too_few_enemy_spawns(self):
        self.actors = [a for a in self.actors if a["location"] != (3500, -600, 0)]
        problems = self.problems()
        self.assertIn("Found 5 TargetPoints tagged 'EnemySpawn'; the battle spawns up to 6 interceptors.", problems)
        self.assertIn("No TargetPoint tagged 'EnemySpawn' at (3500, -600, 0).", problems)

    def test_enemy_spawn_positions_match_one_to_one(self):
        doubled = next(a for a in self.actors if a["location"] == (3500, -600, 0))
        doubled["location"] = (3500, 600, 0)
        self.assertIn("No TargetPoint tagged 'EnemySpawn' at (3500, -600, 0).", self.problems())

    def test_location_tolerance(self):
        next(a for a in self.actors if a["kind"] == core.PLAYER_START_KIND)["location"] = (-4000.5, 0.4, 0)
        self.assertEqual(core.validate_combat_level("GM_Tactical", self.actors), [])

    def test_reports_missing_light_post_process_and_manager(self):
        self.actors = [a for a in self.actors if a["kind"] not in (core.DIRECTIONAL_LIGHT, core.COMBAT_MANAGER)]
        next(a for a in self.actors if a["kind"] == core.POST_PROCESS)["unbound"] = False
        problems = self.problems()
        for expected in ("No DirectionalLight.", "No PostProcessVolume with Infinite Extent",
                         "Expected one BP_CombatManager, found 0."):
            self.assertIn(expected, problems)


if __name__ == "__main__":
    unittest.main()
