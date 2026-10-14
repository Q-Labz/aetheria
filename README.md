# Aetheria: Beyond the Arch

Narrative fleet-command RPG with real-time-with-pause combat. UE5, Blueprint only.

## What's here
| Path | Contents |
|---|---|
| `Aetheria.uproject` | Project file; enables ModelContextProtocol, AllToolsets, EnhancedInput, Python, Niagara |
| `Config/` | Game Instance class, startup maps, Enhanced Input defaults, packaging map list |
| `Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini` | Auto-starts the Unreal MCP server with the editor |
| `.mcp.json` | Connects Claude Code to the editor's MCP server |
| `Source_Art/Meshes/*.fbx` | Placeholder fleet: Crystal Dreadnought (1,020 m), the Wayward, Phantom battle-cruiser and heavy carrier, Corsair salvage gunship and boarding pod, Iron Tithe cruiser and interceptor. Contact sheet: `Source_Art/fleet_sheet.png` |
| `Tools/Blender/make_fleet.py` | Regenerates those meshes (`blender --background --python make_fleet.py -- <out_dir>`) |
| `Tools/Blender/render_fleet_sheet.py` | Renders the contact sheet (`blender --background --python render_fleet_sheet.py -- <meshes_dir> <out.png>`) |
| `BuildData/*.csv` | Import into DT_StarNodes / DT_LeapRoutes |
| `BuildData/Narrative/*.json` | Dialogue and council content for DT_Sequences / DT_Council_Relay / DT_Council_Directive |
| `Plugins/AetheriaTools/` | Editor-only Python toolset (`AetheriaNarrativeToolset`) exposed through Unreal MCP |
| `Docs/Story/ACT_VII_CINDER_ARCHIVE.md` | Story and systems spec for Act VII, the act after the slice |
| `SLICE_SPEC.md` | Full Blueprint spec for the vertical slice |
| `CLAUDE.md` | Build order Claude Code follows through Unreal MCP |

## Start building
1. Set `EngineAssociation` in `Aetheria.uproject` to your engine version (it is set to 5.7; the
   ModelContextProtocol plugin must exist in your engine build).
2. Open `Aetheria.uproject` (choose to create the Content folder if asked). The MCP server starts automatically.
3. In this folder, run `claude` and say: "Build the game following CLAUDE.md, starting at phase 1."

## Tests
Engine-independent narrative tests (no Unreal needed):
```bash
python -m unittest discover -s Plugins/AetheriaTools/Content/Python -p "test_narrative_core.py" -t Plugins/AetheriaTools/Content/Python
```
In-editor toolset tests: `Plugins/AetheriaTools/Content/Python/aetheria_tools/tests/test_narrative_toolset.py`
(run through `AutomationTestToolset` via Unreal MCP).
