# 60 Hz physics + decoupled rendering (deliberate remaster-side change)

## Status

This is **not** a faithful-port change. It is a deliberate, explicitly
approved deviation from the original 2010 behavior: the physics tick rate
is raised from the shipped 30 Hz reference to 60 Hz, and rendering is
decoupled from the physics tick so motion stays smooth on high-refresh
(144 Hz) displays. Everything else in this document exists to make that
change *not* alter how the game feels, and to spell out exactly what was
touched.

All changes described here are **compile-verified only**. The game
executable cannot link yet (GUI/Hikari, Sound/FMOD and Scripting are not
ported -- see the main `RUNNING.md`/README Phase 3 notes), so nothing in
this document has been observed running. See "What remains unverifiable"
at the end.

## The measured coupling

Two gameplay-tuning constants in `HovercraftUniverse/data/engine_settings.cfg`
are applied per physics step in `HovercraftUniverse/HovercraftUniverse/
HavokHovercraft.cpp`, and both are step-rate-coupled:

- `[Movement] TurnAngle` (0.12) is read into `mRotationDelta` at
  `HavokHovercraft.cpp:79` and applied as a fixed per-step rotation angle
  in `update()` (~lines 260-290): every step where the player turns, the
  hovercraft's side vector rotates by exactly this angle. That makes it
  **rate-linear** -- turning at 60 Hz instead of 30 Hz would double the
  effective turn rate (degrees/second) if left unscaled, because twice as
  many fixed-angle steps happen per second.
- The orientation-correction gain, a local `const hkReal gain = 0.25f;` at
  `HavokHovercraft.cpp:359` (previously; now the normalized `mAngularGain`
  member), is used at line ~366 as
  `angularVelocity.setMul4(gain / dt, angle)` to steer the character rigid
  body's angular velocity so it closes 25% of the remaining orientation
  error every step. That makes it **rate-exponential** -- the fraction of
  error closed *per second* is `1 - (1 - gain)^(steps per second)`, which
  is not linear in step rate; leaving `gain` fixed at a higher step rate
  would make orientation snap to target much faster (stiffer, twitchier
  feel).
- `[Movement] Damping` is read into `mSpeedDamping` at
  `HavokHovercraft.cpp:80` but is **never used anywhere** in the
  simulation -- verified by searching all reads of `mSpeedDamping`
  (constructor only). This is dead config, left alone (see "What was NOT
  changed").
- `CharacterGravity` is integrated through `hkStepInfo::m_deltaTime` (the
  actual physics dt each step), which is already correctly rate-invariant.
  It was not touched.

## The ~21 Hz finding

The original physics loop in
`HovercraftUniverse/HovercraftUniverse/HavokThread.cpp` (before this
change) was:

```cpp
while ( HavokThread::run ) {
    world->step();
    Sleep( world->getTimeStep() * 1000 );
}
```

Windows' default system timer granularity is ~15.6 ms (the multimedia
timer is not raised by default). `Sleep(33)` (the value for a 30 Hz
`dt = 1/30 s`) rounds up to the next ~15.6 ms tick boundary, which in
practice measures out to roughly 47 ms per iteration rather than 33 ms --
i.e. the shipped "30 Hz" physics loop actually ran at roughly **21 Hz**
on stock Windows. This also means 60 Hz or 144 Hz targets were never
reachable through `Sleep()` alone at default granularity: `Sleep(16)`
(60 Hz) and `Sleep(7)` (144 Hz) round up to the same ~15.6/31 ms
boundaries. Separately, sleeping a fixed duration *after* stepping means
the real loop period is `dt + step_cost`, which drifts under load (a
slow step steals time from the "sleep" budget instead of being accounted
for).

This finding is a byproduct of implementing the new loop (part of design
work, not a runtime measurement -- see "What remains unverifiable"): it
follows directly from Windows' documented default timer resolution and
`Sleep()`'s documented rounding-up behavior, but has not been measured on
the shipped binary with a profiler.

## A. Rate normalization

Added a single named reference constant in `HavokHovercraft.cpp`:

```cpp
static const hkReal HU_PHYSICS_REFERENCE_RATE = 30.0f;
```

with a comment explaining that every per-step tuning constant in
`engine_settings.cfg` was authored/tuned by feel at 30 Hz, and must be
re-derived against this reference whenever the physics step rate
changes, so that the *feel* stays constant.

Both coupled constants are now computed once in the `HavokHovercraft`
constructor (dt is fixed for the process lifetime, since it comes from
`[Havok] Framerate`, so this is safe and cheap):

- `mRotationDelta` (rate-linear): `TurnAngle * (dt * HU_PHYSICS_REFERENCE_RATE)`.
  At the 30 Hz reference this is a no-op: `0.12 * (1/30 * 30) = 0.12`.
  At 60 Hz: `0.12 * (1/60 * 30) = 0.06`.
- `mAngularGain` (rate-exponential, new member replacing the local
  `gain` in `update()`):
  `1.0f - powf(1.0f - 0.25f, dt * HU_PHYSICS_REFERENCE_RATE)`.
  At 30 Hz: `1 - 0.75^1 = 0.25` (no-op).
  At 60 Hz: `1 - 0.75^0.5 ~= 0.1340`.

`engine_settings.cfg`'s `TurnAngle` value itself is **unchanged** (still
`0.12`) -- the normalization happens in code, not in config, specifically
so the config keeps describing the original 30 Hz-authored tuning and any
future `Framerate` value automatically re-derives the correct per-step
constants without editing the `.cfg`.

### What was NOT changed

- `[Movement] Damping` / `mSpeedDamping`: confirmed dead (read, never
  used). Left as-is; a comment was added at its declaration in
  `HavokHovercraft.h` explicitly noting it is unused, so a future editor
  doesn't "fix" it by wiring it up (which would be a real gameplay change,
  not a normalization).
- `CharacterGravity`: already dt-integrated via `hkStepInfo::m_deltaTime`;
  untouched.
- No other constants in `engine_settings.cfg` (`Height`, `ProbeLength`,
  `ProbeRadius`) were touched or rescaled.

## B. `engine_settings.cfg`

`HovercraftUniverse/data/engine_settings.cfg`: `[Havok] Framerate` changed
from `30` to `60`. `TurnAngle` stays `0.12` (part A's normalization
handles the scaling in code).

`local-game/data/engine_settings.cfg` (an untracked scratch copy used to
test the *old* binary with `Framerate=60`/`TurnAngle=0.06`) was
deliberately **not** touched -- it belongs to a different experiment.

## C. Physics timing loop (`HavokThread.cpp`)

Replaced the `Sleep(dt*1000)`-after-step loop with a
`QueryPerformanceCounter`-based deadline accumulator:

- `timeBeginPeriod(1)` is called once before the loop (and
  `timeEndPeriod(1)` once after) to raise the process-wide Windows timer
  granularity to ~1 ms for the lifetime of the physics thread, making
  `Sleep()` precise enough to hit 60 Hz/144 Hz-scale periods. Requires
  `<timeapi.h>` and linking `winmm` -- added via `#pragma comment(lib,
  "winmm.lib")` in `HavokThread.cpp` and via `target_link_libraries
  (hu_game_physics PUBLIC ... winmm)` in `modern/CMakeLists.txt` (belt and
  suspenders: either alone would suffice under MSVC, both together keep
  the intent explicit at both the source and build-system level).
- A `nextDeadlineMs` accumulator advances by `dt` every iteration
  (independent of when the previous iteration actually woke up), so the
  loop tracks an ideal schedule rather than "now + dt", avoiding the
  drift the original fixed-Sleep-after-step approach had under load.
- Each iteration sleeps for the bulk of the remaining time toward the
  deadline, leaving a small margin (`HU_PHYSICS_SLEEP_MARGIN_MS = 2.0`),
  then short-spins with `YieldProcessor()` until the deadline is reached
  -- accurate timing without permanently burning a full core the way a
  pure busy-wait would.
- **Catch-up clamp**: if the loop falls more than
  `HU_PHYSICS_MAX_CATCHUP_STEPS = 4` steps behind the schedule (e.g. a
  stall stole a large chunk of wall-clock time), `nextDeadlineMs` is
  resynchronized to "now" instead of letting the loop try to run a
  backlog of steps back-to-back, which would otherwise risk a death
  spiral (steps take real time, so "catching up" can make the loop fall
  further behind, forever, at 100% CPU).

The existing try/catch structure and `HavokThread::run` exit condition
are unchanged. The original author's commented-out stopwatch attempt
(previously at lines 98-99) is what this implements properly (with
QueryPerformanceCounter instead of `hkStopwatch`, plus the granularity
fix, sleep/spin split, and catch-up clamp it was missing).

## D. Decoupling rendering from physics (`Entity.h` / `Entity.cpp`)

### The problem

`Entity::update(float timeSince)` maintains a render-facing
`mTmpPosition` separate from the authoritative `mPosition` (set by
physics or network replication). Previously:

```cpp
if (mLastPosition != mPosition) {
    mTmpPosition = mPosition;      // snap
    mLastPosition = mPosition;
} else {
    mTmpPosition += mVelocity * timeSince;  // dead-reckon
}
```

`mLastPosition != mPosition` detects "a new authoritative value arrived
since the last `update()` call" (network replication and the physics
integration write `mPosition` directly, bypassing `changePosition()`,
which is what keeps `mLastPosition` in sync the rest of the time). At a
render rate much higher than the physics tick rate, this produces
several frames of smooth extrapolation followed by a visible hard "pop"
back onto the authoritative position every physics tick -- exactly the
judder the decoupling is meant to fix. At 60 Hz physics and a 144 Hz
display, that's roughly one correction every ~2.4 rendered frames.

### The fix

`Entity::update()` now always dead-reckons first (`mTmpPosition +=
mVelocity * timeSince`, unconditionally, every frame -- this is the part
that makes motion advance smoothly between physics ticks), and then, only
if a new authoritative `mPosition` arrived, corrects `mTmpPosition`
towards it instead of snapping:

```cpp
mTmpPosition += mVelocity * timeSince;

if (mLastPosition != mPosition) {
    const Ogre::Vector3 error = mPosition - mTmpPosition;
    if (error.squaredLength() > HU_ENTITY_TELEPORT_THRESHOLD * HU_ENTITY_TELEPORT_THRESHOLD) {
        mTmpPosition = mPosition;                 // large jump: snap
    } else {
        const float alpha = 1.0f - std::exp(-timeSince / HU_ENTITY_SMOOTH_TAU);
        mTmpPosition += error * alpha;             // small jump: blend
    }
    mLastPosition = mPosition;
}
```

The blend is a critically-damped exponential decay of the positional
error, parameterized by a time constant `tau` rather than a fixed
per-frame fraction, specifically so it behaves the same regardless of the
render frame rate (the `timeSince`-dependent `alpha` is what makes it
frame-rate independent -- a fixed `alpha` per frame would smooth more at
low frame rates and less at high ones for the same real-world correction
time).

### Tau and teleport threshold

Both are named constants in `Entity.h`:

- `HU_ENTITY_SMOOTH_TAU = 0.08f` seconds. Chosen as roughly 1-2 physics
  ticks at the new 60 Hz rate -- small enough that the correction should
  read as "instant" rather than laggy, large enough to fully absorb the
  positional judder from physics-rate corrections arriving at a lower
  rate than the render rate. This is a starting estimate, not a
  playtested value (see "What remains unverifiable").
- `HU_ENTITY_TELEPORT_THRESHOLD = 15.0f` world units. Any correction
  larger than this snaps immediately instead of blending. The game has
  portals (`Portal.cpp`/`PortalPhantom.cpp`) and a checkpoint-respawn
  reset (`ResetSpawn.cpp`) that both cause large, deliberate positional
  jumps; smoothing across those would visibly slide the entity across the
  level instead of an instant teleport, which would look broken. 15 units
  is a first estimate based on the general scale of collision probes in
  `engine_settings.cfg` (`ProbeLength`/`ProbeRadius`, single-digit units)
  and needs confirmation once the game can actually run: specifically,
  that ordinary per-tick physics corrections never approach 15 units (they
  shouldn't, since that would already imply a physics-integration
  instability) and that real teleports/respawns/portal jumps reliably
  exceed it.

### `CameraSpring` decision

`Entity.h`/`Entity.cpp` had a disabled `CameraSpring mSpringInterpolator`
member (initialized in `Entity::init()`, its one call site in `update()`
already commented out). **Removed** rather than wired up, for three
reasons:

1. `CameraSpring` (see `CoreEngine/CameraSpring.h`) is a mass-spring-damper
   model built and tuned for a **camera-follow** use case (it's actively
   used that way elsewhere, in `RaceCamera.cpp`, which this change does
   not touch). Its parameters (mass/stiffness/dampening) have no obvious
   correspondence to "smooth a network/physics positional correction";
   repurposing it here would mean re-deriving an entirely different
   tuning problem instead of solving the stated one.
2. It is not `dt`-normalized against a documented reference the way this
   task requires for the rest of the change -- reusing it here would add
   another hidden rate-coupled constant of exactly the kind this whole
   effort is trying to eliminate.
3. The exponential blend is simpler, has a closed-form steady state (no
   overshoot/oscillation risk the way an underdamped spring can have),
   and its one parameter (`tau`) has a direct, documented physical
   meaning ("time to close ~63% of the error").

The `CameraSpring` class itself is untouched -- only Entity's disabled,
dead usage of it was removed.

### `getPosition()` / `getSmoothPosition()`

Both still return `mTmpPosition` (unchanged behavior) and are now
documented in `Entity.h`: `getPosition()` is what rendering, cameras and
effects should call every frame (the smoothed, frame-rate-independent
visual position); `getSmoothPosition()` is currently identical, kept as a
distinctly-named accessor in case the two are ever given different
smoothing behavior later. Neither the raw authoritative `mPosition` nor a
new dedicated accessor for it was added, since the task scope is limited
to fixing render-side judder, not adding new API surface.

## E. What a player will notice

- Turning and orientation-snap-to-target *feel* should be unchanged --
  that is the entire point of part A's normalization. If it is not
  unchanged once the game links, that indicates a bug in the
  normalization math above (report it against `HU_PHYSICS_REFERENCE_RATE`
  and the two formulas in `HavokHovercraft.cpp`'s constructor).
- Physics now actually runs at 60 Hz (versus the ~21 Hz the shipped build
  achieved in practice -- see "The ~21 Hz finding"), so hovercraft
  movement, collisions and character control should be noticeably more
  responsive and precise, not just smoother.
- On a 144 Hz (or any >60 Hz) display, motion between physics ticks should
  look smooth rather than stepped, and there should be no visible "pop"
  when a new physics/network position arrives -- that popping is exactly
  what part D removes.
- Teleports, checkpoint respawns and portal traversal should still be
  instant (no visible slide), because of the teleport-threshold clamp.
  If a respawn *does* visibly slide, `HU_ENTITY_TELEPORT_THRESHOLD` is set
  too high for this world's scale and should be lowered.

## What remains unverifiable until the game links

None of this has been run. Specifically unverified:

- That `HU_ENTITY_SMOOTH_TAU` (0.08s) and `HU_ENTITY_TELEPORT_THRESHOLD`
  (15.0 units) are actually good values at this game's real world scale
  and network latency -- both are first estimates from reading the code
  and config, not measurements.
- The ~21 Hz claim for the original loop's real-world behavior -- derived
  from Windows' documented `Sleep()`/timer-granularity behavior, not
  measured against the shipped binary with a profiler or timer trace.
- That the new `HavokThread.cpp` timing loop actually achieves a stable
  60 Hz (or 144 Hz, if `Framerate` were raised further) once real physics
  step costs, real collision counts and real thread scheduling are in
  play, rather than in isolation.
- That part A's normalization produces an indistinguishable feel in
  practice (versus just being algebraically correct against the stated
  30 Hz reference).
- Everything in part D end-to-end, since it depends on rendering, which
  cannot run until GUI/Hikari, Sound/FMOD and Scripting are ported (see
  the main README's Phase 3 notes).

All of the above is compile-verified only: `hu_utils`, `hu_exceptions`,
`hu_ogremax`, `hu_coreengine`, `hu_networking`, `hu_zoidcom_compat`,
`hu_havok_compat` and `hu_game_physics` all build clean under the
`modern/` CMake tree (MSVC v143, `cmake --preset x86`) after these
changes.
