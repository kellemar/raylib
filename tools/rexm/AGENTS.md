# AGENTS.md - tools/rexm/

## OVERVIEW
rexm manages the examples collection and build metadata.

## STRUCTURE
tools/rexm/
├── rexm.c
├── Makefile
├── reports/
└── VS2022/

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Command flow | tools/rexm/rexm.c | Single binary |
| Validation output | tools/rexm/reports/ | Generated reports |
| Windows build | tools/rexm/VS2022/ | Solution/project files |

## CONVENTIONS
- Example metadata parsed from examples_template.c header
- Categories: core, shapes, textures, text, models, shaders, audio, others
- GLSL resources expected under glsl100/glsl120/glsl330

## ANTI-PATTERNS
- Editing reports by hand
- Changing example header format without updating parser
