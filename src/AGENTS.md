# AGENTS.md - src/

## OVERVIEW
Implementation modules and platform backends for raylib.

## STRUCTURE
src/
├── rcore.c
├── rshapes.c
├── rtextures.c
├── rtext.c
├── rmodels.c
├── raudio.c
├── utils.c
├── raylib.h
├── rlgl.h
├── config.h
├── platforms/
└── external/

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Public API declarations | src/raylib.h | Keep ordered by module |
| GL abstraction | src/rlgl.h | Header-only implementation |
| Feature flags | src/config.h | SUPPORT_* groups |
| Platform backends | src/platforms/ | See src/platforms/AGENTS.md |
| Vendored libs | src/external/ | See src/external/AGENTS.md |

## CONVENTIONS
- New public APIs: declare in src/raylib.h with RLAPI, implement in module file
- Guard optional features with SUPPORT_* in src/config.h
- Keep public function declarations single-line (rlparser constraint)

## ANTI-PATTERNS
- Adding public APIs without updating src/raylib.h
- Bypassing SUPPORT_* guards for optional features
- Editing src/external directly (see src/external/AGENTS.md)
