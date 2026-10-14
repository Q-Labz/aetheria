"""Registers the Aetheria toolsets with the MCP ToolsetRegistry at editor startup."""
import unreal

if hasattr(unreal, "ToolsetRegistry"):
    from aetheria_tools.narrative_toolset import AetheriaNarrativeToolset
    from aetheria_tools.starchart_toolset import AetheriaStarChartToolset

    unreal.ToolsetRegistry.register_toolset_class(AetheriaNarrativeToolset)
    unreal.ToolsetRegistry.register_toolset_class(AetheriaStarChartToolset)
else:
    unreal.log_warning("AetheriaTools: ToolsetRegistry is not loaded; toolsets were not registered.")
