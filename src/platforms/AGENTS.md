# AGENTS.md - src/platforms/

## OVERVIEW
Platform backend implementations for raylib core (window, input, timing).

## STRUCTURE
src/platforms/
├── rcore_desktop_glfw.c
├── rcore_desktop_sdl.c
├── rcore_desktop_win32.c
├── rcore_desktop_rgfw.c
├── rcore_web.c
├── rcore_web_emscripten.c
├── rcore_android.c
├── rcore_drm.c
├── rcore_memory.c
└── rcore_template.c

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| New platform skeleton | src/platforms/rcore_template.c | Full stubbed interface |
| Default desktop | src/platforms/rcore_desktop_glfw.c | Primary backend |
| Windows native | src/platforms/rcore_desktop_win32.c | Win32 API |
| Web | src/platforms/rcore_web.c | GLFW + web glue |
| Emscripten alt | src/platforms/rcore_web_emscripten.c | Direct Emscripten path |
| Android | src/platforms/rcore_android.c | NDK glue |
| DRM/KMS | src/platforms/rcore_drm.c | Raspberry Pi/DRM |

## CONVENTIONS
- Keep PlatformData struct + static platform instance pattern
- Implement InitPlatform/ClosePlatform pair
- Use TRACELOG(LOG_WARNING, "...not available...") for unsupported calls

## ANTI-PATTERNS
- Removing platform-specific warning comments
- Introducing platform headers into shared modules
