# rbx-external

`C++17` · `Windows x64` · `MIT`

An external (out-of-process) cheat for the Roblox Windows client, written in
modern C++17. It reads game memory, renders through a transparent DirectX 11 /
Dear ImGui overlay, and writes back only through documented property offsets —
**no injection, no DLL, no usermode hooks**.

> Offsets target client build `version-f5a60436d48947d3` (Oct 2026). Roblox
> rebuilds roughly weekly; see [Updating offsets](#updating-offsets).

## Features

**ESP** (`src/ESP.cpp`)
- Corner boxes, player names, health bars, distance readout, snaplines
- Team filter and max-distance culling

**Aimbot** (`src/Aimbot.cpp`)
- Closest-to-crosshair target selection inside a configurable FOV circle
- Bone selection: Head / UpperTorso / HumanoidRootPart
- Adjustable smoothing, rebindable aim key, team check

**Silent aim** (`src/Aimbot.cpp`)
- On left-click, camera euler angles are pointed at the target for a single
  frame and then restored — your view never visibly snaps

**Movement** (`src/Movement.cpp`)
- **Fly** — camera-relative movement (WASD + Space/Shift), velocity zeroed to
  avoid rubber-banding
- **Noclip** — clears the `CanCollide` flag on every character part
- **Speed** — WalkSpeed writes · **Infinite jump** — upward velocity impulse

**Overlay & menu** (`src/Overlay.cpp`, `src/Menu.cpp`)
- Transparent fullscreen DX11 overlay, click-through when the menu is hidden
- In-game ImGui menu (`INSERT`) with per-feature tabs and keybind rebinding

## Preview

_Add a screenshot of the overlay here: `assets/preview.png`_

## Project structure

```
rbx-external/
├── src/
│   ├── main.cpp        # entry point, attach + cache thread + render loop
│   ├── Memory.h/.cpp   # process attach, RPM/WPM via ntdll
│   ├── SDK.h           # instance traversal, entity cache, world-to-screen
│   ├── Offsets.hpp     # curated offsets (see below)
│   ├── ESP.h/.cpp      # ESP rendering
│   ├── Aimbot.h/.cpp   # aimbot + silent aim
│   ├── Movement.h/.cpp # fly, noclip, speed, infinite jump
│   ├── Overlay.h/.cpp  # transparent DX11 overlay
│   ├── Menu.h/.cpp     # ImGui menu
│   └── Config.h        # every tunable in one place
├── thirdparty/         # place Dear ImGui here (see build steps)
└── CMakeLists.txt
```

## Building

Requirements: **Windows 10/11 x64**, Visual Studio 2022 (MSVC), CMake 3.20+.

```powershell
# 1. Clone Dear ImGui (ocornut/imgui) into thirdparty/imgui
# 2. Configure & build (x64 Release)
cmake -S . -B build -A x64
cmake --build build --config Release

# 3. Run AFTER the Roblox client is open
.\build\Release\rbx_external.exe
```

Default keybinds:

| Action        | Key             |
|---------------|-----------------|
| Menu          | `INSERT`        |
| Aimbot        | Mouse X2 (side) |
| Fly toggle    | `F`             |
| Noclip toggle | `N`             |

All keybinds can be rebound from the in-game menu.

## Updating offsets

Offsets live in `src/Offsets.hpp`, originally curated from the
`Offsetmanager/Roblox-Internal-External-Offsets` repository on GitHub
(`Windows/version-f5a60436d48947d3/External_Offsets.hpp`).

When the client updates and things stop resolving:

1. Open that repo and take the newest `Windows/version-*` folder.
2. Diff its `External_Offsets.hpp` against `src/Offsets.hpp` — namespace and
   member names are identical, so this is mechanical.
3. Replace the values, rebuild. Nothing else in the codebase hardcodes offsets.

Other live sources: the `4anti/Roblox-Offsets-ALL` endpoint mirrors and the
imtheo offset endpoints (stable + dev).

## Notes

- **Byfron (Hyperion)** strips usermode handles to `RobloxPlayerBeta.exe`, so
  `OpenProcess`-based attach fails on a stock system. The intended integration
  point is `Memory::ReadRaw` / `Memory::WriteRaw` — route those through your
  own kernel read/write primitive (e.g. a mapped driver exposing RPM IOCTLs)
  and everything above keeps working unchanged.
- World-to-screen uses the view matrix at `VisualEngine + 0x180`, re-read on
  every resolve tick (500 ms). The entity cache refreshes on a background
  thread; the render thread never blocks on a read.
- Silent aim writes `Camera + 0xD8` (euler pitch/yaw). If a future build
  changes the camera rotation layout, silent aim is the first thing that
  breaks — the mouse-movement aimbot is unaffected.

## Disclaimer

For educational and research purposes. Cheating violates the Roblox Terms of
Use and can get your account banned — use at your own risk, preferably on
alternate accounts. The authors are not responsible for any consequences.

## License

MIT — see [LICENSE](LICENSE).
