# Aetheria: Beyond the Arch

Narrative fleet-command RPG with real-time-with-pause combat. UE5, Blueprint only.

## What's here
| Path | Contents |
|---|---|
| `Aetheria.uproject` | Project file; enables ModelContextProtocol, AllToolsets, EnhancedInput, Python, Niagara |
| `Config/` | Game Instance class, startup maps, Enhanced Input defaults, packaging map list |
| `Saved/Config/WindowsEditor/EditorPerProjectUserSettings.ini` | Auto-starts the Unreal MCP server with the editor |
| `.mcp.json` | Connects Claude Code to the editor's MCP server |
| `Source_Art/Meshes/*.fbx` | Placeholder fleet: Crystal Dreadnought (1,020 m), Phantom cruiser, Tithe interceptor, Tithe cruiser |
| `Tools/Blender/make_fleet.py` | Regenerates those meshes (`blender --background --python make_fleet.py -- <out_dir>`) |
| `BuildData/*.csv` | Import into DT_StarNodes / DT_LeapRoutes |
| `SLICE_SPEC.md` | Full Blueprint spec for the vertical slice |
| `CLAUDE.md` | Build order Claude Code follows through Unreal MCP |

## Start building
1. Set `EngineAssociation` in `Aetheria.uproject` to your engine version (it is set to 5.7; the
   ModelContextProtocol plugin must exist in your engine build).
2. Open `Aetheria.uproject` (choose to create the Content folder if asked). The MCP server starts automatically.
3. In this folder, run `claude` and say: "Build the game following CLAUDE.md, starting at phase 1."
