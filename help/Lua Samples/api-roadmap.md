# Lua API Integration Roadmap

This document compares Gens Re-Recording's Lua modules with the current GensKMod implementation. KMod-specific bindings live in `src/Gens/kmod`; the Re-Recording source is a behavioral reference, not code to copy verbatim.

GensKMod registers `emu` and legacy `gens` as aliases to one shared table, plus a separate `kmod` table. The `emu`/`gens` table currently provides `framecount`, `ispaused`, `getromsize`, and `getsystems`; both API tables expose signature metadata. `kmod.log` is also registered with its `(message)` signature. Lua state creation opens the standard libraries with `luaL_openlibs`, and scripts run through the existing KMod update/lifecycle flow. GensKMod uses Lua 5.5.1. The Re-Recording source uses older registration APIs such as `luaL_register`, so its module names and behavior are compatibility references—not code to copy verbatim.

## Module comparison

| Module | Gens Re-Recording status and functions | GensKMod status and functions | Gens-core changes needed? |
|---|---|---|---|
| `emu` / `gens` | **Implemented:** `frameadvance`, speed controls, wait/pause controls, frame emulation/redraw/count and lag queries, frame boundary queries, callbacks, persistent globals, messages, script/ROM loading. | `framecount`, `ispaused`, `getromsize`, `getsystems` | Shared `emu`/`gens` aliases; signatures are available in `.signatures`. Other Re-Recording methods are not implemented. Control and callbacks need lifecycle/frame-loop analysis. |
| `gui` | **Implemented:** register, text/box/line/pixel drawing and reads, opacity/transparency, popup, color parsing, screenshot/overlay, redraw, plus drawing aliases. | — | Not implemented. KMod-local drawing needs an appropriate render hook; overlay drawing may require Gens render-path integration. |
| `savestate` | **Implemented:** create/save/load, script-data save/load, save/load callbacks. | — | Not implemented. Existing Gens state handling and lifecycle need investigation before defining Lua behavior. |
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

## Samples and validation

- `help/Lua Samples/test_emu.lua` checks emulator method return types, system flags, signatures, and the `emu`/`gens` alias.
- `help/Lua Samples/test_memory.lua` checks read/write/register signatures and aliases, region boundaries, signed reads, scalar and byte-range RAM round trips, ROM write/read round trip with restoration, main and optional sub 68000 register round trips with restoration, and invalid inputs. Hooks are intentionally not tested or exposed.
- `help/Lua Samples/test_joypad.lua` checks `joypad.get`/`set` signatures, button booleans, queued partial-button overrides with restoration, and invalid player rejection.
- `help/Lua Samples/test_palette.lua` checks palette creation, color conversion, transparency, raw bytes, aliases, and read-only CRAM bounds.
- `help/Lua Samples/test_tile.lua` checks tile creation, packed pixel access, raw bytes, aliases, and invalid input rejection; it does not exercise VDP/VRAM transfer.
- `help/Lua Samples/test_vdp.lua` checks VDP method signatures and address bounds, reads VRAM/VSRAM words and a cell at fixed address `0x0000`, exercises tile and palette userdata/list transfers, and restores the modified cell and CRAM line. Run it with an active Genesis ROM; live behavior has been smoke-tested in the emulator.
- `help/Lua Samples/test_bitops.lua` checks native Lua 5.5 integer bitwise operators.
- `help/Lua Samples/list_modules.lua` lists registered modules and their available signatures.
- The C++ workspace build succeeds. Lua samples are intended to be run in GensKMod's Lua Script window; only the samples explicitly noted above have runtime confirmation.
