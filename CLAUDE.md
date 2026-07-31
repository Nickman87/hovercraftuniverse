# Working notes for Claude

Non-obvious facts about this repo that are expensive to rediscover. Keep this
short and pointer-shaped; the detail lives in `docs/porting/`.

## Ground rules

- Work and commit on the `revival` branch.
- **The game worked before the recompilation.** When something misbehaves,
  suspect what changed -- the Havok/ZoidCom/collision shims, Ogre 1.7 -> 14,
  the VC9 -> v143 toolchain, or a deliberate revival edit -- before suspecting
  game logic. Fixing core game code needs an explanation for why it ever
  worked.
- `HovercraftUniverse/data/` is recovered byte-for-byte and is a deliverable.
  Do not modify assets or the Lua scripts.
- Single-player is **not** a networking bypass: it hosts two `ZCom_Control`
  instances in one process. A genuine two-process test is still mandatory.

## Build and deploy

**Always** use the script. Never hand-roll cmake, never hand-copy binaries:

```powershell
.\scripts\build-and-deploy.ps1                      # build everything + deploy
.\scripts\build-and-deploy.ps1 -Target HovercraftUniverse   # faster, one target
```

It configures if needed, builds Debug|Win32, refuses to run while the game is
running, warns on a stale exe, and deploys exe + pdb + linker map to
`C:\hu-modern-run`. Details and rationale: `docs/porting/build-and-deploy.md`.

`C:\hu-modern-run` is **not** disposable -- it holds hand-fixed runtime config
and DLLs. Do not recreate it from `local-game/`.

Never run unbounded filesystem scans (`find /`, `Get-ChildItem -Recurse` over
a whole volume). These disks are slow and network-backed; such scans wedge in
uninterruptible I/O that `Stop-Process` cannot kill. Use Glob/Grep, or bound
the depth.

## Layout

| path | what |
|---|---|
| `HovercraftUniverse/` | the original 2010 game, essentially unmodified |
| `compat/havok/` | Havok 6.6 -> Bullet 3.25 shim |
| `compat/zoidcom/` | ZoidCom -> ENet shim |
| `compat/collision/` | rebuilds collision bodies from scene + mesh data |
| `compat/skyx/` | inert stub |
| `modern/CMakeLists.txt` | the build |
| `C:\hu-modern-build` | build dir (Debug\|Win32) |
| `C:\hu-modern-run` | runtime dir |

## Physics -- read this before touching anything physics-shaped

**The track is an asteroid.** There is no global "down". A level's drivable
surface is a scene entity tagged `<Asteroid>` with its own `<Gravity>` value
(SimpleTrack2: `Asteroid01` gravity 100, `Asteroid02` gravity 75). Planet
gravity, the craft's up vector, and the hover force all derive from it.

The craft's orientation is **commanded, not simulated**: `PlanetGravityAction`
feeds the planet's surface normal into `mUp`, and `HavokHovercraft::update()`
forces the rigid body to match. That is the self-righting. If
`PlanetGravityAction` stops running, the craft loses all attitude control --
flies off ramps, ends up inverted, clips through terrain.

Full model, including the three separate gravity sources and the shim
pitfalls: **`docs/porting/physics-model.md`**.

## Other docs

- `docs/porting/phase-b-plan.md` -- the menu-to-race ladder and the
  reproduction-vs-reconstruction boundary
- `docs/porting/first-run.md` -- smoke-test findings, known runtime gotchas
  (e.g. `d3dx9_43.dll` delegates HLSL compilation to `D3DCompiler_43.dll`;
  missing that DLL renders the world white with a NULL error buffer)
- `docs/porting/havok-compat.md`, `zoidcom-compat.md`, `ogre-api-gap.md`,
  `phase-b-collision.md` -- per-shim design notes

## Testing

The user drives the GUI and performs in-game actions. Claude launches
processes and reads logs -- no screenshots, no synthetic clicking. Announce a
launch and wait for confirmation *before* starting anything that competes for
the machine.

Always run test sessions through the script, never by launching the exe by
hand:

```powershell
.\scripts\run-test-session.ps1 -Mode twoprocess -Seconds 45   # headless
.\scripts\run-test-session.ps1 -Mode client -Seconds 0        # human testing
.\scripts\run-test-session.ps1 -Collect                       # snapshot logs after
.\scripts\run-test-session.ps1 -Mode client -Seconds 0 -Debugger   # catch a crash
```

`-Debugger` runs the game under `cdb.exe` (x86, from the WinDbg MSIX package;
resolve the path via `Get-AppxPackage Microsoft.WinDbg`, the version is in the
path). First-chance exceptions pass through untouched so the game's own SEH
handlers behave normally; an UNHANDLED access violation writes all-thread
stacks (`~*kb`) and a full dump into the run folder. Use it instead of
inferring crash sites from Windows event-log offsets -- note the game's own
"Exception in HavokThread!" dialog CATCHES the exception, so those faults are
first-chance and need `sxe av` rather than the default second-chance trap.

It refuses to start while another instance is running, captures stdout/stderr
per process, and snapshots every log into `C:\hu-modern-run\testruns\<stamp>-<mode>\`.

**Never run two sessions at once.** Instances share the same Ogre log files
and UDP 2375; a subagent test session running against the user's has already
produced misleading results once.

Log layout worth knowing:
- Ogre logs land in `C:\hu-modern-run\data\`, not the run root, because
  `Application::parseIni()` chdir's into `DataPath` before `Root` is created.
- Two-process mode writes `HovercraftUniverse.log` (client) and
  `DedicatedServer.log` (server) -- genuinely separate.
- **Single-player hosts the server inside the client process**, so collision
  reconstruction, physics and rendering all share one process and one log
  (`SinglePlayerServer.log` is never created). Bugs that reproduce only in
  single-player are usually about that sharing -- always check whether the
  two-process path behaves differently before blaming game logic.
- Any temporary diagnostic logging must include the PID in its filename. A
  single hardcoded name is ambiguous the moment two processes write to it.
