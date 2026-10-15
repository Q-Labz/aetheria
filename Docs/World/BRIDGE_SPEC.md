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
| SM_Bridge_Props | — | Mug, jacket on a chair, Coalition badge, Nix's handheld (small warm accents) |

Materials (`/Game/Aetheria/Bridge/Materials`): M_BridgeCeramic (`#E4E8EA`, satin, LocalPosition panel noise
like M_ShipHull), M_BridgeCrystalFloor (`#141C22`, glossy, cyan inlay `#38E1FF` emissive lines),
M_ViewportGlass (thin translucent, slight cyan tint), M_HoloProjection (additive cyan; amber instance for Cormaker),
reuse M_Crystal for crystal trim. Check: SM_Bridge_Shell bounds ≈ 2,800 × 2,400 × 700 uu.

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
