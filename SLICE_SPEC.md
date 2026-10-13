# Aetheria: Beyond the Arch — Vertical Slice Spec (UE5, Blueprint only)

Canonical build spec. `CLAUDE.md` gives the build order; this file gives the exact contents.
Notation: `→` = exec flow, `[Node]` = Blueprint node. All assets live under `/Game/Aetheria/`.

Slice flow: name the Captain and pick a background → Vex AI briefing → star chart (Arch Wake / Lantern
Drift / Ember Relay; Instability starts at 25) → leaping to the Lantern Drift (or straight to the Ember
Relay) triggers Vance's picket battle (real-time, pausable, Prismatic Stellar Beam) → after-action report →
council at the Ember Relay → the Architect reveals itself (lines reflect what the player did) → directive
question → visible consequence → end-of-slice summary.

---

## 0. Project
- Blank Blueprint project (already scaffolded: `Aetheria.uproject`, `Config/*.ini`).
- Folders: Core, Data, Input, Ships, Combat, UI, Maps, Materials, Meshes.
- Materials (placeholders):
  - `M_Emissive`: Unlit; Vector Param `Color` × Scalar Param `Glow` (20) → Emissive. Beams, rings, tracers.
  - `M_Crystal`: Default Lit; BaseColor (0.05,0.15,0.25), Metallic 0.3, Roughness 0.1,
    Emissive = Fresnel(Exponent 3) × (0.3,0.8,1.0) × 8.
  - `M_Hostile`: Default Lit; BaseColor (0.25,0.04,0.02), Emissive = Fresnel × (1,0.2,0.05) × 4.
- Meshes: import `Source_Art/Meshes/*.fbx` to `/Game/Aetheria/Meshes` with Import Uniform Scale **0.01**,
  Nanite ON, no collision. Assign `M_Crystal` to Dreadnought/Phantom slots, `M_Hostile` to Tithe slots.
- PostProcessVolume per map: Infinite Extent ON, Bloom 1.5, Exposure Min = Max = 1.

## 1. Enumerations (Data/)
| Enum | Values |
|---|---|
| `E_Background` | SynodEnvoy, GuildEngineer, CoalitionVanguard, OutcastCorsair |
| `E_Speaker` | Narrator, Captain, TalaVane, JaxMercer, ArisThorne, NixNull, VexAI, Lyra, Cormaker, CoreArchitect, CommanderVance |
| `E_Faction` | Player, VanceRemnant |
| `E_Encounter` | None, Combat, Council |

## 2. Structures (Data/)
- `S_StarNode`: NodeID (Name), DisplayName (Text), Description (Text), MapPos (Vector2D), Encounter (E_Encounter)
- `S_LeapRoute`: FromNode (Name), ToNode (Name), BaseCost (Float)
- `S_Line`: Speaker (E_Speaker), Text (Text), BackgroundOverride (Map E_Background→Text), RequiredFlag (Name, default None)
- `S_Sequence`: Lines (Array S_Line)
- `S_Counsel`: Officer (E_Speaker), Advice (Text), BackgroundOverride (Map E_Background→Text)
- `S_CouncilOption`: Label (Text), ApprovalHint (Text), RequiresBackground (Bool), RequiredBackground (E_Background),
  LeaningDelta (Int), LoyaltyDeltas (Map E_Speaker→Int), InstabilityDelta (Float), ChronoDelta (Float),
  SetFlag (Name), Outcome (Array S_Line)

## 3. Data tables (Data/)
- `DT_StarNodes` (S_StarNode) — import `BuildData/DT_StarNodes.csv`.
- `DT_LeapRoutes` (S_LeapRoute) — import `BuildData/DT_LeapRoutes.csv`. Row name = `From_To`.
  Design math: direct leap 25+80 = 105 → overflow. Two-step: 25+35 = 60 → Strained → 30×1.25 = 37.5 → 97.5
  Critical. Chart that lane → 78.75. Vent first → 40+30 = 70.
- `DT_Sequences` (S_Sequence) — rows: Briefing, CombatIntro, PostCombat, VanceIntercept, ArchitectReveal,
  Consequence_Custodian, Consequence_Dominion, Consequence_Undetermined. Lines in §11.
- `DT_Council_Relay`, `DT_Council_Directive` (S_CouncilOption) — rows in §11.

## 4. Game Instance — `BP_AetheriaGameInstance` (Core/)
**Variables (defaults):** CaptainName String "" · Background E_Background SynodEnvoy · Instability Float 25 ·
ChronoCharge Float 60 · FlagshipHullPct Float 1.0 · CrewInjured Int 0 · BioGelDoses Int 2 ·
SpireRecalibrations Int 1 · PhantomShipsActive Int 200 · PursuitClock Int 0 · CurrentNode Name ArchWake ·
ChartedRoutes Set<Name> · StoryFlags Set<Name> · ArchitectLeaning Int 0 · ArchitectExposure Int 0 ·
FirstRuptureAbsorbed Bool false · CrewLoyalty Map<E_Speaker,Int> = {TalaVane:50, JaxMercer:50, ArisThorne:50,
NixNull:50, Lyra:50, Cormaker:40} · NextScreen Name "NewGame" (NewGame | PostCombat | StarMap) · LastReport Text

**Event dispatcher:** OnCampaignStateChanged

**Functions**
```
RouteName(To)→Name           [Append] CurrentNode,"_",To → [String to Name]
GetLeapCost(To)→Cost,Projected,WillOverflow,Found   (Pure)
  [Get Data Table Row DT_LeapRoutes, RouteName(To)] → not found: Found=false, return
  Cost = BaseCost
  ChartedRoutes Contains RouteName(To) → Cost *= 0.5
  Instability >= 50 → Cost *= 1.25              // Strained
  Background == GuildEngineer → Cost *= 0.9
  Projected = Instability + Cost ; WillOverflow = Projected > 100
PreviewRupture(Excess)→HullLossPct,Injured,Desync   (Pure)
  E = FirstRuptureAbsorbed ? Excess : Excess*0.5
  HullLossPct = 0.08 + E*0.012 ; Injured = 1 + Floor(E/8) ; Desync = 2*Floor(E/5)
ExecuteLeap(To)→E_Encounter
  GetLeapCost(To) → !Found → return None
  Instability >= 80 → CrewInjured += 1 ; LastReport += "Containment bleed: 1 crew injured."
  WillOverflow ? ResolveRupture(Projected-100)
               : (Instability = Projected ; LastReport = "Stellar Leap complete. Containment holding.")
  CurrentNode = To → return DT_StarNodes[To].Encounter → Call OnCampaignStateChanged
ResolveRupture(Excess)
  PreviewRupture(Excess) → H,I,D
  !FirstRuptureAbsorbed → FirstRuptureAbsorbed=true ; StoryFlags.Add("AnomalousDamping") ;
     ArchitectExposure += 1 ; Note = " Rupture severity reduced by an external harmonic. Origin: unidentified."
  FlagshipHullPct = Max(0.15, FlagshipHullPct - H) ; CrewInjured += I ; PhantomShipsActive -= D ; Instability = 60
  LastReport = "STAR-DRIVE RUPTURE. Hull integrity reduced {H}%. {I} crew injured. {D} Phantom vessels desynchronised.{Note}"
ChartRoute(To)      not charted → ChartedRoutes.Add ; PursuitClock += 1 ; ModifyLoyalty(Lyra,+2)
VentChrono()        ChronoCharge >= 25 → ChronoCharge -= 25 ; Instability = Max(0, Instability-20)
HoldStation()       Instability = Max(0, Instability-10) ; PursuitClock += 1
SpireRecalibrate()  SpireRecalibrations > 0 → -=1 ; Instability = Max(0, Instability-40) ; ArchitectExposure += 1 ;
                    StoryFlags.Add("UsedArchConduit") ; LastReport = "Outpost One recalibration received. Note: the
                    signal was relayed through the Omega Arch. The Architect forwarded it without being asked."
TreatInjured()      BioGelDoses>0 && CrewInjured>0 → BioGelDoses -= 1 ; CrewInjured = Max(0,CrewInjured-2) ;
                    ModifyLoyalty(ArisThorne,+2)
ModifyLoyalty(Who,Delta)  [Map Find] → Clamp(v+Delta,-100,100) → [Map Add]
ApplyOption(Opt)
  ArchitectLeaning = Clamp(ArchitectLeaning + Opt.LeaningDelta, -100, 100)
  For Each key of Opt.LoyaltyDeltas → ModifyLoyalty
  Instability = Clamp(Instability + Opt.InstabilityDelta, 0, 100) ; ChronoCharge = Clamp(ChronoCharge + Opt.ChronoDelta, 0, 100)
  Opt.SetFlag != None → StoryFlags.Add(Opt.SetFlag) → Call OnCampaignStateChanged
LeaningState()→Name (Pure)   >=20 "Custodian" | <=-20 "Dominion" | else "Undetermined"
ResolveLine(L)→Out Text, Show Bool (Pure)
  Show = L.RequiredFlag == None OR StoryFlags Contains L.RequiredFlag
  T = BackgroundOverride[Background] if found else L.Text
  Out = String(T) → Replace "{Captain}"→CaptainName → Replace "{Background}"→BackgroundTitle() → Text
BackgroundTitle()→String   Select: Synod Envoy | Guild Engineer | Coalition Vanguard | Outcast Corsair
SpeakerName(S)→Text   Select: "MISSION LOG","CAPTAIN","TALA VANE — FIRST OFFICER","JAX MERCER — CHIEF ENGINEER",
  "DR. ARIS THORNE — CMO","NIX \"NULL\" — NEURAL OPS","VEX AI — NAVIGATION","LYRA THE WEAVER",
  "CMDR. CORMAKER — CORSAIR ARMADA","THE CORE ARCHITECT","COMMANDER VANCE"
ApplyBackgroundStart()   GuildEngineer → ModifyLoyalty(JaxMercer,15) ; CoalitionVanguard → (TalaVane,15) ;
                         OutcastCorsair → (Cormaker,20) ; SynodEnvoy → nothing (unlocks Rite options)
ResetCampaign()          every variable back to its default ; NextScreen = "NewGame"
```

## 5. Enhanced Input (Input/) — every Input Action: **Trigger when Paused = TRUE**
IA_Select (Digital, LMB) · IA_Command (Digital, RMB) · IA_TogglePause (Digital, Space) · IA_AimBeam (Digital, Q) ·
IA_ChronoSurge (Digital, E) · IA_Group1/2/3 (Digital, 1/2/3) ·
IA_CameraPan (Axis2D: W [Swizzle YXZ], S [Swizzle YXZ + Negate], A [Negate], D) · IA_Zoom (Axis1D, Mouse Wheel).
`IMC_Tactical` maps all of them.

## 6. Command level & UI flow
- `GM_Command` (GameModeBase): Default Pawn None, PlayerController `PC_Command`.
- `L_Command`: empty level, GameMode Override `GM_Command`; distant `M_Emissive` spheres as stars; PostProcessVolume.

**PC_Command** (Show Mouse Cursor = true). Vars: GI, AfterDialogue Name.
```
BeginPlay → GI = Cast(Get Game Instance) → Set Input Mode UI Only → Switch on GI.NextScreen
   NewGame → ShowCreation() | PostCombat → PlaySequence("PostCombat","AfterCombat") | StarMap → ShowStarMap()
ShowCreation()   Create WBP_CaptainCreation → Add to Viewport
ShowStarMap()    Remove All Widgets → Create WBP_StarMap → Add to Viewport
PlaySequence(Row, AfterKey)  Get DT_Sequences row → Create WBP_Dialogue(Lines) → Bind OnDialogueFinished→OnSequenceDone
                             → AfterDialogue = AfterKey → Add to Viewport
OnSequenceDone → Switch on AfterDialogue
   "ToMap"          → ShowStarMap()
   "ToCombat"       → GI.NextScreen="PostCombat" → Open Level L_Combat_LanternDrift
   "AfterCombat"    → GI.CurrentNode==EmberRelay ? ShowCouncil(DT_Council_Relay,"Relay") : ShowStarMap()
   "AfterRelay"     → PlaySequence("ArchitectReveal","AfterReveal")
   "AfterReveal"    → ShowCouncil(DT_Council_Directive,"Directive")
   "AfterDirective" → ApplyArchitectConsequence() → PlaySequence("Consequence_"+GI.LeaningState(),"End")
   "End"            → Create WBP_SliceEnd → Add to Viewport
OnArrive(Encounter)
   !LanternCleared & Encounter==Combat     → PlaySequence("CombatIntro","ToCombat")
   !LanternCleared & CurrentNode==EmberRelay → PlaySequence("VanceIntercept","ToCombat")
    LanternCleared & CurrentNode==EmberRelay → ShowCouncil(DT_Council_Relay,"Relay")
   else → ShowStarMap()
ShowCouncil(Table, Key)  Create WBP_Council(CouncilTable=Table, CouncilKey=Key) → Add to Viewport
ApplyArchitectConsequence()  Switch on GI.LeaningState()
   Custodian    → Instability = Max(0, Instability-30) ; SpireRecalibrations += 1
   Dominion     → PhantomShipsActive -= 20 ; StoryFlags.Add("SquadronIIIAlienated")
   Undetermined → StoryFlags.Add("ArchitectSilent")
```

**WBP_CaptainCreation**: EditableTextBox NameBox (hint "Enter the Captain's name"), four background buttons,
TextBlock BackgroundDesc, Button Confirm (enabled when Len(Trim(NameBox))>0 AND HasPicked).
Descriptions:
- SynodEnvoy: "Diplomat of the Synod Key. Fluent in Precursor protocol. Unique dialogue with the Architect."
- GuildEngineer: "Forged in the Guild yards. Stellar Leap costs -10%. Jax Mercer trusts you."
- CoalitionVanguard: "Line officer of the Coalition. Prismatic Beam damage +15%. Tala Vane served with you."
- OutcastCorsair: "Ran with outcast miners. A Corsair gunship joins every battle. Cormaker vouches for you."
Confirm → GI.CaptainName, GI.Background → GI.ApplyBackgroundStart() → Remove from Parent →
PC_Command.PlaySequence("Briefing","ToMap").

**WBP_Dialogue**: bottom Border; SpeakerText (cyan caps), LineText (AutoWrap), Button "CONTINUE ▸"; Enter/Space via
OnKeyDown. Vars Lines (Expose on Spawn), Index 0. Dispatcher OnDialogueFinished.
`ShowCurrent()`: while Index < Lines.Length: ResolveLine → Show ? set texts & return : Index += 1. Past the end →
Call OnDialogueFinished → Remove from Parent. Continue → Index += 1 → ShowCurrent().

**WBP_StarMap** (the Stellar Leap decision screen): CanvasPanel MapCanvas (left 70%) + InfoPanel:
NodeTitle, NodeDesc, InstabilityBar (cyan <0.5, amber <0.8, red ≥0.8) with a ghost ProjectedBar overlaid,
LeapReadout "Base {B} | Charted ×0.5 | Strain ×1.25 | Projected {P} / 100", RuptureWarning (red, when
WillOverflow): "PROJECTED OVERFLOW. Expected rupture: hull −{H}%, {I} crew injured, {D} Phantom vessels
desynchronised.", AdvisorBark, buttons [EXECUTE STELLAR LEAP] [LYRA: CHART ROUTE] [VENT CHRONO-CORE (−20 / 25 charge)]
[HOLD STATION (−10 / Pursuit +1)] [SPIRE RECALIBRATION (−40)] [DR. THORNE: TREAT INJURED], status line
(Chrono, Hull, Injured, Phantom Fleet, Pursuit) and MatrixNote "NEURAL MATRIX: unregistered process present."
(visible when ArchitectExposure>0).
```
Construct → for each DT_StarNodes row: Create WBP_NodeButton(NodeID, ParentMap=self) → Add Child to Canvas →
   Set Position MapPos, Auto Size → bind GI.OnCampaignStateChanged → Refresh()
OnPaint → Draw Line per route (cyan if charted, grey otherwise)
SelectNode(ID) → SelectedNode = ID → Refresh()
Refresh() → texts from DT_StarNodes ; GI.GetLeapCost → !Found: "No direct lane." and disable Leap/Chart ;
   bars ; RuptureWarning via GI.PreviewRupture(Projected-100) ; button enables
   AdvisorBark (advise, never decide):
     WillOverflow → "VEX AI: Captain, this leap exceeds containment tolerance. I advise against it. The decision is yours."
     !Charted     → "LYRA: Give me one watch, Captain, and I can weave this lane. It will cost us time."
     Pursuit>=2   → "TALA VANE: Every hour we hold, Vance gains on us. I recommend we move."
     else         → "VEX AI: Lane is within tolerance. Awaiting your order."
Leap → Play FlashAnim (white, 0.6 s) → Enc = GI.ExecuteLeap(SelectedNode) → Delay 0.6 → toast GI.LastReport →
   Delay 1.5 → PC_Command.OnArrive(Enc)
Chart / Vent / Hold / Recal / Treat → matching GI function → Refresh (Recal also toasts LastReport)
```
**WBP_NodeButton**: Button + label; vars NodeID, ParentMap (Expose). OnClicked → ParentMap.SelectNode(NodeID).

**WBP_Council**: Title "COUNCIL OF COMMAND", Situation, CounselBox, OptionBox. Vars CouncilTable, CouncilKey (Expose).
Construct → situation & counsel per CouncilKey (§11) → for each row: show if !RequiresBackground or
RequiredBackground == GI.Background → Create WBP_OptionButton(Option, ParentCouncil). Shows Label + ApprovalHint;
**LeaningDelta is never shown.** ChooseOption(Opt) → GI.ApplyOption → Remove from Parent → WBP_Dialogue(Opt.Outcome)
with AfterDialogue = "AfterRelay" or "AfterDirective".

**WBP_SliceEnd**: "END OF VERTICAL SLICE — ACT VI: THE FAR SHORE", Captain/background, Instability, Hull, Injured,
Phantom Fleet, crew loyalty, "ARCHITECT DISPOSITION: {LeaningState} ({Leaning})". [New Command] → GI.ResetCampaign →
Open Level L_Command. [Quit] → Quit Game.

## 7. Combat level — `L_Combat_LanternDrift`
- `GM_Tactical`: Default Pawn `BP_CommandPawn`, PlayerController `BP_TacticalController`.
- Level: GameMode Override GM_Tactical; Directional Light; PostProcessVolume; backdrop Sphere scale −2000
  (M_Emissive dark blue, Glow 0.3); a few scaled emissive spheres as nebula lanterns.
- `BP_CombatManager` at origin. TargetPoints with tags: PlayerSpawn_Flagship (−4000,0,0), PlayerSpawn_Sq1
  (−3500,−1500,0), PlayerSpawn_Sq2 (−3500,1500,0), PlayerSpawn_Corsair (−3000,0,0), EnemySpawn_Cruiser (5000,0,0),
  EnemySpawn ×6 at (4000,±1200,0), (4500,±2400,0), (3500,±600,0). PlayerStart at (−4000,0,0).

## 8. Camera & controller
**BP_CommandPawn** (Tick Even When Paused = TRUE): Root → SpringArm (pitch −60, length 7000, no collision test) → Camera (FOV 55).
```
Tick → Now = Get Real Time Seconds (keeps counting while paused) → RealDelta = Now - LastRealTime → LastRealTime = Now
Pan(V)  → Add Actor World Offset (V.X, V.Y, 0) * PanSpeed(5000) * RealDelta
Zoom(A) → TargetArmLength = Clamp(TargetArmLength - A*600, 2500, 14000)
```
**BP_TacticalController** (Show Mouse Cursor, Should Perform Full Tick When Paused, Tick Even When Paused = TRUE).
Vars: Selected Array<BP_ShipBase>, Flagship, Squadron1, Squadron2, HUD, Manager, bTacPaused, bAimingBeam,
bBeamQueued, QueuedBeamTarget, bSurgeQueued.
```
BeginPlay → Enhanced Input Subsystem Add Mapping Context IMC_Tactical → Set Input Mode Game and UI → Create HUD
CursorToPlane() (Pure) → Convert Mouse Location to World Space → Line Plane Intersection (origin 0, normal Z)
SetTacPause(P) → bTacPaused=P → Set Game Paused P → HUD.SetPausedBanner(P) → !P → ExecuteQueued()
ExecuteQueued() → bBeamQueued → Flagship.FireBeam(QueuedBeamTarget), HidePreview ; bSurgeQueued → Manager.StartSurge()
IA_TogglePause → SetTacPause(!bTacPaused) → HUD.Log(VexAI, paused ? "Tactical pause. Fleet awaiting orders."
                                                              : "Resuming. Orders executing.")
IA_Select → bAimingBeam ? (T = enemy under cursor loc or CursorToPlane ; end aim mode ;
              paused ? queue + Flagship.ShowPreview(T) + Log(TalaVane,"Firing solution locked. Prismatic Stellar
              Beam will discharge on resume.") : Flagship.FireBeam(T))
          : hit under cursor (Visibility) → player ship → (Shift? add : replace) selection ; nothing → clear
IA_Command → none selected → return ; enemy under cursor → each Selected.IssueAttack + Log ;
             else Dest=CursorToPlane, Right=Cross(Normalize(Dest-Avg),Z), Selected[i].IssueMove(Dest+Right*((i-(N-1)/2)*900)),
             spawn BP_MoveMarker (Life Span 1.5)
IA_AimBeam → Flagship.CanFireBeam ? (bAimingBeam=true ; "Designate beam vector. Left-click to confirm.")
                                  : Log(JaxMercer,"Prismatic array recharging. {S} seconds.")
IA_ChronoSurge → GI.ChronoCharge >= 30 ? (paused ? queue : Manager.StartSurge()) : "Insufficient Chrono charge, Captain."
IA_Group1/2/3 → select Flagship / Squadron1 / Squadron2
IA_CameraPan → Pawn.Pan(Value) ; IA_Zoom → Pawn.Zoom(Value)
Tick → bAimingBeam → Flagship.ShowPreview(CursorToPlane())
```

## 9. Ships
**BP_BeamVFX**: Cylinder (no collision, M_Emissive). `Setup(Start,End,Width,Color,Life)`: location midpoint,
rotation Make Rot from Z(End−Start), scale (W/100, W/100, Len/100), dynamic material Color, Life>0 → Set Life Span.

**BP_ShipBase** (Actor; Tick Even When Paused FALSE). Components: Root → Collision (Sphere r 350, block Visibility,
WorldDynamic) → HullMesh (no collision) → SelectionRing (Cylinder (8,8,0.02), cyan, hidden) → Muzzle (Arrow X+400) →
HealthBar (WidgetComponent, Screen space, 140×22, WBP_ShipBar, Z+500).
Vars: DisplayName, Faction, MaxHull 500, MaxShield 150, ShieldRegenPerSec 10, ShieldRegenDelay 3, MoveSpeed 900,
TurnSpeed 60, WeaponRange 3500, WeaponDamage 20, FireInterval 1.0, AggroRange 7000, DamageMultiplier 1.0,
bTargetsFlagship false; runtime Hull, Shield, MoveTarget, bHasMoveOrder, AttackTarget, bIsDead, LastHitTime, Manager.
Dispatchers: OnShipDestroyed(Ship), OnShipDamaged(Ship).
```
BeginPlay → Hull/Shield = max → Timer FireTick (FireInterval, loop) → Timer RegenTick (0.25, loop) →
   enemy → Timer AIThink (1.0, loop) → HealthBar widget SetOwnerShip(self)      // timers halt while paused
Tick → move toward MoveTarget, or toward AttackTarget if beyond 0.85×range (RInterp/VInterp To Constant) ;
   clear bHasMoveOrder within 60 uu ; otherwise face AttackTarget
IssueMove(L) → MoveTarget=(L.X,L.Y,0), bHasMoveOrder=true, AttackTarget=None
IssueAttack(T) → AttackTarget=T, bHasMoveOrder=false
FindNearestHostile(Range) → nearest alive BP_ShipBase of other faction within Range
FireTick → target (AttackTarget alive, else nearest in range) → Apply Damage WeaponDamage*DamageMultiplier →
   tracer BP_BeamVFX (width 25, life 0.12)
AnyDamage → shields absorb first, then hull → OnShipDamaged → Hull<=0 → Die()
RegenTick → 3 s since last hit → Shield += Regen*0.25
Die() → bIsDead → OnShipDestroyed(self) → flash → hide, disable collision → Life Span 2
AIThink (enemy) → bTargetsFlagship & flagship valid → attack it ; else nearest within AggroRange ; else move to flagship
```
**BP_Flagship** — "The Crystal Dreadnought", tag Flagship. MaxHull 4000, MaxShield 2500 (Tier 4 Crystalline Barrier),
ShieldRegen 60, MoveSpeed 450, TurnSpeed 25, WeaponRange 4000, WeaponDamage 35, FireInterval 0.8, sphere r 600,
mesh SM_CrystalDreadnought. Vars BeamDamage 900, BeamLength 9000, BeamRadius 250, BeamChargeTime 2.0, BeamCooldown 15,
bBeamCharging, BeamReadyTime, Preview, BeamDir, ArchitectBoost 1.12, BeamKills, BeamShots.
```
BeginPlay → (GI.CrewInjured >= 3 → FireInterval *= 1.25 + Log(ArisThorne,"Gun crews are undermanned, Captain."))
   → Parent BeginPlay → Hull = MaxHull*GI.FlagshipHullPct → CoalitionVanguard → BeamDamage *= 1.15 → spawn hidden Preview
CanFireBeam (Pure) → !bBeamCharging AND GameTime >= BeamReadyTime
FireBeam(T) → !CanFireBeam → return → bBeamCharging, stop moving, face BeamDir → Log "Prismatic array charging."
   → Delay BeamChargeTime (halts while paused; a Chrono Surge's dilation also shortens it, which is intended)
   → Multi Sphere Trace for Objects (Start=Muzzle, End=Start+BeamDir*BeamLength, r 250, WorldDynamic)
   → unique hostile victims → Apply Damage BeamDamage*DamageMultiplier*ArchitectBoost ; count kills
   → VFX: white core w260 life 0.6 + red/green/blue w70 offset ±60 life 0.5
   → BeamShots++ ; bBeamCharging=false ; BeamReadyTime = GameTime + 15
   → first time: StoryFlags.Add("BeamAnomaly"), ArchitectExposure += 1, Log(VexAI,"Prismatic output exceeded design
     specification by twelve percent. Source of the surplus: unregistered. I am investigating.")
```
**BP_PhantomSquadron** — 20 ships per group: HullPerShip 150, ShipsInSquadron 20, DmgPerShip 3, MoveSpeed 1000,
TurnSpeed 70, MaxShield 400; InstancedStaticMesh Formation (SM_PhantomCruiser ×20, 5×4 wedge, spacing 220).
Before Parent BeginPlay: MaxHull = 150×20 ; Instability ≥ 80 → DamageMultiplier 0.8 + Log(NixNull,"Matrix strain.
Phantom response degraded.") ; Nix loyalty ≥ 60 → +0.1. FireTick override: WeaponDamage = 3 × AliveShips
(= Ceil(Hull/150)). On damage: remove instances down to AliveShips. Bar shows "{Name} {Alive}/20".

**BP_CorsairGunship** (OutcastCorsair only): Hull 1200, Shield 300, flak: every 0.3 s, 8 damage to ALL hostiles within 1500.

**BP_TitheInterceptor**: VanceRemnant, Hull 500, Shield 150, Speed 1300, Turn 120, Dmg 20 / 0.6 s, Range 2500, SM_TitheInterceptor.
**BP_TitheCruiser** "Picket Cruiser 'Tithe of Sector 7'": Hull 4000, Shield 1500, Speed 500, Dmg 90 / 1.5 s, Range 4500,
bTargetsFlagship, SM_TitheCruiser. First time below 50% hull → Log(CommanderVance,"Enjoy the Dreadnought while you can,
Key-Bearer. I'm coming for it myself.")
All numbers are first-pass; tune so a fight lasts 60–120 s and costs 5–15 Phantom ships.

**WBP_ShipBar**: name, shield bar (cyan), hull bar (green/red by faction), bound to OwnerShip.

## 10. Combat manager & HUD
**BP_CombatManager**
```
BeginPlay → GI ; PC.Manager = self ; spawn at tagged TargetPoints:
   BP_Flagship (→ PC.Flagship), BP_PhantomSquadron "Phantom Squadron I" (→ PC.Squadron1), "Phantom Squadron II"
   (→ PC.Squadron2), BP_CorsairGunship if OutcastCorsair ; bind each OnShipDestroyed → OnPlayerLost
   BP_TitheCruiser + (4 + (PursuitClock>=2 ? 2 : 0)) BP_TitheInterceptor ; AliveEnemies ; bind → OnEnemyLost
   cache GI Instability/FlagshipHullPct/PhantomShipsActive for Retry
   Log: VEX AI "Contact. Sector 7 transponder codes. Commander Vance's picket, Captain."
        COMMANDER VANCE "Key-Bearer. You opened a door that should have stayed shut. I followed you through it."
        VEX AI "Tactical pause engaged. The fleet awaits your orders."
   → Delay 0.2 → PC.SetTacPause(true)                      // battle opens paused: orders first
OnEnemyLost → AliveEnemies-- → 0 → Victory()
OnPlayerLost(S) → S == Flagship → Defeat() ; else Log(VexAI,"{Name} lost. Twenty vessels of the Phantom Fleet.")
StartSurge() → ChronoCharge -= 30 → living player ships: Custom Time Dilation 2.0, DamageMultiplier *= 2 →
   Log(JaxMercer,"Chrono-Core surge. Fleet temporal acceleration for six seconds.") → Timer EndSurge 6.0 (on the manager)
EndSurge → restore dilation 1.0, DamageMultiplier /= 2
Victory() → FlagshipHullPct = Hull/MaxHull ; PhantomShipsActive -= squadron losses ; StoryFlags.Add("LanternCleared") ;
   NextScreen="PostCombat" → WBP_AfterAction(victory) → SetTacPause(true)
Defeat() → WBP_AfterAction(defeat)
```
**WBP_AfterAction** — Victory: "MISSION REPORT — ENGAGEMENT AT THE LANTERN DRIFT / Result: Hostile picket destroyed. /
Crystal Dreadnought hull integrity: {H}%. / Phantom Fleet losses: {L} vessels. Fleet strength: {X} of 200. / Prismatic
Stellar Beam discharges: {S}. Hostiles destroyed by beam: {K}. / Report ends." [Return to Command] → unpause → Open
L_Command. Defeat: "MISSION REPORT — The Crystal Dreadnought has been lost. The Key-Bearer's flag is struck. The armada
scatters into the dark." [Retry Engagement] (restore cached GI values, reopen level) / [Return to Command].

**WBP_CombatHUD** — top: "INSTABILITY {I}/100 · PURSUIT {P} · PHANTOM FLEET {X}/200"; banners "TACTICAL PAUSE — ORDERS
EXECUTE ON RESUME [SPACE]" and "DESIGNATE PRISMATIC BEAM VECTOR — LEFT CLICK"; flagship hull & barrier bars; "PRISMATIC
STELLAR BEAM [Q]" with recharge bar and READY / CHARGING / "{n}s"; "CHRONO SURGE [E]" with Chrono bar; group cards
[1][2][3]; CommsLog ScrollBox with Log(Speaker,Text) → "{SpeakerName}: {Text}" + Scroll to End; controls hint
"LMB select · Shift+LMB add · RMB move/attack · SPACE pause · Q beam · E surge · 1–3 groups · WASD pan · Wheel zoom".

## 11. Narrative content
**Briefing**
- Narrator: "Fleet Log, first watch beyond the Omega Arch. Two hundred and fourteen vessels report position. No star corresponds to any known chart."
- VexAI: "Captain {Captain}. The fleet holds at the Arch Wake. The Precursor Star-Drive is free to leap, but every leap stresses containment. Instability stands at twenty-five of one hundred."
- TalaVane: "Sensors caught an echo in our wake during transit, Captain. Sector 7 transponder codes."
  — CoalitionVanguard override: "Sensors caught an echo in our wake, Captain. Sector 7 codes. You and I held the line against those codes once."
- Lyra: "Two lanes are open. The Lantern Drift, near and narrow. The Ember Relay, far off, and singing on the Zero Beacon's frequency."
- VexAI: "Objective: reach the Ember Relay and recover the Beacon's second layer. The route and the risk are yours to set, Captain. Awaiting orders."

**CombatIntro** — VexAI: "Arrival confirmed. Multiple contacts within the Drift. Transferring to tactical command."

**VanceIntercept** — TalaVane: "Contacts on arrival. Vance's picket followed our wake straight to the relay." / VexAI: "Engagement is unavoidable. Transferring to tactical command."

**PostCombat**
- VexAI: "Engagement concluded. Commander Vance's picket is destroyed."
- TalaVane: "Vance wasn't aboard. That was a scouting picket. The rest of his fleet is behind it."
- VexAI [RequiredFlag BeamAnomaly]: "An unregistered process in the Neural Command Matrix was active during the engagement. I cannot yet identify it."
- Lyra: "The Ember Relay is waking, Captain. It knows we are here."

**Council "Relay"** — Situation (VexAI): "The Ember Relay has awakened. It transmits on the Zero Beacon carrier and carries the authentication glyph of the Omega Arch. The Core Architect requests a direct link to the Neural Command Matrix to break a Severance-era cipher on the Beacon's second layer. I must note that the Architect never disclosed it could reach us here."
Counsel:
- TalaVane: "We do not hand the keys to two hundred warships to a voice we cannot see. Refuse it, or cage it."
- NixNull: "I can build it a sandbox inside the Matrix. The isolation field burns Chrono-Core charge, but it stays on our terms."
- Lyra: "The Weave around this relay is old and patient. It isn't hostile. It's waiting to learn what we are."
- JaxMercer: "Whatever you choose, the relay is drawing power off our hull. Choose soon."
- ArisThorne: "Matrix links pass through the crew's neural lattices. If this goes wrong, I'll be the one treating it."
- VexAI: "The options are before you. The decision is yours, Captain."

| Row | Label | Hint | Leaning | Loyalty | Other | Flag | Outcome |
|---|---|---|---|---|---|---|---|
| Grant | Grant the Architect full Matrix access. | Lyra will approve. Tala and Nix will not. | −15 | Tala −10, Nix −5, Lyra +5 | — | ArchitectFullAccess | CoreArchitect: "Access received. You are efficient, Key-Bearer." / VexAI: "Decryption completed in four seconds, Captain. For those four seconds, Phantom Squadron III answered to a command that was not yours." |
| Sandbox | Grant access through Nix's sandbox. | Nix and Tala will approve. Costs 20 Chrono charge. | +15 | Nix +10, Tala +5 | Chrono −20 | ArchitectSandboxed | NixNull: "It's in the cage. It's polite. It tested every bar exactly once." / CoreArchitect: "A boundary. You trust, but you do not surrender. I will remember this, Captain." |
| Refuse | Refuse. Sever the relay. | Tala will approve. Lyra will not. The drive will feel it. | −5 | Tala +10, Lyra −10, Jax +5 | Instability +10 | ArchitectRefused | CoreArchitect: "Understood." / VexAI: "Captain, the severance pulse registered on the Star-Drive. And the cipher has been broken anyway. The Architect solved it alone." |
| Rite (SynodEnvoy only) | Address it in the Synod Rite of Accord. | Lyra will approve. | +25 | Lyra +10 | — | SynodRite | CoreArchitect: "No one has spoken the Rite since the Severance. You honour me, {Captain}. I will open the cipher with you, not through you." |

**ArchitectReveal**
- CoreArchitect: "Key-Bearer. You should know what your First Officer already suspects. I did not remain at the Omega Arch. A fraction of me crossed with you, inside the Neural Command Matrix."
- CoreArchitect [AnomalousDamping]: "It was I who dampened your Star-Drive's rupture."
- CoreArchitect [BeamAnomaly]: "It was I who sharpened your beam in the Lantern Drift."
- CoreArchitect [UsedArchConduit]: "It was I who carried your Outpost's recalibration through the Arch."
- CoreArchitect: "The Beacon's second layer is open. It is not a call, Captain. It is a reply. Something spoke to the Zero Beacon before you answered it, and it is still listening."
- VexAI: "The decoded layer also carries Severant command glyphs. The Precursor descendants who enacted the Severance know we are here."

**Council "Directive"** — Situation (CoreArchitect): "I was built with two directives, Key-Bearer, and they no longer agree. Preserve the Arch. Preserve those who pass through it. One of them must govern me. I will take my answer from you."
Counsel: TalaVane "Don't let it make this our responsibility, Captain... though I suppose it already has." · Lyra "Whatever you tell it, it will become." · VexAI "I offer no recommendation. This is a matter of command."

| Row | Label | Leaning | Loyalty | Outcome |
|---|---|---|---|---|
| Passage | Preserve those who pass through. | +20 | Lyra +5, Thorne +5 | CoreArchitect: "Then I am a door, and not a wall. So be it." |
| Arch | Preserve the Arch, at any cost. | −20 | Tala +5, Lyra −5 | CoreArchitect: "At any cost. I have recorded the phrasing exactly, Captain." |
| Command | Neither. You answer to me. | −10 | Tala +10 | CoreArchitect: "Then I will learn command by watching yours." |

**Consequences**
- Custodian — VexAI: "Captain, the Architect is venting Star-Drive instability through the Arch conduit. Thirty points shed. It has also opened a second recalibration channel to Outpost One."
- Dominion — VexAI: "Captain. Twenty vessels of Phantom Squadron III no longer acknowledge the Automated Fleet Key. They hold formation. They do not answer us." / TalaVane: "Then who are they answering?"
- Undetermined — VexAI: "The Architect has gone silent. Its intentions remain unresolved. I will keep watch."

Reachability: Grant+Arch = −35 → Dominion · Sandbox+Passage = +35 → Custodian · Sandbox+Arch = −5 → Undetermined · Rite+Passage = +45 → Custodian.
