# Lua API Integration Roadmap

This document compares Gens Re-Recording's Lua API with GensKMod's Lua API. KMod bindings are implemented in `src/Gens/kmod`.

## Module comparison

| Module | Gens Re-Recording status and functions | GensKMod status and functions | Notes |
|---|---|---|---|
| `emu` / `gens` | **Implemented:** `frameadvance`, speed and pause controls, frame emulation and drawing, frame/lag queries, callbacks, persistent globals, messages, script/ROM loading. | `framecount`, `ispaused`, `pause`, `unpause`, `reset`, `getromsize`, `getromname`, `getromheader`, `getcountry`, `getframeskip`, `getvisiblelines`, `getsystems` | `emu` and `gens` are aliases. Pause methods directly set the pause flag; a paused script may require UI unpause. Reset acts immediately and returns false during Kaillera or when no system is active. Metadata methods return nil when unavailable. |
| `gui` | **Implemented:** register, drawing, drawing reads, opacity, popup, color parsing, screenshot, overlay, redraw. | — | Not implemented. |
| `savestate` | **Implemented:** create/save/load, script data and callbacks. | `save`, `load` | Saves and loads numbered file slots 0–9; no in-memory states or script data. |
| `memory` | **Implemented:** scalar/range reads and writes, validity checks, registers, hooks and aliases. | `isvalid`, `readbyte`, `readbytesigned`, `readword`, `readwordsigned`, `readdword`, `readdwordsigned`, `readbyterange`, `writebyte`, `writeword`, `writedword`, `writebyterange`, `getregister`, `setregister` | Mapped ROM/RAM only; ranges capped at 64 KiB. Register access supports main and Sega CD sub 68000s. Writes directly modify emulated state; hooks are not implemented. |
| `joypad` | **Implemented:** get/down/up, peek/down/up, set and aliases. | `get`, `set` | Reads standard and configured Team Player slots. `set` applies supplied buttons across frames until replaced; an empty table clears overrides. |
| `input` | **Implemented:** host input, hotkey registration, popup and prompt. | — | Not implemented. |
| `movie` | **Implemented:** status, metadata, controls, frame count and aliases. | — | Not implemented as a Lua module; GMV compatibility is unverified. |
| `sound` | **Implemented:** `get`, `clear`. | — | Not implemented. |
| `bit` | **Implemented:** conversion, bitwise operations, shifts, rotates, byte swap and formatting. | — | Use Lua 5.5 native integer bitwise operators; no compatibility module is provided. |
| `vdp` | **Implemented:** cell, palette, VRAM/VSRAM access. | `readcell`, `writecell`, `readpalette`, `writepalette`, `readwordvram`, `readwordvsram` | Requires an active Genesis VDP; writes directly modify video memory. |
| `tile` | **Implemented:** create, pixel read/write, raw data. | `create`, pixel read/write, raw data | Script-owned 8×8 4-bpp tile userdata. |
| `palette` / `pal` | **Implemented:** create, color read/write, raw data. | `create`, color read/write, raw data | Script-owned Genesis palettes; live CRAM reads are read-only. |
| Standard libraries | Base, package, table, I/O, OS, string, math, debug. | Base, package, table, I/O, OS, string, math, debug, coroutine, UTF-8 | Lua 5.5 standard libraries. |
| Global helpers | `print`, `tostring`, `addressof`, `copytable`, legacy bit globals. | `print`, `tostring` | No Re-Recording-specific helpers. |
| `kmod` | No direct equivalent. | `log` | KMod-specific logging. |

## Deferred work and risks

- Memory, register, and VDP writes modify emulated state; tests should restore changed data.
- Deferred: memory hooks, non-68000 registers, frame advancement and scheduling, callbacks, savestate script data, GUI overlays, host-input UI, and joypad injection beyond `set`.
