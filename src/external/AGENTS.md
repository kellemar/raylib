# AGENTS.md - src/external/

## OVERVIEW
Vendored third-party libraries used to keep raylib dependency-free.

## STRUCTURE
src/external/
├── glfw/
├── stb_*.h / stb_vorbis.c
├── dr_*.h
├── miniaudio.h
├── cgltf.h
├── par_shapes.h
└── tinyobj_loader_c.h

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Window/input | src/external/glfw/ | Locally modified for unity build |
| Audio | src/external/miniaudio.h | Core audio backend |
| Audio decoders | src/external/dr_*.h | WAV/FLAC/MP3/QOA |
| Images | src/external/stb_image.h | Image loader |
| Fonts | src/external/stb_truetype.h | Not safe for untrusted fonts |
| Models | src/external/cgltf.h | glTF loader |

## CONVENTIONS
- Preserve upstream style and license headers
- Treat files as read-only unless integration requires changes

## ANTI-PATTERNS
- Refactoring or reformatting vendored files
- Removing license blocks
