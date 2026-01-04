# AGENTS.md - projects/

## OVERVIEW
IDE-specific project scaffolds and configuration bundles.

## STRUCTURE
projects/
├── scripts/
├── CMake/
├── VS2022/
├── VS2019-Android/
├── VSCode/
├── CodeBlocks/
├── Geany/
├── Notepad++/
├── SublimeText/
├── 4coder/
└── Builder/

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Template index | projects/README.md | IDE support matrix |
| Build scripts | projects/scripts/ | Shell/batch templates |
| VS solution | projects/VS2022/ | See projects/VS2022/AGENTS.md |
| CMake template | projects/CMake/ | Minimal starter |

## CONVENTIONS
- Templates follow the standard raylib main.c structure
- Keep platform paths configurable (RAYLIB_PATH / RAYLIB_SRC)

## ANTI-PATTERNS
- Adding a new template without a README.md
- Hand-editing generated example projects instead of using rexm
