# Phase B — Getting to a Playable Game

**Status:** planning complete, implementation starting
**Branch:** `revival`
**Predecessor:** Phase A ended at commit `fd99b9f` — the modern build compiles, links, and
runs; the dedicated server reaches *"Ready for incoming connections"* and the client reaches
the Flash main menu.

Phase B's single goal: **drive a hovercraft around a track in the modern build.**

---

## 1. The three findings that shape this plan

Before writing any code we inventoried the original game's actual requirements. Three
discoveries changed the plan that was sketched at the end of Phase A.

### 1.1 Single-player is not a networking bypass

This is the most important finding, and it inverts the priority order.

`MainMenu::onSingleplayer` ([MainMenu.cpp:84-125](../../HovercraftUniverse/HovercraftUniverse/MainMenu.cpp)) does this:

```cpp
mLocalServer = new HUDedicatedServer("SingleplayerServer.ini");
mLocalServer->init();
mLocalServer->run(false);          // full server on a boost thread, same process
Sleep(1000);                       // "give the server a second to load"
mListener->onConnect("localhost", this);   // <-- identical to multiplayer "join by IP"
```

`HUClient("localhost")` sets `mRemote = true` and calls `ZCom_Connect` with a real
`eZCom_AddressUDP`. There is **no** branch anywhere in `MainMenu` / `MainMenuState` /
`HUClient` / `NetworkClient` that skips the network stack for single-player. A local
loopback mode does exist in `NetworkClient` (`eZCom_AddressLocal`) but only the demo chat
system uses it — the game's single-player path does not.

**Consequence:** ZoidCom replication and node linking are on the critical path to *any*
racing at all. They cannot be deferred as "the multiplayer part". This is why
[workstream C](#c-zoidcom-phase-b) is ordered before collision.

### 1.2 Collision can be reconstructed by name, without parsing `.hkx`

The 9 `.hkx` files are opaque, and we are not going to reverse-engineer Havok's
serialization format. We don't have to. The game only ever asks the `.hkx` for bodies
*by name*:

```cpp
mPhysicsData->findRigidBodyByName(entityname.c_str())
```

…called from exactly three places: `createAsteroid`, `createStaticBody`
([HoverCraftUniverseWorld.cpp:182-230](../../HovercraftUniverse/HovercraftUniverse/HoverCraftUniverseWorld.cpp))
and `HavokHovercraft::load` ([HavokHovercraft.cpp:398-419](../../HovercraftUniverse/HovercraftUniverse/HavokHovercraft.cpp)).

And the name it passes is read straight out of the `.scene` XML — the `<OgreEntity>` tag of
an `<externals>` item, which is also the name of the scene node carrying the render mesh:

```xml
<node name="Asteroid01"><entity name="Asteroid01" meshFile="Asteroid01.mesh"/></node>
...
<externals><item name="Asteroid01_ent">
  <userData><![CDATA[<Asteroid><OgreEntity>Asteroid01</OgreEntity>...]]></userData>
  <position .../><rotation .../><scale .../>
</item></externals>
```

So **the render mesh and the missing collision body are joined by a shared name**, and the
`.scene` supplies the world transform. Collision geometry can be rebuilt from the mesh data
the game already ships. See [phase-b-collision.md](phase-b-collision.md).

Corroborating evidence: there are no separate collision meshes in the asset tree — no
`_collision` / `_phys` / `_lowpoly` naming anywhere. This was a single-LOD pipeline; the
original Havok shapes were baked from these same render meshes in a content pipeline that
no longer exists.

Note also that **trigger volumes need no reconstruction at all.** Start, Finish, Checkpoint,
Portal and SpeedBoost are built in C++ as `hkpAabbPhantom` / `hkpBoxShape` from the
`.scene` transforms and never touch the `.hkx`. Those already work.

### 1.3 The modern build has never been driven past the main menu

`C:\hu-modern-run\data\HovercraftUniverse.log` ends at line 256, right after the main
menu's overlay materials are created. No connect attempt, no lobby, no `InGameState`.

Everything we believe about what breaks after the menu is *inference from source reading*.
That's a bad basis for planning implementation, and the fix costs minutes. Hence
[workstream A](#a-smoke-test-first) runs first and is allowed to reorder everything after it.

---

## 2. The path from menu to race

Established by source trace; this is the ladder Phase B has to climb.

| # | Step | Code | Currently |
|---|------|------|-----------|
| 1 | Click Singleplayer | `MainMenu::onSingleplayer` — [MainMenu.cpp:84](../../HovercraftUniverse/HovercraftUniverse/MainMenu.cpp) | ✅ Flash callback works |
| 2 | Start in-process server | `HUDedicatedServer::run(false)` → `HUServerThread` | ✅ reaches "Ready for incoming connections" |
| 3 | Client dials `localhost` | `HUClient` → `ZCom_Connect` over ENet | ✅ ENet transport is real |
| 4 | Server accepts | `ZCom_cbConnectionRequest` → `Lobby::onConnectAttempt` | ✅ |
| 5 | Server spawns `PlayerSettings` | `ZCom_cbConnectionSpawned` → `Lobby::onConnect` | ✅ server-side object exists |
| 6 | **Client is asked to build the proxy objects** | `ZCom_cbNodeRequest_Dynamic` → `HUClient::onNodeDynamic` | ❌ **never invoked — no node linking** |
| 7 | Lobby state syncs to client | replicated `mTrack`/`mAdmin`/`mCurrentPlayers` | ❌ replication tick is a no-op |
| 8 | Click Start | `LobbyState::onPressStart` → `Lobby::start()` → `StartTrackEvent` | ❌ events are dropped on send |
| 9 | Server builds `RaceState`, loads track | `Lobby::process` → `new RaceState(...)` → `mLoader->load(trackfile)` | ❌ never reached |
| 10 | Client enters `InGameState` | `LobbyState::onStart` → `switchState(IN_GAME)` | ❌ never reached |
| 11 | Race state machine advances | `INITIALIZING→LOADING→INTRO→COUNTDOWN→RACING`, driven by client acks over events | ❌ events |
| 12 | Hovercraft rests on the track | `findRigidBodyByName` on level + hull `.hkx` | ❌ returns `nullptr` |
| 13 | Input reaches physics | `BasicEntityEvent` (5 bools) owner→authority | ❌ events |
| 14 | Craft position renders on client | replicated `mPosition`/`mOrientation`/`mVelocity` | ❌ replication |

Steps 6–11, 13 and 14 are ZoidCom. Step 12 is Havok. **That ratio is why ZoidCom is the
larger workstream and goes first.**

---

## 3. Workstreams

### A. Smoke test first

Launch the modern client, actually click Singleplayer, and capture what happens.

Purpose is information, not repair. Specifically we want to know:
- does the in-process server start and does the client's ENet connect succeed?
- where exactly does it stall or crash — and with what log output?
- does anything unexpected fail that the source read didn't predict? (Phase A's experience
  was that compiling and running found roughly a dozen things that reading did not.)

**Deliverable:** a findings section appended to [first-run.md](first-run.md), and licence to
reorder workstreams B–D based on what we learn.

**Acceptance:** we can state, from a log, the exact line the menu→race path dies on.

### B. Asset fixes

Small, independent, and worth doing early because it clears noise out of the logs we'll be
reading for the rest of Phase B.

Three legacy `.fontdef` files, the vintage-profile shadow programs, and `asteroid.program`'s
Cg shaders (referenced by 3 of the 5 tracks). Full detail and exact edits in
[phase-b-assets.md](phase-b-assets.md).

**Acceptance:** zero `ScriptCompiler` errors and zero "not supported: Cannot assemble" lines
in a clean client log.

### C. ZoidCom Phase B

The critical path. ~85 stubbed symbols, but only four mechanisms matter for racing:

1. **Cross-network node linking** — the one blocker behind steps 6–11 above. Today every
   `ZCom_Node` is islanded on its own `ZCom_Control`.
2. **Node-to-node events** — `sendEvent` / `sendEventDirect` / `checkEventWaiting` /
   `getNextEvent`. Carries player input, the race state-machine handshake, the countdown,
   checkpoints, chat.
3. **Basic replication tick** — `ZCom_processReplicators` actually walking each node's
   registered fields.
4. **`ZCom_ReplicatorAdvanced` dispatch** — needed specifically by the game's hand-rolled
   `EntityPropertyMapReplicator`.

Design, wire protocol and ordering in [phase-b-replication.md](phase-b-replication.md).

**Acceptance:** single-player reaches `RaceState::RACING` with a hovercraft spawned,
owned by the local player, responding to input; and a second client on the LAN sees it move.

### D. Collision reconstruction

Build Bullet shapes from the `.scene` + `.mesh` data and expose them through the shim's
existing `findRigidBodyByName`, so no game code changes. Design in
[phase-b-collision.md](phase-b-collision.md).

**Acceptance:** the hovercraft rests on the track surface instead of falling through it, and
can be driven a full lap on `SoccerField` (smallest track) and `Junkyard`.

### E. Physics tuning and 60 Hz validation

Bullet is not Havok. Friction, restitution, damping, solver iteration count and contact
handling all differ, so the *feel* will not match even once collision exists. We have an
unusually good instrument for this: `local-game/` is a known-good, playable build of the
original.

Also outstanding from the 60 Hz work in `89335b4`: `HU_ENTITY_SMOOTH_TAU = 0.08f` and
`HU_ENTITY_TELEPORT_THRESHOLD = 15.0f` are **unvalidated guesses** that have never been
observed at runtime.

**Acceptance:** a documented side-by-side comparison against `local-game/`, and tau/threshold
values confirmed or corrected from observation rather than assumption.

### F. Optional polish

Explicitly out of scope for "playable", tracked so it isn't forgotten: a real SkyX 0.4 port
(currently an inert shim — no sky, sun or clouds), working shadows, and the bootstrap script
updates for OIS + `Codec_STBI`.

---

## 4. Order of work

```
A. Smoke test ──┬─> B. Assets (independent, parallel)
                │
                └─> C. ZoidCom ──> D. Collision ──> E. Tuning ──> F. Polish
                    (linking,      (needs C to      (needs D)
                     events,        be testable
                     replication)   at all)
```

C before D is deliberate: without node linking you cannot reach a race, so you cannot
observe whether collision works. Conversely a race with no collision still renders the
track and proves steps 1–11 — a genuinely useful intermediate state, and one worth
committing on its own.

B is fully independent and can run alongside C.

---

## 5. Scope boundary — reproduction vs. reconstruction

Worth stating plainly, because Phase B crosses a line the earlier phases did not.

Everything up to now was **reproduction**: the same game, built with different tools.
Where a dependency was dead we wrote a shim behind its original API, and where the original
headers survived we implemented against a real specification.

Two parts of Phase B are **reconstruction** — they produce something that behaves like the
original but is not derived from it:

- **Collision geometry.** Rebuilt from render meshes. The original hand-baked Havok proxies
  are gone. Contact behaviour will differ, probably in the direction of being more detailed
  and more expensive than what shipped.
- **The ZoidCom wire protocol.** Our shim is self-consistent but not wire-compatible with
  vendor ZoidCom. This is harmless — every peer runs the same shim — but it does mean a
  modern client can never talk to a 2010 server.

Neither is avoidable without the original tooling, and both are documented as such at their
call sites. Flagging them here so the distinction is visible in the history rather than
buried.

---

## 6. Known risks

| Risk | Impact | Mitigation |
|------|--------|------------|
| Ogre discards mesh vertex data after GPU upload, so it can't be read back for collision | Blocks D entirely | Load collision meshes through the `MeshManager::load` overload that requests shadowed vertex/index buffers; see collision doc §4 |
| `eZCom_EventInit` delivery direction (authority vs proxy) is inferred, not documented | Race state machine could deadlock | The original ZoidCom headers survive in `archive-mirror/`; verify against them before implementing |
| Convex hull of a visual hovercraft hull may be spiky or oversized for a character body | Craft catches on geometry | Decimate via `btShapeHull`; keep an auto-fitted-capsule fallback behind a config switch |
| Bullet contact behaviour on a `btBvhTriangleMeshShape` seam differs from Havok's MOPP | Craft snags on track seams | Known Bullet issue; mitigate with contact-processing threshold tuning in E |
| Track collision meshes are full-detail render geometry | Load time and step cost | Measure first; only optimize if it actually hurts |
| The 1000 ms `Sleep` in `onSingleplayer` may be too short once collision loading is added | Intermittent single-player failure | Replace with a real readiness handshake if it bites |

---

## 7. Progress

| Workstream | Status |
|------------|--------|
| A. Smoke test | in progress |
| B. Asset fixes | in progress |
| C. ZoidCom Phase B | not started |
| D. Collision reconstruction | not started |
| E. Physics tuning / 60 Hz validation | not started |
| F. Optional polish | not started |
