# AGENTS.md - cmake/

## OVERVIEW
Build-system glue and option mapping for CMake.

## STRUCTURE
cmake/
├── CompilerFlags.cmake
├── CompileDefinitions.cmake
├── LibraryConfigurations.cmake
├── ParseConfigHeader.cmake
├── InstallConfigurations.cmake
├── PackConfigurations.cmake
├── Uninstall.cmake
└── raylib-config*.cmake

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| CMake options | CMakeOptions.txt | Master option list |
| Compiler flags | cmake/CompilerFlags.cmake | C99 + warnings |
| Platform libs | cmake/LibraryConfigurations.cmake | OS-specific links |
| config.h parsing | cmake/ParseConfigHeader.cmake | SUPPORT_* mapping |

## CONVENTIONS
- Add new options in CMakeOptions.txt first
- Prefer target-based configuration

## ANTI-PATTERNS
- Adding options in CMakeLists.txt without CMakeOptions.txt entry
- Hardcoding platform libs outside LibraryConfigurations.cmake
