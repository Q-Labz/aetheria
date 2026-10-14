# Aetheria: Beyond the Arch

Narrative fleet-command RPG with real-time-with-pause combat. UE5, Blueprint only.

## What's here
| Path | Contents |
|---|---|
| `Aetheria.uproject` | Project file; enables ModelContextProtocol, AllToolsets, EnhancedInput, Python, Niagara |
| `Config/` | Game Instance class, startup maps, Enhanced Input defaults, packaging map list |
| `Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini` | Auto-starts the Unreal MCP server with the editor |
| `.mcp.json` | Connects Claude Code to the editor's MCP server |
| `setup.bat`, `Tools/setup.ps1` | One-time Windows setup: choose the engine, update the project file, open the editor |
| `Source_Art/Meshes/*.fbx` | Placeholder fleet: Crystal Dreadnought (1,020 m), the Wayward, Phantom battle-cruiser and heavy carrier, Corsair salvage gunship and boarding pod, Iron Tithe cruiser and interceptor, and Act VII's Severant choir-skiff, Silence emitter frigate, *The Last Silence* and Kesh's refuge tender. Contact sheet: `Source_Art/fleet_sheet.png` |
| `Source_Art/Environments/*.fbx` | Act VI–VII set pieces: Omega Arch, Ember Relay, Lantern Drift cluster, Arch Wake debris, Pyre Gate bastion and pylon, Vault of Records, Glass Sea shards. Contact sheet: `Source_Art/environment_sheet.png` |
| `Tools/Blender/make_fleet.py` | Regenerates those meshes (`blender --background --python make_fleet.py -- <out_dir>`) |
| `Tools/Blender/make_environments.py` | Regenerates the set pieces (`blender --background --python make_environments.py -- <out_dir>`) |
| `Tools/Blender/aetheria_mesh.py` | Shared mesh helpers used by both generators |
| `Tools/Blender/render_fleet_sheet.py` | Renders a contact sheet of any asset folder (`blender --background --python render_fleet_sheet.py -- <assets_dir> <out.png>`) |
| `BuildData/*.csv` | Import into DT_StarNodes / DT_LeapRoutes |
| `BuildData/Narrative/*.json` | Dialogue and council content for DT_Sequences / DT_Council_Relay / DT_Council_Directive |
| `Plugins/AetheriaTools/` | Editor-only Python toolsets exposed through Unreal MCP: `AetheriaNarrativeToolset` (story tables, council-path previews), `AetheriaStarChartToolset` (chart validation, leap costs, route planning) and `AetheriaCombatToolset` (fleet roster, fleet canon, ship stats, combat-level layout) |
| `BuildData/ActVII/*.csv` | Act VII star chart (nodes with ash-storm hazards, leap routes) |
| `Docs/Story/ACT_VII_CINDER_ARCHIVE.md` | Story and systems spec for Act VII, the act after the slice |
| `SLICE_SPEC.md` | Full Blueprint spec for the vertical slice |
| `CLAUDE.md` | Build order Claude Code follows through Unreal MCP |

## Start building (Windows)
1. Clone the repo next to your other projects, e.g. `cd D:\` then `git clone https://github.com/Q-Labz/aetheria.git`.
2. Double-click `setup.bat`. It lists your installed engines, points `Aetheria.uproject` at the newest one that
   has the Unreal MCP plugin (`ModelContextProtocol`), and opens the editor, which starts the MCP server.
   Options: `setup.bat -Engine 5.6` picks a version; `setup.bat -NoLaunch` only updates the project file.
3. In the same folder, run `claude` and say: "Build the game following CLAUDE.md, starting at phase 1."

## Tests
Engine-independent narrative tests (no Unreal needed):
```bash
python -m unittest discover -s Plugins/AetheriaTools/Content/Python -p "test_*_core.py" -t Plugins/AetheriaTools/Content/Python
```
In-editor toolset tests: `Plugins/AetheriaTools/Content/Python/aetheria_tools/tests/test_*_toolset.py`
(run through `AutomationTestToolset` via Unreal MCP).
