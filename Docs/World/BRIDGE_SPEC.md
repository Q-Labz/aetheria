# Bridge & Crew — build spec (expansion phase B)

The Crystal Dreadnought's bridge becomes the stage for the command layer. Dialogue, the star map and both
councils play over a Lumen-lit bridge set, with the camera framing whoever is speaking. Crew are MetaHumans
at their stations. Source of the layout, palette and stations: `WORLD_BIBLE.md` §5.2–5.3. Where this file
and the bible disagree, the bible is canon and this file is fixed to match.

Scale: 1 uu = 1 cm (real-world interior scale; the ships stay at their own scale).

## B1. Bridge kit (Blender → `Source_Art/Bridge/*.fbx` → `/Game/Aetheria/Bridge/Meshes`)

Generator: `Tools/Blender/make_bridge.py` (reuses `aetheria_hd.py` panelling), export 1 BU = 1 cm,
import at Uniform Scale 1.0, Nanite on, simple collision off (it's a set, not a walkable level).

| Mesh | Size (W×D×H cm) | Notes |
|---|---|---|
| SM_Bridge_Shell | 2800×2400×700 | Floor (two levels: command walk +120, crew pits −150 forward), aft wall with lift doors, side walls with the Loom alcove (port) and holo pads (stbd), ceiling stepping 700 → 450 aft |
| SM_Bridge_Viewport | 2200 wide, 600 high, curved | Crystal frame and mullions every 360; the glass is a separate material slot |
| SM_CaptainsChair | 90×90×160 | High back, crystal inlays, two armrest screens |
| SM_Holotable | Ø450×110 | Dark crystal drum, cyan rim, projection plate on top |
| SM_Console_Tactical | 180×70×120 | Standing console + fleet status wall panel (Tala) |
| SM_Console_Pit | 160×80×90 | Helm rating console ×2 in the forward pit |
| SM_VexColumn | Ø60×350 | Light column (emissive, brightens when Vex speaks) |
| SM_NeuralCradle | 160×120×200 | Reclined chair in a cage of crystal rods, cables to deck (Nix) |
| SM_Console_Engineering | 200×70×150 | Drive/Chrono readouts, Instability gauge (Jax) |
| SM_Console_Medical | 140×60×120 | Lattice/injury console (Thorne) |
| SM_LoomFrame | 260×80×260 | Curved frame strung with light threads (Lyra) |
| SM_HoloPad | Ø180×20 | Hologram pad ×2: Cormaker (amber), Kesh envoy (Act VIII+) |
| SM_Bridge_WallBank | 280×54×420 | Wall computer bank: six readouts, a wide schematic or radar display, a status-light grid, data drums, vents, conduits. Eight, between the side-wall pilasters |
| SM_Bridge_OverheadMonitor | 260×60×254 | Ceiling-hung cluster of six angled readouts (three aft, three forward). Two, over the command walk |
| SM_Console_PitBank | 320×135×143 | Long crew-pit console: three slanted readouts, a status strip, a riser of four small readouts, two ratings' stools. Two, along the pit walls |
| SM_Bridge_Props | — | Mug, jacket on a chair, Coalition badge, Nix's handheld (small warm accents) |

Materials (`/Game/Aetheria/Bridge/Materials`): M_BridgeCeramic (`#E4E8EA`, satin, LocalPosition panel noise
like M_ShipHull), M_BridgeCrystalFloor (`#141C22`, glossy, cyan inlay `#38E1FF` emissive lines),
M_ViewportGlass (thin translucent, slight cyan tint), M_HoloProjection (additive cyan; amber instance for Cormaker),
reuse M_Crystal for crystal trim. Check: SM_Bridge_Shell bounds ≈ 2,800 × 2,400 × 700 uu.

**Screens and deck (slots `M_Screen`, `M_Floor`).** Every display face is in the `M_Screen` slot with its own
UVs: channel 0 spans the face 0–1, channel 1 carries the display's content as (mode + variation) / 6 and
(palette + 0.5) / 4. `M_BridgeScreen` (one Custom HLSL node; MI_BridgeScreen Glow 7.5) draws six animated modes on a
dark CRT ground: scrolling data, radar sweep with contacts, bar graph, waveform, a wedge-hull schematic with a
tracking reticle, and a blinking status-light grid. Palettes are pale blue, amber, red and green. Each placed copy
of a kit piece shifts its timing and palette by its world position, so repeated banks never show the same readouts.
Floors are `M_Floor` (MI_BridgeFloor: near-black, metallic, roughness 0.14), with plating seams aft and grating
down the command walk and across the pit. The shell also gains ceiling ribs every 200, edge conduits, and a dado of
vent grilles along the side walls.

**Reimporting.** Rebuild with Blender, then `aetheria_tools.art_toolset.AetheriaArtToolset.reimport_static_meshes`
replaces the meshes in place, keeping references and stored import settings. Create a new kit mesh with
`StaticMeshTools.import_file` first: a first import through Interchange with legacy FBX options comes in 100 times
too large.

## B2. Bridge set (built into L_Command, outliner folder `Bridge/`)

- Kit placed per the bible plan, prow (viewport) toward +X, origin at (-3000, 0, 0) so the viewport looks
  at the Omega Arch already in L_Command. (Built in place rather than as a streamed L_Bridge: nothing extra to cook.)
- **Outside the viewport:** MI_SpaceSky dome, the nebula, and a slow drift of 6–10 Phantom Cruiser and Corsair
  meshes at distance (the fleet in formation). The view changes per node later (planet/station set pieces, phase C).
- **Lighting (Lumen, HW RT):** cool skylight through the viewport; ceiling light strips (rect lights, 5,600 K);
  cyan emissive inlays and holotable; warm practical at Lyra's alcove. PPV: exposure fixed (no auto-exposure
  pumping between shots), mild bloom, film grain off.
- **Shot cameras** (CineCameraActor, tagged): `Shot_Wide` (establishing, from aft over the holotable to the
  viewport), `Shot_Captain` (over the chair), `Shot_Council` (Captain's place at the holotable, facing aft, crew
  past the shoulder per §5.2), `Shot_Chart` (top-down onto the holotable), and one `Shot_<Speaker>` per officer
  station. `CommandCamera` tag moves to `Shot_Wide`.

## B3. Camera direction (C++, `Source/Aetheria`)

- New `AAetheriaBridgeDirector` actor in L_Command: maps `E_Speaker` → shot camera (editable map), plus
  named shots (Wide, Council, Chart). `ShowSpeaker(E_Speaker)` and `ShowShot(FName)` blend the
  PC_Command view target (0.6 s cubic). Narrator / CoreArchitect / CommanderVance use Wide (Architect also dims
  bridge lights 10 % and lights the holotable lattice, §5.3).
- `UAetheriaDialogue` raises `OnLineShown(Speaker)`; `AAetheriaCommandController` forwards it to the director.
  Star map → `Shot_Chart`; councils → `Shot_Council`; slice end → `Shot_Wide`.
- Vex speaking: the director brightens SM_VexColumn's emissive (dynamic material parameter).
- The UI keeps working with no director present (a level without one keeps the CommandCamera).
- Check: PIE run b plays with the camera cutting to each speaker; no UI change needed to finish the run.

## B4. Crew (MetaHumans, `/Game/Aetheria/Crew/<Name>`)

Built with the in-editor **MetaHuman Creator** (plugins MetaHumanCharacter + MetaHumanGenerator toolset).
Body shape, skin tone and eye colour can be set by tool; **face, hair, wardrobe and the final Assemble are done
in the Creator UI** (the user does or approves these; texture synthesis needs an Epic account sign-in).

Proposed looks (all **[NEW]**, not canon; approve or change before building):

| Character | Proposed look | Wardrobe | Pose / place |
|---|---|---|---|
| Captain (player) | Built from the captain's background; generic, seen mostly from behind | Long naval coat, high collar | Chair / holotable, back to camera |
| Tala Vane | 40s, lean, close-cropped dark hair, steady grey eyes, scar at the jaw | Coalition-grey uniform, unit badge | Standing at Tactical |
| Jax Mercer | 50s, broad, shaved head, short beard, brown eyes | Guild engineer coveralls, sleeves rolled | At Engineering repeater |
| Dr. Aris Thorne | 60s, slight, silver hair tied back, glasses | Pale medical coat over uniform | At Medical liaison |
| Nix "Null" | 20s, slight, asymmetric bleached undercut, dark eyes | Hoodie under an open uniform jacket, cables, stickers | Reclined in the Neural cradle |
| Lyra the Weaver | ageless-young, very pale, long dark braid, pale violet eyes (Severant heritage, subtle) | Layered soft fabric, the only non-uniform on the bridge | Standing at the Loom |
| Cmdr. Cormaker | 50s, weathered, salt-and-pepper beard, eyepatch-free scarred brow | Corsair long coat, salvaged armour plates | Amber hologram on the holo pad |
| Vex AI / Core Architect | no body | — | Light column / holotable lattice |

Animation: idle loops per station (MetaHuman animation library / retargeted UE5 mannequin idles); talking
characters get a subtle look-at toward the camera. Lip-sync is out of scope (no VO yet).

## B5. Order of work and checks

1. B1 kit → check bounds and a lit capture of the empty bridge.
2. B2 level + B3 director → check PIE run b with speaker framing (camera only, crew stand-ins = simple mannequins).
3. B4 crew, one at a time: create via tool → user finishes face/hair/wardrobe in Creator → Assemble → place →
   replace stand-in. Check: each officer appears at their station and is framed in their shot.
4. Package check: Windows Shipping still cooks with 0 errors (MetaHumans raise the build size substantially).

## B4 status and hand-off (MetaHuman Creator)

Created by tool in `/Game/Aetheria/Crew/<Name>/MH_<Name>`: TalaVane, JaxMercer, ArisThorne, NixNull, Lyra, Cormaker,
with body shape (masculine/feminine, fat, muscularity, height), skin tone and eye colour from the table above.
Lyra's eyes are the palest cool option; the toolset has no violet, so set the violet tint in the Creator.

Finish each in the Creator (double-click the asset):
1. **Face:** start from a preset close to the look, then blend or sculpt (age, jaw, nose); Tala's jaw scar and
   Cormaker's scarred brow are skin details.
2. **Hair, brows, beard:** per the table (Jax: shaved head and short beard; Thorne: silver, tied back; Nix: asymmetric
   bleached undercut; Lyra: long dark braid; Cormaker: salt-and-pepper beard; Tala: close-cropped dark).
3. **Wardrobe:** nearest available outfit; uniform colours come later with material overrides.
4. **Assemble:** "UE Optimized" quality, output in the same folder. Texture synthesis needs the Epic account sign-in.

After assembly, the stand-in mannequins in L_Command's `Bridge/Crew` folder are swapped for the assembled
Blueprints at the same transforms (same actor tags, so the shots still frame them).

### B4 done (scripted)
The Creator steps were scripted instead of done by hand: `aetheria_tools.crew_toolset.AetheriaCrewToolset`
(`finish_crew` → face coefficients from an Epic preset, grooms and DefaultGarment from the engine wardrobe;
`rig_crew` → face rig and texture sources from the MetaHuman service, needs the editor's Epic sign-in;
`build_crew` → UE Optimized/Medium assembly into `/Game/Aetheria/Crew/<Name>`, shared rig assets in `Crew/Common`).
Assemble one character per editor session: memory from an assembly is only released when the editor exits
(two in a row ran the editor out of commit memory).

| Crew | Preset face | Grooms |
|---|---|---|
| Tala Vane | Jelena | Hair_S_Pixie, Eyebrows_M_Natural |
| Jax Mercer | Kelvin | (shaved) Eyebrows_M_Thick, Beard_S_Full |
| Dr. Aris Thorne | Walter | Hair_S_LowPonytail, Eyebrows_M_Messy |
| Nix "Null" | Aoi | Hair_M_SideSweptFringe, Eyebrows_S_FlatThin |
| Lyra | Celeste | Hair_M_TwistedBraids, Eyebrows_M_Fine |
| Cmdr. Cormaker | Lorenzo | Hair_S_RecedeMessy, Eyebrows_L_Scraggly, Beard_L_Full |

The assembled `BP_MH_<Name>` replace the mannequins in L_Command (same tags, turned toward their shot camera);
officer shots are 35 mm head-and-shoulders. Open items: the engine ships only the plain DefaultGarment (white
shirt and shorts; its colour is internal to the material), so uniforms need outfit assets; hair melanin is
the default; the bodies play MM_Idle in single-node mode (verify it animates; otherwise retarget an idle);
Cormaker is no longer an amber hologram.

## C. Node scenery (slice nodes)
Each star node has its set pieces tagged `Node_<NodeId>`; an `AAetheriaNodeScenery` actor in L_Command and in
L_Combat_LanternDrift shows only the current node's (and switches when a leap lands, via OnCampaignStateChanged).
Bodies use `SM_PlanetSphere` (Tools/Blender/make_planet_sphere.py) with `M_Planet` (bands, night-side glow,
Fresnel rim) or `M_Star` (granulation, limb darkening); instances in /Game/Aetheria/Environments/Materials.

| Node | Through the bridge viewport | In combat |
|---|---|---|
| ArchWake | Omega Arch and debris, white dwarf Threnody behind the ring | (no battle) |
| LanternDrift | gas giant Lantern Mother low to starboard, four lantern clusters | Lantern Mother below the battle plane, the existing clusters |
| EmberRelay | Ember Relay silhouetted against the red giant Cinderwake | Cinderwake below and aft, the relay at the battlefield edge |

### C (Act VII nodes)
Same system for the Act VII chart. Ring bands (`SM_Ring_Wide/Mid/Thin`, double-sided, additive `MI_Ring_*`) and the
Tithe marker buoy come from `Tools/Blender/make_planet_sphere.py`.

| Node | Through the bridge viewport | In combat |
|---|---|---|
| AshfallVerge | the Verge with its ash ring, two dead convoy wrecks | the Verge below, wrecks at the edges |
| GlassSea | the Glass Sea with its shard ring and grey moon, the glass shard field | the Glass Sea below, shard field |
| TetherPoint | the near-black Tether with a thin accretion line and a faint lensing halo | the Tether below, ringed by its halo |
| CinderArchive | the Cinder Archive, the Hollow as a small moon | the Archive below |
| QuietHollow | the Hollow up close under its damaged shield dome, the Archive behind | the Hollow below under its dome, the Archive far |
| PyreGate | the Archive's north pole and the Vault shaft's light, the Bastion and pylons | the pole below (the Bastion is a combat unit) |
| TitheMarker | the marker buoy, Grieve with its ring and the Anchorage light | Grieve below, the buoy |
| VaultOfRecords | the Vault of Records over the polar shaft, glass lattice walls lining the shaft | the pole below, the Vault and lattice walls at the edges |

**Close-range detail (`M_Planet`, group *Close Detail*).** At the Pyre Gate and the Vault of Records the Archive's
pole is a planet 100,000 uu across, about 12,000 uu below the combat camera. M_Planet's own noise is too coarse at
that range, so a second, ridged noise (`DetailScale` 60 on the unit sphere) shades the surface and roughens the
cloud edges (`DetailEdge` 1.5). It fades by camera distance, from full at `DetailFadeStart` 20,000 uu to none at
`DetailFadeEnd` 35,000 uu. Past that a planet renders as before; the Glass Sea in combat (no closer than about
37,000 uu) is unchanged. Set `DetailStrength` to 0 on an instance to turn the layer off.

## D. Depth of field and lighting

**Bridge (L_Command, level data).**
- *Focus.* The officer close-ups (`Shot_TalaVane`, `Shot_NixNull`, `Shot_JaxMercer`, `Shot_ArisThorne`,
  `Shot_Lyra`, `Shot_Cormaker`) track-focus their officer's head (`Crew_<Name>`, +162 uu) at f/2. The other
  shots use manual focus:

  | Shot | Focused on | Aperture |
  |---|---|---|
  | Captain | the captain's chair | f/2 |
  | VexAI | Vex's column | f/2 |
  | CoreArchitect | the holotable | f/2.8 |
  | Council, Chart | the holotable | f/4 |
  | Wide | the holotable | f/5.6 |

  All shots use smooth focus changes.
- *Lights.* The mood is a dark warship bridge in the Star Wars manner. Gunmetal panels and a black deck are lit
  mostly by the screens. Low overhead pools, the light through the viewport, and each officer's console glow
  on their face do the rest:

  | Light | Setting |
  |---|---|
  | Ceiling | 6 at 6,200 K |
  | Coves | 7 at 6,800 K |
  | Holotable glow | pale blue `(150, 195, 255)`, 14 |
  | Lyra's loom | amber `(1.0, 0.60, 0.30)`, 10 |
  | Vex glow | pale blue `(165, 205, 255)`, 7 |
  | `Bridge_FaceLight_<Officer>` | one soft rect light per officer, low and in front between the officer and their shot camera, shadowless, 4, tinted by their station (Tala and Thorne blue, Nix green, Jax, Lyra and Cormaker amber) |
  | `Bridge_PitSpill_P/S` | cool screen spill in the crew pit, 5 |
  | `Bridge_AlertRed` | faint red over the pit, 2 |

  Bridge materials: MI_BridgeCeramic is dark gunmetal `(0.11, 0.115, 0.12)`, metallic 0.35; MI_BridgeDarkTrim is
  near-black `(0.012, 0.013, 0.014)`, metallic 0.7; MI_BridgeLightStrip is cool white, Glow 2.2; MI_BridgeInlay
  floor lines are dim, Glow 0.6. The holotable rim and M_Crystal trim keep their cyan.

  The lights carry volumetric scattering.
- *Haze and grading.* `Bridge_Haze` is volumetric fog limited to the bridge (fog cut off at 3,000 uu), so the view
  outside stays clear; its density is 0.006, so the screens glow faintly in the air. The post-process volume adds
  contrast 1.08, saturation 1.06, film grain 0.06 and exposure bias +0.6 against the fixed exposure (for the dark
  palette).

**Combat (`AAetheriaCommandPawn`).** The camera's focus follows the battle plane at every zoom. It uses f/4 with a
1,200 mm virtual sensor: a long-lens, miniature look. The ships stay sharp while the planets and starfield below
soften. Tune or disable it with `FocusFstop`, `FocusSensorWidth` and `bFocusBattlePlane`.

**Node light (`AAetheriaNodeScenery`, both maps).** Lights tagged `NodeKeyLight` take the colour of the current
node's star (`NodeLightColors`). That covers the sun through the bridge viewport and the combat key light.

| Node | Light |
|---|---|
| ArchWake | Threnody's blue-white |
| LanternDrift | Lantern Mother's teal |
| EmberRelay | Cinderwake's ember |
| AshfallVerge | ash amber |
| GlassSea | glass glare, cool white |
| TetherPoint | lensed violet |
| CinderArchive | pale ash-gold |
| QuietHollow | warm lamplight |
| PyreGate | the shaft's white |
| TitheMarker | the Anchorage's red |
| VaultOfRecords | pale blue |

The colour switches when a leap lands; nodes without an entry keep the light as placed.
