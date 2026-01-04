# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

raylib is a simple C99 library for videogames programming. Written with no external dependencies (all libraries embedded in `src/external/`). Uses PascalCase/camelCase notation. Licensed under zlib/libpng.

## Build Commands

### Library (from src/)
```bash
make                              # Build static library
make RAYLIB_LIBTYPE=SHARED        # Build shared library
make RAYLIB_BUILD_MODE=DEBUG      # Debug build
make clean                        # Clean artifacts
```

### Examples (from examples/)
```bash
make                              # Build all examples
make core                         # Build all core examples
make core/core_basic_window       # Build single example
```

### CMake (from root)
```bash
cmake -B build -S . -DBUILD_EXAMPLES=ON
cmake --build build
```

### Zig Build
```bash
zig build                         # Build library
zig build examples                # Build all examples
```

## Testing/Verification

No formal test suite. **Examples serve as tests**. To verify changes:
```bash
cd src && make clean && make
cd ../examples && make core/core_basic_window && ./core/core_basic_window
```

## Code Style

- **Functions**: TitleCase (`InitWindow`, `LoadTexture`)
- **Variables**: lowerCase (`screenWidth`, `playerPos`)
- **Defines/Macros**: ALL_CAPS (`PLATFORM_DESKTOP`, `MAX_VALUE`)
- **Indentation**: 4 spaces (no tabs)
- **Braces**: Allman style (open brace on new line)
- **Operators**: No spaces for `*` and `/`, spaces for `+` and `-`
- **Floats**: Always `x.xf` format (`10.0f` not `10.f`)
- **Comments**: Before code, capital letter, no trailing period
- **Variables**: Always initialize (`int x = 0;`, `Vector2 pos = { 0 };`)
- **Pointers**: Asterisk with type (`MyType *pointer;`)

## Architecture

```
src/
├── rcore.c          # Core: window, input, timing
├── rshapes.c        # 2D shape drawing
├── rtextures.c      # Image/texture management
├── rtext.c          # Font/text rendering
├── rmodels.c        # 3D models
├── raudio.c         # Audio playback
├── utils.c          # Internal utilities
├── raylib.h         # Main public API
├── raymath.h        # Math operations (Vector, Matrix, Quaternion)
├── rlgl.h           # OpenGL abstraction layer
├── rcamera.h        # Camera system
├── config.h         # Build configuration flags
└── platforms/       # Platform implementations (GLFW, SDL, RGFW, Web, Android, DRM)
```

## Key Patterns

### Standard Program Structure
```c
#include "raylib.h"

int main(void)
{
    InitWindow(800, 450, "title");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```

### Adding New API Functions
1. Declare in `raylib.h` with `RLAPI` prefix
2. Implement in appropriate module (`rcore.c`, `rshapes.c`, etc.)
3. Add example demonstrating usage

## Configuration

Build-time flags in `src/config.h`:
- `SUPPORT_MODULE_*` - Enable/disable modules
- `SUPPORT_FILEFORMAT_*` - File format support
- Various `SUPPORT_*` feature flags

CMake options in `CMakeOptions.txt`:
- `PLATFORM`: Desktop, Web, Android, DRM, SDL, RGFW
- `OPENGL_VERSION`: OFF, 4.3, 3.3, 2.1, 1.1, ES 2.0, ES 3.0
- `BUILD_EXAMPLES`, `BUILD_SHARED_LIBS`, `CUSTOMIZE_BUILD`
