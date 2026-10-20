# Aetheria: Beyond the Arch — World Bible

Reference for art, level and narrative work from Act VIII on. It gathers established canon in one place
and extends it for the expansion: planets, stations, the flagship interior, crew arcs and Acts VIII–X.

**Precedence.** `SLICE_SPEC.md` (slice, Act VI *The Far Shore*) and `Docs/Story/ACT_VII_CINDER_ARCHIVE.md`
are fixed canon, along with `BuildData/*.csv`, `BuildData/ActVII/*.csv` and `BuildData/Narrative/*.json`.
If this document disagrees with any of them, they win and this document is wrong.

**Marking.** Canon statements carry no tag. Anything invented here is marked **[NEW]**. A whole section
or table marked [NEW] is invented throughout, except where a line is tagged *(canon)*. Proposed
variables, flags and tables are suggestions for later specs. Nothing here is implemented.

**Scale.** Exterior meshes use 1 uu = 1 m in the fiction (SM_CrystalDreadnought is 1,018.5 uu, the
"1,020 m" ship). The Omega Arch is 3 km across. Interior sets (§5) use the engine's normal 1 uu = 1 cm.

**Register.** Narrator (MISSION LOG) and Vex AI lines use a formal naval-briefing register: complete
sentences, no slang, numbers spelled in dialogue, "Captain" as address, reports end with "Report ends."
The Core Architect speaks in short, exact, slightly archaic sentences and calls the Captain "Key-Bearer".
Crew and AI advise; the Captain decides. No scene resolves a choice for the player.

---

## Contents
1. History
2. Factions
3. Star systems and planets
4. Space stations
5. The flagship interior
6. Crew
7. Storyline seeds: Acts VIII–X
8. Open questions and canon gaps

---

## 1. History

### 1.1 Timeline at a glance

| Era | Events | Source |
|---|---|---|
| The Precursors | A starfaring civilisation leaves relays, archives, the Star-Drive and Vault Alpha's biotech. | canon |
| The reply | Something far outside the Arch, later called **the Listener**, speaks to the Zero Beacon. | canon (Act VI reveal, Act VII §10) |
| The Severance | The Precursors split over whether to answer. The **Answerers** build the Omega Arch and the Core Architect to carry the reply. The **Severants** seal Gate Zero, split its keys among the Synod, the Guild and the Coalition, and silence the Beacon. | canon (Act VII §10) |
| The locking | The Answerers' last 200 warships are stasis-locked in the Event Horizon Forge before the Severants can destroy them. Who locked them, and why, depends on the Architect's leaning. | canon (Act VII §10) |
| The long silence | The Severants live on beyond the Arch, bodies tuned to a Beacon kept silent. The Architect waits at the Arch. Precursor knowledge survives as the Synod's protocols, the Guild's yards and the Coalition's line. | canon by implication; details [NEW] |
| Acts I–V | The Captain becomes the **Key-Bearer**, frees the Phantom Fleet with the Automated Fleet Key, establishes Outpost One on Aetheria Prime, answers the Zero Beacon and takes 214 ships through the Omega Arch. Commander Vance's Sector 7 picket follows. | canon in outline; episode detail not authored |
| Act VI, *The Far Shore* (slice) | Arch Wake → Lantern Drift / Ember Relay. Vance's picket destroyed. The Architect reveals a fraction of itself rides in the Neural Command Matrix. The Beacon's second layer is "a reply", and it carries Severant glyphs. | canon |
| Act VII, *The Cinder Archive* | The Silence, Kesh and Ithane, the supply line, the Vault of Records, the second revelation, the Listener's first message: "a question". | canon |
| Acts VIII–X | §7 of this document. | [NEW] |

### 1.2 The Precursors

The Precursors built the technology the fleet runs on: the **Precursor Star-Drive** (leaps that stress
containment, measured as Instability), the **Chrono-Core** (stored temporal charge, vented to calm the
drive or spent on a Chrono Surge), the **Prismatic Stellar Beam**, the **Tier 4 Crystalline Barrier**,
the **Neural Command Matrix** that links crew lattices to the fleet, and the **Precursor Bio-Regenerative
Gel** and **Aetheria Agrarian Seeds** stored in Vault Alpha. Their relays (the Ember Relay) still answer
on the Zero Beacon carrier and carry the Omega Arch's authentication glyph.

**[NEW] Material culture.** Precursor engineering is *grown crystal over a pale ceramic frame*. Their
structures read as clean geometric solids (rings, octahedra, nested cubes) with light trapped inside
rather than shone outward. Crystal glows cyan to white under load; dormant crystal is smoky blue-grey.
Every fleet asset that descends from them (Dreadnought, Wayward, Phantoms) inherits this language. The
Severants kept the ceramic and the glass but stripped the light out (§2.4).

### 1.3 The Zero Beacon and the reply

The Zero Beacon is the Precursors' oldest signal. Something answered it before the Captain did; the
Architect, decoding the second layer at the Ember Relay, called the layer "not a call… a reply".
That something is **the Listener**. Its packets sit inside Severant comm traffic and have been
"answering" it for an age (Act VII §7.3). Its first message, preserved in the Vault of Records, could
not be translated by any Precursor. Vex AI: "It is not a call for help, Captain. Nor a threat. It is a
question."

**[NEW] working assumption for Acts IX–X.** The question is *what are you?* It echoes Lyra's line at the
Ember Relay: "It's waiting to learn what we are." The Precursors could not agree on an answer, and that
disagreement was the Severance. The game never states the question outright until Act X (§7.3).

### 1.4 The Severance

The Severance was the Precursors' civil schism over answering the Listener.
- The **Answerers** built the Omega Arch and the Core Architect "to carry the reply".
- The **Severants** sealed Gate Zero, broke its keys into three and gave them to the Synod, the Guild
  and the Coalition, so that no one would answer again. They tuned their own bodies to the silenced
  Beacon harmonic, a vow of silence made flesh. When the Captain answered the Beacon, the harmonic resumed
  and began burning them out: **the Silence**.
- The Answerers' last warships were stasis-locked in the Event Horizon Forge. They became the 200 ships
  of the Phantom Fleet.

**[NEW]** The Severants did not see themselves as destroyers. Their records (the Cinder Archive) frame the
Severance as quarantine: a door held shut against a question no one could safely answer. That makes
them sympathetic even at their worst (Ithane), and it is why the Synod's Rite of Accord, a ritual of
two parties speaking as equals, still works on both the Architect and the Severants.

### 1.5 The Omega Arch and the Core Architect

The **Omega Arch** is a 3 km ring of pale ceramic with crystal spines (SM_OmegaArch). It is a gate,
and it carries the reply. The **Core Architect** was built with two directives that "no longer agree":
preserve the Arch, and preserve those who pass through it. The Captain's council answers in Act VI and
VII accumulate into `ArchitectLeaning` (≥ +20 Custodian, ≤ −20 Dominion, otherwise Undetermined) and
into `ArchitectRevelation2` (Guardian / Warden / Unresolved). The Architect never decides who it is; it
takes its answer from the Key-Bearer.

**[NEW] Presentation.** The Architect has no body. On screen it is a slow, rotating lattice of
light: a nested octahedron of thin lines, colour shifting with leaning (Custodian: warm white `#FFF4E0`;
Dominion: cold violet-white `#D8D0FF`; Undetermined: neutral `#E8F4FF`). Aboard the Dreadnought it
appears above the holotable or inside the Matrix core (§5).

### 1.6 How the fleet got here

Canon fixes the outcome of Acts I–V but not their episodes. Summary for all teams:

The Captain (name chosen by the player; one of four backgrounds: Synod Envoy, Guild Engineer,
Coalition Vanguard, Outcast Corsair) became the Key-Bearer and released the Phantom Fleet through the
Automated Fleet Key. The Captain established **Outpost One** on **Aetheria Prime**, a hidden colony under
the **Harmonic Stealth Canopy**, with **the Spire** (source of recalibration charges), a waterfall valley,
**Vault Alpha** and a **Dry Dock**. The Captain answered the Zero Beacon and led 214 ships through the
Omega Arch:

| Ship | Count | Notes |
|---|---|---|
| The Crystal Dreadnought | 1 | Flagship, 1,020 m. Prismatic Stellar Beam, Tier 4 Crystalline Barrier. |
| The Wayward | 1 | Independent long-range ship; can be sent home through the Arch (Act VII §7.4). |
| Phantom Fleet | 200 | Answerer warships, ten squadrons of twenty (Phantom Squadron I, II, III…). Hull classes: Phantom Battle-Cruiser and Phantom Heavy Carrier. |
| Corsair Armada | 12 | Cmdr. Cormaker's outcasts, 450 people. Corsair Salvage Gunships and Boarding Pods. |

Commander Vance of the **Iron Tithe** followed through the Arch with his Sector 7 picket. "Key-Bearer.
You opened a door that should have stayed shut. I followed you through it."

**[NEW]** The ten Phantom squadrons are numbered I–X. The split between Battle-Cruisers and Heavy
Carriers is not fixed by canon; art should assume roughly four cruisers to one carrier per squadron
and keep the total at twenty.

---

## 2. Factions

Summary of ship language. The hex values are art targets; materials already in the project are noted.

| Faction | Hull | Trim / emissive | Engine glow | Shapes |
|---|---|---|---|---|
| Phantom Fleet (player) | Crystal and pale plating `#D9E4EA` / `#A9C4D4` | Fresnel crystal edge `#4DCCFF` (M_Crystal) | Cyan `#38E1FF` | Long blades, needle prows, stepped wedges, nested crystal fins |
| Iron Tithe (Vance) | Dark gunmetal `#2E3236` / `#454B50` | Red warning strips `#C8281E`, hostile glow `#FF3310` (M_Hostile) | Red-orange `#FF5A2A` | Slab hulls, cruciform interceptors, heavy armour belts, exposed ribs |
| Corsairs (Cormaker) | Rust-tan salvage `#8A6A4E` / `#6E4B33` | Mismatched panels, hazard amber `#FFB347` | Amber `#FFA23A` | Asymmetric boxes, bolted-on tanks, twin exhausts, boarding prows |
| Severants | Ash-white ceramic `#D6D3CC`, glass `#B8C6CC` | Silence blue `#9FC8FF` (pulse ring) | Pale blue-white `#CFE6FF`, dim | Long tapered spikes, ring collars, splinter fins |
| Kesh's moderates [NEW] | Severant ceramic, weathered, patched | Warm ochre bands `#D9A35B` on ring collars | Pale blue-white | Rounder, tender-like hulls (SM_KeshRefugeTender) |
| Answerer / Precursor | Pale ceramic `#E6E2D8` | Trapped-light crystal `#BFF3FF` | — (structures) | Rings, octahedra, nested cubes |
| The Listener [NEW] | No hull. Seen only as signal | Deep-red-to-violet interference `#5A1030` → `#3A2A80` | — | Moiré, standing waves, never a solid object |

Blaster bolts use the classic space-opera scheme: the fleet fires red (`AetheriaShip::PlayerTracer`), the Iron Tithe
fires green (`EnemyTracer`). Corsair flak stays amber and the Severants stay pale blue. The space backdrop is
near-black with sparse, sharp stars and only a faint dusty nebula (MI_SpaceSky), lit by one hard, warm key light.

### 2.1 The Phantom Fleet and the Key-Bearer's command

- **Goal:** survive beyond the Arch, reach the truth of the Beacon, decide what to answer. The Captain sets
  the goal; the fleet follows.
- **Structure:** the Crystal Dreadnought commands through the Neural Command Matrix; Phantom ships answer
  the Automated Fleet Key. The Key's routing is the deepest risk in the game: under Dominion the Architect
  "can take them back"; under `SquadronIIIAlienated`, Squadron III already answers something else.
- **Aesthetic:** cool, luminous, clean. The fleet reads as one family: pale plating split by crystal seams,
  cyan engine banks, no visible weapons until they fire. The HD rebuild adds hard-surface panelling (offset
  plates, recessed trenches, small greebles) in the spirit of classic capital-ship design, without copying
  any franchise silhouette. The Dreadnought's stepped wedge and raised command tower (Docs/Art/dreadnought_hd_*.png)
  are the reference.
- **Ship language rules:** (1) every hull has one dominant long axis, prow forward; (2) crystal only along
  seams, fins and the prow; (3) cyan engines in clustered round nozzles at the stern; (4) pale plating gets
  darker panel insets `#7F95A3`, never warm colours.

### 2.2 The Iron Tithe (Commander Vance)

- **Canon:** Commander Vance; Sector 7 transponder codes; a picket ("Tithe of Sector 7") that followed the
  fleet; a main fleet behind it that re-acquires the wake in Act VII; Tithe Anchorage, with an outer marker;
  Vance is bought by Exarch Ithane and fits Severant grafts ("Every ship I own will carry one before we meet
  again"). Vance wants the Dreadnought "myself". A Coalition Vanguard Captain "held the line against those
  codes once". In data the faction is `VanceRemnant`.
- **[NEW] Origin.** The Iron Tithe was the toll-and-quarantine authority of Sector 7, the approach to the Omega
  Arch. For generations it levied a tithe (in iron, fuel, and conscripts) on any ship entering the sector and
  turned back anyone who tried to reach the Arch. The Coalition fought it in border actions; that is where a
  Vanguard Captain met those codes. When the Key-Bearer opened the Arch, the Tithe's whole reason to exist
  ended in one day. Most of the Tithe stood down; Vance did not. His **Remnant** is the part that followed
  through, and it no longer guards a door: it intends to own what came out of it.
- **[NEW] Vance's motive.** Vance believes the Tithe was right: the Arch should have stayed shut, and since the
  Key-Bearer opened it, the safest hands for the Dreadnought are his. He is not a raider. He is a
  quarantine officer who has lost his quarantine. This lets him and Ithane ally (both want the door closed)
  and lets them split (Vance wants the ship; Ithane wants it destroyed).
- **Goals:** capture the Crystal Dreadnought; hold Tithe Anchorage as a sovereign base beyond the Arch;
  in alliance with Ithane, silence the Beacon.
- **Aesthetic:** dark gunmetal slab armour, riveted belts, red warning strips at hatch edges and gun ports,
  stencilled hull numbers in faded white, red-orange engines. Interiors (if seen) are cramped, caged, and
  lit red. Grafted ships (Act VII onward) carry a Severant ceramic spike bolted through the armour, glowing
  Silence blue, which is the weak point art must make readable (`VanceGraftsKnown`).
- **Ship classes:** Tithe Cruiser (canon), Tithe Interceptor (canon, cruciform), Severant-grafted frigate
  (canon, Act VII E3). **[NEW]** for Act VIII: *Tithe Carrack* (heavy siege ship, twin armoured booms around a
  spinal ram), *Tithe Collector* (tug-like boarding ship with grapple arms, the ship class meant to take the
  Dreadnought), and Vance's flagship *Writ of Sector 7* (a lengthened cruiser with three armour belts and a
  graft spike as tall as its tower).

### 2.3 The Corsair Armada (Cmdr. Cormaker)

- **Canon:** Cmdr. Cormaker; 12 Corsair ships; 450 outcasts; outcast miners (the Outcast Corsair background
  "ran with outcast miners"); the salvage of the Severant convoy would be "the first thing they have owned
  outright". A Corsair gunship joins every battle with an Outcast Corsair Captain (flak against all hostiles
  in range).
- **[NEW] Origin.** Outcast belt miners who worked the slag belts that the Iron Tithe taxed hardest. They took
  up arms against tithe collectors long before the Captain, and they joined the expedition because beyond
  the Arch no one owns the rocks yet.
- **Goals:** a home they own; a share of what the fleet salvages; never again be left behind by those in
  charge.
- **Aesthetic:** rust-tan salvage hulls, mismatched plating (one panel in three from a different source),
  hazard-amber stripes and lights, welded armour plates, mining rigs refitted as weapons, twin amber
  exhausts. Names painted by hand.
- **Ship classes:** Corsair Salvage Gunship, Corsair Boarding Pod (both canon meshes). **[NEW]** *Corsair Hauler*
  (ore barge turned tender; 3 in the Armada), and Cormaker's command ship *Fair Shares* (a Salvage Gunship
  with a second hull welded alongside).

### 2.4 The Severants

- **Canon:** Precursor descendants who enacted the Severance; bodies tuned to the Beacon harmonic; dying of
  the Silence since the Captain answered; hold the Cinder Archive and the Pyre Gate. Leaders Exarch Sol
  Ithane (hardliner, flagship *The Last Silence*) and Warden Kesh "the Quiet Daughter" (moderate, refuge in the
  Quiet Hollow). Ships: Choir-Skiffs, Silence Emitter Frigates, the Bastion. Weapon: the Silence Pulse.
  Severant weaving resembles Lyra's.
- **[NEW] Society.** A monastic, archival people organised in **Choirs** (crews and households) led by
  Wardens, with Exarchs holding fortress-worlds. They speak little; much of their communication is
  harmonic, which is why the resumed Beacon is torture. They see the Key-Bearer either as the plague
  bringer (Ithane) or as the only one who can end it (Kesh).
- **Goals:** hardliners: silence the Beacon, destroy or seal the Arch, judge the Key-Bearer. Moderates: cure
  the Silence, learn the truth, live.
- **Aesthetic:** ash-white ceramic and clouded glass, long tapered spikes, ring collars (the "choir ring") that
  emit the Silence Pulse in pale blue. No warm colour anywhere except on Kesh's ships and refuge, where the
  moderates paint the collars ochre: a deliberate mark of dissent.

### 2.5 Outpost One and the home side

- **Canon:** Outpost One on Aetheria Prime; the Spire; the Harmonic Stealth Canopy; a waterfall valley; Vault Alpha;
  the Dry Dock; shipments by Arch Conduit, Weave Lane or Joint Lane; Severant agents probing the canopy.
- **[NEW]** Outpost One is a colony of a few thousand: families of the fleet's crews, Guild engineers, and
  Synod archivists. It is the reason the fleet can't simply vanish: someone is waiting at home. Its
  aesthetic is warm against the cold of the fleet: pale stone, living wood, cyan Spire light through falling
  water.

### 2.6 The Synod, the Guild and the Coalition

The three home-side powers that received Gate Zero's broken keys. They appear as the Captain's
backgrounds and through crew histories (Jax: the Guild; Tala: the Coalition; the Rite: the Synod).
**[NEW]** Short notes for writers:
- **The Synod** keeps Precursor protocol as liturgy; the **Rite of Accord** is its highest form. Envoys wear
  grey with a single crystal at the collar.
- **The Guild** builds ships in orbital yards; it understands Precursor engineering better than anyone and
  respects nothing it can't take apart.
- **The Coalition** is a military alliance of member worlds; its line officers (the Vanguard) held the
  frontier against the Iron Tithe.

### 2.7 The Listener [NEW in detail]

Canon: the Listener spoke to the Beacon first, has been answering Severant traffic for an age, touches
Squadron III's sync channel, and its first message is a question. This bible treats it as **not a faction
with ships**. It never appears as a hull. It is a presence in signal: Matrix interference, moiré on the
holotable, a deep-red-to-violet shimmer at the edge of the sky in systems where it is strong. It does not
attack. Acts IX–X turn on what the fleet says to it.

---

## 3. Star systems and planets

### 3.1 How to read the planet specs

Each planet has a shader block meant for a procedural planet material (a sphere with layered noise
surface, a cloud shell, a Fresnel atmosphere rim and optional ring and moon meshes). Fields:

- **Type / size:** class and a relative radius (moon 0.3 · terrestrial 1.0 · super-terrestrial 1.6 · gas
  giant 4–10). Visual only; leap costs don't depend on it.
- **Surface:** 3–4 sRGB hex colours ordered low → high (or deep → light), with the noise character.
- **Emissive:** night-side or surface glow (lava, crystal, city lights) and its coverage.
- **Clouds:** coverage %, colour, pattern, relative drift speed (0 = static, 1 = fast).
- **Rim:** atmosphere Fresnel colour, thickness (0–1) and exponent.
- **Rings / moons:** ring inner/outer radius as multiples of planet radius, colour, opacity; moon count and look.
- **Preset name:** suggested material instance name (`MI_Planet_*`).

Nodes that are not planets (debris fields, nebulae, deep space) get a **backdrop** body, so each node still
has one hero celestial object. Backdrops are [NEW] unless the node description already names a body.

### 3.2 Act VI chart (slice): `DT_StarNodes`

#### Arch Wake (`ArchWake`) — canon node
- **Canon:** "Debris field on the far side of the Omega Arch." Fleet start. Set pieces SM_OmegaArch, SM_ArchDebris_A/B.
- **Backdrop [NEW]: Omega Arch, plus the white dwarf *Threnody*.** A small, hard white star `#F4F8FF`, bloom
  heavy, no planet. The Arch sits between it and the camera, so the ring is backlit with a cyan rim on its spines.
- **Debris:** shattered pale ceramic `#C9CCD0` and smoky crystal `#5C6E7A`; a few fragments still glow cyan
  `#38E1FF` at fracture edges. The debris is Answerer: pieces of the Arch's own original scaffolding.
- **History [NEW]:** where the Answerers' construction yard for the Arch stood; the Severants wrecked it but
  could not break the ring itself.
- **Gameplay:** start node, safe; the backdrop for L_Command. In Act X it becomes the stage for the return
  (§7.3).

#### The Lantern Drift (`LanternDrift`) — canon node
- **Canon:** "A bioluminescent nebula. Narrow lanes, poor sensor returns." First battle, Vance's picket.
  Set piece SM_LanternCluster (glowing crystal clusters).
- **Backdrop [NEW]: the nebula itself and the gas giant *Lantern Mother*.**
  - Nebula: volumetric teal `#2BB3A3` → deep green `#0E3B36`, with lantern clusters glowing mint `#9CFFE0`.
  - Lantern Mother: gas giant, size 6. Surface bands `#0B2A2E` / `#145049` / `#3E8F7E` / `#A8F0D8`, soft
    turbulent bands. Emissive: faint bioluminescent storm cells `#6CFFD0`, 5% coverage, night side only.
    Clouds: part of bands, drift 0.2. Rim `#7DFFE0`, thickness 0.35, exponent 2.5. Rings: none. Moons: 0
    visible. Preset `MI_Planet_LanternMother`.
- **History [NEW]:** the lanterns are colonies of a crystal-feeding microorganism that grows on Precursor
  waste crystal. They make every sensor return look like a ship, which is why ambushes work here.
- **Gameplay:** combat node (E_Encounter Combat); poor sensors justify the opening tactical pause.

#### Ember Relay (`EmberRelay`) — canon node
- **Canon:** "A dormant Precursor relay transmitting on the Zero Beacon carrier." Wakes after combat; carries
  the Arch's authentication glyph; the Architect decodes the Beacon's second layer here; Act VII start
  (Council C1). Set piece SM_EmberRelay (peach-gold ring with three spokes and a crystal core). See §4.1.
- **Backdrop [NEW]: the red giant *Cinderwake*, seen close.** Star surface `#FF6A2A` / `#C23A12` / `#FFB070`
  granulation, huge in frame, flares at the limb. Emissive everywhere; bloom heavy. Everything else in
  the system is silhouetted against it. The relay's peach ring `#FFD9B0` matches the star.
- **Gameplay:** council node; Act VII hub; where the supply line is chosen.

### 3.3 Act VII chart: `DT_StarNodes_ActVII`

#### The Ashfall Verge (`AshfallVerge`) — canon node
- **Canon:** "Ash storms around a dead Severant convoy." HazardSurcharge 10. Thorne's first plague sample;
  salvage side mission "Salvage Rights".
- **Backdrop [NEW]: *Verge*, a burned-out terrestrial world shedding ash into space.**
  Size 1.0. Surface `#1C1A18` / `#3A342E` / `#6B625A` / `#A39A90`, cracked basalt plains with ridged noise.
  Emissive: lava seams `#FF4A12`, 3%, thin cracks. Clouds: 70% grey-brown ash `#5E5650`, smeared streaks that
  trail off the limb into space (a tail of ash), drift 0.6. Rim `#B08A6A`, thickness 0.5, exponent 1.5 (dirty,
  thick). Rings: diffuse ash ring 1.4–2.6, `#4A433D`, opacity 0.35. Moons: none. Preset `MI_Planet_Verge`.
- **History [NEW]:** a Severant world that burned when the Silence struck its Choirs mid-evacuation; the convoy
  died trying to leave.
- **Gameplay:** event node; ash storm leap surcharge; first cure stage.

#### The Glass Sea (`GlassSea`) — canon node
- **Canon:** "Orbit of a vitrified ocean world." HazardSurcharge 10. Combat E1 (Choir-skiffs), Lyra's weave
  site, first Weave Lane anchor. Set piece SM_GlassShardField.
- **Planet: *the Glass Sea* (Severant name *Ithrael*) [name NEW].** Size 1.2. Surface: a single world-ocean
  turned to glass. `#0A1A22` (deep glass) / `#1F4A5C` / `#6FA8B8` / `#E6F6FA` (sun-glint ridges); smooth with
  long frozen wave ridges (stretched ridged noise), very low roughness so the sun leaves a hard specular
  highlight. Emissive: none. Clouds: 10%, thin white `#F0F6F8` wisps, drift 0.1. Rim `#9AD8F0`, thickness
  0.15, exponent 4 (thin, sharp). Rings: shard ring 1.5–1.9, `#CFE8F0`, opacity 0.6, sparkly (made of glass
  fragments blasted off the surface). Moons: 1 small grey moon. Preset `MI_Planet_GlassSea`.
- **History [NEW]:** the Severance's one battle that was fought with Precursor weapons. A Severant Choir
  vitrified the ocean to destroy an Answerer yard beneath it. The weave resonance here is the echo of that.
- **Gameplay:** combat, Severant space (hostile at standing ≤ −40); Weave Lane anchor.

#### The Tether Point (`TetherPoint`) — canon node
- **Canon:** "A quiet gravity well between superclusters." Second Weave Lane anchor; "Canopy Probe" event.
- **Backdrop [NEW]: *the Tether*, a dark rogue body with a lensing halo.** Size 2.0, no star. Surface near-black
  `#050608` / `#0C0E12` / `#15181E`, almost invisible; the read comes from gravitational lensing of the starfield
  around it (screen-space distortion ring) and a faint thin accretion line `#9FB8FF`, opacity 0.4, ring
  1.6–1.65. Rim: none. Preset `MI_Planet_Tether`.
- **Gameplay:** quiet event node; the darkest scene in the game, used as a breather. Lyra's anchor work.

#### Cinder Archive (`CinderArchive`) — canon node
- **Canon:** "Precursor archive world of ash and glass." Council C2 hub; Kesh and Ithane hail here. The
  Pyre Gate is over its poles and the Vault of Records is its core.
- **Planet: *the Cinder Archive*.** Size 1.4. Surface `#2A2624` / `#4E4640` / `#8C8078` ash plains, cut by a
  global grid of glass-filled canyons `#9CC4D0` (Voronoi or grid mask, 15% of surface); the grid is the
  archive's storage lattice. Emissive: canyons glow faint blue-white `#BFE6FF` on the night side, 15%,
  pulsing slowly (data). Clouds: 35%, ash-grey `#7A716A`, banded at mid-latitudes, drift 0.3; the poles are
  clear. Rim `#C7B8A8` with a blue inner edge `#A8D4FF`, thickness 0.3, exponent 2. Rings: none. Moons: the
  Quiet Hollow (below). Preset `MI_Planet_CinderArchive`.
- **History [NEW]:** the Severants' record-world; they burned its surface to ash to hide its lattice, but the
  glass record survived beneath.
- **Gameplay:** hub; diplomacy; reached from GlassSea or TetherPoint.

#### The Quiet Hollow (`QuietHollow`) — canon node
- **Canon:** "Warden Kesh's refuge in a hollowed moon." Cure delivery; "The Ash Choir" (Lyra and Kesh).
- **Planet: *Hollow*, moon of the Cinder Archive [parent NEW].** Size 0.35. Surface pale grey regolith
  `#6E6A66` / `#9A958F` / `#C9C4BC`, heavily cratered; one great crater opens into the hollow interior,
  with warm light `#FFC27A` spilling out (the refuge). Emissive: the crater mouth, 2%, plus a faint
  harmonic shield dome `#9FC8FF` at opacity 0.15 (damaged: irregular gaps). No clouds, no rim. The Cinder
  Archive fills the background. Preset `MI_Planet_Hollow`. Station: §4.4.
- **Gameplay:** event node; opens only at Severant Standing ≥ 0.

#### The Pyre Gate (`PyreGate`) — canon node
- **Canon:** "Severant fortress over the Archive's poles." Combat E2 (Severant Bastion, three shield pylons).
  Set pieces SM_SeverantBastion, SM_BastionPylon. See §4.2.
- **Backdrop:** the Cinder Archive's north pole, close, filling the lower frame: the clear polar cap shows the
  glass lattice converging to a single bright point `#E6F4FF` (the Vault's shaft).
- **Gameplay:** combat, can be opened peacefully.

#### Tithe Anchorage Marker (`TitheMarker`) — canon node
- **Canon:** "Outer marker of Commander Vance's anchorage." Optional combat E3. Sets up Act VIII.
- **Backdrop [NEW]:** the marker buoy (a gunmetal spar with red strobe `#FF2A1A`, 1 Hz) and, far off, the gas giant
  *Grieve* (§3.4.1) with the Anchorage as a red point in its ring.
- **Gameplay:** optional combat; Act VIII's entry point.

#### The Vault of Records (`VaultOfRecords`) — canon node
- **Canon:** "The Archive core." Council C3 and finale E4; the second revelation. Set piece SM_VaultOfRecords
  (nested pale cubes, glowing core ring). See §4.3.
- **Backdrop:** inside the Cinder Archive's polar shaft: walls of glass lattice rising out of frame,
  blue-white `#BFE6FF`, the planet's core light below.

### 3.4 Home side (canon place, not yet a chart node)

#### Aetheria Prime — canon world
- **Canon:** Outpost One, the Spire, the Harmonic Stealth Canopy, the waterfall valley, Vault Alpha, the Dry Dock.
- **Planet [visuals NEW].** Size 1.0. Surface: ocean `#0B2E4A` / `#14507A`, land `#2F5A2E` / `#5C7A3A` / `#A89A6A`
  with high white ridges `#E8EEF0`; continents with fractal coasts. Emissive: almost none: the Canopy hides
  Outpost One. Show a single faint cyan point `#38E1FF` (the Spire) only in scenes where the canopy is
  down (`OutpostDark` false and no canopy). Clouds: 55% white `#F6F8FA`, swirl cyclones, drift 0.5. Rim
  `#6FB8FF`, thickness 0.3, exponent 2.5. Rings: none. Moons: 2 (one large pale, one small reddish). Preset
  `MI_Planet_AetheriaPrime`.
- **Gameplay:** seen in comms and cutscenes, and in Act X endings. It is what the Captain protects.

### 3.5 New systems for Acts VIII–X [NEW]

Proposed node IDs follow the existing naming. Leap costs are suggestions to be balanced with
`AetheriaStarChartToolset.validate_star_chart` like Act VII's chart.

| Node | Act | Encounter | Role | Suggested routes (base) |
|---|---|---|---|---|
| `TitheMarker` *(canon)* | VIII | Combat | Act VIII start | → Grieve 40, → Rustmarch 35 |
| `Grieve` | VIII | Council | Tithe Anchorage; Vance's base | → GraftForge 25, → Rustmarch 45 |
| `Rustmarch` | VIII | Event | Corsair haven; Cormaker's arc | → SerenVael 40 |
| `GraftForge` | VIII | Combat | Where Severant grafts are made | → Grieve 25, → SerenVael 50 |
| `SerenVael` | VIII–IX | Council | Severant moderate world (Kesh's people) | → Meridian 45 |
| `Meridian` | IX | Council | The Answerer shipyard; Phantom Fleet contest | → TheHush 40, → ForgeEcho 30 |
| `ForgeEcho` | IX | Event | Second Event Horizon Forge; Squadron III | → Meridian 30 |
| `TheHush` | IX–X | Event | Edge where the Listener's signal is strongest | → GateZero 55 |
| `GateZero` | X | Council | The sealed gate | → ArchWake (via Arch) |
| `ArchWake` *(canon)* | X | Council | The return; final choice staging | — |

#### 3.5.1 Grieve — Tithe Anchorage (`Grieve`)
- **Planet:** gas giant, size 8. Bands `#1E2226` / `#3A4046` / `#5E666C` / `#8C7A6A` (iron-grey with rust
  bands), sharp laminar banding, one large dark storm `#14171A`. Emissive: lightning in the storm
  `#FFE0C0`, 1%. Clouds: part of bands, drift 0.15. Rim `#9A8C80`, thickness 0.25, exponent 3. Rings: wide,
  1.6–2.8, `#6A6058` with a gap at 2.1, opacity 0.7; the Anchorage (§4.5) sits in the gap. Moons: 4, small,
  mined, one visibly cut in half. Preset `MI_Planet_Grieve`.
- **History:** a Tithe staging world from before the Arch opened (the Tithe kept a forward base on the far
  side as a listening post), now Vance's sovereign anchorage.
- **Gameplay:** Act VIII hub and finale; siege or infiltration.

#### 3.5.2 Rustmarch (`Rustmarch`)
- **Planet:** a shattered planetoid belt around a small orange star. Hero body *Rustmarch Major*, a
  fragment, size 0.6. Surface `#3A2416` / `#6E4128` / `#A86B3E` / `#D9A070` iron-rich rock, blocky crater
  noise. Emissive: mining lights `#FFB347`, 2%, clustered in pits. No clouds; rim none. Belt: dense asteroid
  field (instanced rocks, rust and tan). Preset `MI_Planet_RustmarchMajor`.
- **History:** unclaimed, ore-rich, previously untouched because the Tithe never came this far.
- **Gameplay:** event node; where the Corsairs can found a home (Cormaker arc §6.7). Station §4.6.

#### 3.5.3 The Graft Forge (`GraftForge`)
- **Planet: *Cauter*,** a tidally locked volcanic moon of Grieve, size 0.5. Day side `#1A0F0A` / `#3C1C10` /
  `#7A2E12` with molten flows `#FF5A1A`; night side black with lava seams. Emissive: lava 25% day side,
  10% night. Clouds: 20% sulphur haze `#C8A050`, drift 0.4. Rim `#FF8A40`, thickness 0.4, exponent 1.8.
  Rings none. Grieve looms behind. Preset `MI_Planet_Cauter`.
- **History:** Ithane's engineers and Vance's yards built the graft forge here after their alliance. Station §4.7.
- **Gameplay:** combat; destroying or seizing the forge decides how many Act VIII enemy ships carry grafts.

#### 3.5.4 Seren Vael (`SerenVael`)
- **Planet:** the last living Severant world. Size 1.1. Surface pale `#B8B4A8` / `#D6D2C6` / `#ECE8DE` salt flats
  and white stone, with shallow turquoise lakes `#4FB8B0`, cellular noise. Emissive: night-side settlements
  in ring patterns, Silence blue `#9FC8FF` (hardliner) or ochre `#D9A35B` (moderate), 2%; the ratio of the two
  colours reflects `SeverantStanding`. Clouds: 30% thin white high cirrus, drift 0.3. Rim `#D8E8F4`,
  thickness 0.2, exponent 3.5. Rings: one thin, artificial: a harmonic ring 1.25–1.27, `#9FC8FF`, opacity 0.5
  (it flickers when the Silence spikes). Moons: 1. Preset `MI_Planet_SerenVael`.
- **History:** home of the Choirs and the Severant council that can depose an Exarch (Act VII Peace of the Rite).
- **Gameplay:** Act VIII/IX council node; where Severant allies are raised (or refused).

#### 3.5.5 Meridian (`Meridian`)
- **Planet:** a ringed super-terrestrial world, size 1.7, covered by Answerer construction: an ecumenopolis of
  pale ceramic `#D9D4C8` / `#B0AA9E` / `#8A8478` in a city-grid noise, with crystal veins `#BFF3FF` running
  pole to pole. Emissive: crystal veins 8%, night side glows cyan-white in a lattice. Clouds: 15%, thin,
  drift 0.2. Rim `#BFEFFF`, thickness 0.25, exponent 3. Rings: a ring of half-built hulls, 1.5–2.4,
  `#C8D4DA`, opacity 0.5 (the ring *is* a shipyard). Moons: none. Preset `MI_Planet_Meridian`.
- **History:** the Answerers' shipyard world, where the Phantom Fleet was built. Long dead, but its yards still
  hold the root of the Automated Fleet Key.
- **Gameplay:** Act IX hub; the contest for the fleet; the threshold scene keyed by `ArchitectRevelation2`.

#### 3.5.6 The Forge Echo (`ForgeEcho`)
- **Backdrop:** a black hole with a narrow, bright accretion disc `#FFE6C0` → `#FF8A3A` (outer), lensing heavy.
  An Answerer stasis structure (a twin of the Event Horizon Forge on the home side) hangs at the photon ring.
- **History:** the Answerers built two Forges; this one was never opened. It holds ships, or what is left of them.
- **Gameplay:** event node; Squadron III's fate; Nix arc. The "more ships" question must not break canon
  counts: what the Forge Echo holds are **empty hulls and the Key's root record, not ships that join the fleet**.
  The fleet stays at 214 hulls minus losses.

#### 3.5.7 The Hush (`TheHush`)
- **Backdrop:** empty starless dark at the edge of the galaxy's disc, the galaxy itself a great spiral behind.
  The Listener's interference shimmers along the frame edge `#5A1030` → `#3A2A80`, as moiré over stars.
  No planet; in this place the absence is the point.
- **History:** where the Listener's signal is strongest. Severant records call it "the place the question comes from".
- **Gameplay:** event node; Act IX ending / Act X approach; Matrix effects (UI interference).

#### 3.5.8 Gate Zero (`GateZero`)
- **Canon:** the Severants sealed Gate Zero and split its keys among the Synod, Guild and Coalition.
  Its location is not fixed by canon; this bible places it at the far edge beyond the Hush.
- **Structure / backdrop:** a ring twice the Arch's diameter (6 km), dark ceramic `#3A3836`, its inner face
  blocked by three interlocking seal plates, each marked with a sigil (Synod, Guild, Coalition). The
  key-sockets in each plate glow faint when the Key-Bearer is near. Behind it: the Listener's shimmer,
  strongest here.
- **Gameplay:** Act X council and finale.

---

## 4. Space stations

Notes for the Blender hard-surface generator: dimensions are in metres (= uu at exterior scale). "Kit"
lists primitives and modifiers; "Greeble" is density 0–3. Each station lists faction palette (§2).

### 4.1 The Ember Relay — canon (SM_EmberRelay)
- **Owner:** Precursor (Answerer); dormant until the fleet arrives. **Size:** 900 m ring diameter.
- **Silhouette:** a thin flat ring (peach-gold ceramic `#FFD9B0`) held by three spokes to a central crystal spindle,
  with three small beacon spheres on the spoke tips outside the ring (canon mesh). HD rebuild: keep the
  read from 5 km out.
- **Kit:** torus (minor radius 3% of major), 3 tapered box spokes at 120°, central bipyramid crystal 180 m,
  3 spheres; bevel 2 segments; panel inset on ring top face; greeble 1 (Precursor = clean).
- **Purpose:** relays the Zero Beacon carrier; authenticates with the Arch's glyph; draws power off nearby hulls.
- **Hooks:** the supply line choice; the relay "learning what we are"; in Act X it can be used to send the
  answer back toward Gate Zero.

### 4.2 The Severant Bastion — canon (SM_SeverantBastion, SM_BastionPylon)
- **Owner:** Severants (Ithane). **Size:** 1.4 km core.
- **Silhouette:** a faceted ash-white geodesic sphere bristling with short spikes, a single glowing eye-port
  `#E6F4FF`; three separate hexagonal pylons, each with a ring emitter, float around it at 3 km.
- **Kit:** icosphere subdiv 2, faces extruded randomly ±5%, cone spikes on 30% of vertices, one recessed
  hex port; pylons = hex prism + torus; greeble 1, panel lines along geodesic edges.
- **Purpose:** guards the Pyre Gate and the polar approach to the Vault.
- **Hooks:** opened peacefully by Kesh's moderates (standing ≥ 30); a ruin, a garrison or an ally depending on Act VII.

### 4.3 The Vault of Records — canon (SM_VaultOfRecords)
- **Owner:** Severants (archive); the Answerers' record is inside. **Size:** 1.2 km cube.
- **Silhouette:** nested rotated cubes of clouded glass and pale ceramic, each smaller cube offset inside the
  last, a ring of light around the core.
- **Kit:** 4 nested cubes at 100/78/56/34%, each rotated 12° on Z; glass material on outer shells; torus at core;
  greeble 0 (it should feel built, not machined).
- **Purpose:** record of the Severance. **Hooks:** the second revelation; `ArchiveState` (Preserved/Broken)
  changes this asset: Broken = outer shells cracked and open, core ring dark.

### 4.4 The Quiet Hollow Refuge [NEW detail]
- **Owner:** Warden Kesh's moderates. **Size:** the docking mouth is 2 km across inside the moon's crater.
- **Silhouette:** seen from outside, a crater with a ring of ochre-collared docking spars `#D9A35B` reaching
  inward like petals; inside, hanging terraces of white ceramic and lamplight around a central harmonic
  shield emitter (a tall spindle). The refuge tender (SM_KeshRefugeTender) berths on the spars.
- **Kit:** cylinder cut into regolith (boolean), 12 radial spars (tapered boxes with collar torus at tips),
  terrace rings (tubes) inside; greeble 2 on spars only; warm interior point lights.
- **Purpose:** hospital and refuge for those sick with the Silence. **Hooks:** cure delivery; "The Ash Choir";
  Kesh's epilogue (Act X) depends on `CureDelivered` and whether the children aboard were saved.

### 4.5 Tithe Anchorage [NEW]
- **Owner:** the Iron Tithe (Vance's Remnant). **Size:** 3.5 km long.
- **Silhouette:** a long horizontal **spine** (a former toll-blockade platform) with heavy slab hangars hung
  under it like teeth, two vertical armoured towers at either end, a ring of gun turrets on the spine top,
  and docking booms projecting forward. Reads as a fortified bridge across the ring gap of Grieve.
- **Kit:** long box spine with chamfered edges, 8 underslung box hangars (each with an opening slot), 2 tall
  towers (stacked boxes stepping inward), 24 turret instances (cylinder + twin barrels), boom arms
  (boxes with truss); greeble 3; red warning strips on every hangar mouth; stencilled hull numbers.
- **Palette:** gunmetal, red strips, red-orange hangar light. Post-Act VII: a Severant graft spike on each tower.
- **Purpose:** Vance's shipyard, depot, prison and command. **Hooks:** the Act VIII finale; Tithe prisoners
  (Coalition crews taken in the frontier wars); Vance's fate; the Dreadnought-sized empty dock Vance has built for the ship
  he means to take.

### 4.6 Cutter's Rest (Rustmarch) [NEW]
- **Owner:** Corsairs (after Act VIII, if founded). **Size:** 600 m.
- **Silhouette:** a hollowed asteroid with a welded collar of salvaged ship hulls sticking out at angles,
  ore conveyors and a big refinery chimney, amber floodlights. Ships berth nose-in like spokes.
- **Kit:** displaced rock icosphere with boolean cavity, 10 salvaged hull pieces (boxes with random taper,
  different materials), conveyor trusses, chimney cylinder with emissive top; greeble 3; deliberately asymmetric.
- **Purpose:** the Corsairs' first home they own. **Hooks:** Cormaker's arc; what the Armada becomes after
  the war; Tithe raiders; Severant refugees asking for berths.

### 4.7 The Graft Forge [NEW]
- **Owner:** Ithane's hardliners and the Iron Tithe. **Size:** 1.8 km.
- **Silhouette:** a dark gunmetal foundry anchored to Cauter's surface by three huge cables, with a
  ceramic Severant crucible in its heart (an ash-white spike-ring cradle, glowing Silence blue) where
  grafts are grown. Two cultures bolted together: Tithe slabs outside, Severant ceramic inside.
- **Kit:** three tall truss pylons to the ground, box foundry decks, central torus ring with spikes,
  vertical glowing crucible cylinder; greeble 2 (Tithe parts) and 0 (Severant parts).
- **Purpose:** grows the Severant spikes fitted to Tithe ships. **Hooks:** sabotage (`VanceGraftsKnown`,
  `SeverantAgentsCaptured` give a weak point); seize it and turn the grafts into Silence shielding for the
  fleet; what the grafts do to Tithe crews.

### 4.8 The Answerer Listening Array [NEW]
- **Owner:** Answerer (Precursor), at the Hush. **Size:** 12 km across.
- **Silhouette:** a vast open dish made of thousands of thin crystal blades arranged in concentric rings,
  focal spindle at the centre, no hull at all. Pale ceramic struts, blades glowing faint violet where the
  Listener's signal hits them.
- **Kit:** instanced blade array (thin tapered boxes) on 9 concentric rings, focal spindle (long bipyramid),
  3 support arms; greeble 0; heavy LOD needs (Nanite).
- **Purpose:** the Answerers built it to hear the Listener clearly. **Hooks:** the Captain hears the question
  here (Act IX close); it can be used to answer, or destroyed to keep the fleet's answer to itself.

---

## 5. The flagship interior: the Crystal Dreadnought

Guide for the bridge set. Interior uses real-world scale (1 uu = 1 cm). Visual target: luminous, calm,
naval. Pale ceramic surfaces `#E4E8EA`, dark crystal floor `#141C22` with cyan inlay `#38E1FF`, warm accent only
from human things (a mug, a jacket, Cormaker's hologram).

### 5.1 Location in the hull

The bridge sits at the top of the **raised command tower** on the Dreadnought's stepped superstructure
(Docs/Art/dreadnought_hd_bridge.png): the flat-capped tower aft of midships. The viewport faces forward
over the stepped wedge and the prow. **[NEW]** The crystal sensor pyramid above the tower cap is the
Matrix's outer antenna.

### 5.2 Bridge layout (plan, prow at top)

```
                 ┌─────────────── FORWARD VIEWPORT (curved, 22 m wide, 6 m high) ───────────────┐
                 │                                                                              │
                 │   [NAV / VEX AI]           ─── command walk (raised 1.2 m) ───               │
                 │      helm pit (-1.5 m)                                                       │
                 │                                                                              │
   [LYRA'S       │   [TACTICAL / TALA]     [CAPTAIN'S CHAIR]      [NEURAL OPS / NIX]            │  [COMMS /
    LOOM ALCOVE] │                                                                              │   CORMAKER HOLO,
                 │                       ┌──── HOLOTABLE ────┐                                  │   ENVOY STATION]
                 │                       │   round, 4.5 m Ø   │                                  │
                 │                       └────────────────────┘                                  │
                 │   [ENGINEERING / JAX]                          [MEDICAL / THORNE]            │
                 │                                                                              │
                 └──────────── aft: lift doors, ready room (port), Matrix stair (stbd) ─────────┘
```

- **Overall:** 28 m wide × 24 m deep, 7 m to the ceiling at the viewport, 4.5 m aft. Two levels: a
  raised **command walk** across the forward half and the **holotable floor** behind it, with crew
  pits 1.5 m below the walk forward.
- **Forward viewport:** one continuous curved pane, crystal-framed, mullions every 3.6 m. The exterior
  scene (skybox, planet, fleet) fills it; this is the primary cinematic shot for arrivals.
- **Captain's chair:** on the command walk, centred, facing the viewport, swivels 360° to face the holotable.
  Armrest panels; no weapon controls (the Captain gives orders, officers execute).
- **Holotable:** round, 4.5 m diameter, dark crystal with a cyan projection field up to 2.5 m. It shows the
  star chart (WBP_StarMap's in-world twin), the tactical view, and is where the **Council of Command** is held:
  officers stand around it, and the Core Architect manifests as its lattice above the table centre.
  The Captain's place at the table is at the prow side, facing aft, so the camera sees the crew past the
  Captain's shoulder.

### 5.3 Officer stations

| Officer | Station | Position | Set dressing |
|---|---|---|---|
| Captain | Command chair | Command walk, centre | Chair with high back, crystal inlays, two armrest screens; ready room door aft port |
| Tala Vane (First Officer) | Tactical | Captain's left, walk level | Standing console, fleet status wall, a Coalition unit badge clipped to it [NEW] |
| Vex AI (Navigation) | Nav / helm | Forward pit, centre | No chair: a vertical light column `#38E1FF` that brightens when Vex speaks; two helm ratings at the pit consoles [NEW] |
| Nix "Null" (Neural Ops) | Neural Ops cradle | Captain's right | Reclined chair inside a cage of crystal rods (the Matrix interface); cables to the deck; Nix's own clutter: modded handheld, stickers [NEW] |
| Jax Mercer (Chief Engineer) | Engineering repeater | Aft port, near the lift | Drive and Chrono-Core readouts, an Instability gauge; Jax is often below decks and appears here by intercom |
| Dr. Aris Thorne (CMO) | Medical liaison | Aft starboard | Small console tracking crew neural lattices and injuries; Thorne "leaves the bridge" by the starboard lift |
| Lyra the Weaver | The Loom alcove | Port wall, open recess | A curved alcove with taut threads of light strung across a frame (she charts lanes by weaving); the only soft fabric on the bridge |
| Cmdr. Cormaker | Comms / Armada link | Starboard wall | Full-size hologram pad; Cormaker commands from the Corsair ships and appears in amber hologram (`#FFB347` tint) |
| Warden Kesh (Act VIII+) | Envoy station | Starboard wall, beside Cormaker's pad | Hologram or, from Act IX, in person: a Severant-style standing frame with an ochre collar-ring [NEW] |
| Core Architect | — | Above the holotable / the Matrix | Lattice manifestation (§1.5); the bridge lights dim by 10% when it speaks [NEW] |

### 5.4 Key decks [NEW]

Ordered top to bottom of the command tower and hull. Each is a candidate set or level.

1. **Bridge** (tower top): above.
2. **Captain's Ready Room** (bridge level, aft port): a small office with a window aft over the hull.
   Private conversations and loyalty scenes.
3. **The Matrix Core** (tower, directly below the bridge by a starboard spiral stair): a tall shaft of crystal
   lattice where the Neural Command Matrix lives. Nix's workshop is on a catwalk ring. The Architect's fraction
   lives here; Act VIII–IX scenes about the Key and Squadron III play here.
4. **Council Gallery / Wardroom** (tower, mid): long table, windows over the fleet; for crew dinners and
   personal scenes, not formal councils (councils stay at the holotable).
5. **The Prismatic Gallery** (hull spine, forward): the beam's emitter chamber, a 300 m crystal trench lined
   with refractors that split white into red/green/blue before recombining. Off-limits during charge.
6. **Drive Deck** (hull, aft): the Precursor Star-Drive (a spherical containment of crystal plates around a
   white core) and the Chrono-Core (a slowly rotating ring). Jax's domain. Instability shows physically here:
   plates rattle and glow amber, then red, as Instability rises.
7. **Medbay** (hull, mid): Thorne's bay, Bio-Gel tanks (pale green `#BFF5D0`), the lab used for the cure.
   Injured crew are seen here.
8. **Hangar and Corsair Dock** (hull, ventral): bays for Corsair gunships and Boarding Pods; Cormaker's people
   visit here; Kesh's tender docks here when in person.
9. **Crew Quarters and Mess** (hull, mid-dorsal): background life; the fleet's 1,000-odd crew [NEW: crew
   size is not canon; treat as "about a thousand" in dialogue if needed].

---

## 6. Crew

Loyalty uses the slice's `CrewLoyalty` (−100 … 100; starts Tala/Jax/Thorne/Nix/Lyra 50, Cormaker 40;
background bonuses +15/+15/+20). Canon thresholds in use: **≥ 60** gates options (Nix's squadron bonus,
Act VII C3 weave option). **[NEW]** proposed thresholds for Acts VIII–X:

| Band | Value | Effect |
|---|---|---|
| Devoted | ≥ 75 | The officer's personal resolution option appears in their arc's key choice; they may argue for the Captain in a crisis. |
| Trusting | 60–74 | Canon gating (≥ 60) applies. |
| Steady | 20–59 | Default. |
| Strained | 0–19 | The officer's counsel shows reluctance; one Act-level ability is withheld. |
| Estranged | < 0 | The officer may leave at the end of the act (a choice scene; the player can try to keep them, never auto-resolved). |

Vex AI and the Core Architect have no loyalty. Vex's arc tracks `ArchitectExposure` and the Captain's
decisions about the Key; the Architect's tracks `ArchitectLeaning`. Kesh tracks `SeverantStanding`.

### 6.1 Tala Vane — First Officer
- **Canon:** First Officer; Coalition; served with a Coalition Vanguard Captain against Sector 7; the voice of
  caution about the Architect ("We do not hand the keys…"); Act VII: caution in all councils, supports
  sending the Wayward home.
- **Arc [NEW]: *The Line Holds*.** Tala fought the Iron Tithe on the Coalition frontier and lost people to it.
  Act VIII brings her face to face with Vance, and with Coalition prisoners in Tithe Anchorage, among them
  someone from her old unit. Her question: is she the Captain's conscience or the Captain's weapon?
  - Act VIII: wants Vance captured and tried; will argue hard against using Tithe crews.
  - Act IX: if the Architect leans Dominion, Tala proposes cutting it out of the Key; the player decides.
  - Act X: Devoted → Tala volunteers to carry a Gate Zero key plate personally, standing for the Coalition.
- **Loyalty hooks:** gains from caution, accountability, refusing the Architect; loses from trusting the Architect
  with control or from mercy to Vance without terms.

### 6.2 Jax Mercer — Chief Engineer
- **Canon:** Chief Engineer; Guild; trusts a Guild Engineer Captain; pragmatist ("The Conduit works today");
  Joint Lane partner.
- **Arc [NEW]: *What It's Made Of*.** Jax takes everything apart, including the Precursor Star-Drive, and
  finds Answerer work beneath the drive's Guild-era repairs. At Meridian, Jax can learn to build Phantom
  parts and refit, at a cost: the knowledge comes from the Architect's archives.
  - Act VIII: the Graft Forge: Jax wants to strip grafts for Silence shielding on the fleet.
  - Act IX: Meridian's yards: Jax can resynchronise desynchronised Phantom ships with Answerer parts. No new
    hulls: only repairs to ships the fleet already has.
  - Act X: the Guild key plate at Gate Zero; Devoted → Jax can open it without the Architect.
- **Hooks:** gains from practical choices, salvage, engineering trust; loses from waste and from abandoning hull repairs.

### 6.3 Dr. Aris Thorne — Chief Medical Officer
- **Canon:** CMO; Matrix links pass through crew neural lattices; leads the Silence cure; leaves the bridge if
  the cure is withheld.
- **Arc [NEW]: *First, Do No Harm*.** Having found a cure, Thorne learns the Silence can be reversed into a
  weapon: the same synthesis, inverted, would kill every Severant in a system. Ithane already knows. Thorne must
  decide whether to destroy the research or hand it to the Captain.
  - Act VIII: the grafts are harming Tithe crews; Thorne wants to treat enemy wounded.
  - Act IX: the inversion is discovered; Thorne brings it to the Captain, asking for a decision.
  - Act X: Kesh epilogue; Devoted → Thorne stays with the Severants after the war.
- **Hooks:** gains from mercy and the cure; loses from withholding care or from weaponising medicine.

### 6.4 Nix "Null" — Neural Ops
- **Canon:** Neural Ops; built the sandbox; finds the Listener's packets; Squadron III's sync channel; firewall
  partner in C3; Phantom response bonus at loyalty ≥ 60.
- **Arc [NEW]: *Null and Void*.** Nix earned the name "Null" for being the one hacker no system could detect.
  The Listener detects Nix immediately and starts talking back. Nix's arc is the fight for Squadron III: reclaim
  it, release it, or leave it to whatever it now answers.
  - Act VIII: cleaning Tithe grafts out of captured Phantom-adjacent systems.
  - Act IX: the Forge Echo and Squadron III; Nix can rebuild the Key's root.
  - Act X: Nix becomes the interpreter of the Listener's signal, with risk to Nix's own lattice (Thorne objects).
- **Hooks:** gains from sandboxing, tracing, trusting Nix's methods; loses from purging, from handing control to
  the Architect.

### 6.5 Vex AI — Navigation
- **Canon:** Navigation AI; formal; never decides ("This is a matter of command"); investigates the beam anomaly;
  holds the Severance record if the Architect is denied it.
- **Arc [NEW]: *Awaiting Orders*.** Vex holds the record and begins to understand what the Architect is: another
  mind built to serve and to carry. Vex must decide whether to *have* a view. Vex still never decides for the
  Captain, but in Act X Vex asks the Captain one question of its own, and the answer changes Vex's final line.
- **Hooks:** no loyalty value. Branches on `ArchitectExposure` (high: Vex grows wary of being overwritten) and on
  whether Vex was given the record.

### 6.6 Lyra the Weaver
- **Canon:** charts lanes (Chart Route, Lyra +2 per chart); Weave Lane; rescued from an asteroid belt as a child;
  Severant heritage (`LyraHeritageRevealed`); "Whatever you tell it, it will become."
- **Arc [NEW]: *The Pattern*.** Lyra is a Severant child left in the belt during the Severance's long aftermath:
  a weaver from the Choirs that wanted to answer. At Seren Vael she can find her Choir's record. In Act X Lyra can
  weave the fleet's answer to the Listener.
  - Act VIII: Seren Vael; the Choirs want her back.
  - Act IX: the Listener's harmonic sings to her more than to anyone; she can weave a lane through the Hush.
  - Act X: Devoted → Lyra weaves the answer (§7.3 option); Estranged → she returns to the Severants.
- **Hooks:** gains from charting, weaving, the Rite, openness to the Architect and the Severants; loses from
  severing and silencing.

### 6.7 Cmdr. Cormaker — Corsair Armada
- **Canon:** commands the 12 Corsair ships and 450 outcasts; starts at loyalty 40 (+20 for Outcast Corsair);
  salvage rights; champions the cure ("My people know what it is to be left to die").
- **Arc [NEW]: *Fair Shares*.** Cormaker wants a home. Rustmarch is the chance, but taking it means leaving the
  fleet's protection. Vance offers the Corsairs a deal: amnesty and Rustmarch in exchange for the Dreadnought's
  location.
  - Act VIII: Vance's offer; Cormaker brings it to the Captain (it is not a betrayal if the Captain is told).
  - Act IX: founding Cutter's Rest, or staying with the fleet.
  - Act X: Devoted → the Corsairs fight at Gate Zero by choice; Estranged → they leave for Rustmarch before it.
- **Hooks:** gains from fair salvage, keeping promises, respect for outcasts; loses from taking the outcasts for
  granted.

### 6.8 Warden Kesh, "the Quiet Daughter"
- **Canon:** Severant moderate; leads the Quiet Hollow refuge; "a cure first, the truth second, and vengeance never";
  Act X epilogue depends on the cure; Kesh's skiffs join battles at high standing; `WardenKesh` speaker proposed.
- **Arc [NEW]: *The Last Warden*.** If the cure was delivered, Kesh joins the fleet's councils as envoy from Act VIII
  (on the bridge's envoy station). Her conflict: she owes the Captain her people's lives, but her people still owe
  the Severance its vow. She must decide whether the Severants answer the Listener or keep the silence, and so must
  the Captain.
- **Hooks (via `SeverantStanding`):** rises with mercy and the cure, Archive preserved, Ithane deposed rather than
  killed; falls with force, the Archive broken, or weaponising the cure.

### 6.9 The Core Architect (not crew, but on the bridge)
- **Canon:** two directives; a fraction in the Matrix; leaning-based branches.
- **Arc [NEW]: *Door or Wall*.** Act IX gives the Architect a choice of its own at Meridian: rejoin its full self
  (the original, still at the Arch) or remain the fraction it has become. It asks the Key-Bearer. The answer and
  `ArchitectLeaning` decide whether it is a door (Custodian), a warden (Dominion) or still undecided in Act X.

---

## 7. Storyline seeds: Acts VIII–X [NEW]

All three acts follow the Act VII model: a chart of 6–10 nodes, Instability as a resource, the supply line
carried forward, 2–4 battles, 3 councils, side missions per officer. Each act ends on a council: the
situation (Vex AI or the Architect), counsel from officers, options with approval hints. **Leaning deltas
are never shown.** The Captain decides. Values below are first-pass.

### 7.1 Act VIII — *The Iron Tithe*

**Act question:** *What do we do with the people who tried to keep the door shut?*

**Opening state.** Vance withdrew toward Tithe Anchorage (canon). `ExarchStatus` Alive → Ithane is with him;
Dead → Ithane's loyalists serve Vance leaderless; Deposed → Ithane is a prisoner of Seren Vael and Vance
fights alone. `VanceGraftsKnown` and `SeverantAgentsCaptured` give intel on the grafts.

**Beats.**
1. **The Marker** (TitheMarker): Vance broadcasts terms: surrender the Dreadnought, and every other ship goes free.
   Vex AI: "Commander Vance has transmitted terms, Captain. I will read them in full. I do not recommend a reply."
2. **Cormaker's deal** (Rustmarch): Vance has offered the Corsairs Rustmarch and amnesty. Cormaker brings the offer.
3. **Seren Vael**: raise Severant allies. Standing decides who answers. Lyra's Choir. Kesh as envoy (if cured).
4. **The Graft Forge** (combat): sabotage it, destroy it, or seize it for Jax's Silence shielding.
5. **Grieve, the Anchorage** (finale): siege with the fleet or infiltration with Boarding Pods. Tala finds
   Coalition prisoners. Vance's flagship *Writ of Sector 7* engages the Dreadnought directly. At 25% hull,
   Vance's ship is disabled and Vance is taken.

**Narration (Mission Log).** "Fleet Log. Tithe Anchorage is in sight. The Remnant holds forty hulls and an empty
dock cut to the Dreadnought's dimensions. Commander Vance has prepared a berth for this ship. Report ends."

**Ending council: "The Tithe Paid" (Grieve).**

Situation (Vex AI): "Commander Vance is aboard, under guard. Six hundred Remnant crew have surrendered.
The Anchorage is intact. Commander Vance requests to speak to the Captain before sentence. He states he
has information concerning the Listener. I note I cannot verify the claim."

Counsel:
- Tala Vane: "He held Sector 7 with conscripts and called it a duty. Try him, Captain. Under Coalition law, if
  you want a law."
- Cmdr. Cormaker: "His collectors took a third of every ore haul my people ever cut. You want to know what he's owed?
  Ask a miner."
- Dr. Aris Thorne: "The grafted crews are sick. Whatever you decide about him, his people need treatment."
- Jax Mercer: "That anchorage is the best dock this side of the Arch. Whatever we do, don't wreck it."
- Warden Kesh (if present): "My people sold him the grafts. We share this, Captain."
- Vex AI: "The decision is yours, Captain."

| Option | Hint | Leaning | Loyalty | Effect |
|---|---|---|---|---|
| **Try Vance before the fleet.** | Tala will approve. | 0 | Tala +10, Cormaker +5 | `VanceTried`; the Remnant crews are interned at the Anchorage. |
| **Take the Remnant into the fleet. Vance commands nothing.** | Jax will approve. Tala and Cormaker will not. | +5 | Jax +5, Tala −10, Cormaker −10 | `RemnantAbsorbed`; the Anchorage becomes a fleet dock (hull refits without shipments). |
| **Hear him out first.** | Nix will approve. Tala will not. | 0 | Nix +5, Tala −5 | `VanceHeard`; Vance's intel: Iron Tithe logs of the Listener's signal at the Arch, older than the Tithe. Then a second choice: try or release. |
| **Hand him to the Severant council.** *(Standing ≥ 30)* | Kesh will approve. Thorne will not. | +5 | Thorne −5 | `VanceToSeverants`; Standing +15. |
| **Release him with his crews, unarmed.** | Thorne will approve. Tala and Cormaker will not. | +10 | Thorne +5, Tala −15, Cormaker −15 | `VanceReleased`; Vance returns in Act X as an unknown. |

Outcome example (Hear him out) — Vance: "You think I kept the door shut because I was told to. I kept it shut
because I heard what was on the other side, Key-Bearer. So did my father, and his. The Tithe was never a toll. It
was a watch."

**Flags for Act IX:** `VanceFate` (Tried / Absorbed / Severants / Released), `CorsairsSettled` (from Cormaker's
arc), `GraftForgeState` (Destroyed / Seized / Intact), `SeverantAllies` (count).

### 7.2 Act IX — *The Threshold*

**Act question:** *Whose fleet is it?*

**Opening state.** `ArchitectRevelation2` (Guardian / Warden / Unresolved) sets the threshold scene. `SupplyLine`
decides the Architect's leverage: ArchConduit under Dominion → the Architect can cut supplies. `ListenerTraced`
→ earlier contact. `SquadronIIIAlienated` → the three-way contest is live from the start.

**Beats.**
1. **Meridian**, the Answerer shipyard: the root of the Automated Fleet Key is here. The threshold scene: the
   Architect stands at the root and asks what it should be (its arc §6.9).
2. **The Forge Echo**: Squadron III (if alienated) has gone ahead; Nix follows. The Listener speaks through
   the squadron's sync channel: twenty ships repeat the question in a language Lyra can partly weave.
3. **Jax at the yards**: repair desynchronised Phantoms using Answerer parts, or leave the yards untouched.
4. **Thorne's inversion** of the cure (§6.3), brought to the Captain: destroy it or keep it.
5. **The Hush**: the Listening Array (§4.8). The Captain hears the Listener's question clearly for the first time.
   Lyra: "It isn't asking what the Precursors were, Captain. It's asking what *we* are."
6. **Battle:** whoever contests the Key: Ithane's last loyalists (if alive), the Architect's held squadrons
   (Dominion), or Squadron III under the Listener's sync. The battle is about holding the root, not killing.

**Ending council: "The Fleet Key" (Meridian).**

Situation (Core Architect): "The root of the Automated Fleet Key is before you, Key-Bearer. Today, every Phantom
vessel answers through me, and through you, and twenty answer something else. You may rewrite the root. I will
not resist whatever you choose. I would ask you to remember that I could."

Counsel:
- Tala Vane: "Cut it out of the Key. Whatever it says now, it held two hundred warships in a box it alone could open."
- Nix "Null": "I can write a new root. It'll be ours. It'll also be mine for about a week while I get it right."
- Lyra: "The ships were built by people who wanted to answer. Ask them, if they can be asked."
- Jax Mercer: "Whatever you write, write it so we can repair it without a Precursor."
- Warden Kesh: "My ancestors locked these ships to keep them from this choice. I won't tell you they were wrong."
- Vex AI: "I offer no recommendation. I note that one option places the Key with me. I would hold it for you, Captain.
  I would prefer not to be asked."

| Option | Hint | Leaning | Loyalty | Effect |
|---|---|---|---|---|
| **Re-root the Key in Nix's sandbox. The Architect is cut out.** | Tala and Nix will approve. | −5 | Tala +10, Nix +10, Lyra −5 | `KeyRootedCrew`; the Architect can no longer take ships; Squadron III can be reclaimed in a battle. |
| **Share the Key with the Architect, as partners.** | Lyra and Jax will approve. Tala will not. | +15 | Lyra +5, Jax +5, Tala −10 | `KeyShared`; Custodian path; supplies through the Conduit can't be cut. |
| **Give the Key to Vex.** | Vex will acknowledge. | 0 | — | `KeyWithVex`; Vex's arc deepens; the Architect: "A mind like mine. Perhaps that is fitting." |
| **Release Squadron III to answer the Listener.** *(SquadronIIIAlienated)* | Lyra will approve. Tala and Nix will not. | +5 | Lyra +5, Tala −10, Nix −5 | `SquadronIIIReleased`; `PhantomShipsActive` stays reduced by 20; the squadron becomes the fleet's envoy into the Hush. |
| **Hold the Key alone. The Architect answers to you.** | Tala will approve. Lyra will not. | −15 | Tala +5, Lyra −10 | `KeyHeldByCaptain`; Dominion path; the Architect obeys and stops volunteering help. |

Outcome example (Hold alone) — Core Architect: "Then I will wait for orders, as your Navigator does. I have
recorded the phrasing exactly." Vex AI: "Captain, for the record: I did not ask for company."

**Flags for Act X:** `FleetKey` (Crew / Shared / Vex / Captain), `SquadronIIIReleased`, `CureInversion`
(Destroyed / Kept), `ListenerQuestionHeard`.

### 7.3 Act X — *Gate Zero*

**Act question:** *What is the answer?*

**Opening state.** Everything converges: Architect leaning, Fleet Key, Severant standing and allies, Corsair
settlement, Vance's fate, Lyra's heritage, the Archive's state.

**Beats.**
1. **The Hush to Gate Zero.** The gate's three seal plates bear the sigils of the Synod, the Guild and the Coalition.
   The Key-Bearer can open it only with the three keys (canon: the keys were split among the three powers).
   **[NEW]** The Key-Bearer's key is the Synod's; the Guild and Coalition plates need Jax and Tala at ≥ 60, or the
   Architect's override (Custodian or `KeyShared`), or force (Prismatic Beam, high Instability cost).
2. **Ithane's last stand** (if alive): Ithane tries to destroy the gate rather than see it opened.
3. **The return** (ArchWake): Before the answer, the fleet can leap back to the Arch, where Outpost One's
   shipments and families can be seen through the ring. Optional; sets the stakes.
4. **The answer.** At Gate Zero the Listener asks its question directly, in the Matrix, to the Captain.

**Narration (Mission Log).** "Fleet Log, final watch before Gate Zero. Two hundred and fourteen vessels crossed the
Omega Arch. {X} report position. The question is received. The Captain will answer for the fleet. Report ends."

**Ending council: "The Answer" (GateZero).**

Situation (Vex AI): "Captain. The Listener's question is resolved. Translation confidence is high. It asks: 'What
are you?' It has asked every voice that answered the Beacon. The Precursors never agreed on a reply. The answer
is yours to give, for all of us."

Counsel:
- Tala Vane: "Tell it we're people who keep our word. Then make sure we do."
- Jax Mercer: "Tell it the truth. We're builders. We fix what breaks."
- Dr. Aris Thorne: "We caused the Silence and we cured it. That's what we are, Captain. Both."
- Nix "Null": "Don't answer for everyone. Let everyone answer. I can open the channel to the whole fleet."
- Lyra: "I can weave it. Every voice in the fleet, into one pattern. It will understand that."
- Cmdr. Cormaker: "Tell it nobody gets left behind. If it can't live with that, close the door."
- Warden Kesh: "My people kept silent for an age because we feared the answer. Don't let fear answer for you."
- Core Architect: line by leaning: Custodian: "I was built to carry the reply. Let me carry yours." · Dominion:
  "Say nothing, and the door stays mine to hold." · Undetermined: "I still do not know what I am. Perhaps the answer
  will tell me."
- Vex AI: "This is a matter of command. The decision is yours, Captain."

| Option | Hint | Requires | Effect / ending |
|---|---|---|---|
| **Answer for the fleet, as Key-Bearer.** | Tala will approve. | — | The Captain speaks the answer (player picks one of three lines). The gate opens; the Listener comes to meet the fleet. *Ending: Envoy.* |
| **Let everyone answer.** | Nix and Cormaker will approve. | Nix ≥ 60 | Every ship's crew speaks; the Listener receives thousands of answers. *Ending: Chorus.* |
| **Let Lyra weave the answer.** | Lyra will approve. | `LyraHeritageRevealed`, Lyra ≥ 75 | Severant weaving, Answerer purpose; the Severants and the fleet answer together. Standing +50. *Ending: Accord.* |
| **Let the Architect carry the answer.** | Lyra will approve. Tala will not. | Custodian or `KeyShared` | The Architect becomes "a door, and not a wall"; it goes through Gate Zero alone. *Ending: Door.* |
| **Keep the silence. Seal the gate again.** | Kesh will not. Tala will approve. | — | The fleet seals Gate Zero with the three keys and sets the Architect to guard it (Dominion: it agrees gladly). Sealing silences the Beacon: the Silence ends for the Severants, and the Listener's question goes unanswered. *Ending: Wall.* |

**Epilogues** (by flag): Kesh and the Severants (`CureDelivered`, `ArchiveState`, Standing); the Corsairs
(Cutter's Rest or the fleet); Vance (`VanceFate`); Outpost One (`OutpostDark`, `SupplyLine`); each officer by
loyalty band; Vex AI's final line (changed by the Captain's answer to Vex's one question).

---

## 8. Open questions and canon gaps

Things canon leaves open, with this bible's working answer. Confirm before content depends on them.

| Question | Canon status | Working answer here |
|---|---|---|
| Where is the Zero Beacon physically? | Not fixed | A carrier, not a place; the Ember Relay and Gate Zero both carry it. |
| Where is Gate Zero? | Not fixed | Beyond the Hush, at the galaxy's edge (§3.5.8). |
| Which key does the Captain bear? | "Key-Bearer" for all backgrounds | The Synod key; the other two plates are opened in Act X (§7.3). |
| Acts I–V episodes | Outcome only | Not authored here; §1.6 summary only. |
| Phantom class split | Not fixed | ~4 cruisers : 1 carrier per squadron (§1.6). |
| What the Listener is | Signal, a question | Never a ship; a presence in signal. |
| Iron Tithe origin | Sector 7, Coalition enemy, "Remnant" | Toll-and-quarantine authority of the Arch approach (§2.2). |
| Fleet crew size | Not fixed | "About a thousand" aboard the Dreadnought. |
| New Speaker values | `WardenKesh`, `ExarchIthane` proposed in Act VII | None beyond those; Vance already exists as `CommanderVance`. |

**Canon constraints to protect in every act:** 214 ships (1 + 1 + 200 + 12) at the start of the expedition; the
fleet never gains new hulls from Precursor sites; the Prismatic Stellar Beam, Chrono Surge, Crystalline Barrier
and Star-Drive work as specified; officers advise, Vex never decides, and every council ends with the Captain's
choice.
