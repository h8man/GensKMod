# Lua API Integration Roadmap

This document compares the Lua modules registered by Gens Re-Recording with a proposed, phased implementation for GensKMod. The intent is to reuse current Gens interfaces and keep KMod-specific implementation in `src/Gens/kmod`; do not modify Gens core for the initial slice.

GensKMod registers `emu` and legacy `gens` as aliases to one shared table, plus a separate `kmod` table. The `emu`/`gens` table currently provides `framecount`, `ispaused`, `getromsize`, and `getsystems`; both API tables expose signature metadata. `kmod.log` is also registered with its `(message)` signature. Lua state creation opens the standard libraries with `luaL_openlibs`, and scripts run through the existing KMod update/lifecycle flow. GensKMod uses Lua 5.5.1. The Re-Recording source uses older registration APIs such as `luaL_register`, so its module names and behavior are compatibility references—not code to copy verbatim.

## Module comparison

| Module | Gens Re-Recording status and functions | GensKMod status and functions | Gens-core changes needed? |
|---|---|---|---|
| `emu` / `gens` | **Implemented:** `frameadvance`, speed controls, wait/pause controls, frame emulation/redraw/count and lag queries, frame boundary queries, callbacks, persistent globals, messages, script/ROM loading. | **Implemented:** shared `emu`/`gens` aliases with `framecount`, `ispaused`, `getromsize`, and `getsystems`; signatures are in `.signatures`. Other Re-Recording methods are not implemented. | No for current metadata methods; control and callbacks require more lifecycle/frame-loop analysis. |
| `gui` | **Implemented:** register, text/box/line/pixel drawing and reads, opacity/transparency, popup, color parsing, screenshot/overlay, redraw, plus drawing aliases. | **Not implemented.** Possible later work: KMod-local drawing after confirming an appropriate render hook. | Overlay drawing may require Gens render-path integration. |
| `savestate` | **Implemented:** create/save/load, script-data save/load, save/load callbacks. | **Not implemented.** Existing Gens state handling needs investigation before defining Lua behavior. | Unclear for full parity; assess existing interfaces and lifecycle. |
| `memory` | **Implemented:** signed/unsigned byte/word/dword reads, byte-range reads, writes, validity checks, CPU registers, read/write/execute hooks and aliases. | **Planned, not implemented:** bounded read-only byte/word/dword methods, signed variants, byte-range reads, and `isvalid`, initially limited to ROM/main 68K RAM. | No new core API expected for bounded reads; hooks or writes need separate review. |
| `joypad` | **Implemented:** get/down/up, peek/down/up, set, and read/write aliases. | **Not implemented.** Read-only input observation may be evaluated later; defer input injection until frame timing and playback behavior are clear. | Input injection may require core input integration. |
| `input` | **Implemented:** host input get, hotkey registration, popup, prompt, and read alias. | **Not implemented.** Host-input and UI access need separate assessment. | Unclear; defer. |
| `movie` | **Implemented:** active/recording/playing/mode/length/name, rerecord count and controls, read-only state, frame count, play/replay/stop and aliases. | **Not implemented as a Lua module.** KMod has GMV record/playback features, but equivalence is unverified. | Partial mapping may be possible; full format/behavior parity is unestablished. |
| `sound` | **Implemented:** `get`, `clear`. | **Not implemented.** Check for safe existing sound controls before adding bindings. | Unclear; defer. |
| `bit` | **Implemented:** LuaBitOp-compatible conversion, boolean bitwise operations, shifts, rotates, byte swap, hex formatting. | **Not implemented as a compatibility module.** Lua 5.5 has native bitwise operators; legacy global aliases are also absent. | No core change needed for a KMod-local compatibility library. |
| `vdp` | **Implemented:** read/write cell, palette, VRAM/VSRAM word access. | **Not implemented.** Consider read-only methods after confirming address/format/timing semantics. | Some reads may be KMod-local; writes require more review. |
| `tile` | **Implemented:** create, pixel read/write, raw data. | **Not implemented.** Could be a self-contained KMod utility module. | Likely no core changes. |
| `palette` / `pal` | **Implemented:** create, color read/write, raw data. | **Not implemented.** Could be a self-contained KMod utility module with `pal` alias. | Likely no core changes. |
| Standard libraries | **Opened per the supplied list:** base, package, table, I/O, OS, string, math, debug. | **Implemented:** `luaL_openlibs` opens the Lua 5.5 standard libraries, including coroutine and UTF-8. | No. |
| Global helpers | **Implemented:** `print`, `tostring`, `addressof`, `copytable`, and legacy bit globals. | **Available:** Lua base `print` and `tostring`. **Not implemented:** Re-Recording-specific `addressof`, `copytable`, and legacy bit globals. | No for KMod-local helpers; `addressof` is intentionally excluded from the current plan. |
| `kmod` (KMod-specific) | No direct equivalent. | **Implemented:** `kmod.log(message)` with signature metadata in `kmod.signatures.log`. | No. |

## Implemented API and remaining first slice

The initial emulator metadata slice is implemented in `src/Gens/kmod/lua_bind.c` and uses existing Gens state without Gens-core edits:

- `emu.framecount()` / `gens.framecount()` return the existing frame counter.
- `emu.ispaused()` / `gens.ispaused()` return the pause state.
- `emu.getromsize()` / `gens.getromsize()` return the current ROM size.
- `emu.getsystems()` / `gens.getsystems()` return the Genesis, Sega CD, and 32X active flags separately; combined configurations are possible.
- Signatures for these methods are available through `emu.signatures` and `gens.signatures`. `kmod.log(message)` is described by `kmod.signatures.log`.

The remaining first-slice work is the planned read-only `memory` API: `isvalid(address[, size])`, byte/word/dword reads and signed variants, and `readbyterange(address, length)`, initially restricted to validated ROM and main 68K RAM addresses.

Before adding memory methods, confirm the existing Gens 68K read accessors are safe for the whitelisted ROM/RAM ranges. Check that the entire requested width/range lies within one supported region before reading; reject invalid addresses and overflow rather than wrapping. Do not expose mapped I/O or VDP addresses in this slice, because reads there may have hardware side effects.

## Deferred work and risks

- Do not include memory writes, CPU register mutation, or read/write/execute hooks in the first slice. Hooks can run on performance-critical emulator paths, and write behavior can have hardware side effects.
- Do not include `frameadvance`, `wait`, speed controls, frame callbacks, or multi-script scheduling. GensKMod currently has one Lua state/coroutine and a KMod update path; matching Re-Recording's callback and scheduling model needs separate design.
- Do not claim GMV is equivalent to Re-Recording movie files or semantics without checking format and behavior.
- Treat savestate callbacks/persistent script data, GUI overlays, joypad injection, host-input UI, and VDP writes as later investigations.
- Preserve the existing KMod init/reset/close lifecycle. Keep KMod-specific implementation in `src/Gens/kmod` wherever possible.
- Use Lua 5.5-compatible registration APIs; do not copy the old `luaL_register` calls directly.

## Validation status and future checks

- `help/Lua Samples/test_emu.lua` exercises each implemented `emu` method and checks its return types, system flags, signatures, and alias relationship.
- `help/Lua Samples/list_modules.lua` dynamically lists global/module functions and prints available signatures.
- For future memory methods, test region boundaries, invalid/unsupported addresses, negative sizes, and address-plus-size overflow.
- Confirm future read-only memory methods cannot write memory or access mapped I/O.
- The C++ workspace build passed after adding the emulator metadata methods; no Gens-core or project files were modified.
