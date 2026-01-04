# AGENTS.md - examples/shaders/

## OVERVIEW
Shader examples and GLSL resources.

## STRUCTURE
examples/shaders/
├── shaders_<name>.c
├── shaders_<name>.png
└── resources/
    ├── shaders/glsl100/
    ├── shaders/glsl120/
    ├── shaders/glsl330/
    ├── models/
    └── *.png

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Example pattern | examples/shaders/shaders_basic_lighting.c | Typical loader |
| GLSL sources | examples/shaders/resources/shaders/* | Versioned shaders |
| Assets | examples/shaders/resources/ | Textures/models |

## CONVENTIONS
- Keep shader filenames consistent across GLSL versions
- Select GLSL version based on platform macros
- Align example name with shader filenames

## ANTI-PATTERNS
- Mixing GLSL versions in a single folder
- Mismatched shader filenames across versions
