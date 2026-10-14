# Lua API Integration Roadmap

This document compares Gens Re-Recording's Lua modules with the current GensKMod implementation. KMod-specific bindings live in `src/Gens/kmod`; the Re-Recording source is a behavioral reference, not code to copy verbatim.

## Module comparison

| Module | Gens Re-Recording status and functions | GensKMod status and functions | Gens-core changes needed? |
|---|---|---|---|
| `emu` / `gens` | **Implemented:** `frameadvance`, speed controls, wait/pause controls, frame emulation/redraw/count and lag queries, frame boundary queries, callbacks, persistent globals, messages, script/ROM loading. | `framecount`, `ispaused`, `getromsize`, `getromname`, `getromheader`, `getcountry`, `getframeskip`, `getvisiblelines`, `getsystems` | Shared `emu`/`gens` aliases with signature metadata. ROM metadata methods return `nil` when unavailable; `getcountry` returns `usa_ntsc`, `japan_ntsc`, `europe_pal`, `asia_pal`, or nil. `getvisiblelines` returns nil when Genesis is inactive. Other Re-Recording methods are not implemented; controls and callbacks need lifecycle/frame-loop analysis. |
| `gui` | **Implemented:** register, text/box/line/pixel drawing and reads, opacity/transparency, popup, color parsing, screenshot/overlay, redraw, plus drawing aliases. | — | Not implemented. KMod-local drawing needs an appropriate render hook; overlay drawing may require Gens render-path integration. |
| `savestate` | **Implemented:** create/save/load, script-data save/load, save/load callbacks. | `save`, `load` | File-based quick state slots 0–9 using existing Gens state files. No in-memory state objects, script variables, or callbacks. |
| `memory` | **Implemented:** signed/unsigned byte/word/dword reads, byte-range reads/writes, scalar writes, validity checks, CPU registers, read/write/execute hooks and aliases. | `isvalid`, `readbyte`, `readbytesigned`, `readword`, `readwordsigned`, `readdword`, `readdwordsigned`, `readbyterange`, `writebyte`, `writeword`, `writedword`, `writebyterange`, `getregister`, `setregister` | Bounded to ROM, mapped 68K RAM, Z80 RAM, active Sega CD PRG/word RAM, and active 32X RAM. Range writes are capped at 64 KiB. Register access covers main and Sega CD sub 68000 A0–A7, D0–D7, PC, SR; `main.`/`m68k.` and `sub.`/`s68k.` prefixes are accepted, with unqualified names targeting main. Writes directly mutate memory/register state; hooks are not implemented. |
| `joypad` | **Implemented:** get/down/up, peek/down/up, set, and read/write aliases. | `get`, `set` | `get` reads players 1–2 and configured Team Player slots. `set(player, buttons)` applies the supplied partial button state across emulated frames until the next `set`; unlisted buttons retain polled input, and an empty table clears overrides for that player. |
| `input` | **Implemented:** host input get, hotkey registration, popup, prompt, and read alias. | — | Not implemented; host-input and UI access need separate assessment. |
| `movie` | **Implemented:** active/recording/playing/mode/length/name, rerecord count and controls, read-only state, frame count, play/replay/stop and aliases. | — | Not implemented as a Lua module. KMod has GMV record/playback features, but equivalence and full format/behavior parity are unverified. |
| `sound` | **Implemented:** `get`, `clear`. | — | Not implemented; check for safe existing sound controls before adding bindings. |
| `bit` | **Implemented:** LuaBitOp-compatible conversion, boolean bitwise operations, shifts, rotates, byte swap, hex formatting. | — | No compatibility module planned for this slice. Use Lua 5.5 native integer operators (`&`, `|`, `~`, `<<`, `>>`); legacy global aliases are absent. `help/Lua Samples/test_bitops.lua` exercises native operators. |
| `vdp` | **Implemented:** read/write cell, palette, VRAM/VSRAM word access. | `readcell`, `writecell`, `readpalette`, `writepalette`, `readwordvram`, `readwordvsram` | Requires a running Genesis VDP. Reuses VDP memory arrays and refresh flags; cell/CRAM writes directly mutate emulated video memory. |
| `tile` | **Implemented:** create, pixel read/write, raw data. | `create`, pixel read/write, raw data | Script-owned 8×8 4-bpp tile userdata; VDP transfers use this userdata. |
| `palette` / `pal` | **Implemented:** create, color read/write, raw data. | `create`, color read/write, raw data | Script-owned 16-color Genesis-format palettes with Re-Recording aliases; live CRAM color reads are read-only for hardware palette indices 0–3 and reuse KMod's CRAM conversion helper. |
| Standard libraries | **Opened per the supplied list:** base, package, table, I/O, OS, string, math, debug. | Base, package, table, I/O, OS, string, math, debug, coroutine, UTF-8 | Opened with Lua 5.5 `luaL_openlibs`. |
| Global helpers | **Implemented:** `print`, `tostring`, `addressof`, `copytable`, and legacy bit globals. | `print`, `tostring` | Lua base functions; Re-Recording-specific `addressof`, `copytable`, and legacy bit globals are absent. `addressof` is outside the current plan. |
| `kmod` (KMod-specific) | No direct equivalent. | `log` | `kmod.log(message)`; signature metadata is available in `kmod.signatures.log`. |

## Deferred work and risks

- Read/write/execute hooks and non-68000 CPU register access are not implemented. Hooks can run on performance-critical emulator paths. Memory scalar and byte-range writes directly modify mapped memory, including ROM; register setters directly mutate CPU state. Scripts should preserve and restore ROM bytes and registers when testing writes.
- Do not include `frameadvance`, `wait`, speed controls, frame callbacks, or multi-script scheduling. GensKMod currently has one Lua state/coroutine and a KMod update path; matching Re-Recording's callback and scheduling model needs separate design.
- Do not claim GMV is equivalent to Re-Recording movie files or semantics without checking format and behavior.
- Treat savestate callbacks/persistent script data, GUI overlays, and host-input UI as later investigations. Joypad overrides are applied during KMod frame input preparation after GMV playback, so scripted input takes precedence for the specified buttons. VDP transfer writes are implemented and directly mutate emulated video memory.
- Preserve the existing KMod init/reset/close lifecycle. Keep KMod-specific implementation in `src/Gens/kmod` wherever possible.
- Use Lua 5.5-compatible registration APIs; do not copy the old `luaL_register` calls directly.

## Validation

- `help/Lua Samples/test_emu.lua` checks emulator method return types, system flags, signatures, the `emu`/`gens` alias, and read-only query coverage for ROM metadata, country mapping, frame skip, and visible line count.
