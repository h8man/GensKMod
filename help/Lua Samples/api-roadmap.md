# Lua API Integration Roadmap

This document compares Gens Re-Recording's Lua modules with the current GensKMod implementation. KMod-specific bindings live in `src/Gens/kmod`; the Re-Recording source is a behavioral reference, not code to copy verbatim.

GensKMod registers `emu` and legacy `gens` as aliases to one shared table, plus a separate `kmod` table. The `emu`/`gens` table currently provides `framecount`, `ispaused`, `getromsize`, and `getsystems`; both API tables expose signature metadata. `kmod.log` is also registered with its `(message)` signature. Lua state creation opens the standard libraries with `luaL_openlibs`, and scripts run through the existing KMod update/lifecycle flow. GensKMod uses Lua 5.5.1. The Re-Recording source uses older registration APIs such as `luaL_register`, so its module names and behavior are compatibility references—not code to copy verbatim.

## Module comparison

| Module | Gens Re-Recording status and functions | GensKMod status and functions | Gens-core changes needed? |
|---|---|---|---|
| `emu` / `gens` | **Implemented:** `frameadvance`, speed controls, wait/pause controls, frame emulation/redraw/count and lag queries, frame boundary queries, callbacks, persistent globals, messages, script/ROM loading. | **Implemented:** shared `emu`/`gens` aliases with `framecount`, `ispaused`, `getromsize`, and `getsystems`; signatures are in `.signatures`. Other Re-Recording methods are not implemented. | No for current metadata methods; control and callbacks require more lifecycle/frame-loop analysis. |
| `gui` | **Implemented:** register, text/box/line/pixel drawing and reads, opacity/transparency, popup, color parsing, screenshot/overlay, redraw, plus drawing aliases. | **Not implemented.** Possible later work: KMod-local drawing after confirming an appropriate render hook. | Overlay drawing may require Gens render-path integration. |
| `savestate` | **Implemented:** create/save/load, script-data save/load, save/load callbacks. | **Not implemented.** Existing Gens state handling needs investigation before defining Lua behavior. | Unclear for full parity; assess existing interfaces and lifecycle. |
| `memory` | **Implemented:** signed/unsigned byte/word/dword reads, byte-range reads, writes, validity checks, CPU registers, read/write/execute hooks and aliases. | **Implemented (read-only subset):** signed/unsigned byte/word/dword reads, byte-range reads capped at 64 KiB per call, and `isvalid`; supports ROM, 68K RAM, Z80 RAM, active Sega CD PRG/word RAM, and active 32X RAM. Writes, CPU registers, and hooks are not exposed. | No new core API needed for bounded reads; hooks or writes need separate review. |
| `joypad` | **Implemented:** get/down/up, peek/down/up, set, and read/write aliases. | **Implemented (read-only subset):** `joypad.get(player)` returns button booleans from emulated state; players 1–2 are available, and extra Team Player slots depend on port configuration. Input injection is not exposed. | No for state reads; input injection may require core input integration. |
| `input` | **Implemented:** host input get, hotkey registration, popup, prompt, and read alias. | **Not implemented.** Host-input and UI access need separate assessment. | Unclear; defer. |
| `movie` | **Implemented:** active/recording/playing/mode/length/name, rerecord count and controls, read-only state, frame count, play/replay/stop and aliases. | **Not implemented as a Lua module.** KMod has GMV record/playback features, but equivalence is unverified. | Partial mapping may be possible; full format/behavior parity is unestablished. |
| `sound` | **Implemented:** `get`, `clear`. | **Not implemented.** Check for safe existing sound controls before adding bindings. | Unclear; defer. |
| `bit` | **Implemented:** LuaBitOp-compatible conversion, boolean bitwise operations, shifts, rotates, byte swap, hex formatting. | **No compatibility module planned for this slice.** Use Lua 5.5 native integer operators (`&`, `|`, `~`, `<<`, `>>`); legacy global aliases are absent. `help/Lua Samples/test_bitops.lua` exercises the native operators. | No core change needed for a KMod-local compatibility library if later required. |
| `vdp` | **Implemented:** read/write cell, palette, VRAM/VSRAM word access. | **Implemented:** `readcell`/`writecell` transfer tile userdata at bounded pattern-data addresses; `readpalette`/`writepalette` transfer palette userdata and lists; `readwordvram` and `readwordvsram` read bounded words. Requires a running Genesis VDP. | No new core API; reuses VDP memory arrays and refresh flags. Cell/CRAM writes directly mutate emulated video memory. |
| `tile` | **Implemented:** create, pixel read/write, raw data. | **Implemented:** script-owned 8×8 4-bpp tile userdata with pixel read/write and raw bytes; VDP transfers use this userdata. | No core changes. |
| `palette` / `pal` | **Implemented:** create, color read/write, raw data. | **Implemented:** script-owned 16-color Genesis-format palettes with Re-Recording aliases, plus read-only live CRAM color reads for hardware palette indices 0–3. | No core changes; live color reads reuse KMod's CRAM conversion helper. |
| Standard libraries | **Opened per the supplied list:** base, package, table, I/O, OS, string, math, debug. | **Implemented:** `luaL_openlibs` opens the Lua 5.5 standard libraries, including coroutine and UTF-8. | No. |
| Global helpers | **Implemented:** `print`, `tostring`, `addressof`, `copytable`, and legacy bit globals. | **Available:** Lua base `print` and `tostring`. **Not implemented:** Re-Recording-specific `addressof`, `copytable`, and legacy bit globals. | No for KMod-local helpers; `addressof` is intentionally excluded from the current plan. |
| `kmod` (KMod-specific) | No direct equivalent. | **Implemented:** `kmod.log(message)` with signature metadata in `kmod.signatures.log`. | No. |

## Deferred work and risks

- Memory writes, CPU register mutation, and read/write/execute hooks are not implemented. Hooks can run on performance-critical emulator paths, and write behavior can have hardware side effects.
- Do not include `frameadvance`, `wait`, speed controls, frame callbacks, or multi-script scheduling. GensKMod currently has one Lua state/coroutine and a KMod update path; matching Re-Recording's callback and scheduling model needs separate design.
- Do not claim GMV is equivalent to Re-Recording movie files or semantics without checking format and behavior.
- Treat savestate callbacks/persistent script data, GUI overlays, joypad injection, and host-input UI as later investigations. VDP transfer writes are implemented and directly mutate emulated video memory.
- Preserve the existing KMod init/reset/close lifecycle. Keep KMod-specific implementation in `src/Gens/kmod` wherever possible.
- Use Lua 5.5-compatible registration APIs; do not copy the old `luaL_register` calls directly.

## Samples and validation

- `help/Lua Samples/test_emu.lua` checks emulator method return types, system flags, signatures, and the `emu`/`gens` alias.
- `help/Lua Samples/test_memory.lua` checks memory signatures, region boundaries, signed byte values, unsupported addresses, capped ranges, and rejected reads.
- `help/Lua Samples/test_joypad.lua` checks `joypad.get` signature, button booleans, and invalid player rejection.
- `help/Lua Samples/test_palette.lua` checks palette creation, color conversion, transparency, raw bytes, aliases, and read-only CRAM bounds.
- `help/Lua Samples/test_tile.lua` checks tile creation, packed pixel access, raw bytes, aliases, and invalid input rejection; it does not exercise VDP/VRAM transfer.
- `help/Lua Samples/test_vdp.lua` checks VDP method signatures and address bounds, reads VRAM/VSRAM words and a cell at fixed address `0x0000`, exercises tile and palette userdata/list transfers, and restores the modified cell and CRAM line. Run it with an active Genesis ROM; live behavior has been smoke-tested in the emulator.
- `help/Lua Samples/test_bitops.lua` checks native Lua 5.5 integer bitwise operators.
- `help/Lua Samples/list_modules.lua` lists registered modules and their available signatures.
- The C++ workspace build succeeds. Lua samples are intended to be run in GensKMod's Lua Script window; only the samples explicitly noted above have runtime confirmation.
