# Aetheria: Beyond the Arch — build instructions for Claude Code

Unreal Engine **5.8** project: Blueprints on top of one C++ game module (`Source/Aetheria`). Build it through
the `unreal-mcp` server against the live editor; the server URL is in `.mcp.json` (http://127.0.0.1:8010/mcp),
and the editor auto-starts it via `Config/DefaultEditorPerProjectUserSettings.ini`. `SLICE_SPEC.md` is the
source of truth for every asset; this file is the order of work.

## Rules
- Discover tools with `list_toolsets` / `describe_toolset`; dispatch with `call_tool`.
  At session start also run `AgentSkillToolset.ListSkills` and follow any relevant project skill.
- Save all before and after each phase. Stop PIE before creating assets.
- Read every tool result. Compile each Blueprint after editing it; a failed compile stops the phase.
- C++ lives only in the `Aetheria` game module (`Source/Aetheria`): `AAetheriaCharacter`, `AAetheriaPlayerController`
  and `AAetheriaGameMode` are the parents of BP_TopDownCharacter / BP_TopDownController / BP_TopDownGameMode.
  C++ may be added there; don't add other modules, and keep the AetheriaTools plugin Python-only. After a C++ change,
  rebuild (or Live Coding) before touching the Blueprints that depend on it. Slice gameplay in SLICE_SPEC.md stays in
  Blueprints unless it moves to C++ deliberately. Editor Python is allowed only for bulk asset setup (imports,
  folders), never for runtime logic.
- Canon is fixed: names, ship classes, counts (214 ships = 1 Dreadnought + 1 Wayward + 200 Phantom + 12 Corsair),
  abilities. Narrator/AI lines use a formal naval-briefing register.
- Crew and AI advise; the player decides. Never auto-resolve a choice.

## Build order (each phase ends with its check)
1. **Foundation** — folders; materials M_Emissive, M_Crystal, M_Hostile; import `Source_Art/Meshes/*.fbx`
   to /Game/Aetheria/Meshes and `Source_Art/Environments/*.fbx` to /Game/Aetheria/Environments
   (Uniform Scale 0.01, Nanite on, no collision).
   Check: SM_CrystalDreadnought bounds ≈ 1,020 uu long.
2. **Data** — enums E_Background, E_Speaker, E_Faction, E_Encounter; structs S_StarNode, S_LeapRoute,
   S_Line, S_Sequence, S_Counsel, S_CouncilOption; DT_StarNodes and DT_LeapRoutes from `BuildData/*.csv`;
   then call `AetheriaNarrativeToolset.load_narrative_tables` (plugin `AetheriaTools`) to build
   DT_Sequences, DT_Council_Relay and DT_Council_Directive from `BuildData/Narrative/*.json`.
   Enter E_Speaker / E_Background values with display names exactly as in SLICE_SPEC §1.
   Check: row counts 3 / 6 / 8 / 4 / 3; `validate_narrative` returns no problems;
   `AetheriaStarChartToolset.validate_star_chart(DT_StarNodes, DT_LeapRoutes, "ArchWake")` returns no problems.
   Narrative edits go in `BuildData/Narrative/*.json`, then re-run `load_narrative_tables`.
3. **Game Instance** — BP_AetheriaGameInstance per §4 (already referenced by Config/DefaultEngine.ini).
   Check: compiles; GetLeapCost(EmberRelay) from ArchWake at defaults returns 80 / 105 / overflow.
4. **Input** — IA_* (Trigger when Paused = TRUE on all) and IMC_Tactical per §5.
5. **Ships** — BP_BeamVFX, BP_ShipBase, BP_Flagship, BP_PhantomSquadron, BP_CorsairGunship,
   BP_TitheInterceptor, BP_TitheCruiser, WBP_ShipBar, BP_MoveMarker per §9.
   Check: `AetheriaCombatToolset.validate_fleet()` returns no problems.
6. **Combat** — BP_CommandPawn, BP_TacticalController, BP_CombatManager, GM_Tactical, WBP_CombatHUD,
   WBP_AfterAction; level L_Combat_LanternDrift with tagged TargetPoints per §7/§10.
   Check: with the combat map open, `AetheriaCombatToolset.validate_combat_level()` returns no problems.
   Check (PIE on the combat map): opens paused; Q + click while paused queues the beam; Space fires it;
   enemies die; victory report shows.
7. **Command layer** — GM_Command, PC_Command, WBP_CaptainCreation, WBP_Dialogue, WBP_StarMap,
   WBP_NodeButton, WBP_Council, WBP_OptionButton, WBP_SliceEnd; level L_Command per §6.
8. **End-to-end test** — PIE from L_Command, three runs:
   a) direct leap to Ember Relay → rupture with "external harmonic" note → combat → council;
   b) Sandbox + Passage → Custodian; c) Grant + Arch → Dominion (Phantom Fleet −20).
9. **Package** — Windows, Shipping. Both maps are already in Config/DefaultGame.ini MapsToCook.
