# PROJECT KNOWLEDGE BASE

Generated: 2026-01-04 02:31:56 +08
Commit: 9fe51a61
Branch: master

## OVERVIEW
raylib is a C99 game-programming library focused on simplicity and teaching.
Zero external runtime dependencies via vendored libraries in src/external/.

## STRUCTURE
raylib/
├── src/                 # Library sources + public headers
├── examples/            # Example programs (primary verification)
├── tools/               # Support tools (rexm, rlparser)
├── projects/            # IDE templates + build scripts
├── cmake/               # CMake modules
└── logo/                # Branding assets

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Public API | src/raylib.h | Declarations grouped by module |
| Core window/input/timing | src/rcore.c | Platform abstraction + loop helpers |
| 2D shapes | src/rshapes.c | Drawing + collisions |
| Textures/images | src/rtextures.c | Loading/processing |
| Fonts/text | src/rtext.c | Glyph atlas + text draw |
| Models/3D | src/rmodels.c | Mesh/model loaders |
| Audio | src/raudio.c | Built on miniaudio |
| GL abstraction | src/rlgl.h | Header-only with RLGL_IMPLEMENTATION |
| Build flags | src/config.h | SUPPORT_* feature gates |
| Example index | examples/README.md | Category tables |

## CODE MAP
| Symbol | Type | Location | Role |
|--------|------|----------|------|
| InitWindow | Function | src/raylib.h | Public window init API |
| InitWindow | Function | src/rcore.c | Window/platform setup |
| LoadTexture | Function | src/rtextures.c | Texture loading |
| DrawText | Function | src/rtext.c | Text rendering |
| LoadModel | Function | src/rmodels.c | Model loading |
| PlaySound | Function | src/raudio.c | Sound playback |
| rlglInit | Function | src/rlgl.h | GL abstraction init |

## CONVENTIONS
- Indentation: 4 spaces; Allman braces; no tabs
- Naming: TitleCase functions, lowerCase vars, ALL_CAPS macros
- Operators: no spaces around * and /
- Comments: before code, capitalized, no trailing period
- Floats: use x.xf literals

## ANTI-PATTERNS (THIS PROJECT)
- Editing vendored code in src/external without strong reason
- Multi-line public function declarations in src/raylib.h (breaks rlparser)
- Uninitialized variables
- Tabs or inline end-of-line comments with periods

## UNIQUE STYLES
- Examples are the primary verification path (no unit tests)
- rlgl is a single-header implementation guarded by RLGL_IMPLEMENTATION
- Feature gates live in src/config.h (SUPPORT_*)

## COMMANDS
```bash
cd src && make
cd examples && make core/core_basic_window
cmake -B build -S . && cmake --build build
```

## NOTES
- src/external contains vendored libs; GLFW is locally modified for unity build
- See subdirectory AGENTS.md files for deeper guidance
