# Act VIII — The Iron Tithe

Story and systems spec for the second act after the vertical slice, following the Act VII model
(`ACT_VII_CINDER_ARCHIVE.md`). Canon follows the Fleet & Crew Dossier and `Docs/World/WORLD_BIBLE.md` §7.1;
anything marked **[NEW]** extends it. Variable, flag and table names match `SLICE_SPEC.md` and Act VII §14.
Values are first-pass and get balanced in playtest.

**Act question:** *What do we do with the people who tried to keep the door shut?*
**Act length:** 5–7 leaps, 3 battles plus the pursuit ambush, 3 major councils, 3 side missions. Target 2–3 hours.

---

## 1. Premise

Commander Vance withdrew from the Cinder Archive toward **Tithe Anchorage**, a fortified spine of hangars across
the ring gap of the gas giant **Grieve**. The Anchorage was a Tithe listening post on the far side of the Arch long
before the Key-Bearer opened it. Vance now holds it as a sovereign base: forty hulls, a prison of Coalition crews
taken in the frontier wars, and an empty dock cut to the Crystal Dreadnought's dimensions.

Vance is not a raider. He is a quarantine officer who has lost his quarantine. He believes the Arch should have
stayed shut, and that since it was opened, the safest hands for the Dreadnought are his. His alliance with the
Severant hardliners gave him **grafts**: Severant ceramic spikes, grown at the **Graft Forge** on the volcanic moon
Cauter, bolted through Tithe armour. They carry Silence emitters. They are also making Tithe crews sick.

The act asks the Captain to end the Tithe's war and decide what Vance and his people are owed when it is over.

## 2. Opening state (carried from Act VII)

| Variable / flag | Use in Act VIII |
|---|---|
| `ExarchStatus` | Alive → Ithane is with Vance at Grieve (`BP_LastSilence` joins E3). Dead → Ithane's loyalists serve Vance leaderless (2 extra grafted frigates in E2). Deposed → Ithane is a prisoner at Seren Vael and can be questioned there (§7.2). |
| `VanceGraftsKnown`, `SeverantAgentsCaptured` | Each gives forge intel: one opens the **sabotage** option in C2b; both expose the *Writ of Sector 7*'s graft spike as a weak point in E3 (§8). |
| `SeverantStanding` | Decides who answers at Seren Vael (§6). ≥ 30 opens the "Hand him to the Severant council" option in C3. |
| `CureDelivered` | Warden Kesh joins councils as envoy (`WardenKesh` counsel lines with `RequiredFlag CureDelivered`). |
| `SupplyLine`, shipments | Continue unchanged. The Weave and Joint lanes keep working; Act VIII adds no new anchors. |
| `ArchitectLeaning`, `ArchitectRevelation2` | The Architect's lines in C1 and C3 vary by `ArchitectRevelation2`. |
| `Instability`, `CrewInjured`, `FlagshipHullPct`, `PhantomShipsActive`, `CrewLoyalty` | Carry over. `PursuitClock` resets to 0: Vance knows exactly where the fleet is. |
| `Background` | Decides where the Coalition key is (§4.5). Outcast Corsair: the Captain stole it from the Tithe, and Vance knows it. |

## 3. Star chart

| Node | Location | Encounter | Notes |
|---|---|---|---|
| `TitheMarker` *(canon)* | Outer marker of Tithe Anchorage | Council C1 | Act start. Vance's terms (§5). |
| `Rustmarch` | A shattered planetoid belt around a small orange star | Event | Cormaker's deal, side mission §7.1. Future site of Cutter's Rest. |
| `TollLine` **[NEW]** | The Toll Line: the Tithe's old blockade, a field of derelict toll platforms and live mines | Combat E1 | Hazard +10 out of it unless charted. First *Tithe Carrack*. |
| `SerenVael` | The last living Severant world | Council (allies) | Lyra's Choir; Ithane's custody if Deposed. |
| `GraftForge` | Orbit of Cauter, Grieve's volcanic moon | Council C2 + Combat E2 | Sabotage, destroy or seize the forge. |
| `Grieve` | Tithe Anchorage in Grieve's ring gap | Approach council + Combat E3 + Council C3 | Finale. Gated: the Anchorage only opens once the forge is resolved (`ForgeResolved`). |

`TollLine` is the one node not in the world bible: the act needs a combat node between the Marker and Grieve
that is not the finale, and the Tithe's abandoned blockade is the natural place for it.

**Leap costs** (before charting, strain and background modifiers; all routes both ways):

| Route | Base | Route | Base |
|---|---|---|---|
| TitheMarker → Rustmarch | 35 | Rustmarch → SerenVael | 40 |
| TitheMarker → TollLine | 25 | SerenVael → GraftForge | 50 |
| TitheMarker → Grieve | 40 | TollLine → GraftForge | 30 |
| TollLine → Grieve | 25 | GraftForge → Grieve | 25 |
| Rustmarch → Grieve | 45 | | |

The **mines** at the Toll Line add +10 to every leap out of it unless Lyra has charted the route
(`HazardSurcharge 10`, as Act VII's ash storms). Arriving at Grieve before the forge is resolved plays
`Grieve_Standoff` and nothing else: the Anchorage's grafted guns hold the fleet at range.

Chart data: `BuildData/ActVIII/*.csv`. Check with `AetheriaStarChartToolset.validate_star_chart(..., "TitheMarker")`.

## 4. Systems introduced

### 4.1 Grafted enemies
Tithe hulls in Act VIII can carry a graft. A grafted ship has a **Silence emitter** (Act VII §4.4: 1.5 s blue
telegraph, then the Prismatic beam charge is cancelled and an active Chrono Surge ends inside the radius) and
takes **+50% damage while its emitter is telegraphing**: the spike glows and is exposed. The tactical pause
counters it as in Act VII: pause, spot the ring, fire into the spike.

How many enemy ships carry grafts depends on `GraftForgeState` (§8, E2):

| `GraftForgeState` | E3 grafts |
|---|---|
| `Intact` (forge left running or E2 lost) | Every Tithe cruiser and carrack in E3 |
| `Destroyed` | Only the *Writ of Sector 7* |
| `Seized` | Only the *Writ*; the fleet gains **Silence shielding** (§4.2) |

### 4.2 Silence shielding (seized forge)
Jax strips the forge's crucible into dampers for the Dreadnought: Silence pulses no longer cancel the Prismatic
beam charge (Chrono Surge is still ended). Flag `SilenceShielding`. Thorne will not approve: the dampers are grown
the same way the grafts are.

### 4.3 Severant allies
`SeverantAllies` (0–6) is set at Seren Vael (§6) and fields that many Kesh-collared allied skiffs in E3
(`BP_KeshSkiff`, escort aggro as in Act VII). Lyra's Choir counts as 2 when Lyra rejoins (§7.3).

### 4.4 Siege or infiltration
The approach council at Grieve chooses how E3 is fought:

| Approach | Requires | E3 |
|---|---|---|
| **Siege** | — | Full engagement: Anchorage defence group, *Writ of Sector 7*, carracks. Win: disable the *Writ*. |
| **Infiltration** | Corsairs still with the fleet (`CorsairsDeparted` not set) | **Hold** the boarding lane for 90 s while Cormaker's Boarding Pods take the prison, the vault and the dock controls; the *Writ* engages directly. Fewer enemies; the prisoners are freed in the fight (`PrisonersFreed`). |

### 4.5 The Coalition key
Gate Zero's three keys went to the Synod, the Guild and the Coalition, and the Captain carries one of them by
background (`WORLD_BIBLE.md` §1.7). The Iron Tithe seized the Coalition key in the frontier wars. What happened to it
next depends on the Captain's background:

| Background | Where the Coalition key is | At the Anchorage vault (after E3) |
|---|---|---|
| Synod Envoy, Guild Engineer | Still in the Anchorage vault | Recovered: `CoalitionKeyRecovered`. Scene `VIII_Vault_Key`. |
| Coalition Vanguard | The Coalition took it back; the Captain carries it | Empty. Scene `VIII_Vault_Vanguard`. |
| Outcast Corsair | The Captain's outcasts stole it from a Sector 7 vault; the Captain carries it | Empty, with Vance's own lock still broken. Scene `VIII_Vault_Corsair`. |

The vault is reached either way: Infiltration takes it during E3; after a Siege, Tala's boarding party finds it with
the prisoners. Recovering the key is automatic (it is an item, not a choice); what it opens is decided in Act X
(`WORLD_BIBLE.md` §7.3: with the key, the Coalition plate needs no officer, override or force).

Lines (formal register for Vex; first-pass):
- `VIII_Vault_Key` — Vex AI: "Captain, the vault holds a Precursor seal-key bearing the Coalition sigil. It matches
  the plate pattern of Gate Zero. Commander Vance kept it under three locks and no inventory." Tala Vane: "We lost
  that key at Halden Reach. I was there."
- `VIII_Vault_Vanguard` — Vex AI: "The vault is empty, Captain. Its record lists one item, removed in the frontier
  wars." Vance: "Your Coalition took back what we kept safe. Look where it brought you, Vanguard."
- `VIII_Vault_Corsair` — Vex AI: "The vault is empty, Captain. The lock was cut from outside, years ago." Vance:
  "Every miner in Sector 7 knew that story. I never knew it was you."

Vance's terms (C1) also change for an Outcast Corsair Captain: an added line, Vance: "And bring back what you
took from my vault, thief." (`BackgroundOverride` on the C1 arrival, `OutcastCorsair`.)

## 5. Council C1 — "Terms" (TitheMarker)

**Arrival (Narrator):** "Fleet Log. The Tithe Anchorage marker transmits on every channel the fleet can hear.
The transmission is addressed to the Key-Bearer by name. Report ends."

**Situation (Vex AI):** "Commander Vance has transmitted terms, Captain. I will read them in full. I do not recommend
a reply. Surrender the Crystal Dreadnought at Tithe Anchorage, and every other vessel of the fleet goes free:
the Phantom squadrons, the Corsairs, the Wayward. He gives us until the fleet reaches Grieve."

**Counsel**
- Tala Vane: "He held Sector 7 with conscripts and called it a duty. Answer him with the fleet, Captain."
- Cmdr. Cormaker: "'Every other vessel goes free.' He's bargaining with my people's lives to buy your ship."
- Jax Mercer: "Keep him talking. Every hour he spends writing terms is an hour he isn't fitting grafts."
- Nix "Null": "His carrier wave has a back channel in it. Give me a reply to send and I can ride it in."
- Core Architect (`ArchitectRevelation2 = Warden`): "He wants the ship because he believes the ship is the key. He is
  half right, Key-Bearer."
- Vex AI: "The decision is yours, Captain."

| Option | Hint | Leaning | Loyalty | Effect |
|---|---|---|---|---|
| **Refuse, on the open channel.** | Tala and Cormaker will approve. | 0 | Tala +5, Cormaker +5 | `TermsRefused`; Pursuit +1 (Vance moves first). |
| **Reply and stall for time.** | Jax will approve. Tala will not. | 0 | Jax +5, Tala −5 | `TermsStalled`; E1 is fought without the carrack (it waits for a reply). |
| **Reply, and let Nix ride the signal in.** | Nix will approve. | 0 | Nix +10 | `AnchorageMapped`; the Grieve approach council shows the prison block and the dock (Infiltration hold drops to 60 s). |
| **Do not answer.** | Vex will approve. | 0 | — | Vex: "Acknowledged. Silence is also an answer." |

## 6. Council — "The Choir Calls" (SerenVael)

**Situation (Warden Kesh** if `CureDelivered`**, else Vex AI):** "The Severant council will hear the Key-Bearer.
Some of them want the Silence kept. Some want it ended. None of them have forgotten who answered the Beacon."

Who answers is decided by `SeverantStanding`:

| Standing | Council outcome |
|---|---|
| ≤ −40 | The council refuses to receive the fleet. Kesh (if present): "I'm sorry, Captain. They will not hear me either." `SeverantAllies = 0`. |
| −39 … 29 | Options below; the best is 2 allies. |
| ≥ 30 | Options below; the best is 4 allies, and Kesh offers her own skiffs (+2). |

| Option | Hint | Leaning | Loyalty | Effect |
|---|---|---|---|---|
| **Ask them to stand with the fleet.** | Kesh will approve. | 0 | — | `SeverantAllies` = 2 (Standing ≥ 30: 4); Standing +5. |
| **Offer the Anchorage's grafts back to them.** *(after E2 = Seized)* | Kesh and Thorne will approve. Jax will not. | +5 | Thorne +5, Jax −10 | Lose `SilenceShielding`; Standing +20; `SeverantAllies` +2. |
| **Ask nothing. Report what we found at the Archive.** | Lyra will approve. | +5 | Lyra +5 | Standing +10; no allies. |

If `ExarchStatus = Deposed`, a second scene follows: **Ithane in custody** (§7.2).

## 7. Side missions

### 7.1 "Fair Shares" (Rustmarch; Cormaker)
Vance has offered the Corsairs Rustmarch and amnesty in exchange for the Dreadnought's position. Cormaker brings
the offer to the Captain unasked. It is not a betrayal if the Captain is told.

**Situation (Cmdr. Cormaker):** "He sent it to my ship, not yours. I'm telling you because that's what fair looks
like. Rustmarch, amnesty, and nobody ever calls us outcasts again. All I have to do is tell him where you are."

| Option | Hint | Leaning | Loyalty | Effect |
|---|---|---|---|---|
| **Promise them Rustmarch when this is over.** | Cormaker will approve. | 0 | Cormaker +15 | `RustmarchPromised` (Act IX founds Cutter's Rest). |
| **Use the offer: feed Vance a false position.** | Tala and Cormaker will approve. | 0 | Tala +5, Cormaker +5 | `VanceMisled`; Pursuit −2. |
| **Let them take the deal and go.** | Cormaker will not. | 0 | Cormaker −20 | `CorsairsDeparted`: the 12 Corsair ships leave the fleet; Infiltration is unavailable. |
| **The Corsairs stay with the fleet. No promises.** | Tala will approve. Cormaker will not. | 0 | Tala +5, Cormaker −10 | — |

### 7.2 "The Exarch's Cell" (SerenVael; `ExarchStatus = Deposed`)
Ithane, held by the Severant council, knows the forge. One question may be asked:
- **How do the grafts die?** → `ForgeWeakPoint` (sabotage in C2b needs no other intel).
- **What did Vance hear at the Arch?** → `ListenerTraced` if not set; Ithane: "The same thing we did. He was
  only braver about it."
- **Nothing. Leave the Exarch to the council.** → Standing +10.

### 7.3 "The Choir" (SerenVael; Lyra)
The Choirs want Lyra back. Choice: let Lyra sing with them for one watch (Pursuit +1, Lyra loyalty +10,
`SeverantAllies` +2 when she rejoins) or keep her aboard (Lyra loyalty −5). Requires `LyraHeritageRevealed`.

### 7.4 Treating the grafted (after E2; Thorne)
If any grafted Tithe crews survive E2, Thorne asks to treat them with Bio-Gel (`BioGelDoses` −2,
Thorne loyalty +10, `GraftedTreated`). Tala objects; Cormaker shrugs. Refusing costs Thorne −5.

## 8. Battles

| ID | Where | Enemies | Win condition | Notes |
|---|---|---|---|---|
| **E1** | TollLine | 1 *Tithe Carrack* **[NEW]**, 2 Tithe cruisers, 6 interceptors | Destroy all | `TermsStalled`: no carrack. The Carrack rams: it charges the nearest capital ship and deals heavy damage on contact unless killed first. |
| **E2** | GraftForge | Graft Forge (static objective: 3 cable pylons, then the crucible) + 2 grafted Tithe cruisers + 4 interceptors | Destroy the crucible (Destroy), or hold the forge deck for 90 s (Seize) | Decided in C2b before the battle (below). `ExarchStatus = Dead`: +2 grafted frigates. |
| **E3** | Grieve | Siege or Infiltration (§4.4); *Writ of Sector 7* **[NEW]**; *Tithe Collectors* **[NEW]** try to grapple the Dreadnought | Disable the *Writ* (25% hull) | `ExarchStatus = Alive`: *The Last Silence* joins. Severant allies join. At 25% hull the *Writ* is disabled, not destroyed: `VanceCaptured`. |

**C2b — "The Crucible" (GraftForge, before E2).** Situation (Jax Mercer): "That crucible grows Silence like coral.
Blow it and the grafts stop. Take it, and I can make the stuff work for us."

| Option | Requires | Effect |
|---|---|---|
| **Destroy the forge.** | — | E2 Destroy; win → `GraftForgeState = Destroyed`. Thorne +5. |
| **Seize the forge.** | — | E2 Hold; win → `GraftForgeState = Seized`, `SilenceShielding`. Jax +10, Thorne −5. |
| **Sabotage it from inside.** | `ForgeWeakPoint`, or both `VanceGraftsKnown` and `SeverantAgentsCaptured` | No battle. `GraftForgeState = Destroyed`; Nix +5. |

Losing E2 (withdrawal) sets `GraftForgeState = Intact`. Every outcome sets `ForgeResolved`.

**Writ of Sector 7 weak point.** With both `VanceGraftsKnown` and `SeverantAgentsCaptured`, the *Writ*'s graft
spike is a separate target: destroying it (hull 800) ends the *Writ*'s Silence pulses and its damage reduction.

**Dynamic ambush:** as Act VII: `PursuitClock` 5 → the next arrival triggers a Tithe ambush (E1's enemies,
grafted if `GraftForgeState = Intact`).

## 9. Council C3 — "The Tithe Paid" (Grieve, after E3)

Text as `WORLD_BIBLE.md` §7.1 (situation by Vex AI; counsel by Tala, Cormaker, Thorne, Jax, Kesh if present,
Vex). Options:

| Option | Hint | Leaning | Loyalty | Effect |
|---|---|---|---|---|
| **Try Vance before the fleet.** | Tala will approve. | 0 | Tala +10, Cormaker +5 | `VanceFate = Tried`; Remnant crews interned at the Anchorage. |
| **Take the Remnant into the fleet. Vance commands nothing.** | Jax will approve. Tala and Cormaker will not. | +5 | Jax +5, Tala −10, Cormaker −10 | `VanceFate = Absorbed`; `AnchorageDock` (hull refits without shipments). |
| **Hear him out first.** | Nix will approve. Tala will not. | 0 | Nix +5, Tala −5 | `VanceHeard`, Vance's Listener intel; then `NextCouncil` "Sentence" (try or release). |
| **Hand him to the Severant council.** *(Standing ≥ 30)* | Kesh will approve. Thorne will not. | +5 | Thorne −5 | `VanceFate = Severants`; Standing +15. |
| **Release him with his crews, unarmed.** | Thorne will approve. Tala and Cormaker will not. | +10 | Thorne +5, Tala −15, Cormaker −15 | `VanceFate = Released`; Vance returns in Act X as an unknown. |

Hear-him-out outcome (Vance): "You think I kept the door shut because I was told to. I kept it shut because I heard
what was on the other side, Key-Bearer. So did my father, and his. The Tithe was never a toll. It was a watch."

**Prisoners.** If `PrisonersFreed` (Infiltration) or after Siege, Tala finds the Coalition prisoners. A Coalition
Vanguard Captain recognises one of them (`Background = CoalitionVanguard`: Tala +10, scene `Prisoners_Vanguard`).

## 10. Crew spotlight summary

| Officer | Act VIII role |
|---|---|
| Tala Vane | Wants Vance tried; the Coalition prisoners. |
| Jax Mercer | Wants the forge seized and the Anchorage dock intact. |
| Dr. Aris Thorne | The grafted crews; objects to Silence shielding. |
| Nix "Null" | Rides Vance's signal; maps the Anchorage. |
| Vex AI | Reads the terms; recommends no reply; still will not decide. |
| Lyra the Weaver | Seren Vael and the Choir. |
| Cmdr. Cormaker | *Fair Shares*: Vance's offer and Rustmarch; the Boarding Pods. |
| Warden Kesh | Envoy (if cured): Seren Vael; "We share this, Captain." |
| Core Architect | Comments on what Vance believes the ship is. |

## 11. Act end: report and flags for Act IX

**Act-end mission report:** "MISSION REPORT — THE IRON TITHE. Tithe Anchorage is in fleet hands. Commander Vance:
{tried / absorbed into the fleet / handed to the Severant council / released}. The Graft Forge: {destroyed /
seized / intact}. Severant vessels standing with the fleet: {N}. The Corsair Armada: {with the fleet / departed for
Rustmarch}. Phantom Fleet strength: {X} of 200. Report ends."

| Flag / state | Values | Used in |
|---|---|---|
| `VanceFate` | Tried / Absorbed / Severants / Released | Act IX, Act X (Released → returns) |
| `GraftForgeState` | Destroyed / Seized / Intact | Act IX enemy grafts; `SilenceShielding` |
| `SeverantAllies` | 0–6 | Act IX battles |
| `CorsairsSettled` | `RustmarchPromised` / `CorsairsDeparted` / neither | Cormaker's Act IX arc (Cutter's Rest) |
| `VanceHeard`, `ListenerTraced` | bool | Act IX threshold scene |
| `PrisonersFreed`, `GraftedTreated`, `AnchorageDock` | bool | Act IX crew scenes; refits |
| `CoalitionKeyRecovered` | bool (Synod Envoy and Guild Engineer only) | Act X: opens Gate Zero's Coalition plate |

## 12. Data, code and assets

**C++ (generalise the act flow; Act VII hard-codes it):**
- Game Instance: replace the three `ActVII*Table` soft pointers with per-act lookups keyed by `CurrentAct`
  (`DT_StarNodes_Act<N>`, `DT_LeapRoutes_Act<N>`, `DT_NodeEvents_Act<N>`), `StartAct(int32 Act, FName StartNode)`;
  keep `StartActVII()` as a wrapper. Act VIII start: node `TitheMarker`, `PursuitClock = 0`.
- Command controller: opening sequence `Act<N>_Opening`; act-complete flag per act (`ActVIIIComplete`); the end
  screen's Continue starts the next act.
- Combat: `bDisableAtHullPct` on a battle's objective group (the *Writ* is disabled at 25%, sets the win flag);
  graft damage bonus while an emitter telegraphs (§4.1); `SilenceShielding` check in the flagship's beam interrupt;
  ramming behaviour for the Carrack; grapple behaviour for Collectors (attach and drain flagship speed).
- New ship classes (thin BP children of C++ classes, as §9 ships): `BP_TitheCarrack`, `BP_TitheCollector`,
  `BP_WritOfSector7` (+ `BP_WritGraftSpike`), `BP_GraftForge` + `BP_ForgePylon`, `BP_GraftedCruiser`.

**Data (`BuildData/Narrative/*.json`, imported by `load_narrative_tables`):**
- Chart: `BuildData/ActVIII/DT_StarNodes_ActVIII.csv`, `DT_LeapRoutes_ActVIII.csv`.
- `DT_NodeEvents_ActVIII`; `DT_Councils` rows Terms, ChoirCalls, ExarchCell, Choir, FairShares, Crucible,
  GraftedWounded, Approach, TithePaid, Sentence; their `DT_Council_<Name>` option tables; `DT_Battles` rows
  E1_TollLine, E2_ForgeDestroy, E2_ForgeHold, E3_Siege, E3_Infiltration, Ambush_ActVIII;
  `DT_Sequences_ActVIII` (arrivals, outcomes, act report; vault scenes `VIII_Vault_Key` / `_Vanguard` / `_Corsair`,
  §4.5). Toolset `TABLES` / `GAMEPLAY_FLAGS` gain the new names, including `CoalitionKeyRecovered`.

**Assets (generated in Blender like Act VII, `Tools/Blender/`):**
- Ships: SM_TitheCarrack, SM_TitheCollector, SM_WritOfSector7.
- Set pieces: SM_TitheAnchorage (§4.5 of the bible), SM_GraftForge (§4.7), SM_TollPlatform + mines (TollLine).
- Node scenery: MI_Planet_Grieve (+ rings), MI_Planet_RustmarchMajor (+ asteroid belt), MI_Planet_Cauter,
  MI_Planet_SerenVael (+ harmonic ring); `NodeLightColors` for the six nodes.

## 13. Build phases (each ends with its check)

1. **Chart** — CSVs, import, `validate_star_chart(DT_StarNodes_ActVIII, DT_LeapRoutes_ActVIII, "TitheMarker")`
   returns no problems. Grieve's finale is gated by `ForgeResolved`, not by drive cost.
2. **Act flow (C++)** — per-act tables, `StartAct`, Act VII still plays exactly as before (playtest
   `start_act_vii`, `go_to_node VaultOfRecords`), Act VIII starts at the Marker from the Act VII end screen.
3. **Narrative data** — node events, councils, options, sequences; `validate_narrative` returns no problems.
4. **Combat** — new ship classes and battle rows; `validate_fleet` clean; each battle staged with `stage_battle`.
5. **Assets and scenery** — meshes, planet instances, node lights; node captures for review.
6. **Playtest** — three runs: (a) Refuse → Destroy forge → Siege → Try; (b) Nix rides the signal → Seize →
   Infiltration → Hear him out → Release; (c) Standing ≥ 30 with Kesh → Sabotage → Hand to the Severants.

## 14. As built (implementation notes)

- **Chart (phase 1).** `BuildData/ActVIII/*.csv` imported by `load_narrative_tables` (`CSV_TABLES` in
  `narrative_core.py`). `validate_star_chart(..., "TitheMarker")` and `validate_narrative` report no problems.
  Route plans (`starchart_core.plan_route`, uncharted, Coalition Vanguard): Marker → Grieve 40 → 40 with no rupture;
  from 30 instability, Marker → Seren Vael and Marker → Graft Forge each rupture once; Seren Vael → Graft Forge from
  45 ends at 95. Drive management matters on the long way round. (`plan_route` through MCP returns an empty plan for
  every act; run the core function locally until that serialisation is fixed.)
- **Act flow (phase 2).** Per-act tables are looked up by name: `GetActTable(Act, Kind)` loads
  `/Game/Aetheria/Data/DT_<Kind>_Act<Numeral>`; `GetStarNodes`, `GetLeapRoutes` and `GetNodeEvents` use the current
  act. `StartAct(Act)` (Game Instance and PC_Command) starts any act whose chart exists at `ActStartNode`
  (VII: EmberRelay, VIII: TitheMarker); `StartActVII()` wraps it. Per-act names: opening `Act<N>_Opening`, flags
  `Act<N>FinaleWon` / `Act<N>Complete`, ambush flag `AmbushSprung_Act<N>`, battle `Ambush_Act<N>`, intro
  `Ambush_Act<N>_Intro` (Act VII keeps `AmbushSprung`, `Ambush`, `Ambush_Intro`). Only Act VII plays the second
  revelation before its report; `PlayActReport` builds each act's report. The end screen's Continue appears when
  the next act has a chart and calls `StartNextAct`. Playtest: `start_act(8)`.
- **Import caveat.** Run `load_narrative_tables` once per editor session. A second run in the same session failed
  and left every JSON-built table empty in memory (the files on disk were untouched); restart the editor without
  saving if that happens.
