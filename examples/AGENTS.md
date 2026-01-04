# AGENTS.md - examples/

## OVERVIEW
Example programs organized by module with resources and screenshots.

## STRUCTURE
examples/
├── core/
├── shapes/
├── textures/
├── text/
├── models/
├── shaders/
├── audio/
└── others/

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Example template | examples/examples_template.c | Header metadata + structure |
| Example catalog | examples/README.md | Ratings and credits |
| Example list | examples/examples_list.txt | Category ordering |
| Shader resources | examples/shaders/ | See examples/shaders/AGENTS.md |

## CONVENTIONS
- Naming: <category>_<name>.c and <category>_<name>.png
- Screenshot size: 800x450
- Resources live under each category resources/ directory
- Example header metadata follows template format

## ANTI-PATTERNS
- Missing screenshots or metadata headers
- Resources placed outside category resources/ folders
- Renaming examples without updating examples_list.txt
