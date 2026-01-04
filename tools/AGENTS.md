# AGENTS.md - tools/

## OVERVIEW
Internal utilities for example management and API parsing.

## STRUCTURE
tools/
├── rexm/
└── rlparser/

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Example manager | tools/rexm/ | See tools/rexm/AGENTS.md |
| API parser | tools/rlparser/ | See tools/rlparser/AGENTS.md |
| Validation reports | tools/rexm/reports/ | Generated outputs |

## CONVENTIONS
- Tools are C99 and follow raylib formatting rules
- Keep tools dependency-free when possible

## ANTI-PATTERNS
- Editing generated outputs under tools/rlparser/output
- Editing reports under tools/rexm/reports
