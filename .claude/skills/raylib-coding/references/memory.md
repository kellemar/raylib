# Memory Management

## Table of Contents
1. [Allocation Fundamentals](#allocation-fundamentals)
2. [Ownership Patterns](#ownership-patterns)
3. [Memory Pools](#memory-pools)
4. [Leak Prevention](#leak-prevention)
5. [Debugging Memory Issues](#debugging-memory-issues)
6. [Advanced Techniques](#advanced-techniques)

---

## Allocation Fundamentals

### Stack vs Heap

| Aspect | Stack | Heap |
|--------|-------|------|
| Speed | Very fast | Slower (syscall possible) |
| Size | Limited (~1-8 MB) | Limited by system RAM |
| Lifetime | Automatic (scope) | Manual (malloc/free) |
| Fragmentation | None | Possible |
| Thread safety | Per-thread | Requires synchronization |

```c
// Stack allocation (preferred for small, fixed-size data)
void process_data(void) {
    char buffer[4096];      // Fast, automatic cleanup
    int array[100];         // No free() needed
}

// Heap allocation (for large, dynamic, or escaped data)
void create_large_buffer(void) {
    char *buffer = malloc(1024 * 1024);  // 1 MB
    if (!buffer) return;
    // ... use buffer ...
    free(buffer);
}
```

### Allocation Functions

```c
// malloc: Uninitialized memory
void *p = malloc(size);

// calloc: Zero-initialized, overflow-safe size calculation
void *p = calloc(count, element_size);

// realloc: Resize allocation (may move data)
void *new_p = realloc(p, new_size);
if (!new_p) { /* p is still valid */ }
else { p = new_p; }

// aligned_alloc (C11): Aligned allocation
void *p = aligned_alloc(64, size);  // size must be multiple of alignment

// Always check return values
void *p = malloc(size);
if (!p) {
    fprintf(stderr, "Allocation failed\n");
    return NULL;  // or handle appropriately
}
```

### Common Allocation Mistakes

```c
// Wrong: Integer overflow in size calculation
size_t size = count * sizeof(struct Item);  // Can overflow!
void *p = malloc(size);

// Correct: Use calloc or check for overflow
void *p = calloc(count, sizeof(struct Item));

// Or manually check:
if (count > SIZE_MAX / sizeof(struct Item)) {
    return NULL;  // Overflow would occur
}
void *p = malloc(count * sizeof(struct Item));
```

---

## Ownership Patterns

### Single Ownership
```c
// Creator owns and frees
char *create_string(const char *input) {
    char *s = malloc(strlen(input) + 1);
    if (s) strcpy(s, input);
    return s;  // Caller takes ownership
}

void use_string(void) {
    char *s = create_string("hello");
    if (!s) return;
    // ... use s ...
    free(s);  // Caller must free
}
```

### Transfer Ownership
```c
// Document ownership transfer in function name
typedef struct Buffer {
    char *data;
    size_t size;
} Buffer;

// "take" implies ownership transfer
void container_take_buffer(Container *c, Buffer *buf) {
    free(c->buffer.data);
    c->buffer = *buf;
    buf->data = NULL;  // Clear source to prevent double-free
    buf->size = 0;
}
```

### Borrowed References
```c
// "view" or const implies borrowed (no ownership transfer)
void process_buffer_view(const Buffer *buf) {
    // Read only, don't free
    for (size_t i = 0; i < buf->size; i++) {
        putchar(buf->data[i]);
    }
}
```

### Reference Counting
```c
typedef struct RefCounted {
    int ref_count;
    char *data;
} RefCounted;

RefCounted *rc_create(const char *data) {
    RefCounted *rc = malloc(sizeof(*rc));
    if (!rc) return NULL;
    rc->data = strdup(data);
    if (!rc->data) { free(rc); return NULL; }
    rc->ref_count = 1;
    return rc;
}

void rc_retain(RefCounted *rc) {
    if (rc) rc->ref_count++;
}

void rc_release(RefCounted *rc) {
    if (rc && --rc->ref_count == 0) {
        free(rc->data);
        free(rc);
    }
}
```

---

## Memory Pools

### Fixed-Size Block Pool
```c
#define POOL_BLOCK_SIZE 64
#define POOL_BLOCK_COUNT 1024

typedef struct Pool {
    char memory[POOL_BLOCK_SIZE * POOL_BLOCK_COUNT];
    int free_list[POOL_BLOCK_COUNT];
    int free_count;
} Pool;

void pool_init(Pool *pool) {
    for (int i = 0; i < POOL_BLOCK_COUNT; i++) {
        pool->free_list[i] = i;
    }
    pool->free_count = POOL_BLOCK_COUNT;
}

void *pool_alloc(Pool *pool) {
    if (pool->free_count == 0) return NULL;
    int index = pool->free_list[--pool->free_count];
    return &pool->memory[index * POOL_BLOCK_SIZE];
}

void pool_free(Pool *pool, void *ptr) {
    int index = ((char *)ptr - pool->memory) / POOL_BLOCK_SIZE;
    pool->free_list[pool->free_count++] = index;
}
```

### Arena Allocator (Linear Allocator)
```c
typedef struct Arena {
    char *memory;
    size_t size;
    size_t offset;
} Arena;

Arena arena_create(size_t size) {
    Arena a = { .memory = malloc(size), .size = size, .offset = 0 };
    return a;
}

void *arena_alloc(Arena *a, size_t size) {
    // Align to 8 bytes
    size_t aligned_offset = (a->offset + 7) & ~7;
    if (aligned_offset + size > a->size) return NULL;
    void *ptr = a->memory + aligned_offset;
    a->offset = aligned_offset + size;
    return ptr;
}

void arena_reset(Arena *a) {
    a->offset = 0;  // "Free" everything at once
}

void arena_destroy(Arena *a) {
    free(a->memory);
    a->memory = NULL;
}
```

### When to Use Pools

| Scenario | Pool Type |
|----------|-----------|
| Many small, same-size objects | Fixed-size block pool |
| Batch allocations freed together | Arena allocator |
| Real-time systems (no malloc) | Pre-allocated pools |
| Frequently allocated/freed objects | Object pool with free list |

---

## Leak Prevention

### Patterns for Cleanup

```c
// Pattern 1: Single cleanup label
int process_file(const char *path) {
    int result = -1;
    FILE *f = NULL;
    char *buffer = NULL;
    
    f = fopen(path, "r");
    if (!f) goto cleanup;
    
    buffer = malloc(4096);
    if (!buffer) goto cleanup;
    
    // ... do work ...
    result = 0;
    
cleanup:
    free(buffer);  // free(NULL) is safe
    if (f) fclose(f);
    return result;
}

// Pattern 2: Nested ifs with early returns
int process_file_v2(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    
    char *buffer = malloc(4096);
    if (!buffer) {
        fclose(f);
        return -1;
    }
    
    // ... do work ...
    
    free(buffer);
    fclose(f);
    return 0;
}
```

### Resource Wrappers
```c
// Pair create/destroy functions
typedef struct Resource {
    int fd;
    void *buffer;
    size_t size;
} Resource;

Resource *resource_create(size_t size) {
    Resource *r = malloc(sizeof(*r));
    if (!r) return NULL;
    
    r->buffer = malloc(size);
    if (!r->buffer) {
        free(r);
        return NULL;
    }
    
    r->fd = open("/dev/null", O_RDONLY);
    if (r->fd < 0) {
        free(r->buffer);
        free(r);
        return NULL;
    }
    
    r->size = size;
    return r;
}

void resource_destroy(Resource *r) {
    if (!r) return;
    close(r->fd);
    free(r->buffer);
    free(r);
}
```

### NULL After Free
```c
// Always NULL pointers after freeing
free(ptr);
ptr = NULL;

// Macro for convenience
#define SAFE_FREE(p) do { free(p); (p) = NULL; } while(0)

SAFE_FREE(buffer);
```

---

## Debugging Memory Issues

### Valgrind
```bash
# Detect leaks and errors
valgrind --leak-check=full --show-leak-kinds=all ./program

# Track origins of uninitialized values
valgrind --track-origins=yes ./program
```

### AddressSanitizer
```bash
# Compile with ASan
gcc -fsanitize=address -g -O1 program.c -o program

# Run (ASan is automatic)
./program
```

### Custom Debug Allocator
```c
#ifdef DEBUG
#define malloc(s)  debug_malloc(s, __FILE__, __LINE__)
#define free(p)    debug_free(p, __FILE__, __LINE__)

void *debug_malloc(size_t size, const char *file, int line) {
    void *p = malloc(size);
    fprintf(stderr, "ALLOC: %p size=%zu at %s:%d\n", p, size, file, line);
    return p;
}

void debug_free(void *p, const char *file, int line) {
    fprintf(stderr, "FREE: %p at %s:%d\n", p, file, line);
    free(p);
}
#endif
```

---

## Advanced Techniques

### Memory-Mapped Files
```c
#include <sys/mman.h>
#include <fcntl.h>

void *map_file(const char *path, size_t *size_out) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return NULL;
    
    struct stat st;
    if (fstat(fd, &st) < 0) { close(fd); return NULL; }
    
    void *p = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);  // fd can be closed after mmap
    
    if (p == MAP_FAILED) return NULL;
    *size_out = st.st_size;
    return p;
}

void unmap_file(void *p, size_t size) {
    munmap(p, size);
}
```

### Custom Allocators
```c
// Hook into malloc (glibc)
void *__libc_malloc(size_t size);
void *malloc(size_t size) {
    void *p = __libc_malloc(size);
    // Custom tracking here
    return p;
}

// Replaceable operators (C11 aligned_alloc)
void *my_aligned_alloc(size_t alignment, size_t size) {
    void *p;
    if (posix_memalign(&p, alignment, size) != 0) return NULL;
    return p;
}
```

### Zero-Copy Techniques
```c
// Avoid copying by using views/slices
typedef struct Slice {
    const char *data;
    size_t length;
} Slice;

Slice slice_from_string(const char *s) {
    return (Slice){ .data = s, .length = strlen(s) };
}

Slice slice_substr(Slice s, size_t start, size_t len) {
    if (start >= s.length) return (Slice){ .data = "", .length = 0 };
    if (start + len > s.length) len = s.length - start;
    return (Slice){ .data = s.data + start, .length = len };
}
```
