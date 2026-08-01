# How Hovercraft Universe physics actually works

Notes on the *original* game's physics model, written down because the shim
work kept re-deriving it from scratch. Nothing here is a porting decision --
it is a description of what the 2010 game code does, so that shim behaviour
can be judged against it.

## 1. The track is an asteroid

This is the single most surprising fact about this codebase, and almost
everything else follows from it.

There is no global "down". The drivable surface of a level is a **planet**:
a scene entity whose userData declares `<Asteroid>` with a `<Gravity>` value.
`HoverCraftUniverseWorld::createAsteroid()` tags that body
`HavokEntityType::PLANET` and hangs a `PlanetGravityPhantom` on it.

Concrete example -- `data/levels/SimpleTrack2.scene`:

| item | gravity | scale | role |
|---|---|---|---|
| `Asteroid01_ent` ("Begin") | 100 | 596 x 158 x 302 | the first landmass you drive on |
| `Asteroid02_ent` ("Middle") | 75 | 593 x 252 x 371 | the second landmass |

`SoccerField.scene` does the same with `FootballPlanet` (gravity 65). These
are not decorative rocks. They are the ground.

Everything else in a level (`Rock01`, `Engine01`, `Wing_Part_*`, boxes,
barrels) is a `<StaticBody>` -- ordinary collision geometry with no gravity
field of its own.

## 2. "Up" is the planet's surface normal, and it is what self-rights the craft

`PlanetGravityPhantom` covers the asteroid's AABB. Any non-fixed, non-planet
body that enters it gets a `PlanetGravityAction` attached
(`PlanetGravityPhantom::addOverlappingCollidable`), removed again on exit.

`PlanetGravityAction::applyAction()` then runs every physics step and does
two things:

1. **Gravity.** Closest-point query between the craft and the planet; the
   contact normal, negated, is the pull direction; force is
   `mass * asteroidGravity * dir`.
2. **Up vector.** The same normal (negated again, so it points away from the
   surface) is slerped into the craft's current up at 80% per step and pushed
   through `HavokEntity::updateUp()`.

Step 2 is the self-righting the game is known for. `HavokHovercraft::update()`
builds its *entire* desired orientation out of `mUp`:

```
newOrientation.getColumn(0) = mSide;              // side, kept perpendicular to up
newOrientation.getColumn(1) = mUp;                // <- from PlanetGravityAction
newOrientation.getColumn(2).setCross(mSide, mUp); // forward
```

and then forces the rigid body to that orientation with a proportional
angular-velocity controller (`estimateAngleTo` -> `setAngularVelocity`,
gain 0.25 per 30 Hz step). The craft's rotation is therefore *not* simulated;
it is commanded, every step, from the surface normal under it.

Consequences worth remembering:

- If `PlanetGravityAction` does not run, `mUp` stays at its constructor value
  `(0,1,0)` forever. The craft then has no surface-relative up, no planet
  gravity, and nothing to right it after a jump -- it just keeps whatever
  attitude the last contact impulse gave it. Symptom: flies off ramps, ends
  up inverted, clips through terrain.
- Driving on walls, loops and the underside of a landmass is *supposed* to
  work. Inverted is a legal attitude here, as long as the up vector agrees.

## 3. Hovering is separate, and only over planets

`HoverAction::applyAction()` casts a short ray along `-mUp`
(`from = pos + 0.1*up`, `to = pos - 2*hoverHeight*up`) using
`PlanetRayCastCallback`, which **ignores every hit that is not a PLANET
entity**. On a hit closer than `Hovering/Height`, it applies an upward force
of `mass * asteroidGravity / dist`. That `1/dist` term is the spring that
holds the hover gap.

So: gravity pulls toward the planet, hover pushes away from it, both scaled
by the *asteroid's own* gravity value. `StaticBody` geometry is collided
with, but never hovered over.

## 4. `Havok/CharacterGravity` is a second, separate thing -- but it does NOT stack with planet gravity

`HavokHovercraft::update()` passes
`input.m_characterGravity = -CharacterGravity * mUp` into the character state
machine, which integrates it into the output velocity while the craft is in
the air. Both this and the planet gravity from section 2 are aimed along the
*local* surface normal, not along world -Y -- but they are not simply
additive.

**Planet gravity supersedes character gravity while a `PlanetGravityAction`
is acting on the craft.** While the craft is inside a
`PlanetGravityPhantom`'s overlap, `PlanetGravityAction::applyAction()` is
already applying `mass * asteroidGravity * dir` as a force every step
(section 1). The shim's character state machine (`hkpCharacterStateInAir`,
`compat/havok/include/havok_compat/HavokAll.h`) additionally added
`m_characterGravity * dt` to the character's own velocity on top of that,
which double-counts gravity for any craft that is airborne over a planet.
`Havok/CharacterGravity` remains the fallback for a character that is
airborne with no planet gravity field acting (e.g. off the edge of a level
with no asteroid overlap) -- there real Havok's character gravity is the only
source and must still apply.

The shim detects "is a planet field acting on me right now" the same way the
game itself does: `PlanetGravityPhantom::add/removeOverlappingCollidable`
tags the attached action with `PlanetGravityAction::HK_SPHERE_ACTION_ID`
(`0x28021978`) and scans a body's action list for it
(`HovercraftUniverse/HovercraftUniverse/PlanetGravityAction.h`,
`PlanetGravityPhantom.cpp`). `hkpCharacterRigidBody::checkSupport()`
(`compat/havok/src/HavokAll.cpp`) mirrors that exact scan once per character
per step and records the result in a shim-only `hkpSurfaceInfo::
m_planetGravityActive` field; `hkpCharacterStateInAir::handle()` skips adding
`m_characterGravity` whenever that flag is set.

**Measured evidence for why this matters (SimpleTrack2, bot furthest
progress along the course, which runs toward negative x):**

| configuration | total gravity in flight | bot's furthest progress |
|---|---|---|
| CharacterGravity=50 (stacked with planet gravity 100) | ~150 m/s^2 | reaches only x ~= -190, ends up below the track |
| CharacterGravity=0 (planet gravity 100 alone) | 100 m/s^2 | reaches x = -876, climbing to y = 265 -- makes the jump, gets most of the way round |

With both sources stacked, the level's asteroid-to-asteroid jump is
unmakeable for both the human player and the AI bot.

There is a second, independent argument for the same conclusion: the craft's
hover force is `asteroidGravity / dist` (section 3), so its resting distance
off the surface is `dist = asteroidGravity / totalGravity`. The craft's own
collision capsule has radius 2 (`Collision/ProbeLength=6`,
`Collision/ProbeRadius=2`, `HavokHovercraft.cpp:66`). At total gravity 150
that equilibrium is `100/150 ~= 0.67` -- *inside* the craft's own radius, so
the craft is constantly trying to rest inside itself and fighting contact
resolution. At total gravity 100 the equilibrium is exactly `100/100 = 1.0`,
comfortably outside the capsule.

So a hovercraft in flight over a planet sees `asteroidGravity` alone (100 on
SimpleTrack2's first landmass) along `-mUp`, not `asteroidGravity +
CharacterGravity`. On the ground it sees neither: the on-ground state does
not apply gravity, and the hover force from section 3 holds the gap. Off any
planet field, `CharacterGravity` alone applies. Do not assume changing one of
these gravity sources is equivalent to changing another.

## 5. Where the shim has to be careful

- `hkpCollisionDispatcher::GetClosestPointsFunc` is called by
  `PlanetGravityAction` with **craft (convex hull) vs planet (concave
  triangle mesh)**. The collision reconstruction builds every non-hovercraft
  body as a `btBvhTriangleMeshShape` (`OgreCollisionProvider.cpp`,
  `buildStaticShape`). A convex-vs-convex-only implementation silently
  answers "no hit" for the single most important query in the game.
- The game's call site is
  `do { ...; tolerance *= 2; } while (!collector.hasHit());` -- an
  unconditional loop. A query that can never hit is an infinite loop, not a
  degraded result. Any shim implementation needs a bail-out.
- `hkpCharacterRigidBody` sets `angularFactor = 0` on the Bullet body on
  purpose: orientation is commanded (section 2), so the solver must not
  fight it. `setAngularVelocity()` still works with `angularFactor = 0` --
  Bullet applies the factor to torques and solver impulses, not to a directly
  assigned angular velocity.

## 6. Rate dependence (remaster 60 Hz change)

`HU_PHYSICS_REFERENCE_RATE = 30.0f` in `HavokHovercraft.cpp`. Two constants
are normalized against the real step rate:

- `TurnAngle` (0.12) scales **linearly**: `x (dt * 30)` -- it is an angle per
  step.
- The orientation gain (0.25) scales **exponentially**:
  `1 - 0.75^(dt*30)` -- it closes a fraction of the remaining error per step.

At `Framerate=30` both collapse to the original constants exactly, which makes
30 Hz a clean A/B baseline against the 2010 tuning.

Already checked and *not* rate-dependent: `CharacterGravity` (a rate, applied
`x dt`), probe dimensions (geometric), `Movement/Damping` (dead config -- read
into `mSpeedDamping` and never used, in the original too).
