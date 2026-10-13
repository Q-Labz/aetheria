# Aetheria: Beyond the Arch — build instructions for Claude Code

Unreal Engine 5 project, **Blueprint only (no C++)**. Build it through the `unreal-mcp` server
(`.mcp.json`, http://127.0.0.1:8000/mcp) against the live editor. `SLICE_SPEC.md` is the
source of truth for every asset; this file is the order of work.

## Rules
- Discover tools with `list_toolsets` / `describe_toolset`; dispatch with `call_tool`.
  At session start also run `AgentSkillToolset.ListSkills` and follow any relevant project skill.
- Save all before and after each phase. Stop PIE before creating assets.
- Read every tool result. Compile each Blueprint after editing it; a failed compile stops the phase.
- Never add C++ classes or a Source/ folder. Game logic lives in Blueprints; editor Python is allowed
  only for bulk asset setup (imports, folders), never for runtime logic.
- Canon is fixed: names, ship classes, counts (214 ships = 1 Dreadnought + 1 Wayward + 200 Phantom + 12 Corsair),
  abilities. Narrator/AI lines use a formal naval-briefing register.
- Crew and AI advise; the player decides. Never auto-resolve a choice.

## Build order (each phase ends with its check)
1. **Foundation** — folders; materials M_Emissive, M_Crystal, M_Hostile; import `Source_Art/Meshes/*.fbx`
   to /Game/Aetheria/Meshes (Uniform Scale 0.01, Nanite on, no collision).
   Check: SM_CrystalDreadnought bounds ≈ 1,020 uu long.
2. **Data** — enums E_Background, E_Speaker, E_Faction, E_Encounter; structs S_StarNode, S_LeapRoute,
   S_Line, S_Sequence, S_Counsel, S_CouncilOption; DT_StarNodes and DT_LeapRoutes from `BuildData/*.csv`;
   DT_Sequences, DT_Council_Relay, DT_Council_Directive from SLICE_SPEC §11.
   Check: row counts 3 / 6 / 8 / 4 / 3.
3. **Game Instance** — BP_AetheriaGameInstance per §4 (already referenced by Config/DefaultEngine.ini).
   Check: compiles; GetLeapCost(EmberRelay) from ArchWake at defaults returns 80 / 105 / overflow.
4. **Input** — IA_* (Trigger when Paused = TRUE on all) and IMC_Tactical per §5.
5. **Ships** — BP_BeamVFX, BP_ShipBase, BP_Flagship, BP_PhantomSquadron, BP_CorsairGunship,
   BP_TitheInterceptor, BP_TitheCruiser, WBP_ShipBar, BP_MoveMarker per §9.
6. **Combat** — BP_CommandPawn, BP_TacticalController, BP_CombatManager, GM_Tactical, WBP_CombatHUD,
   WBP_AfterAction; level L_Combat_LanternDrift with tagged TargetPoints per §7/§10.
   Check (PIE on the combat map): opens paused; Q + click while paused queues the beam; Space fires it;
   enemies die; victory report shows.
7. **Command layer** — GM_Command, PC_Command, WBP_CaptainCreation, WBP_Dialogue, WBP_StarMap,
   WBP_NodeButton, WBP_Council, WBP_OptionButton, WBP_SliceEnd; level L_Command per §6.
8. **End-to-end test** — PIE from L_Command, three runs:
   a) direct leap to Ember Relay → rupture with "external harmonic" note → combat → council;
   b) Sandbox + Passage → Custodian; c) Grant + Arch → Dominion (Phantom Fleet −20).
9. **Package** — Windows, Shipping. Both maps are already in Config/DefaultGame.ini MapsToCook.
