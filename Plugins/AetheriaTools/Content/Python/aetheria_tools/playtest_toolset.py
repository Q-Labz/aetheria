"""Playtest helpers for a running PIE session: jump to an act and read the campaign state.

Editor-only test tooling. Nothing here ships or runs in the game itself.
"""
import unreal
import toolset_registry


def _game_world():
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    if not world:
        raise RuntimeError("No PIE session is running.")
    return world


def _set_captain(gi, captain_name, background):
    """Names the Captain and sets the background, if no Captain was created yet."""
    if gi.get_editor_property("captain_name"):
        return
    gi.set_editor_property("captain_name", captain_name)
    # E_SynodEnvoy -> SYNOD_ENVOY in Python
    member = "".join("_" + c if c.isupper() and i else c for i, c in enumerate(background)).upper()
    if hasattr(unreal.E_Background, member):
        gi.set_editor_property("background", getattr(unreal.E_Background, member))


def _game_instance():
    gi = unreal.GameplayStatics.get_game_instance(_game_world())
    if not gi:
        raise RuntimeError("The PIE world has no game instance.")
    return gi


@unreal.uclass()
class AetheriaPlaytestToolset(unreal.ToolsetDefinition):
    """Playtest shortcuts for a running PIE session of Aetheria: jump into Act VII or a later act and read the
    campaign state (node, act, flags, standing, supply line, battle) without replaying earlier content."""

    @toolset_registry.tool_call
    @staticmethod
    def start_act_vii(captain_name: str = "Rook", background: str = "SynodEnvoy") -> str:
        """Carries the running PIE campaign into Act VII at the Ember Relay, as the slice end's Continue does.

        Args:
            captain_name: Captain name to use if none was entered yet
            background: SynodEnvoy, GuildEngineer, CoalitionVanguard or OutcastCorsair, if none was chosen yet

        Returns:
            Confirmation.
        """
        gi = _game_instance()
        _set_captain(gi, captain_name, background)
        pc = unreal.GameplayStatics.get_player_controller(_game_world(), 0)
        if not pc or not hasattr(pc, "start_act_vii"):
            raise RuntimeError("The PIE player controller is not PC_Command; start PIE from L_Command.")
        pc.start_act_vii()
        return "Act VII started; the command level reloads at the Ember Relay."

    @toolset_registry.tool_call
    @staticmethod
    def start_act(act: int, captain_name: str = "Rook", background: str = "SynodEnvoy") -> str:
        """Carries the running PIE campaign into a data-driven act (7 = The Cinder Archive, 8 = The Iron Tithe,
        9 = The Threshold, 10 = Gate Zero)
        at its start node. Earlier acts' choices are not replayed: set any carried flags or states with
        set_campaign_values afterwards.

        Args:
            act: Act number, 7 or later; the act's chart (DT_StarNodes_Act<N>) must exist
            captain_name: Captain name to use if none was entered yet
            background: SynodEnvoy, GuildEngineer, CoalitionVanguard or OutcastCorsair, if none was chosen yet

        Returns:
            Confirmation.
        """
        if act < 7:
            raise ValueError("Acts before VII are the slice; start PIE from L_Command instead.")
        if act == 7:
            return AetheriaPlaytestToolset.start_act_vii(captain_name, background)
        gi = _game_instance()
        if not gi.has_act_data(act):
            raise RuntimeError(f"Act {act} has no chart yet (DT_StarNodes_Act<N>).")
        pc = unreal.GameplayStatics.get_player_controller(_game_world(), 0)
        if not pc or not hasattr(pc, "start_act"):
            raise RuntimeError("The PIE player controller is not PC_Command; start PIE from L_Command.")
        _set_captain(gi, captain_name, background)
        pc.start_act(act)
        return f"Act {act} started; the command level reloads at the act's start node."

    @toolset_registry.tool_call
    @staticmethod
    def get_campaign_state() -> str:
        """Summarises the running campaign: act, node, standing, supply line, cure, pending battle and story flags.

        Returns:
            A one-line summary followed by the story flags.
        """
        gi = _game_instance()
        flags = sorted(str(f) for f in gi.get_editor_property("story_flags"))
        states = {str(k): str(v) for k, v in gi.get_editor_property("states").items()}
        loyalty = {str(k).split(".")[-1].split(":")[0]: v for k, v in gi.get_editor_property("crew_loyalty").items()}
        return (f"act={gi.get_editor_property('current_act')} background={gi.get_editor_property('background')} node={gi.get_editor_property('current_node')} "
                f"next={gi.get_editor_property('next_screen')} battle={gi.get_editor_property('pending_battle')} "
                f"standing={gi.get_editor_property('severant_standing')} leaning={gi.get_editor_property('architect_leaning')} "
                f"instability={gi.get_editor_property('instability'):.0f} pursuit={gi.get_editor_property('pursuit_clock')} "
                f"hull={gi.get_editor_property('flagship_hull_pct'):.2f} phantoms={gi.get_editor_property('phantom_ships_active')} "
                f"destroyed={gi.get_editor_property('phantom_ships_destroyed')} loyalty={loyalty} states={states} flags={flags}")

    @toolset_registry.tool_call
    @staticmethod
    def set_campaign_values(standing: int = -1000, add_flags: list[str] = [], instability: float = -1.0,
                            set_states: list[str] = [], loyalty: list[str] = [], leaning: int = -1000,
                            phantoms: int = -1) -> str:
        """Adjusts the running campaign for a test (only the arguments given are changed).

        Args:
            standing: Severant standing to set (-1000 leaves it)
            add_flags: Story flags to add
            instability: Star-Drive instability to set (-1 leaves it)
            set_states: Campaign states as Key=Value, e.g. ExarchStatus=Alive, VanceFate=Released
            loyalty: Officer loyalty as Officer=Value, e.g. TalaVane=80, NixNull=-10
            leaning: Architect leaning to set, -100 (Dominion) to 100 (Custodian) (-1000 leaves it)
            phantoms: Phantom Fleet vessels active, e.g. 180 after Squadron III was alienated (-1 leaves it)

        Returns:
            The new state summary.
        """
        gi = _game_instance()
        if standing > -1000:
            gi.set_editor_property("severant_standing", standing)
        if add_flags:
            flags = gi.get_editor_property("story_flags")
            for flag in add_flags:
                flags.add(flag)
            gi.set_editor_property("story_flags", flags)
        if instability >= 0.0:
            gi.set_editor_property("instability", instability)
        if set_states:
            states = gi.get_editor_property("states")
            for entry in set_states:
                key, _, value = entry.partition("=")
                states[key] = value
            gi.set_editor_property("states", states)
        if loyalty:
            crew = gi.get_editor_property("crew_loyalty")
            for entry in loyalty:
                officer, _, value = entry.partition("=")
                # TalaVane -> TALA_VANE in Python
                member = "".join("_" + c if c.isupper() and i else c for i, c in enumerate(officer)).upper()
                crew[getattr(unreal.E_Speaker, member)] = int(value)
            gi.set_editor_property("crew_loyalty", crew)
        if leaning > -1000:
            gi.set_editor_property("architect_leaning", leaning)
        if phantoms >= 0:
            gi.set_editor_property("phantom_ships_active", phantoms)
        return AetheriaPlaytestToolset.get_campaign_state()

    @toolset_registry.tool_call
    @staticmethod
    def stage_battle(battle: str, node: str = "", council: str = "") -> str:
        """Opens a DT_Battles battle in the running PIE session (Act VII state), as a council's StartBattle does.

        Args:
            battle: DT_Battles row, e.g. E1, E2, E4_Assault
            node: Star node to fight at, which picks the combat scenery (empty keeps the current node)
            council: DT_Councils row that started the battle (marked held; reopened if the battle is lost)

        Returns:
            Confirmation.
        """
        gi = _game_instance()
        if node:
            gi.set_editor_property("current_node", node)
        gi.set_editor_property("current_act", max(7, gi.get_editor_property("current_act")))
        gi.set_editor_property("pending_battle", battle)
        if council:
            flags = gi.get_editor_property("story_flags")
            flags.add(f"Council_{council}")
            gi.set_editor_property("story_flags", flags)
        gi.set_editor_property("pending_battle_council", council or "None")
        gi.set_editor_property("next_screen", "PostCombat")
        unreal.GameplayStatics.open_level(_game_world(), "L_Combat_LanternDrift")
        return f"Staging {battle} in L_Combat_LanternDrift."

    @toolset_registry.tool_call
    @staticmethod
    def win_battle(return_to_command: bool = False) -> str:
        """Wins the battle in the running combat level: every hostile ship dies, as if destroyed in the fight,
        and a hold battle's timer runs out.

        Args:
            return_to_command: Then leave the after-action report for the bridge, as its Continue does

        Returns:
            How many ships died and the campaign state.
        """
        world = _game_world()
        killed = 0
        for ship in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.AetheriaShipBase):
            if ship.get_editor_property("faction") != unreal.E_Faction.PLAYER:
                ship.die()
                killed += 1
        managers = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.AetheriaCombatManager)
        if not managers:
            raise RuntimeError("The PIE world has no combat manager; stage a battle first.")
        managers[0].force_victory()
        if return_to_command:
            managers[0].return_to_command()
        return f"{killed} hostile ships destroyed. " + AetheriaPlaytestToolset.get_campaign_state()

    @toolset_registry.tool_call
    @staticmethod
    def lose_battle(return_to_command: bool = False) -> str:
        """Loses the battle in the running combat level, as if the Crystal Dreadnought fell.

        Args:
            return_to_command: Then withdraw to the bridge, as the after-action report's Return to Command does

        Returns:
            The campaign state.
        """
        managers = unreal.GameplayStatics.get_all_actors_of_class(_game_world(), unreal.AetheriaCombatManager)
        if not managers:
            raise RuntimeError("The PIE world has no combat manager; stage a battle first.")
        managers[0].force_defeat()
        if return_to_command:
            managers[0].return_to_command()
        return AetheriaPlaytestToolset.get_campaign_state()

    @toolset_registry.tool_call
    @staticmethod
    def go_to_node(node: str, add_flags: list[str] = []) -> str:
        """Puts the running Act VII campaign at a star node and runs its arrival, as a leap landing there does.

        Args:
            node: Act VII node, e.g. VaultOfRecords
            add_flags: Story flags to add first (e.g. GateOpen)

        Returns:
            Confirmation.
        """
        gi = _game_instance()
        if add_flags:
            flags = gi.get_editor_property("story_flags")
            for flag in add_flags:
                flags.add(flag)
            gi.set_editor_property("story_flags", flags)
        gi.set_editor_property("current_node", node)
        pc = unreal.GameplayStatics.get_player_controller(_game_world(), 0)
        if not pc or not hasattr(pc, "run_node"):
            raise RuntimeError("The PIE player controller is not PC_Command.")
        pc.run_node(node)
        return f"At {node}."
