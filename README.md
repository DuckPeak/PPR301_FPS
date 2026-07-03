# Gridfall

FPS / tower defence / horde survival: Unreal Engine 5, PC, single-player.

## Concept

A hybrid FPS and tower defence game set in a collapsing city. The player defends the last functioning power station from zombie waves - fighting directly in first person while placing powered defences on a grid. The goal is to keep players switching between action and strategy, never relying on one alone.

## Core Pillars

- **Fight in first person** - shooting, reloading, sprinting stays tense and active
- **Build under pressure** - place walls/turrets on a grid mid-wave
- **Protect infrastructure** - the power station is the objective, not just survival
- **Manage limited power** - power cost prevents unlimited building
- **Adapt each wave** - escalating pressure forces rebuilding and repositioning

## Setting & Tone

A survivor defends their colony's last power station in a ruined, collapsed city. Zombies are drawn to its noise and energy. Tone is dark, gritty, and industrial - urgency and fragility over heroism.

## Core Loop

Build defences → spend power → fight the wave → zombies damage player/station/defences → collect drops → repair/rebuild → earn power → next wave.

- **Lose if:** the power station is destroyed, or the player dies
- **Currently:** survive as long as possible
- **Planned:** clearer win condition (restore power, evacuate after N waves)

## Key Mechanics

| System | Notes |
|---|---|
| Movement/Shooting/Reload/Sprint | Standard FPS controls; can't reload while sprinting |
| Build mode (TAB) | Grid-based placement, left-click to place, mouse wheel to zoom |
| Defences | Wall (blocks, high HP), Turret (auto-damage, low HP), Microwave Turret (slows, doesn't attack) |
| Power station | Main objective; zombies target it when unblocked |
| Resources | Power (building), Ammo (weapon), Health (survival) |
