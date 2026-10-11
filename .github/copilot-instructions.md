# Copilot Instructions

## Project Guidelines
- All changes related to KMod should go in the `kmod` folder, including KMod.rc and kmod/resource.h; avoid placing KMod-specific changes in files outside that folder when possible.
- Use KMod's existing lifecycle functions for kmod modules ownership and reset behavior; do not introduce a parallel/reset-specific KMod lifecycle entry point. Map module init/reset/close to the existing KMod init/reset/close flow.
