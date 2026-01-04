# Safety & Security

## Table of Contents
1. [Buffer Safety](#buffer-safety)
2. [Integer Safety](#integer-safety)
3. [String Safety](#string-safety)
4. [Input Validation](#input-validation)
5. [Undefined Behavior](#undefined-behavior)
6. [Secure Coding Practices](#secure-coding-practices)

---

## Buffer Safety

### Stack Buffer Overflow Prevention
```c
// Bad: No bounds checking
void bad_copy(char *dest, const char *src) {
    strcpy(dest, src);  // May overflow dest
}

// Good: Bounded copy
void safe_copy(char *dest, size_t dest_size, const char *src) {
    if (dest_size == 0) return;
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

// Better: Use snprintf
void safer_copy(char *dest, size_t dest_size, const char *src) {
    snprintf(dest, dest_size, "%s", src);
}
```

### Array Bounds
```c
// Bad: No bounds check
int get_element(int *arr, int index) {
    return arr[index];  // index could be negative or too large
}

// Good: Validate index
int get_element_safe(int *arr, size_t arr_size, size_t index) {
    if (index >= arr_size) {
        return -1;  // Or handle error appropriately
    }
    return arr[index];
}
```

### Format String Vulnerabilities
```c
// CRITICAL VULNERABILITY: Never pass user input as format string
void bad_log(const char *user_input) {
    printf(user_input);  // DANGER: Format string attack possible
}

// Safe: Use %s format specifier
void safe_log(const char *user_input) {
    printf("%s", user_input);
}

// Or use puts for simple strings
void safest_log(const char *user_input) {
    puts(user_input);
}
```

---

## Integer Safety

### Overflow Prevention
```c
#include <limits.h>
#include <stdint.h>

// Check before addition
bool safe_add(int a, int b, int *result) {
    if ((b > 0 && a > INT_MAX - b) ||
        (b < 0 && a < INT_MIN - b)) {
        return false;  // Overflow would occur
    }
    *result = a + b;
    return true;
}

// Check before multiplication
bool safe_multiply(int a, int b, int *result) {
    if (a > 0 && b > 0 && a > INT_MAX / b) return false;
    if (a > 0 && b < 0 && b < INT_MIN / a) return false;
    if (a < 0 && b > 0 && a < INT_MIN / b) return false;
    if (a < 0 && b < 0 && a < INT_MAX / b) return false;
    *result = a * b;
    return true;
}

// Size calculations (common source of bugs)
bool safe_array_size(size_t count, size_t element_size, size_t *result) {
    if (count != 0 && element_size > SIZE_MAX / count) {
        return false;
    }
    *result = count * element_size;
    return true;
}
```

### Signed/Unsigned Mixing
```c
// Bad: Signed/unsigned comparison
void bad_compare(int user_input, size_t array_size) {
    if (user_input < array_size) {  // Warning: comparison of int and size_t
        // Negative user_input becomes large positive when converted
    }
}

// Good: Explicit handling
void good_compare(int user_input, size_t array_size) {
    if (user_input < 0 || (size_t)user_input >= array_size) {
        // Handle out of bounds
        return;
    }
    size_t index = (size_t)user_input;
    // Safe to use index
}
```

### Truncation
```c
// Bad: Silent truncation
void process_size(uint32_t size) {
    uint16_t short_size = size;  // May truncate
}

// Good: Explicit check
bool process_size_safe(uint32_t size) {
    if (size > UINT16_MAX) {
        return false;
    }
    uint16_t short_size = (uint16_t)size;
    // Use short_size
    return true;
}
```

---

## String Safety

### Safe String Functions
```c
// Prefer these bounded alternatives
// Instead of:         Use:
// strcpy              strncpy or strlcpy (BSD) or snprintf
// strcat              strncat or strlcat (BSD) or snprintf
// sprintf             snprintf
// gets                fgets (never use gets!)
// scanf("%s")         scanf("%Ns") with width or fgets

// Example: Safe string building
char buffer[256];
int written = snprintf(buffer, sizeof(buffer), 
                       "User: %s, ID: %d", username, user_id);
if (written >= sizeof(buffer)) {
    // Output was truncated
    handle_truncation();
}
```

### Null Termination
```c
// strncpy does NOT guarantee null termination!
char dest[10];
strncpy(dest, source, sizeof(dest));
dest[sizeof(dest) - 1] = '\0';  // MUST add this!

// Safer pattern
void safe_strcpy(char *dest, size_t dest_size, const char *src) {
    if (dest_size == 0) return;
    size_t src_len = strlen(src);
    size_t copy_len = (src_len < dest_size - 1) ? src_len : dest_size - 1;
    memcpy(dest, src, copy_len);
    dest[copy_len] = '\0';
}
```

### String Validation
```c
// Validate string length before processing
bool process_username(const char *username) {
    if (!username) return false;
    
    size_t len = strlen(username);
    if (len == 0 || len > MAX_USERNAME_LEN) {
        return false;
    }
    
    // Validate characters
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_') {
            return false;
        }
    }
    
    return true;
}
```

---

## Input Validation

### Validate All External Input
```c
typedef struct Config {
    int port;
    char host[256];
    int timeout;
} Config;

bool validate_config(const Config *cfg) {
    // Validate port range
    if (cfg->port < 1 || cfg->port > 65535) {
        return false;
    }
    
    // Validate host (ensure null-terminated)
    if (strnlen(cfg->host, sizeof(cfg->host)) == sizeof(cfg->host)) {
        return false;  // Not null-terminated
    }
    
    // Validate timeout
    if (cfg->timeout < 0 || cfg->timeout > 3600) {
        return false;
    }
    
    return true;
}
```

### File Path Validation
```c
#include <string.h>

// Prevent path traversal attacks
bool is_safe_filename(const char *filename) {
    // Reject empty or too long
    if (!filename || strlen(filename) > 255) return false;
    
    // Reject absolute paths
    if (filename[0] == '/') return false;
    
    // Reject path traversal
    if (strstr(filename, "..") != NULL) return false;
    
    // Reject special characters
    const char *forbidden = "/\\:*?\"<>|";
    for (const char *p = filename; *p; p++) {
        if (strchr(forbidden, *p)) return false;
    }
    
    return true;
}
```

### Command Injection Prevention
```c
// NEVER do this with user input
void bad_execute(const char *user_input) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "ls %s", user_input);
    system(cmd);  // DANGER: Command injection possible
}

// Better: Use execve with explicit arguments
#include <unistd.h>

void safer_list_directory(const char *dir) {
    // Validate directory first
    if (!is_safe_path(dir)) return;
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char *args[] = {"ls", (char *)dir, NULL};
        execve("/bin/ls", args, NULL);
        _exit(127);
    }
    // Parent waits for child
}
```

---

## Undefined Behavior

### Common UB Sources
```c
// 1. Null pointer dereference
int *p = NULL;
*p = 5;  // UB

// 2. Use after free
free(ptr);
*ptr = 10;  // UB

// 3. Signed integer overflow
int x = INT_MAX;
x = x + 1;  // UB (not defined to wrap)

// 4. Out of bounds access
int arr[10];
arr[10] = 0;  // UB

// 5. Uninitialized variables
int x;
if (x > 0) { }  // UB: x is uninitialized

// 6. Division by zero
int result = 10 / 0;  // UB

// 7. Misaligned pointer
char buffer[5];
int *p = (int *)buffer;  // UB if buffer not aligned for int
*p = 42;

// 8. Modifying string literal
char *s = "hello";
s[0] = 'H';  // UB

// 9. Overlapping memcpy
char buf[10] = "hello";
memcpy(buf, buf + 2, 5);  // UB, use memmove instead

// 10. Missing return value
int get_value(int x) {
    if (x > 0) return x;
    // UB: no return for x <= 0
}
```

### Prevention Strategies
```c
// Always initialize variables
int count = 0;
char *ptr = NULL;

// Check pointers before use
if (ptr != NULL) {
    *ptr = value;
}

// Use assert for invariants (debug)
#include <assert.h>
assert(index < array_size);

// Use static analysis
// gcc -Wall -Wextra -Wpedantic -Werror
// clang --analyze
// cppcheck
```

---

## Secure Coding Practices

### Principle of Least Privilege
```c
// Drop privileges after privileged operations
#include <unistd.h>

void drop_privileges(uid_t target_uid, gid_t target_gid) {
    if (setgid(target_gid) != 0) abort();
    if (setuid(target_uid) != 0) abort();
}
```

### Secure Memory
```c
// Clear sensitive data after use
void process_password(char *password) {
    // Use password...
    
    // Clear from memory
    volatile char *p = password;
    size_t len = strlen(password);
    while (len--) *p++ = 0;
}

// Or use explicit_bzero (glibc) / SecureZeroMemory (Windows)
#include <string.h>
explicit_bzero(password, strlen(password));
```

### Error Handling
```c
// Always check return values
FILE *f = fopen(path, "r");
if (!f) {
    perror("fopen");
    return -1;
}

// Don't leak information in error messages
// Bad: Tells attacker if user exists
if (!user_exists(username)) {
    return "User not found";
}
if (!password_matches(username, password)) {
    return "Wrong password";
}

// Good: Generic message
if (!authenticate(username, password)) {
    return "Authentication failed";
}
```

### Compiler Security Flags
```bash
# Stack protection
gcc -fstack-protector-strong

# Position independent code (for ASLR)
gcc -fPIE -pie

# Fortify source (runtime checks)
gcc -D_FORTIFY_SOURCE=2

# Full RELRO (GOT protection)
gcc -Wl,-z,relro,-z,now

# All together
gcc -Wall -Wextra -Werror \
    -fstack-protector-strong \
    -fPIE -pie \
    -D_FORTIFY_SOURCE=2 \
    -Wl,-z,relro,-z,now \
    -o program program.c
```
