"""Registers the Aetheria toolsets with the MCP ToolsetRegistry at editor startup."""
import unreal

if hasattr(unreal, "ToolsetRegistry"):
    from aetheria_tools.narrative_toolset import AetheriaNarrativeToolset

    unreal.ToolsetRegistry.register_toolset_class(AetheriaNarrativeToolset)
else:
    unreal.log_warning("AetheriaTools: ToolsetRegistry is not loaded; toolsets were not registered.")
