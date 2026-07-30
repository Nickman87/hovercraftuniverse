# Phase B — Legacy Asset Fixes

**Workstream B of [phase-b-plan.md](phase-b-plan.md).** Small, independent, and worth doing
early: it clears the noise out of the logs we'll be reading for the rest of Phase B.

Ground truth is `C:\hu-modern-run\data\HovercraftUniverse.log` from the first successful
modern run — not inference.

---

## 1. Legacy `.fontdef` syntax — 3 files, confirmed failing

Ogre 1.8 made script type prefixes mandatory. Observed error
(`HovercraftUniverse.log:226,228,230`):

```
Error: ScriptCompiler - unexpected token in Badaboom.fontdef(1): 'Badaboom'.
If this is a legacy script you must prepend the type (e.g. font, overlay).
```

Three files under `HovercraftUniverse/data/ogre/fonts/`, all identical in form:

| File | Font | TTF |
|------|------|-----|
| `Badaboom.fontdef` | `Badaboom` | `badabb.ttf` |
| `Lynx.fontdef` | `Lynx` | `Lynx.ttf` |
| `StarWars.fontdef` | `StarWars` | `solo5.ttf` |

Current form:

```
Badaboom
{
	type 		truetype
	source 		badabb.ttf
	size 		10
	resolution 	96
}
```

**Fix:** prepend the `font` keyword. One word per file, nothing else changes:

```
font Badaboom
{
	type 		truetype
	source 		badabb.ttf
	size 		10
	resolution 	96
}
```

The body syntax (`type` / `source` / `size` / `resolution`) is unchanged in modern Ogre. This
is the smallest possible fix and it is a genuine forward-port of the original intent, not a
behaviour change.

## 2. `asteroid.program` — Cg → HLSL

The only game-authored shader program using a dead language. Modern Ogre has no
`Plugin_CgProgramManager` because the NVIDIA Cg toolkit is discontinued.

**This turns out to be nearly trivial**, because the "Cg" shaders are already written in what
is effectively HLSL. Reading `asteroidVS.cg` and `asteroidPS.cg`: `float4x4`, `sampler2D`,
`tex2D`, `mul`, and `POSITION0`/`TEXCOORD0`/`COLOR0` semantics are all valid HLSL as written.
Cg and HLSL were co-designed; for shaders this simple they are the same language.

Three changes needed:

**2a. Declare the language as HLSL and modernize the profile.** Current
`data/levels/materials/programs/asteroid.program`:

```
vertex_program asteroidVS cg
{
	source asteroidVS.cg
	profiles vs_1_1
	entry_point vs_main
}

fragment_program asteroidPS cg
{
	source asteroidPS.cg
	profiles ps_2_0
	entry_point ps_main
}
```

becomes:

```
vertex_program asteroidVS hlsl
{
	source asteroidVS.hlsl
	target vs_2_0
	entry_point vs_main
}

fragment_program asteroidPS hlsl
{
	source asteroidPS.hlsl
	target ps_2_0
	entry_point ps_main
}
```

Note `profiles` → `target` (HLSL's keyword) and `vs_1_1` → `vs_2_0`. The bump is not
optional: the modern `D3DCompiler` dropped support for `vs_1_1` entirely, which is precisely
why Ogre's own shadow programs fail in our log (§3). `ps_2_0` is already fine.

**2b. Fix one dimension mismatch.** `asteroidVS.cg` does:

```c
Output.Normal = ( mul( Input.Normal, matTransform ) + 1.0f ) / 2.0f;
```

`Input.Normal` is `float3` and `matTransform` is `float4x4`. Cg accepted this; HLSL rejects it
as a dimension mismatch. Correct fix, which also happens to be the *right* thing for a normal
(no translation):

```c
Output.Normal = ( mul( Input.Normal, (float3x3)matTransform ) + 1.0f ) / 2.0f;
```

**2c. Rename the sources** `.cg` → `.hlsl`, keeping the old files in place until the new path
is verified.

Program names (`asteroidVS` / `asteroidPS`) stay identical, so **no `.material` file needs
editing.** Consumers, for verification once fixed — 3 of the 5 tracks:

- `data/levels/Bavaraf/materials/scripts/Bavaraf.material:56`
- `data/levels/junkyard/materials/scripts/Junkyard.material:11,62,113,164,213`
- `data/levels/junkyard/materials/scripts/Junkyard2.material:372,423,474,525`
- `data/levels/SimpleTrack2/materials/scripts/SimpleTrack2.material:254,305,356`

(Also `Kopie van Bavaraf.material:56` — an unused Dutch-named copy, and
`SimpleTrack2/.../ShaderExample.txt:11`, reference text. Neither is live; leave both alone.)

This shader does 3-layer alpha-map terrain blending. If it silently fails, those tracks lose
their terrain texturing — so it is worth fixing properly rather than stubbing.

## 3. Ogre's own shadow programs — investigate, probably leave alone

Four errors in the log, from Ogre's stock `OgreCoreMedia`, not from the game's assets
(`HovercraftUniverse.log:207-208,211-212`):

```
Program 'Ogre/ShadowBlendVP' is not supported: Cannot assemble D3D9 high-level shader
Program 'Ogre/ShadowBlendFP' ...
Program 'Ogre/ShadowExtrudeDirLightFinite' ...
Program 'Ogre/ShadowExtrudeDirLight' ...
```

Same root cause — vintage profiles the modern `D3DCompiler` won't assemble.

**Do not reflexively fix these.** Two reasons:

1. `ShadowVolumeExtrude*` is used for **stencil** shadow volumes. The game sets
   `SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED` — texture shadows. If nothing requests stencil
   shadows, these four failures are harmless noise and patching them is pure risk.
2. They live in vcpkg-installed upstream Ogre media, not in our repo. Patching them means
   either forking Ogre's media into our tree or patching during runtime-layout assembly —
   both real maintenance costs for something that may not matter.

**Finding — confirmed, not inferred. Leave them alone.** Traced through both Ogre's media and
its source:

- In `Shadow.material` (`vcpkg/installed/x86-windows/share/ogre/Media/Main/`),
  `Ogre/ShadowBlendVP` and `Ogre/ShadowBlendFP` are referenced *only* by
  `Ogre/StencilShadowModulationPass`, `Ogre/StencilShadowVolumes` and
  `Ogre/Debug/ShadowVolumes` — all stencil-shadow materials.
- In the Ogre 14.5.2 source, `OgreStencilShadowRenderer.cpp` is the only file that references
  those materials. `OgreTextureShadowRenderer.cpp` — the path this game actually uses via
  `SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED` — touches only `Ogre/TextureShadowCaster` and
  `Ogre/TextureShadowReceiver`, both fixed-function materials with **no shader programs at
  all**.

So all four failures are dead code for this game's shadow configuration. They are expected
benign log noise, no runtime-layout patch is needed, and patching them would have been pure
risk for zero benefit.

Shadows themselves remain cosmetic and belong in workstream F, not on the path to playable.

## 4. SkyX shaders — out of scope

`SkyX_Clouds.hlsl`, `SkyX_Ground.hlsl`, `SkyX_Moon.hlsl`, `SkyX_Skydome.hlsl`,
`SkyX_VolClouds.hlsl` all declare vintage profiles. Moot: `compat/skyx` is an inert shim that
never loads these materials, and its resource group is currently disabled to avoid a crash
(see [first-run.md](first-run.md)). Belongs to the optional real-SkyX-0.4 port in workstream F.

## 5. Acceptance criteria

- Zero `ScriptCompiler` errors in a clean client log.
- `asteroidVS` / `asteroidPS` compile, and the three affected tracks show correctly blended
  terrain rather than untextured or flat-coloured ground. **Visual confirmation required** —
  a successful compile does not prove the `(float3x3)` cast preserved the original look, and
  `local-game/` is available for side-by-side comparison.
- Any remaining log errors are explicitly documented as benign, with the reason.
