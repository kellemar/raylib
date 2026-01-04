# Code Style & Conventions

## Table of Contents
1. [Naming Conventions](#naming-conventions)
2. [Formatting](#formatting)
3. [File Organization](#file-organization)
4. [Documentation](#documentation)
5. [Header Files](#header-files)

---

## Naming Conventions

### General Rules
| Element | Style | Example |
|---------|-------|---------|
| Functions | snake_case or PascalCase | `parse_config`, `ParseConfig` |
| Variables | snake_case | `user_count`, `buffer_size` |
| Constants/Macros | UPPER_SNAKE_CASE | `MAX_BUFFER_SIZE`, `PI` |
| Types (struct/enum) | PascalCase | `UserData`, `ConnectionState` |
| Enum values | UPPER_SNAKE_CASE or prefixed | `STATE_IDLE`, `Color_Red` |
| Global variables | g_ prefix (if used) | `g_config`, `g_instance` |
| Static variables | s_ prefix (optional) | `s_initialized` |

### Descriptive Names
```c
// Bad: Unclear abbreviations
int n, m, tmp;
void proc(char *s);

// Good: Self-documenting
int user_count, max_connections, swap_temp;
void process_command(char *command_string);

// Exception: Loop indices and well-known conventions
for (int i = 0; i < n; i++)  // OK
for (size_t i = 0; i < count; i++)  // Better for sizes
```

### Prefixes for Namespacing
```c
// Prefix public APIs to avoid collisions
typedef struct MyLib_Config { ... } MyLib_Config;
int mylib_init(MyLib_Config *cfg);
void mylib_cleanup(void);

// Internal/private functions
static int internal_helper(void);
```

---

## Formatting

### Indentation & Braces
```c
// K&R style (common in Linux kernel, most C projects)
int function(int x)
{
    if (x > 0) {
        do_something();
    } else {
        do_other();
    }
}

// Allman style (common in Windows, some projects)
int function(int x)
{
    if (x > 0)
    {
        do_something();
    }
    else
    {
        do_other();
    }
}

// Pick one style and be consistent
// Use 4 spaces or tabs consistently (configure editor)
```

### Line Length
```c
// Keep lines under 80-100 characters
// Break long function calls
result = very_long_function_name(
    first_argument,
    second_argument,
    third_argument
);

// Break long conditions
if (condition_one &&
    condition_two &&
    condition_three) {
    // ...
}
```

### Spacing
```c
// Operators
x = a + b;
x = a*b + c*d;  // Tighter for higher precedence (optional)

// Control statements
if (condition) { }   // Space before paren
for (int i = 0; i < n; i++) { }
while (running) { }
switch (value) { }

// Function calls
result = function(arg1, arg2);  // No space before paren

// Pointer declarations
int *ptr;      // Asterisk with variable (common)
int* ptr;      // Asterisk with type (less common)
int * ptr;     // Spaced (avoid)
```

### Blank Lines
```c
// Separate logical sections
#include <stdio.h>
#include <stdlib.h>

#include "myheader.h"

#define MAX_SIZE 100

typedef struct Data {
    int value;
} Data;

static int helper(void);

int main(void)
{
    // Group related statements
    int x = 0;
    int y = 0;
    
    // Separate logical blocks with blank line
    initialize();
    
    while (running) {
        process();
    }
    
    cleanup();
    return 0;
}
```

---

## File Organization

### Source File Structure
```c
// 1. File header comment (optional but recommended)
/*
 * module.c - Brief description
 * 
 * Detailed description if needed.
 */

// 2. Includes (grouped and ordered)
#include "module.h"        // Own header first

#include <stdio.h>         // Standard library
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>     // System headers

#include "other_module.h"  // Project headers
#include "utils.h"

// 3. Macros and constants
#define BUFFER_SIZE 1024
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// 4. Type definitions (if not in header)
typedef struct InternalData {
    int value;
} InternalData;

// 5. Static/global variables
static bool s_initialized = false;
static InternalData s_data;

// 6. Static function declarations
static int helper_function(void);

// 7. Public function implementations
void module_init(void)
{
    // ...
}

// 8. Static function implementations
static int helper_function(void)
{
    // ...
}
```

### Header File Structure
```c
// 1. Include guard
#ifndef MODULE_H
#define MODULE_H

// 2. Includes needed for declarations
#include <stddef.h>
#include <stdbool.h>

// 3. C++ compatibility
#ifdef __cplusplus
extern "C" {
#endif

// 4. Macros and constants
#define MODULE_VERSION "1.0.0"
#define MODULE_MAX_SIZE 256

// 5. Type definitions
typedef struct ModuleConfig {
    int option;
    const char *name;
} ModuleConfig;

typedef enum ModuleState {
    MODULE_STATE_INIT,
    MODULE_STATE_RUNNING,
    MODULE_STATE_STOPPED
} ModuleState;

// 6. Function declarations
int module_init(const ModuleConfig *config);
void module_shutdown(void);
ModuleState module_get_state(void);

// 7. C++ compatibility closing
#ifdef __cplusplus
}
#endif

// 8. End include guard
#endif /* MODULE_H */
```

---

## Documentation

### Function Documentation
```c
/**
 * Parse configuration from file.
 *
 * @param path     Path to configuration file (must not be NULL)
 * @param config   Output parameter for parsed config (must not be NULL)
 * @return         0 on success, negative error code on failure
 *                 -1: file not found
 *                 -2: parse error
 *
 * @note Thread-safe. Allocates memory that must be freed with config_free().
 */
int config_parse(const char *path, Config *config);
```

### Inline Comments
```c
// Prefer explaining "why" over "what"

// Bad: Describes what code does
i++;  // Increment i

// Good: Explains why
i++;  // Skip header row

// Use for non-obvious logic
mask = (1 << bit) - 1;  // Create bitmask with 'bit' lower bits set

// TODO/FIXME/HACK markers
// TODO(username): Implement error recovery
// FIXME: This breaks with negative values
// HACK: Workaround for library bug #123
```

### Section Comments
```c
/*
 * ===========================================================================
 * Memory Management
 * ===========================================================================
 */

// Or simpler:
/* --- Memory Management --- */
```

---

## Header Files

### Include Guards
```c
// Traditional (always works)
#ifndef MYPROJECT_MODULE_H
#define MYPROJECT_MODULE_H
// ...
#endif

// Pragma once (widely supported, cleaner)
#pragma once
// ...
```

### Include Order
```c
// 1. Corresponding header (catches missing includes)
#include "mymodule.h"

// 2. C standard library
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 3. System/OS headers
#include <sys/types.h>
#include <unistd.h>

// 4. Third-party library headers
#include <libfoo/foo.h>

// 5. Project headers
#include "config.h"
#include "utils.h"
```

### Minimize Header Dependencies
```c
// In header: Forward declare when possible
typedef struct OtherModule OtherModule;  // Forward declaration
int process(OtherModule *m);             // Only need pointer

// In source: Include full definition
#include "other_module.h"
int process(OtherModule *m) {
    return m->value;  // Now need full struct
}
```

### Self-Contained Headers
```c
// Header should compile standalone
// Test: gcc -fsyntax-only header.h

// Include what you use
#ifndef MODULE_H
#define MODULE_H

#include <stddef.h>  // for size_t
#include <stdbool.h> // for bool

// Now declarations can use size_t and bool
size_t module_get_size(void);
bool module_is_ready(void);

#endif
```
