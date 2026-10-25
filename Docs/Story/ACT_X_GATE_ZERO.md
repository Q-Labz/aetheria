# Act X — Gate Zero

The final act, from `Docs/World/WORLD_BIBLE.md` §7.3, after `ACT_IX_THRESHOLD.md`. Values are first-pass.

**Act question:** *What is the answer?*
**Act length:** 3–5 leaps, up to 3 battles plus the ambush, 10 councils, five endings and an epilogue.

---

## 1. Premise

Beyond the Hush, at the galaxy's edge, Gate Zero: a ring twice the Arch's size, sealed by three plates bearing the
sigils of the Synod, the Guild and the Coalition. Everything the Captain has decided converges here. The Listener
asks its question directly: *What are you?*

## 2. Star chart (`BuildData/ActX/*.csv`, start `TheHush`)

| Node | Encounter | Mission |
|---|---|---|
| `TheHush` | Event (hazard 10) | Act start. *The Watch*: Vance returns (`VanceFate = Released`). |
| `Vigil` **[NEW]** | Combat | *The Wall*: the hardliners' last picket, on a moon that has watched the gate since the Severance. Ithane's last stand if he lives. |
| `GateZero` | Council | The echoes, the three plates, Vex's question, *The Answer*. *The Wall* is fought here if the Vigil was skipped. |
| `ArchWake` | Event | Optional return: one message home to Outpost One through the Arch (`ReturnSeen`). |

Routes (both ways): TheHush–Vigil 30, TheHush–GateZero 55, Vigil–GateZero 30, TheHush–ArchWake 45,
ArchWake–GateZero 50.

## 3. Councils

| Council | Requires | Options |
|---|---|---|
| The Watch | `State_VanceFate_Released` | Stand with us (`VanceAlly`: 3 Tithe cruisers fight beside the fleet) · Stay out of our way · Fight (battle X_E0) |
| The Wall | — | Break the Wall (X_E1) · Kesh speaks first (`CureDelivered`, standing ≥ 30; a smaller X_E1) · Release the inversion (`CureInversion = Kept`; no battle; standing −60, Thorne −40; `InversionUsed`) |
| The Echoes | — | Hold the plates (X_E2) · Lyra's lane (`HushLane`) · the Architect quiets them (Custodian) → `GateHeld` |
| The Synod / Guild / Coalition plate | `GateHeld` | Below → `PlateSynod`, `PlateGuild`, `PlateCoalition` |
| A Question from Navigation | all three plates | Crew / Free / Navigator / Together → `VexAnswer` (changes Vex's final line) |
| The Answer | Vex's question asked | Envoy (→ *For the Fleet*: three phrasings) · Chorus (Nix ≥ 60) · Accord (`LyraHeritageRevealed`, Lyra ≥ 75) · Door (`KeyShared` or Custodian) · Wall → `Ending`, `ActXFinaleWon` |
| The Return | — | A message home · the fleet's roll · nothing |

**The plates** (none automatic; the player chooses among those offered):

| Plate | Captain's own key | Officer | Other |
|---|---|---|---|
| Synod | Synod Envoy | Kesh's Rite of Accord (`CureDelivered`, standing ≥ 30, vow not `Silence`) | Architect override (`KeyShared` or Custodian); the beam (Instability +40, can rupture) |
| Guild | Guild Engineer | Jax's master's seal (Jax ≥ 60, not departed) | same |
| Coalition | Coalition Vanguard; Outcast Corsair (the stolen key) | Tala's codes (Tala ≥ 60, not departed); the Anchorage vault key (`CoalitionKeyRecovered`) | same |

Devoted officers add a line of their own at their plate.

## 4. Battles

| ID | Enemies | Win |
|---|---|---|
| X_E0_VanceAtHush | Vance's Carrack (objective), 3 cruisers, 2 Collectors, 6 interceptors | Disable the Carrack (`VanceDefeated`) |
| X_E1_Vigil | *The Last Silence* (objective, Ithane alive) or 2 emitter frigates (objective); 3 frigates, 8 skiffs, 4 emitter skiffs | Destroy the objective (`VigilBroken`) |
| X_E1_VigilParley | as above with 4 skiffs and 2 emitter skiffs (Kesh talked the rest down) | same |
| X_E2_GateHold | 16 echo hulls (+2 emitter frigates if the Vigil still stands) | Hold 75 s (`GateHeld`) |
| Ambush_ActX | 12 echo hulls, 2 emitter skiffs | Destroy all |

Allies in every Act X battle: Corsairs (two pairs, as Act IX), Kesh skiffs (`SeverantAllies`), Remnant cruisers
(`Absorbed`), Vance's cruisers (`VanceAlly`), Squadron III (`SquadronIIIReclaimed`).

## 5. Endings and epilogue

`PlayActEnd` for Act X plays `X_Ending_<Ending>`, then `X_Epilogue`, then the final report. The epilogue is one
sequence whose lines are gated by conditions: the Severants (`CureDelivered`, `SeverantVow`, `InversionUsed`), the
Corsairs (`CuttersRestFounded`, `CormakerDeparted`), Vance (`VanceFate`, `VanceAlly`), Outpost One (`ReturnSeen`),
each officer by loyalty band or departure, the Architect (`ArchitectChoice`) and Vex's final line (`VexAnswer`).

| Ending | Option | What happens |
|---|---|---|
| Envoy | Answer for the fleet | The Captain speaks one of three lines; the gate opens; the Listener comes to meet the fleet. |
| Chorus | Let everyone answer | Every crew answers; the Listener keeps every answer. |
| Accord | Let Lyra weave it | The Severants and the fleet answer together. Standing +50. |
| Door | Let the Architect carry it | The Architect goes through Gate Zero alone. |
| Wall | Keep the silence | The gate is sealed again; the Silence ends; the question goes unanswered. |

The final report counts the vessels that report position (Dreadnought, Wayward, active Phantoms, Corsairs unless
departed). The end screen shows the answer and Vex's; there is no Continue after Act X.

## 6. As built

- Data: `BuildData/ActX/*.csv`; `DT_Councils_ActX`, `DT_Sequences_ActX`, `DT_Battles_ActX`, `DT_NodeEvents_ActX`
  and the option tables in `BuildData/Narrative`.
- C++: `ActStartNode(10) = TheHush`; the ending chain in `AAetheriaCommandController::PlayActEnd`
  (`AfterEnding` → `X_Epilogue` → report); the final report and end-screen lines.
- Stand-ins: no Gate Zero, Vigil or Hush scenery yet (key-light colours only).
