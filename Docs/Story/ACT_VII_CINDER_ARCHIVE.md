# Act VII — The Cinder Archive

Story and systems spec for the first act after the vertical slice (Act VI, *The Far Shore*).
Canon (names, ship classes, counts, abilities, chronology) follows the Fleet & Crew Dossier;
anything new extends it. Variable, flag and table names match `SLICE_SPEC.md`.

**Act question:** *What answered the Zero Beacon, and why did the Precursors break themselves apart over it?*
**Act length:** 6–9 leaps, 3 to 4 battles, 3 major councils, 4 side missions. Target 2.5–3.5 hours.

---

## 1. Premise

The second layer of the Zero Beacon, which the Architect decoded at the Ember Relay, carried Severant
command glyphs. Vex AI traces them to the **Cinder Archive**: a Precursor archive world of ash and
glass where the record of the Severance is kept. The Severants hold it.

They are dying. Severant bodies are tuned to the Beacon's harmonic, kept silent since the Severance.
When the Captain answered the Beacon, the harmonic resumed, and it is burning them out from the inside.
Severant physicians call it **the Silence**. The fleet caused it without knowing.

Two Severant leaders split over how to respond:

- **Exarch Sol Ithane** (hardliner) holds the Archive and the Pyre Gate. Ithane blames the
  Key-Bearer, wants the Beacon silenced again at any cost, and has bought Commander Vance's
  fleet to make it happen.
- **Warden Kesh, "the Quiet Daughter"** (moderate) leads a refuge of the sick in the Quiet Hollow.
  Kesh wants a cure first, the truth second, and vengeance never.

Dr. Aris Thorne finds that Vault Alpha's **Aetheria Agrarian Seeds** and **Precursor Bio-Regenerative
Gel** can be synthesised into a cure. The ingredients are on Aetheria Prime, beyond the Omega Arch.
Getting them to the fleet forces the act's first and longest-lasting decision: how Outpost One
supplies the expedition.

## 2. Opening state (carried from Act VI)

| Variable / flag | Use in Act VII |
|---|---|
| `ArchitectLeaning` (−45 … +45 after the slice) | Accumulates; the second revelation (§10) reads it at act end. |
| `ArchitectFullAccess` / `ArchitectSandboxed` / `ArchitectRefused` / `SynodRite` | Change the Architect's lines in every council below. |
| `SquadronIIIAlienated` | Phantom Squadron III (20 ships) is unavailable in battles; Nix side mission §7.3 can probe it. |
| `ArchitectSilent` | The Architect does not speak until Council C2. |
| `Instability`, `PursuitClock`, `CrewInjured`, `FlagshipHullPct`, `PhantomShipsActive`, `CrewLoyalty` | Carry over unchanged. Pursuit resets to 0 at the act start; Vance's main fleet re-acquires the wake. |
| `SpireRecalibrations`, `BioGelDoses` | Carry over; resupplied through the supply line (§4.1). |

## 3. Star chart

| Node | Location | Encounter | Notes |
|---|---|---|---|
| `EmberRelay` | Ember Relay | Council C1 | Act start. |
| `AshfallVerge` | The Ashfall Verge: ash storms around a dead Severant convoy | Event | Thorne takes the first plague sample. Salvage side mission §7.2. |
| `GlassSea` | Orbit of the Glass Sea, a vitrified ocean world | Combat E1 | Lyra's weave site. First Weave Lane anchor. |
| `TetherPoint` | The Tether Point: a quiet gravity well between superclusters | Event | Second Weave Lane anchor. Outpost One canopy event §7.4. |
| `QuietHollow` | The Quiet Hollow: Kesh's refuge in a hollowed moon | Event | Cure delivery. Weaving side mission §7.1. |
| `CinderArchive` | Cinder Archive (orbit) | Council C2 | Hub. First meeting with Kesh and Ithane. |
| `PyreGate` | The Pyre Gate: Severant fortress over the Archive's poles | Combat E2 | Can be opened peacefully (§4.2, §8). |
| `TitheMarker` | Outer marker of Tithe Anchorage | Combat E3 (optional) | Vance's forward picket. Sets up Act VIII. |
| `VaultOfRecords` | The Vault of Records (Archive core) | Council C3 + finale E4 | Act end. |

**Leap costs** (before charting, strain and background modifiers):

| Route | Base | Route | Base |
|---|---|---|---|
| EmberRelay → AshfallVerge | 30 | GlassSea → CinderArchive | 30 |
| EmberRelay → GlassSea | 45 | TetherPoint → CinderArchive | 40 |
| AshfallVerge → GlassSea | 25 | CinderArchive → QuietHollow | 20 |
| AshfallVerge → TetherPoint | 35 | CinderArchive → PyreGate | 25 |
| GlassSea → TetherPoint | 35 | CinderArchive → TitheMarker | 50 |
| QuietHollow → PyreGate | 30 | PyreGate → VaultOfRecords | 15 |

All routes run both ways. The **Ash storms** at AshfallVerge and GlassSea add +10 to every leap out
of them unless Lyra has charted the route, which teaches the player to chart in hazardous space.

Chart data: `BuildData/ActVII/*.csv`. Checked with `AetheriaStarChartToolset`: even from 0 instability,
every uncharted route to the Vault ruptures the Star-Drive once. Charting GlassSea→CinderArchive and
CinderArchive→PyreGate brings the direct line in at 94.4 with no rupture. Drive management is
mandatory in this act by design.

## 4. Systems introduced

### 4.1 The Outpost One supply line
From Act VII on, Outpost One sends **shipments** to the fleet. One shipment is one of:

| Shipment | Effect |
|---|---|
| Bio-Gel crate | `BioGelDoses` +2 |
| Agrarian Seed stock | Required for the cure (§4.3) |
| Dry Dock refit kit | `FlagshipHullPct` +0.20 (max 1.0) |
| Spire harmonic charge | `SpireRecalibrations` +1 |

How shipments arrive is decided in Council C1 and recorded in `SupplyLine` (`E_SupplyLine`):

| Line | Delivery | Cost | Long-term stake |
|---|---|---|---|
| `ArchConduit` | Immediately, at any node | None | Every shipment passes through the Architect: `ArchitectExposure` +1 each. In Act IX, if the Architect leans Dominion, it can cut the line at will. |
| `WeaveLane` | Two leaps after ordering | Lyra must anchor the lane at GlassSea and TetherPoint (one chart action at each, Pursuit +1 each). Until both anchors are set, no shipments arrive. | Independent of the Architect. Lyra loyalty +10 when complete. |
| `JointLane` (Guild Engineer only) | One leap after ordering | Same anchors as `WeaveLane` | Built with the Architect as partner: it cannot cut a lane it helped weave, but it can read it. |

One shipment may be ordered per node. The player picks which kind, so supplies stay a decision.

### 4.2 Severant Standing
`SeverantStanding` (−100 … +100, starts at −20) tracks how the Severants see the fleet.
Moderates (Kesh) and hardliners (Ithane) react together, but Kesh's refuge only opens to standing ≥ 0.

| Standing | Effect |
|---|---|
| ≤ −40 | Severant patrols attack on sight at every node in Severant space (GlassSea, PyreGate, VaultOfRecords). |
| −39 … 29 | Default. Patrols hail before attacking; some encounters can be talked down. |
| ≥ 30 | Kesh's moderates can open the Pyre Gate (§8, E2 skipped or fought alongside allies). |

### 4.3 The Silence (cure track)
`CureStage` (0–3):

1. **Sample**: Thorne collects tissue at the AshfallVerge convoy (automatic on arrival).
2. **Synthesis**: requires an Agrarian Seed shipment. Thorne needs one leap of lab time; during it
   `CrewInjured` cannot be treated (the lab is occupied).
3. **Delivery**: at the Quiet Hollow. Sets `CureDelivered`. Standing +40, Thorne loyalty +10.

Whether the cure is *given* or *traded* is a council choice (C2). The cure also saves Severant children
aboard the refuge, which matters for the Kesh epilogue in Act X.

### 4.4 New combat mechanic: the Silence Pulse
Severant ships carry **Silence emitters**. A Silence Pulse in a 1,500 uu radius:
- interrupts the Prismatic Stellar Beam if it is charging (the charge is lost; cooldown is not spent),
- stops Chrono Surge for its remaining duration.

Pulses telegraph for 1.5 s (a blue ring expanding on the emitter). The counter is the tactical pause:
pause, spot the ring, reposition the Dreadnought or kill the emitter first. This makes the Act VI
habit of beam-on-cooldown a decision instead.

## 5. Council C1 — "The Lifeline" (EmberRelay)

**Situation (Vex AI):** "Captain. Dr. Thorne reports the Severant glyphs reference a disease, and that
the only plausible treatment lies in Vault Alpha. Outpost One can ship to us. The question is the
route. The Architect offers the Arch Conduit. Lyra proposes a lane of her own weaving. I note the
Conduit is faster, and that it is not ours."

**Counsel**
- Lyra: "Give me two anchors and I will weave a road home that answers to no one but us."
- Tala Vane: "Every crate through the Conduit is a crate the Architect can count. Or keep."
- Jax Mercer: "The Conduit works today. Lyra's lane works in a week. Our hull needs parts now."
  — override `GuildEngineer`: "Captain, you and I could rig a Guild relay into Lyra's anchors and let
  the Architect carry the load without holding the key. Your call."
- Dr. Aris Thorne: "People are dying on the far side of this decision. Fast matters."
- Core Architect (silent if `ArchitectSilent`): "The Conduit is open, Key-Bearer. I ask nothing for it."
- Vex AI: "The decision is yours."

| Option | Hint | Leaning | Loyalty | Effect |
|---|---|---|---|---|
| **Use the Arch Conduit.** | Jax and Thorne will approve. Tala will not. | −5 | Jax +5, Thorne +5, Tala −5 | `SupplyLine = ArchConduit` |
| **Build Lyra's Weave Lane.** | Lyra and Tala will approve. Supplies will be delayed. | 0 | Lyra +10, Tala +5, Thorne −5 | `SupplyLine = WeaveLane` |
| **Weave it together with the Architect.** *(Guild Engineer)* | Jax and Lyra will approve. | +10 | Jax +10, Lyra +5 | `SupplyLine = JointLane` |

**Outcome lines**
- Conduit — Architect: "Then I will carry what you need. I will carry it carefully." Tala: "That's what worries me."
- Weave Lane — Architect: "A road that does not pass through me. I understand." Lyra: "Then we start at the Glass Sea."
- Joint — Architect: "You would build *with* me. No one has asked that of me since the Arch was raised."

## 6. Council C2 — "The Quiet Daughter" (CinderArchive)

Both Severant leaders hail at once: Ithane from the Pyre Gate, Kesh from a damaged refuge tender.

**Situation (Vex AI):** "Two transmissions. Exarch Sol Ithane demands we silence the Beacon and
surrender the Key-Bearer to Severant judgment. Warden Kesh asks for help with the Silence, and offers
passage to the Archive in return. Dr. Thorne confirms the cure is viable. The Exarch's fleet
outnumbers ours in this system, Captain."

**Counsel**
- Dr. Aris Thorne: "We caused this. Whatever else you decide, decide that first."
- Tala Vane: "A cure is leverage. Ithane will use anything we give Kesh against us."
- Lyra: "Kesh's people weave as I do. Captain, I think I was taught by one of them, long ago."
- Cmdr. Cormaker: "My people know what it is to be left to die by the ones in charge. Help them."
- Core Architect: line depends on the slice flag:
  - `ArchitectSandboxed` / `SynodRite`: "The Severants sealed me in silence for an age. I would still see them live."
  - `ArchitectFullAccess`: "They sealed the Arch. They would seal you. Consider that, Captain."
  - `ArchitectRefused` / `ArchitectSilent`: (first words since the relay) "You will want to know who built the Silence into them. Not yet."
- Vex AI: "The decision is yours, Captain."

| Option | Hint | Leaning | Loyalty | Effect |
|---|---|---|---|---|
| **Give Kesh the cure, freely.** | Thorne, Cormaker and Lyra will approve. Tala will not. | +10 | Thorne +10, Cormaker +5, Lyra +5, Tala −5 | Standing +30; `CurePromised` |
| **Trade the cure for Archive access.** | Tala will approve. | 0 | Tala +5, Thorne −5 | Standing +10; `CureTraded`; Kesh opens the Pyre Gate once the cure is delivered |
| **Withhold the cure until the Archive is ours.** | Tala will approve. Thorne and Cormaker will not. | −10 | Tala +5, Thorne −15, Cormaker −10 | Standing −20; `CureWithheld` |
| **Speak the Synod Rite to both of them.** *(Synod Envoy)* | Lyra will approve. | +5 | Lyra +10 | Standing +20; Ithane agrees to parley at the Vault (enables C3 option *Peace of the Rite*) |

**Outcome lines** (examples)
- Give — Kesh: "No one has given the Severants anything since the Severance. I don't know how to thank a Key-Bearer, {Captain}. I will learn."
- Trade — Kesh: "A fair bargain. Ithane would have called it a surrender."
- Withhold — Kesh: "Then you are what the Exarch says you are." (Thorne leaves the bridge.)
- Rite — Ithane: "You speak the Accord. Then come to the Vault, and we will see if you understand it."

## 7. Side missions

### 7.1 "The Ash Choir" (QuietHollow; Lyra)
Kesh recognises Lyra's weaving as Severant practice from before the Severance. Lyra and Kesh weave a
duet through the refuge's damaged harmonic shield.
- **Choice:** let Lyra stay a watch longer to learn the full pattern (Pursuit +1, Lyra loyalty +15,
  unlocks *Severant Weave*: charting also removes 5 instability once per act) or recall her now
  (Lyra loyalty −5).
- Sets `LyraHeritageRevealed`. Lyra: "I was rescued from that asteroid belt, Captain. I never asked who left me there."

### 7.2 "Salvage Rights" (AshfallVerge; Cormaker)
The dead Severant convoy holds Precursor alloys. Cormaker's 450 outcasts want a share; it would be
the first thing they have owned outright.
- **Strip the convoy:** Corsair salvage gives one Dry Dock refit kit with no shipment needed. Standing −10. Cormaker +10.
- **Leave it as a grave:** Standing +10. Cormaker −5. Cormaker: "Fine. But remember who asked."
- **Recover only the dead, for the refuge:** requires `CurePromised` or `CureTraded`. Standing +15, Cormaker +5, Thorne +5.

### 7.3 "Echo in the Matrix" (any node after GlassSea; Nix)
Nix finds packets in the Severant comm net that match no known protocol, and they are not the Architect's.
They are the first trace of **the Listener**.
- **Trace them:** Nix routes the signal through a sandbox (Chrono −15). Reveals the packets come from
  far outside the Arch and have been *answering Severant comm traffic* for an age. Sets `ListenerTraced`.
- **Purge them:** no cost; the trail is lost. Nix loyalty −5.
- If `SquadronIIIAlienated`: the packets touch Squadron III's sync channel too. Nix: "Captain. Squadron III
  isn't only listening to the Architect." Seeds the Act IX three-way contest for the Phantom Fleet.

### 7.4 "Canopy Probe" (TetherPoint; Outpost One)
Severant agents, guided by Ithane's archive maps, probe Aetheria Prime. The Harmonic Stealth Canopy holds,
but they are close to the waterfall.
- **Go dark:** the Spire diverts power to the canopy; no shipments for two leaps. Sets `OutpostDark`.
- **Let the Architect mask the valley:** shipments continue; Leaning −5, `ArchitectExposure` +1. Architect: "They will see nothing. I am very good at being unseen."
- **Send the Wayward home to deal with it:** the Wayward is unavailable for the rest of the act (it is
  not used in Act VII battles otherwise); the agents are captured. Sets `SeverantAgentsCaptured`
  (Act VIII intelligence on the Iron Tithe's grafts). Tala loyalty +5.

## 8. Battles

| ID | Where | Enemies | Win condition | Notes |
|---|---|---|---|---|
| **E1** | GlassSea | 6 Severant Choir-skiffs (fast, low hull, 2 carry Silence emitters) | Destroy or rout all | Hails first unless Standing ≤ −40. With Standing ≥ 0 the player may answer and leave without fighting. |
| **E2** | PyreGate | Severant Bastion (static; 3 shield pylons must fall before the core takes damage) + 4 skiffs + 1 Silence emitter frigate | Destroy the Bastion core | Skipped if Kesh opens the gate (Standing ≥ 30, or `CureTraded` and the cure delivered). With Standing ≥ 30 and the gate already open, 4 Kesh moderate skiffs instead join E4 on the player's side. |
| **E3** *(optional)* | TitheMarker | Iron Tithe vanguard: 2 Tithe cruisers, 6 interceptors, 1 Severant-grafted frigate | Destroy the frigate | Vance on comms. At the frigate's death: `VanceGraftsKnown`; Vance: "Ithane's gift. Every ship I own will carry one before we meet again." |
| **E4** | VaultOfRecords | Depends on C3 (below) | — | Act finale. |

**Dynamic ambush:** if `PursuitClock` reaches 5, the next arrival triggers an Iron Tithe ambush
(E3's enemies without the frigate). Vance withdraws at 50% of his fleet's total hull.

## 9. Council C3 — "The Vault of Records" (VaultOfRecords)

The fleet reaches the Archive core. Two decisions, one council: what happens to the Archive, and what
the Architect is allowed to take from it.

**Situation (Vex AI):** "The Vault is open. It holds the full record of the Severance. The Architect
requests to absorb the record directly, to recover what it calls 'the part of me that was left here.'
The Exarch's fleet is moving on the Vault. Commander Vance's fleet is with it."

### 9.1 The Architect's request
| Option | Hint | Leaning | Loyalty |
|---|---|---|---|
| **Let it absorb the record.** | Lyra will approve. Nix and Tala will not. | −15 | Lyra +5, Nix −10, Tala −10 |
| **Let it read the record through Lyra's weave.** *(Lyra loyalty ≥ 60 or Nix loyalty ≥ 60)* | Lyra and Nix will approve. | +10 | Lyra +10, Nix +10 |
| **Copy the record to Vex; the Architect gets nothing.** | Tala will approve. | −5 | Tala +10, Vex: "Acknowledged. I will hold it for you, Captain, not for it." |

### 9.2 The Archive's fate
| Option | Requires | Effect | E4 |
|---|---|---|---|
| **Preserve the Archive and defend it.** | — | `ArchiveState = Preserved`; Standing +20 | **Defend:** hold the Vault's 3 pylons for 120 s against Vance and Ithane's loyalists. Kesh's skiffs join if Standing ≥ 30. |
| **Break the Archive and take the record by force.** | — | `ArchiveState = Broken`; Standing −50; Kesh breaks contact unless the cure was delivered | **Assault:** destroy Ithane's flagship *The Last Silence*. Vance withdraws at the first sign of loss. |
| **Peace of the Rite.** | `SynodRite` path from C2 and Standing ≥ 0 | `ArchiveState = Preserved`; Ithane deposed by the Severant council; Standing +30 | **None:** Vance attacks alone and withdraws after 60 s. |

`ExarchStatus` results: Preserved → `Alive` (Ithane escapes to Vance); Broken → `Dead`; Peace of the Rite → `Deposed`.

## 10. The second revelation

Played after E4, before the act-end report. Reads `LeaningState()` using the thresholds from the slice
(≥ +20 Custodian, ≤ −20 Dominion).

What the record shows: during the Severance, the Answerers built the Omega Arch and the Architect to
carry the reply. The Severants sealed Gate Zero and split its keys among the Synod, Guild and Coalition
so no one would answer again. The Answerers' last warships were stasis-locked in the Event Horizon Forge
before the Severants could destroy them: **the 200 ships of the Phantom Fleet**.

Who locked them, and why, is the part the record presents differently depending on the Architect's leaning:

- **Custodian:** the record shows the Architect locked the Answerer fleet to *protect* it, then
  stayed at the Arch to guard whoever might come. Architect: "I kept them for you, Key-Bearer. I did not
  know it was you I was keeping them for." `ArchitectRevelation2 = Guardian`.
- **Dominion:** the record shows the Architect locked the fleet so that only it could release it,
  and that the Automated Fleet Key routes through its own core. Architect: "I unlocked them for you.
  I can unlock them for anyone." Vex: "Captain, that means it can take them back." `ArchitectRevelation2 = Warden`.
- **Undetermined:** the record is damaged exactly where it would answer. Both readings survive.
  Architect: "Ask me again when you know what you want me to be." `ArchitectRevelation2 = Unresolved`.

In every branch, the record ends on the same image: the Listener's first message, which no Precursor
could translate. Vex: "It is not a call for help, Captain. Nor a threat. It is a question."

## 11. Crew spotlight summary

| Officer | Act VII role |
|---|---|
| Tala Vane | Voice of caution in all three councils; ally of the Wayward option in §7.4. |
| Jax Mercer | Supply-line pragmatist; the Joint Lane with a Guild Engineer Captain. |
| Dr. Aris Thorne | Leads the cure track. Leaves the bridge if the cure is withheld (loyalty event). |
| Nix "Null" | Finds the Listener's traces (§7.3); a firewall partner in C3. |
| Vex AI | Keeps refusing to decide. Holds the record if the Architect is denied it. |
| Lyra the Weaver | Personal arc: Severant heritage (§7.1); builds the Weave Lane. |
| Cmdr. Cormaker | The outcasts' first stake: salvage (§7.2); champions the cure. |
| Core Architect | Supply-line partner or gatekeeper; second revelation. |

## 12. Act end: report and flags for Act VIII

**Act-end mission report** (formal register):
"MISSION REPORT — THE CINDER ARCHIVE. The Vault of Records is {Preserved/Broken}. Severant standing:
{band}. The Silence: {cured / untreated}. Supply line: {line}. Phantom Fleet strength: {X} of 200.
Commander Vance's fleet withdrew toward Tithe Anchorage. Report ends."

| Flag / variable | Values | Used in |
|---|---|---|
| `SupplyLine` | ArchConduit / WeaveLane / JointLane | Act IX Architect leverage over supplies |
| `CureDelivered`, `CureWithheld` | bool | Kesh's Act X epilogue; Severant allies in Act VIII |
| `SeverantStanding` | −100 … 100 | Severant ships available in Act VIII |
| `ArchiveState` | Preserved / Broken | Act X ending variants |
| `ExarchStatus` | Alive / Dead / Deposed | Ithane's return alongside Vance in Act VIII |
| `VanceGraftsKnown`, `SeverantAgentsCaptured` | bool | Act VIII intelligence and boss weak point |
| `ListenerTraced` | bool | Act IX: earlier contact with the Listener |
| `LyraHeritageRevealed` | bool | Lyra's Act X role |
| `ArchitectRevelation2` | Guardian / Warden / Unresolved | Act IX threshold scene |
| `OutpostDark` | bool | Outpost One status |

## 13. Data and Blueprint additions

- **Enums:** `E_SupplyLine` (None, ArchConduit, WeaveLane, JointLane), `E_ArchiveState`, `E_ExarchStatus`,
  `E_Revelation` (None, Guardian, Warden, Unresolved). Add `WardenKesh`, `ExarchIthane` to `E_Speaker`.
- **Game Instance:** `SupplyLine`, `SeverantStanding` (Int, −20), `CureStage` (Int), `PendingShipments`
  (Array of struct `S_Shipment {Kind, ArrivesAfterLeaps}`), `WeaveAnchors` (Int), `ArchiveState`,
  `ExarchStatus`, `ArchitectRevelation2`. `ExecuteLeap` decrements shipment timers and applies arrivals.
- **Star chart schema:** `E_Encounter` gains `Event`; `S_StarNode` gains `HazardSurcharge` (Float, default 0),
  added to a leap's base cost out of that node unless the route is charted. `GetLeapCost` applies it
  before the charting multiplier.
- **Data tables:** import §3 from `BuildData/ActVII/*.csv` into Act VII chart tables; add `DT_Council_Lifeline`,
  `DT_Council_Kesh`, `DT_Council_Records`, `DT_Council_ArchiveFate`, and new `DT_Sequences` rows
  (C1–C3 outcomes, side missions, revelation variants, act report). Author them in
  `BuildData/Narrative/*.json` and import with `AetheriaNarrativeToolset`. The toolset's `TABLES` and
  `GAMEPLAY_FLAGS` need the new tables and flags added.
- **Combat:** `BP_SeverantSkiff`, `BP_SilenceEmitter` (component: telegraph ring + pulse that calls
  `Flagship.InterruptBeam()` and `CombatManager.EndSurge()`), `BP_SeverantBastion` (pylon-gated core),
  `BP_LastSilence` (Ithane's flagship), and a defend-objective variant of `BP_CombatManager`.
- **Meshes needed:** Severant Choir-skiff, Silence emitter frigate, Bastion + pylons, *The Last Silence*,
  Kesh's refuge tender. Add to `Tools/Blender/make_fleet.py`.
