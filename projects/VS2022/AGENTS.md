# AGENTS.md - projects/VS2022/

## OVERVIEW
Visual Studio 2022 solution with raylib library and all examples.

## STRUCTURE
projects/VS2022/
├── raylib.sln
├── raylib/                # Library project
└── examples/              # Per-example .vcxproj files

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Solution | projects/VS2022/raylib.sln | Project groups by category |
| Library project | projects/VS2022/raylib/raylib.vcxproj | Static lib |
| Example projects | projects/VS2022/examples/*.vcxproj | One per example |

## CONVENTIONS
- One .vcxproj per example
- Solution grouping mirrors example categories

## ANTI-PATTERNS
- Renaming or regenerating project GUIDs manually
- Hand-editing example projects instead of using rexm
