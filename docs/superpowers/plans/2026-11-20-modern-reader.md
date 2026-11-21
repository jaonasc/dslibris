# Modern Reader Implementation Plan

> Execute inline using superpowers:executing-plans. User authorized implementation directly.

**Goal:** Deliver a modified DS executable with modern menus, reading themes and optional animated page turns.

**Architecture:** Portable RGB15 drawing/theme/transition helpers shared by the native interface and host tests. Integrate palettes into FreeType text, settings into XML and animation into one page-turn entry point.

**Tech Stack:** C++, libnds, FreeType, Expat, devkitARM.

**Spec:** ../specs/2026-11-20-modern-reader-design.md

## Global Constraints
- Preserve original executable, book directory and fonts.
- Native DS/DSi code; no web UI in place of the application.
- At most two temporary 256x256 frame buffers per turn; immediate fallback.
- Animation can be disabled; old XML preferences remain accepted.

## Review Focus
- No books: no indexing empty vectors.
- Long Unicode titles: clip without splitting multibyte text or overrunning buffers.
- Theme survives restart and works in both screens.
- Backward turns, first/last pages and disabled animation.
- Framebuffer stride is 256 while the logical viewport is 192x256.

## Tasks
- [x] 1. Add host tests for RGB15 alpha endpoints/contrast, clipped rounded cards with canaries, forward/backward transition endpoints and row padding. Run RED, implement include/reader_visual.h, run GREEN.
- [x] 2. Integrate Text palette rendering, fix framebuffer sizing and title width traversal, persist theme/animation in prefs.cpp and main.cpp. Compile and run host suite.
- [x] 3. Modernize Button, library, companion screen and paginated settings; retain keyboard and touch navigation, font selection, theme preview, animation toggle. Compile.
- [x] 4. Route touch and buttons through TurnPage(int); render new pages to temporary buffers, animate both screens with original restoration, handle boundaries and allocation failure. Run host suite and ARM build.
- [x] 5. Review diff, generate native drawing previews and package new executable with fonts, example books, instructions and source. Record console verification limitation.
