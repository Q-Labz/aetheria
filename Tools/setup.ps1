<#
.SYNOPSIS
  One-time local setup for Aetheria: picks an installed Unreal Engine, points the project at it,
  and opens the editor (which starts the Unreal MCP server for Claude Code).

.PARAMETER Engine
  Engine to use, as a version ("5.6") or a source-build association ("{GUID}"). Default: the newest
  installed engine that includes the ModelContextProtocol plugin.

.PARAMETER NoLaunch
  Update the project file but don't open the editor.

.PARAMETER LauncherInstalledPath
  Epic Launcher manifest to read. Defaults to the standard ProgramData location.

.PARAMETER ProjectFile
  Project to update. Defaults to Aetheria.uproject next to this script's parent folder.
#>
param(
    [string]$Engine = "",
    [switch]$NoLaunch,
    [string]$LauncherInstalledPath = "",
    [string]$ProjectFile = ""
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
if (-not $ProjectFile) { $ProjectFile = Join-Path $Root "Aetheria.uproject" }
if (-not $LauncherInstalledPath -and $env:ProgramData) {
    $LauncherInstalledPath = [IO.Path]::Combine($env:ProgramData, "Epic", "UnrealEngineLauncher", "LauncherInstalled.dat")
}

function Get-EngineVersion([string]$Path) {
    $buildFile = [IO.Path]::Combine($Path, "Engine", "Build", "Build.version")
    if (-not (Test-Path $buildFile)) { return $null }
    $b = Get-Content $buildFile -Raw | ConvertFrom-Json
    return [version]"$($b.MajorVersion).$($b.MinorVersion)"
}

function New-EngineRecord([string]$Association, [string]$Path) {
    if (-not (Test-Path ([IO.Path]::Combine($Path, "Engine")))) { return $null }
    $version = Get-EngineVersion $Path
    if (-not $version -and $Association -match '^\d+\.\d+$') { $version = [version]$Association }
    $mcp = Get-ChildItem -Path ([IO.Path]::Combine($Path, "Engine", "Plugins")) -Recurse -Filter "ModelContextProtocol.uplugin" `
        -ErrorAction SilentlyContinue | Select-Object -First 1
    [pscustomobject]@{ Association = $Association; Version = $version; Path = $Path; HasMcp = [bool]$mcp }
}

function Find-Engines {
    $found = @{}
    # Epic Launcher installs.
    if ($LauncherInstalledPath -and (Test-Path $LauncherInstalledPath)) {
        $list = (Get-Content $LauncherInstalledPath -Raw | ConvertFrom-Json).InstallationList
        foreach ($item in $list) {
            if ($item.AppName -match '^UE_(\d+\.\d+)$') {
                $rec = New-EngineRecord $Matches[1] $item.InstallLocation
                if ($rec) { $found[$rec.Path] = $rec }
            }
        }
    }
    if ($IsWindows -or $env:OS -eq "Windows_NT") {
        # Launcher installs recorded in the registry.
        foreach ($key in Get-ChildItem "HKLM:\SOFTWARE\EpicGames\Unreal Engine" -ErrorAction SilentlyContinue) {
            $dir = (Get-ItemProperty $key.PSPath -ErrorAction SilentlyContinue).InstalledDirectory
            if ($dir -and -not $found.ContainsKey($dir)) {
                $rec = New-EngineRecord $key.PSChildName $dir
                if ($rec) { $found[$rec.Path] = $rec }
            }
        }
        # Source builds registered by UnrealVersionSelector.
        $builds = Get-ItemProperty "HKCU:\SOFTWARE\Epic Games\Unreal Engine\Builds" -ErrorAction SilentlyContinue
        if ($builds) {
            foreach ($prop in $builds.PSObject.Properties | Where-Object { $_.Name -notlike "PS*" }) {
                if (-not $found.ContainsKey($prop.Value)) {
                    $rec = New-EngineRecord $prop.Name $prop.Value
                    if ($rec) { $found[$rec.Path] = $rec }
                }
            }
        }
    }
    return @($found.Values | Sort-Object Version -Descending)
}

function Select-Engine($engines) {
    if ($Engine) {
        $match = $engines | Where-Object { $_.Association -eq $Engine -or "$($_.Version)" -eq $Engine } | Select-Object -First 1
        if (-not $match) { throw "Engine '$Engine' is not installed. Found: $(($engines | ForEach-Object { $_.Association }) -join ', ')" }
        if (-not $match.HasMcp) { Write-Warning "Engine $($match.Association) has no ModelContextProtocol plugin; Unreal MCP will not start." }
        return $match
    }
    $withMcp = $engines | Where-Object HasMcp | Select-Object -First 1
    if ($withMcp) { return $withMcp }
    Write-Warning "No installed engine includes the ModelContextProtocol plugin; Unreal MCP will not start. Using the newest engine."
    return $engines | Select-Object -First 1
}

function Set-EngineAssociation([string]$Association) {
    $text = Get-Content $ProjectFile -Raw
    $updated = [regex]::Replace($text, '("EngineAssociation"\s*:\s*")[^"]*(")', "`${1}$Association`${2}")
    if ($updated -eq $text -and $text -notmatch [regex]::Escape("""$Association""")) {
        throw "Could not find EngineAssociation in $ProjectFile."
    }
    [System.IO.File]::WriteAllText($ProjectFile, $updated)
}

if (-not (Test-Path $ProjectFile)) { throw "Project file not found: $ProjectFile" }
$engines = Find-Engines
if ($engines.Count -eq 0) { throw "No Unreal Engine installation found. Install one from the Epic Games Launcher." }

Write-Host "Installed engines:"
foreach ($e in $engines) {
    Write-Host ("  {0,-40} {1,-6} {2}  MCP plugin: {3}" -f $e.Association, $e.Version, $e.Path, $(if ($e.HasMcp) { "yes" } else { "no" }))
}
$chosen = Select-Engine $engines
Set-EngineAssociation $chosen.Association
Write-Host "Aetheria.uproject now uses engine $($chosen.Association) ($($chosen.Path))."

if (-not $NoLaunch) {
    $editor = [IO.Path]::Combine($chosen.Path, "Engine", "Binaries", "Win64", "UnrealEditor.exe")
    if (-not (Test-Path $editor)) { throw "Editor not found at $editor." }
    Write-Host "Opening the editor. The Unreal MCP server starts with it."
    Start-Process -FilePath $editor -ArgumentList "`"$ProjectFile`""
}

Write-Host ""
Write-Host "Next: in this folder, run  claude  and say:"
Write-Host "  Build the game following CLAUDE.md, starting at phase 1."
