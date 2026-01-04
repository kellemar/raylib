# AGENTS.md - tools/rlparser/

## OVERVIEW
Parses raylib headers into txt/json/xml/lua for bindings.

## STRUCTURE
tools/rlparser/
├── rlparser.c
├── Makefile
└── output/

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Parser logic | tools/rlparser/rlparser.c | DefineInfo/StructInfo/EnumInfo/FunctionInfo |
| Sample outputs | tools/rlparser/output/ | Generated files |

## CONVENTIONS
- Functions must be declared on a single line in headers
- Structs/enums follow canonical typedef layouts

## ANTI-PATTERNS
- Editing files under tools/rlparser/output
- Breaking raylib.h declaration layout without updating parser
