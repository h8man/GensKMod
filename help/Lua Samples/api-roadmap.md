# Lua API Integration Roadmap

This document compares Gens Re-Recording's Lua API with the API exposed by GensKMod. KMod bindings are implemented in `src/Gens/kmod`. A dash (`—`) means the feature is not exposed by GensKMod.

## Module comparison

| Module | Gens Re-Recording status and functions | GensKMod status and functions | Notes |
|---|---|---|---|
| `emu` / `gens` | **Implemented:** `frameadvance`, speed and pause controls, frame emulation and drawing, frame/lag queries, callbacks, persistent globals, messages, script/ROM loading. | `framecount`, `ispaused`, `pause`, `unpause`, `reset`, `getromsize`, `getromname`, `getromheader`, `getcountry`, `getframeskip`, `getvisiblelines`, `getsystems`, `loadrom` | Aliases. Supports no-ROM startup; synchronous ROM loads preserve the Lua session. Pause and reset act immediately; ROM metadata is unavailable without a ROM. |
| `gui` | **Implemented:** register, drawing, drawing reads, opacity, popup, color parsing, screenshot, overlay, redraw. | — | Not implemented. |
| `savestate` | **Implemented:** create/save/load, script data and callbacks. | `save`, `load` | Active system required; numbered file slots only. |
| `memory` | **Implemented:** scalar/range reads and writes, validity checks, registers, hooks and aliases. | `isvalid`, `readbyte`, `readbytesigned`, `readword`, `readwordsigned`, `readdword`, `readdwordsigned`, `readbyterange`, `writebyte`, `writeword`, `writedword`, `writebyterange`, `getregister`, `setregister` | Active system required. Accesses mapped ROM/RAM and main or Sega CD sub-68000 registers; writes are direct. Hooks are not implemented. |
| `joypad` | **Implemented:** get/down/up, peek/down/up, set and aliases. | `get`, `set` | Active system required. Supports Team Player slots; button overrides persist until changed or cleared. |
| `input` | **Implemented:** host input, hotkey registration, popup and prompt. | — | Not implemented. |
| `movie` | **Implemented:** status, metadata, controls, frame count and aliases. | — | No Lua module; GMV compatibility is unverified. |
| `sound` | **Implemented:** `get`, `clear`. | — | Not implemented. |
| `bit` | **Implemented:** conversion, bitwise operations, shifts, rotates, byte swap and formatting. | — | Use Lua 5.5's native bitwise operators. |
| `vdp` | **Implemented:** cell, palette, VRAM/VSRAM access. | `readcell`, `writecell`, `readpalette`, `writepalette`, `readwordvram`, `readwordvsram` | Genesis required; writes are direct. |
| `tile` | **Implemented:** create, pixel read/write, raw data. | `create`, pixel read/write, raw data | Script-owned 8×8 4-bpp tile userdata. |
| `palette` / `pal` | **Implemented:** create, color read/write, raw data. | `create`, color read/write, raw data | Script-owned palettes; live CRAM reads are read-only and require Genesis. |
| Standard libraries | Base, package, table, I/O, OS, string, math, debug. | Base, package, table, I/O, OS, string, math, debug, coroutine, UTF-8 | Lua 5.5 standard libraries. |
| Global helpers | `print`, `tostring`, `addressof`, `copytable`, legacy bit globals. | `print`, `tostring` | No Re-Recording-specific helpers. |
| `kmod` | No direct equivalent. | `log`, `clearlog`, `should_stop`, `set_window_visible` | `set_window_visible(name, visible)` controls a named window; system-specific windows require an active ROM to show. See `test_kmod_window_visibility.lua` for names and usage. |

## Deferred work and risks

- Tests should restore any memory, register, or VDP state they modify.
- Deferred: memory hooks, non-68000 register access, frame scheduling, callbacks, savestate script data, GUI overlays, host input, and expanded joypad injection.
