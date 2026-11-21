# DSLibris Modern

Requested: modern menus, icons, larger buttons, light/dark/yellow paper reading themes and a page-turn effect on Nintendo DS/DSi.

Build on the exact v1.5.2-2 source in a separate checkout. Keep native C++/libnds and the existing book orientation, fonts and storage format. The original executable stays untouched.

Use a warm neutral interface, teal accent, rounded cards, small drawn icons, four 38-pixel library rows and 28-pixel navigation controls. Settings use two pages, at most four 40-pixel rows per page. Labels are Portuguese. Show an uncluttered companion screen with branding and reading hints. Reading palettes: light, charcoal, black and yellow paper. Preserve text antialiasing by blending glyph coverage between the actual paper and ink colors. Paper is the new-install default; old invert preferences remain readable. Save theme and animation in XML.

Page turns use a short sliding sheet with a folded edge and shadow, with forward/back directions. This is a 2D approximation, not a full 3D curl. Allocate at most two full 256x256 temporary buffers on a turn, with an immediate-render fallback if allocation fails; do not animate at book boundaries or when disabled. Retain the original pagination.

Alternatives considered: binary patching cannot reasonably support these changes; a complete rewrite introduces unnecessary parser and compatibility risk. Modify the existing source and keep graphics lightweight. Full curved-page deformation is deferred until hardware performance can be measured.

Validation: portable C++ tests for theme contrast/blending, frame endpoints/directions, clipped drawing and guard-buffer safety; compile the ARM9 executable with a pinned official SDK. Produce previews using the same drawing primitives. Actual console testing remains necessary for frame pacing and flashcart compatibility.
