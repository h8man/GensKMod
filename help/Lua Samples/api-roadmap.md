# Lua API Integration Roadmap

This document compares the Lua modules registered by Gens Re-Recording with a proposed, phased implementation for GensKMod. The intent is to reuse current Gens interfaces and keep KMod-specific implementation in `src/Gens/kmod`; do not modify Gens core for the initial slice.

GensKMod currently registers only `kmod.log` in `lua_bind.c`. It creates one Lua state, opens the standard libraries with `luaL_openlibs`, and advances scripts through the existing KMod update/lifecycle flow. The GensKMod Lua library is Lua 5.5.1. The Re-Recording source uses older registration APIs such as `luaL_register`, so its module names and behavior are compatibility references—not code to copy verbatim.

## Module comparison

| Module | Gens Re-Recording API examples | Proposed GensKMod API | Gens-core changes for proposed scope? |
|---|---|---|---|
| `emu` / `gens` | `frameadvance`, `speedmode`, `wait`, `pause`, `unpause`, `emulateframe`, `emulateframefast`, `emulateframeinvisible`, `redraw`, `framecount`, `lagcount`, `lagged`, `emulating`, `atframeboundary`, frame/start/exit callbacks, persistent globals, messages, script and ROM loading | **First:** `framecount` in both aliases. KMod additions: `ispaused`, `getromsize`, `getsystems` (Genesis, Sega CD, and 32X active flags). Defer frame control, callbacks, ROM loading, and script persistence. | No for the proposed read-only metadata; control and callback functions require more lifecycle/frame-loop analysis. |
| `gui` | `register`, `text`, `box`, `line`, `pixel`, `getpixel`, opacity/transparency, `popup`, `parsecolor`, screenshot/overlay functions, `redraw`; drawing aliases such as `drawtext`, `drawbox`, `drawline`, `drawpixel` | Later: consider a small overlay/drawing API after confirming a KMod-owned render hook. | Not established; overlay drawing may require integration with the Gens render path. |
| `savestate` | `create`, `save`, `load`, script-data load/save, `registersave`, `registerload` | Later: evaluate state operations and script-data persistence against existing Gens/KMod state handling. | Not established for parity; defer until behavior and lifecycle are checked. |
| `memory` | `readbyte`, `readbytesigned`, `readword`, `readwordsigned`, `readdword`, `readdwordsigned`, `readbyterange`; corresponding write functions; `isvalid`; CPU register access; read/write/execute hooks and aliases | **First:** read-only `readbyte`, `readbytesigned`, `readword`, `readwordsigned`, `readdword`, `readdwordsigned`, `readbyterange`, and `isvalid`. Initially validate and limit access to ROM and main 68K RAM. Defer writes, CPU registers, and hooks. | No for bounded wrappers using existing accessors; no Gens-side hooks or new core interfaces in the first slice. |
| `joypad` | `get`, `getdown`, `getup`, `peek`, `peekdown`, `peekup`, `set`; aliases `read`, `write`, `readdown`, `readup` | Later: inspect current input state for read-only observation; defer input injection until frame timing and playback interactions are clear. | `set` and reliable frame-synchronized behavior may require core input integration. |
| `input` | `get`, `registerhotkey`, `popup`, `prompt`; alias `read` | Later: assess host-input reads and UI functions separately from game/joypad input. | Not established; defer. |
| `movie` | `active`, `recording`, `playing`, `mode`, `length`, `name`, rerecord count and controls, read-only state, frame count, `play`, `replay`, `stop`; aliases `open`, `close`, `playback` | Later: compare each operation with KMod GMV playback/recording. Do not promise Re-Recording movie compatibility based only on the existence of GMV controls. | Partial mapping may be possible, but full behavioral/format parity is not established. |
| `sound` | `get`, `clear` | Later: check for safe existing sound controls before exposing them. | Not established; defer. |
| `bit` | `tobit`, `bnot`, `band`, `bor`, `bxor`, shifts, rotates, `bswap`, `tohex` | Later: offer a compatibility `bit` table if scripts need it. Consider old globals `AND`, `OR`, `XOR`, `SHIFT`, and `BIT` only as compatibility shims. | No; Lua 5.5 already provides native bitwise operators, though its operators are not a drop-in replacement for every LuaBitOp function. |
| `vdp` | `readcell`, `writecell`, `readpalette`, `writepalette`, `readwordvram`, `readwordvsram` | Later: start with read-only operations after confirming address, format, and timing semantics against the current VDP state. | No new Gens interfaces may be needed for some reads, but semantics must be checked; defer writes. |
| `tile` | `new`/`create`, `getpixel`/`readpixel`, `setpixel`/`writepixel`, `rawdata` | Later: consider as a self-contained Lua utility module. | Likely KMod-only; not part of the first slice. |
| `palette` / `pal` | `new`/`create`, `getcolor`/`readcolor`, `setcolor`/`writecolor`, `rawdata` | Later: consider as a self-contained utility module, with `pal` as an alias if compatibility requires it. | Likely KMod-only; not part of the first slice. |
| Standard libraries | Re-Recording explicitly lists base, package, table, I/O, OS, string, math, and debug | No new work: GensKMod calls `luaL_openlibs`; its Lua 5.5 library set also includes coroutine and UTF-8 libraries. | No. |
| Global helpers | `print`, `tostring`, `addressof`, `copytable`; legacy bit globals | `print` and `tostring` already come from Lua's base library. Consider `copytable` later. Defer `addressof` because it exposes pointers. | No for a KMod-local helper; `addressof` is intentionally excluded. |
| `kmod` (KMod-specific) | No direct Re-Recording equivalent | Existing `kmod.log` remains available alongside compatibility modules. | No. |

## Initial implementation slice

Keep the first slice read-only and KMod-local:

- `emu.framecount()` and `gens.framecount()` read the existing frame counter.
- `emu.ispaused()` / `gens.ispaused()` report the existing pause state.
- `emu.getromsize()` / `gens.getromsize()` report the current ROM size.
- `emu.getsystems()` / `gens.getsystems()` report Genesis, Sega CD, and 32X active flags separately; these flags can describe combined hardware configurations.
- `memory.isvalid(address[, size])` validates only the initially supported ROM and main 68K RAM regions.
- `memory.readbyte(address)`, `readword(address)`, and `readdword(address)`, their signed forms, and `readbyterange(address, length)` perform bounded reads only within those supported regions.

Implement the `emu` and legacy `gens` names as aliases over one shared function set. Check that the entire requested width/range lies within one supported region before reading; reject invalid addresses and overflow rather than wrapping. Use existing Gens 68K read accessors only after confirming they behave safely for the whitelisted ROM/RAM ranges. Do not expose mapped I/O or VDP addresses in this slice, because reads there may have hardware side effects.

## Deferred work and risks

- Do not include memory writes, CPU register mutation, or read/write/execute hooks in the first slice. Hooks can run on performance-critical emulator paths, and write behavior can have hardware side effects.
- Do not include `frameadvance`, `wait`, speed controls, frame callbacks, or multi-script scheduling. GensKMod currently has one Lua state/coroutine and a KMod update path; matching Re-Recording's callback and scheduling model needs separate design.
- Do not claim GMV is equivalent to Re-Recording movie files or semantics without checking format and behavior.
- Treat savestate callbacks/persistent script data, GUI overlays, joypad injection, host-input UI, and VDP writes as later investigations.
- Preserve the existing KMod init/reset/close lifecycle. Keep KMod-specific implementation in `src/Gens/kmod` wherever possible.
- Use Lua 5.5-compatible registration APIs; do not copy the old `luaL_register` calls directly.

## Validation for a future implementation

- Confirm Lua scripts can call the shared `emu` and `gens` metadata functions and receive consistent values.
- Test reads at region starts/ends, invalid addresses, unsupported regions, negative sizes, and address-plus-size overflow.
- Confirm scripts cannot write memory or access mapped I/O through the initial API.
- Build and run the existing GensKMod configurations without modifying Gens-core or project files.