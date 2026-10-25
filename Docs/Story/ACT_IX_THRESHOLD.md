# Act IX — The Threshold

Story and systems spec for the third act after the vertical slice, following the Act VIII model
(`ACT_VIII_IRON_TITHE.md`) and the seeds in `Docs/World/WORLD_BIBLE.md` §7.2. Anything marked **[NEW]** extends the
bible. Values are first-pass and get balanced in playtest.

**Act question:** *Whose fleet is it?*
**Act length:** 5–7 leaps, 4 battles plus the pursuit ambush, 14 councils (6 of them officer departures that only
appear for an Estranged officer). Target 2–3 hours.

---

## 1. Premise

Tithe Anchorage is in fleet hands. Beneath the Automated Fleet Key a second signal now runs on every Phantom
channel: the Listener, speaking through the ships the Answerers built to answer it. The root of the Key lies at
Meridian, the Answerer shipyard world. Whoever holds the root decides whose fleet this is: the crew, the Core
Architect, Vex, the Key-Bearer alone, or the thing on the channel.

## 2. Opening state (carried from Act VIII)

| Variable / flag | Use in Act IX |
|---|---|
| `VanceFate` | Arrival lines at Grieve; Remnant cruisers fly with the fleet (`Absorbed`); Vance returns in Act X (`Released`). |
| `ArchitectRevelation2` | The Architect's opening line. |
| `ExarchStatus` | Who contests the root in E4 (*The Last Silence* if `Alive`, leaderless emitters otherwise). |
| `SquadronIIIAlienated` | The Forge Echo holds Squadron III in the Listener's sync (council *Squadron III*, battle E3). |
| `CureDelivered`, `CureSample` | Kesh's council *The Last Warden*; Thorne's *Inversion*. |
| `RustmarchPromised`, `CorsairsDeparted` | *Cutter's Rest* variants. |
| `SeverantAllies` | Kesh skiffs in every Act IX battle. |
| `LyraHeritageRevealed` | Lyra can weave a lane through the Hush; she can speak for the fleet at Seren Vael. |
| `ArchiveState` | The envoy can carry the Archive's record. |

## 3. Star chart (`BuildData/ActIX/*.csv`, start `Grieve`)

| Node | Encounter | Mission |
|---|---|---|
| `Grieve` | Council | *The Anchorage*: hold, strip, seal or garrison it. Act start. |
| `Rustmarch` | Event | *Cutter's Rest* (Cormaker): found the Corsair station, battle E1. |
| `SerenVael` | Council | *The Last Warden* (Kesh) or *The Envoy*; *First, Do No Harm* (Thorne). |
| `MeridianYards` **[NEW]** | Combat | *The Yard Wakes* (battle E2) and *What It's Made Of* (Jax). |
| `Meridian` | Council | *The Threshold* (the Architect's own question); later the root, E4 and *The Fleet Key*. |
| `ForgeEcho` | Event (hazard 15) | *Squadron III* (Nix; battle E3) or *The Echo*. Arrival recovers the Key's root record (`RootRecord`). |
| `TheHush` | Event (hazard 10) | *The Array* (Lyra). Arrival sets `ListenerQuestionHeard`. |

`MeridianYards` is the one node not in the bible: the shipyard ring around Meridian (§3.5.5), split off so the
yard's defences and Jax's refit mission happen apart from the root.

Routes (both ways): Grieve–Rustmarch 45, Grieve–SerenVael 40, Grieve–Meridian 60, Rustmarch–SerenVael 40,
Rustmarch–MeridianYards 40, SerenVael–Meridian 45, SerenVael–ForgeEcho 40, MeridianYards–Meridian 15,
Meridian–ForgeEcho 30, Meridian–TheHush 40, ForgeEcho–TheHush 50.

**The finale is gated by the Hush,** not by drive cost: *The Root*, *The Line Holds* and *The Fleet Key* at Meridian
require `ListenerQuestionHeard`. The first visit to Meridian plays only the threshold scene.

## 4. Systems introduced

### 4.1 Live conditions
Every requirement (lines, counsel, options, councils, battle groups) goes through
`UAetheriaGameInstance::HasFlag`, which reads story flags and also:
`State_<Key>_<Value>`, `Leaning_<Custodian|Dominion|Undetermined>`,
`Loyalty_<Officer>_<Devoted|Trusting|Steady|Strained|Estranged>` (bands of `WORLD_BIBLE.md` §6) and
`Standing_<Hostile|Wary|Trusted>`; `Not_<X>` negates and `A+B` requires every part
(`CureDelivered+Not_InversionUsed`). `validate_narrative` checks they are well formed.

### 4.2 Synced squadrons (Squadron III, the Architect's held squadrons)
Twenty Phantom battle-cruisers of the fleet's own 200. Their hull is the **sync**: ordinary fire wears it down, and
at zero the squadron is **disabled** and its ships come back to the Key (`ResyncPhantoms`). The **Prismatic Stellar
Beam destroys 4 hulls per hit** for good (Nix warns on the first). Held squadrons (Dominion) were never out of the
fleet: disabling them costs nothing, beam losses come out of `PhantomShipsActive`.

### 4.3 Canon count
`PhantomShipsDestroyed` counts hulls lost in battle. Resynchronising (`PhantomDelta` on an option, a reclaimed
squadron) never raises `PhantomShipsActive` past 200 minus the destroyed: desynchronised and alienated ships can
come back; destroyed ones cannot, and the fleet never gains hulls (echo hulls at the Forge Echo are empty frames).

### 4.4 New enemies
| Class | Faction | Behaviour |
|---|---|---|
| `BP_SyncedSquadron` / `BP_HeldSquadron` | Listener / Answerer | §4.2. Disabled, not destroyed. |
| `BP_EchoHull` | Listener | Empty Answerer hull; no guns; drifts at the nearest fleet ship and bursts on contact (300 in 700 uu). |
| `BP_YardSentinel` | Answerer | Static lance: locks a firing line, telegraphs it 1.8 s, then hits every fleet ship on the line (450). Move off the line in the pause. |
| `BP_SentinelDrone` | Answerer | Fast, fragile swarm interceptor (220 hull). |

### 4.5 Officers who leave
An officer whose loyalty is Estranged (< 0) when the Fleet Key is decided brings a resignation council
(`Farewell_<Officer>`): ask them to stay (+25) or accept it (`<Officer>Departed`; Cormaker takes the Corsairs to
Rustmarch). Never automatic.

## 5. Councils

| Council | Node | Requires | Options (effects) |
|---|---|---|---|
| The Anchorage | Grieve | — | Remnant hold it (Thorne +5, Tala −5) · Strip it (+25% hull; Jax +10, Thorne −5) · Seal it (no Carrack in E1; Tala +5) · Corsair garrison (`CorsairsReduced`; Cormaker +10) → `AnchorageHeld` |
| Cutter's Rest | Rustmarch | — | Found it (battle E1) · Found it with half the Armada on guard (`CorsairsReduced`) · Ask the departed Corsairs back (clears `CorsairsDeparted`) · Promise Rustmarch · Decline |
| The Last Warden | SerenVael | `CureDelivered` | Ask them to answer with us / keep their vow / their own choice → `SeverantVow` (Answer / Silence / TheirOwn) |
| The Envoy | SerenVael | not `CureDelivered` | Send Lyra · the Archive's record · the Captain's word · no one |
| First, Do No Harm | SerenVael | `CureSample` | Destroy / Keep / Give to the Severants → `CureInversion` |
| The Yard Wakes | MeridianYards | — | Clear the defences (E2) · Nix talks to the yard (Nix ≥ 60) · the Architect speaks (Custodian) |
| What It's Made Of | MeridianYards | `YardsSecured` | Resync with Answerer parts (+30 ships at most, Exposure +1) · through Nix's firewall (+20) · take the records · strip for plate (+30% hull) |
| The Threshold | Meridian | — | Whole / Fraction / Its own choice / Watched → `ArchitectChoice` |
| Squadron III | ForgeEcho | `SquadronIIIAlienated` | Reclaim (E3) · Nix alone (Nix ≥ 60; +20) · Release · Quarantine (contests E4) |
| The Echo | ForgeEcho | not alienated | Nix answers (`ListenerContact`) · Lyra listens · Cut the channel |
| The Array | TheHush | — | Leave it · Weave a lane (`HushLane`) · Destroy it → `ArrayState` |
| The Line Holds | Meridian | Hush done, Dominion | Tala prepares the cut (`CutPrepared`) · Not yet |
| The Root | Meridian | Hush done | Hold the root (E4) · the Architect holds it (Custodian) |
| The Fleet Key | Meridian | `RootHeld` | Crew (needs `RootRecord` or `CutPrepared`) / Shared / Vex / Captain → `FleetKey`, `ActIXFinaleWon` |

*Release Squadron III* moved from the Fleet Key council (bible §7.2) to *Squadron III*, where the squadron is.

## 6. Battles

| ID | Where | Enemies | Win | Allies |
|---|---|---|---|---|
| E1_CuttersRest | Rustmarch | Carrack (not if Sealed), 2 cruisers, 2 Collectors, 8 interceptors | Hold 90 s | Corsairs, Remnant cruisers, Kesh skiffs |
| E2_YardWake | MeridianYards | 3 Yard sentinels (objective), 10 sentinel drones | Destroy the sentinels | — |
| E3_Sync | ForgeEcho | Squadron III synced (objective), 10 echo hulls | Disable the squadron | — |
| E4_Root | Meridian | 8 drones + Ithane (Last Silence, 2 frigates, 4 skiffs) or leaderless emitters + 2 held squadrons (Dominion) + Squadron III (quarantined) | Hold 120 s | Corsairs, Remnant, Kesh skiffs, Squadron III (reclaimed) |
| Ambush_ActIX | pursuit clock 5 | 8 echo hulls, 2 emitter frigates, 6 drones | Destroy all | — |

Allied Corsairs come in two pairs: one leaves with `CorsairsDeparted`, the other with `CorsairsReduced`.

## 7. Act end

*The Fleet Key* sets `ActIXFinaleWon`; departures, if any, follow; then the report (C++ `PlayActReport`): the
Key's root, Squadron III, the cure's inversion, the Listener's question, Phantom Fleet strength.

| Flag / state | Used in Act X |
|---|---|
| `FleetKey`, `KeyShared` | Opening lines; the Architect's override at the plates; the *Door* ending |
| `SquadronIIIReclaimed` | Squadron III flies with the fleet in Act X battles |
| `CureInversion` | `Kept` opens *Release the inversion* at the Vigil |
| `HushLane`, `ArrayState` | Lyra's lane through the echoes at the gate |
| `SeverantVow` | Kesh can speak the Rite at the Synod plate unless `Silence` |
| `ArchitectChoice` | Epilogue |
| `<Officer>Departed` | Their plate routes, counsel and endings are gone |

## 8. As built

- C++: `AetheriaThresholdShips.*` (the five classes), `UAetheriaBeamDamage` (the beam's damage type),
  `E_Faction::Answerer` / `Listener`, `FS_CouncilOption.ClearFlags` / `PhantomDelta`, `HasFlag`, `LoyaltyBand`,
  `ResyncPhantoms`, `PhantomShipsDestroyed`; the combat manager reclaims synced squadrons on victory and counts an
  allied Phantom squadron's losses; `ApplyOption` ruptures the drive when an option pushes it past 100.
- **Visits are per act** from Act VIII on (`Visited_Act<N>_<Node>`): nodes recur (TitheMarker, Grieve, SerenVael),
  and a visit in one act no longer swallows the next act's arrival scene.
- Data: `BuildData/ActIX/*.csv`; `DT_Councils_ActIX`, `DT_Sequences_ActIX`, `DT_Battles_ActIX`, `DT_NodeEvents_ActIX`
  and the option tables in `BuildData/Narrative`.
- Stand-ins: echo hulls and drones use the Phantom cruiser mesh, sentinels the Bastion pylon.
- Node scenery (L_Command and the combat map, folders `Nodes/<Node>`, tag `Node_<Node>`): built from the existing
  environment kit with new planet instances `MI_Planet_RustmarchMajor`, `_SerenVael`, `_Meridian`, `_Vigil`, rings
  `MI_Ring_Meridian`, `_SerenHarmonic`, `_Hush` and `MI_Star_Rustmarch`. Grieve: the gas giant and rings, Tithe buoys
  for the Anchorage. Rustmarch: an orange star, Rustmarch Major, belt fragments. Seren Vael: the planet and its
  harmonic ring. Meridian / the Yards: the ecumenopolis and its yard ring (sentinel pylons at the Yards). Forge Echo:
  the black hole and accretion disc, the Ember Relay mesh as the stasis Forge. The Hush: crystal-blade fields as the
  Listening Array under a violet shimmer ring. Purpose-built meshes (Anchorage, Array, Cutter's Rest) are still to do.
