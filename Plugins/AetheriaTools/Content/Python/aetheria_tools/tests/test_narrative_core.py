"""Engine-independent tests for narrative_core. Run outside Unreal:
    python -m unittest discover -s Plugins/AetheriaTools/Content/Python -p "test_narrative_core.py" -t Plugins/AetheriaTools/Content/Python
"""
import copy
import json
import os
import tempfile
import unittest

from aetheria_tools import narrative_core as core

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), *[".."] * 6))
SOURCE_DIR = os.path.join(PROJECT_ROOT, "BuildData", "Narrative")


class NarrativeCoreTest(unittest.TestCase):
    def setUp(self):
        self.data = core.load_source(SOURCE_DIR)

    def test_shipped_source_is_valid(self):
        self.assertEqual(core.validate(self.data), [])

    def test_row_counts_match_build_plan(self):
        counts = {t: len(rows) for t, rows in self.data.items()}
        self.assertEqual(counts, {"DT_Sequences": 8, "DT_Council_Relay": 4, "DT_Council_Directive": 3})

    def test_load_source_raises_when_file_missing(self):
        with tempfile.TemporaryDirectory() as empty:
            with self.assertRaises(FileNotFoundError):
                core.load_source(empty)

    def test_load_source_raises_when_not_array(self):
        with tempfile.TemporaryDirectory() as tmp:
            for table in core.TABLES:
                with open(os.path.join(tmp, f"{table}.json"), "w") as f:
                    json.dump({"not": "an array"}, f)
            with self.assertRaises(ValueError):
                core.load_source(tmp)

    def test_validate_reports_unknown_speaker(self):
        data = copy.deepcopy(self.data)
        data["DT_Sequences"][0]["Lines"][0]["Speaker"] = "Stranger"
        self.assertIn("unknown speaker 'Stranger'", "\n".join(core.validate(data)))

    def test_validate_reports_flag_nothing_sets(self):
        data = copy.deepcopy(self.data)
        data["DT_Sequences"][0]["Lines"][0]["RequiredFlag"] = "NeverSet"
        self.assertIn("requires flag 'NeverSet'", "\n".join(core.validate(data)))

    def test_validate_reports_missing_and_duplicate_rows(self):
        data = copy.deepcopy(self.data)
        data["DT_Council_Directive"] = [r for r in data["DT_Council_Directive"] if r["Name"] != "Arch"]
        data["DT_Council_Relay"].append(copy.deepcopy(data["DT_Council_Relay"][0]))
        problems = "\n".join(core.validate(data))
        self.assertIn("missing required row 'Arch'", problems)
        self.assertIn("duplicate row 'Grant'", problems)

    def test_validate_reports_unknown_override_background(self):
        data = copy.deepcopy(self.data)
        data["DT_Sequences"][0]["Lines"][0]["BackgroundOverride"] = {"Pirate": "Arr."}
        self.assertIn("unknown background 'Pirate'", "\n".join(core.validate(data)))

    def test_both_architect_outcomes_are_reachable_for_every_background(self):
        outcomes = core.architect_outcomes(self.data)
        for background in core.BACKGROUNDS:
            reached = {o["disposition"] for o in outcomes if o["background"] == background}
            self.assertEqual(reached, {"Custodian", "Dominion", "Undetermined"}, background)

    def test_rite_only_offered_to_synod_envoy(self):
        outcomes = core.architect_outcomes(self.data)
        rite_backgrounds = {o["background"] for o in outcomes if o["relay"] == "Rite"}
        self.assertEqual(rite_backgrounds, {"SynodEnvoy"})

    def test_spec_reachability_examples(self):
        by_path = {(o["relay"], o["directive"]): o for o in core.architect_outcomes(self.data)}
        self.assertEqual(by_path[("Grant", "Arch")]["leaning"], -35)
        self.assertEqual(by_path[("Sandbox", "Passage")]["disposition"], "Custodian")
        self.assertEqual(by_path[("Sandbox", "Arch")]["disposition"], "Undetermined")
        self.assertEqual(by_path[("Rite", "Passage")]["leaning"], 45)

    def test_disposition_thresholds(self):
        self.assertEqual(core.disposition(20), "Custodian")
        self.assertEqual(core.disposition(19), "Undetermined")
        self.assertEqual(core.disposition(-19), "Undetermined")
        self.assertEqual(core.disposition(-20), "Dominion")

    def test_resolve_sequence_applies_background_override_and_name(self):
        lines = core.resolve_sequence(self.data, "Briefing", "CoalitionVanguard", set(), "Rhen")
        self.assertIn("You and I held the line", lines[2][1])
        self.assertTrue(lines[1][1].startswith("Captain Rhen."))

    def test_resolve_sequence_filters_by_flag(self):
        without = core.resolve_sequence(self.data, "ArchitectReveal", "GuildEngineer", set(), "X")
        with_all = core.resolve_sequence(
            self.data, "ArchitectReveal", "GuildEngineer",
            {"AnomalousDamping", "BeamAnomaly", "UsedArchConduit"}, "X")
        self.assertEqual(len(without), 3)
        self.assertEqual(len(with_all), 6)

    def test_resolve_sequence_raises_on_unknown_sequence(self):
        with self.assertRaises(KeyError):
            core.resolve_sequence(self.data, "Nope", "SynodEnvoy", set(), "X")

    def test_resolve_sequence_raises_on_unknown_background(self):
        with self.assertRaises(ValueError):
            core.resolve_sequence(self.data, "Briefing", "Pirate", set(), "X")

    def test_starting_state_applies_background_bonus(self):
        guild = core.starting_state("GuildEngineer")
        synod = core.starting_state("SynodEnvoy")
        self.assertEqual(guild["crew_loyalty"]["JaxMercer"], 65)
        self.assertEqual(synod["crew_loyalty"], core.START_LOYALTY)
        self.assertEqual((synod["instability"], synod["phantom_ships_active"]), (25.0, 200))

    def test_starting_state_raises_on_unknown_background(self):
        with self.assertRaises(ValueError):
            core.starting_state("Pirate")

    def test_grant_then_arch_alienates_squadron_three(self):
        before = core.starting_state("SynodEnvoy")
        after = core.council_path(self.data, before, "Grant", "Arch")
        self.assertEqual(after["architect_leaning"], -35)
        self.assertEqual(after["phantom_ships_active"], 180)
        self.assertEqual(after["story_flags"], ["ArchitectFullAccess", "SquadronIIIAlienated"])
        self.assertEqual(after["crew_loyalty"]["TalaVane"], 45)
        self.assertEqual(before["phantom_ships_active"], 200)  # input is not modified

    def test_sandbox_then_passage_makes_a_custodian(self):
        before = dict(core.starting_state("GuildEngineer"), instability=70.0)
        after = core.council_path(self.data, before, "Sandbox", "Passage")
        self.assertEqual(after["architect_leaning"], 35)
        self.assertEqual((after["instability"], after["spire_recalibrations"]), (40.0, 2))
        self.assertEqual(after["chrono_charge"], 40.0)
        self.assertEqual(after["crew_loyalty"]["NixNull"], 60)

    def test_sandbox_then_arch_leaves_the_architect_silent(self):
        after = core.council_path(self.data, core.starting_state("SynodEnvoy"), "Sandbox", "Arch")
        self.assertEqual(core.disposition(after["architect_leaning"]), "Undetermined")
        self.assertIn("ArchitectSilent", after["story_flags"])

    def test_council_path_clamps_values(self):
        before = dict(core.starting_state("SynodEnvoy"), instability=95.0, chrono_charge=5.0)
        refuse = core.council_path(self.data, before, "Refuse", "Command")
        sandbox = core.council_path(self.data, before, "Sandbox", "Command")
        self.assertEqual(refuse["instability"], 100.0)
        self.assertEqual(sandbox["chrono_charge"], 0.0)

    def test_council_path_raises_for_option_not_offered(self):
        with self.assertRaises(ValueError):
            core.council_path(self.data, core.starting_state("GuildEngineer"), "Rite", "Passage")

    def test_council_path_raises_for_unknown_option(self):
        with self.assertRaises(KeyError):
            core.council_path(self.data, core.starting_state("SynodEnvoy"), "Grant", "Nope")


if __name__ == "__main__":
    unittest.main()
