# Performance & Optimization

## Table of Contents
1. [Profiling First](#profiling-first)
2. [Cache Optimization](#cache-optimization)
3. [Loop Optimization](#loop-optimization)
4. [Function Optimization](#function-optimization)
5. [Memory Access Patterns](#memory-access-patterns)
6. [Compiler Optimization](#compiler-optimization)
7. [SIMD and Vectorization](#simd-and-vectorization)
8. [Micro-optimizations](#micro-optimizations)

---

## Profiling First

Never optimize without measuring. Profile to find actual bottlenecks.

### Tools
```bash
# CPU profiling
perf record ./program && perf report
gprof ./program gmon.out > analysis.txt

# Memory profiling
valgrind --tool=massif ./program
heaptrack ./program

# Cache analysis
valgrind --tool=cachegrind ./program
perf stat -e cache-misses,cache-references ./program

# Time measurement in code
#include <time.h>
clock_t start = clock();
// ... code ...
double elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
```

### The 80/20 Rule
- 80% of execution time is in 20% of code
- Identify hot paths before optimizing
- Optimize the inner loop, not the outer loop

---

## Cache Optimization

Modern CPUs are memory-bound. Cache efficiency dominates performance.

### Cache Hierarchy
| Level | Size | Latency | Notes |
|-------|------|---------|-------|
| L1 | 32-64 KB | ~4 cycles | Per-core, split I/D |
| L2 | 256-512 KB | ~12 cycles | Per-core |
| L3 | 8-32 MB | ~40 cycles | Shared |
| RAM | GBs | ~200 cycles | Main memory |

### Data Layout for Cache
```c
// Bad: Array of Structs with unused fields
struct Entity {
    float x, y, z;           // Used in position update
    char name[64];           // Rarely accessed
    float vx, vy, vz;        // Used in position update
    int texture_id;          // Used in rendering only
};
Entity entities[1000];       // Cache lines wasted on name/texture

// Good: Struct of Arrays (SoA) for hot data
struct Positions { float x[1000], y[1000], z[1000]; };
struct Velocities { float vx[1000], vy[1000], vz[1000]; };
// Only load what you need, better vectorization
```

### Prefetching
```c
// Manual prefetch for predictable access
#include <xmmintrin.h>
for (int i = 0; i < n; i++) {
    _mm_prefetch(&data[i + 16], _MM_HINT_T0);  // Prefetch ahead
    process(data[i]);
}
```

### Cache-Oblivious Algorithms
- Recursively divide problems to fit in cache
- Matrix multiplication: use blocked/tiled approach
- Sorting: merge sort is more cache-friendly than quicksort

---

## Loop Optimization

### Loop Invariant Code Motion
```c
// Bad: Recomputes constant each iteration
for (int i = 0; i < n; i++) {
    result[i] = data[i] * (config->scale * config->factor);
}

// Good: Hoist invariants
const float multiplier = config->scale * config->factor;
for (int i = 0; i < n; i++) {
    result[i] = data[i] * multiplier;
}
```

### Loop Unrolling
```c
// Manual unrolling (compiler often does this)
for (int i = 0; i < n - 3; i += 4) {
    sum += arr[i];
    sum += arr[i + 1];
    sum += arr[i + 2];
    sum += arr[i + 3];
}
for (; i < n; i++) sum += arr[i];  // Handle remainder
```

### Loop Fusion
```c
// Bad: Two passes over same data
for (int i = 0; i < n; i++) a[i] = b[i] + c[i];
for (int i = 0; i < n; i++) d[i] = a[i] * 2;

// Good: Single pass
for (int i = 0; i < n; i++) {
    a[i] = b[i] + c[i];
    d[i] = a[i] * 2;
}
```

### Loop Interchange
```c
// Bad: Column-major access in row-major language
for (int j = 0; j < cols; j++)
    for (int i = 0; i < rows; i++)
        matrix[i][j] = 0;

// Good: Row-major access
for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++)
        matrix[i][j] = 0;
```

### Strength Reduction
```c
// Bad: Division in loop
for (int i = 0; i < n; i++) {
    result[i] = data[i] / 16;
}

// Good: Multiply by reciprocal or bit shift
for (int i = 0; i < n; i++) {
    result[i] = data[i] >> 4;  // For power of 2
}
```

---

## Function Optimization

### Inlining
```c
// Suggest inlining for small, hot functions
static inline int max(int a, int b) {
    return a > b ? a : b;
}

// Force inline (compiler-specific)
__attribute__((always_inline)) static inline int min(int a, int b);
```

### Avoid Function Call Overhead
```c
// Bad: Function call per element
for (int i = 0; i < n; i++) {
    process_element(arr[i]);
}

// Good: Batch processing
void process_batch(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        // Process inline
    }
}
```

### Restrict Pointers
```c
// Tell compiler pointers don't alias
void add_arrays(float * restrict a, 
                const float * restrict b, 
                const float * restrict c, 
                int n) {
    for (int i = 0; i < n; i++) {
        a[i] = b[i] + c[i];  // Compiler can vectorize safely
    }
}
```

---

## Memory Access Patterns

### Sequential vs Random Access
```c
// Good: Sequential access (cache-friendly)
for (int i = 0; i < n; i++) {
    sum += array[i];
}

// Bad: Random access (cache-hostile)
for (int i = 0; i < n; i++) {
    sum += array[random_indices[i]];
}
```

### Avoid Pointer Chasing
```c
// Bad: Linked list traversal (cache misses)
while (node) {
    sum += node->value;
    node = node->next;
}

// Good: Array-based (cache-friendly)
for (int i = 0; i < n; i++) {
    sum += array[i].value;
}
```

### Memory Alignment
```c
// Aligned allocation for SIMD
void *aligned_alloc(size_t alignment, size_t size);  // C11
posix_memalign(&ptr, 64, size);  // POSIX

// Aligned struct members
struct AlignedData {
    float data[4] __attribute__((aligned(16)));
};
```

---

## Compiler Optimization

### Optimization Levels
| Flag | Effect |
|------|--------|
| -O0 | No optimization (debug) |
| -O1 | Basic optimization |
| -O2 | Standard optimization (recommended) |
| -O3 | Aggressive (may increase code size) |
| -Os | Optimize for size |
| -Ofast | -O3 + fast-math (breaks IEEE compliance) |

### Useful Flags
```bash
# Enable link-time optimization
gcc -flto -O2 file1.c file2.c -o program

# Profile-guided optimization
gcc -fprofile-generate -O2 prog.c -o prog
./prog  # Run with typical workload
gcc -fprofile-use -O2 prog.c -o prog_optimized

# Architecture-specific
gcc -march=native -mtune=native -O2 prog.c

# See what compiler optimized
gcc -O2 -fopt-info-vec-optimized prog.c
```

### Branch Prediction Hints
```c
// Hint likely/unlikely branches
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

if (unlikely(error_condition)) {
    handle_error();
}
```

---

## SIMD and Vectorization

### Auto-vectorization Requirements
- Fixed iteration count (or use `#pragma omp simd`)
- No data dependencies between iterations
- Simple loop body
- Aligned memory access preferred

```c
// Vectorizable
for (int i = 0; i < n; i++) {
    c[i] = a[i] + b[i];
}

// Not vectorizable (loop-carried dependency)
for (int i = 1; i < n; i++) {
    a[i] = a[i-1] + b[i];
}
```

### Intrinsics (x86 SSE/AVX)
```c
#include <immintrin.h>

void add_vectors_simd(float *a, float *b, float *c, int n) {
    int i;
    for (i = 0; i <= n - 8; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        __m256 vc = _mm256_add_ps(va, vb);
        _mm256_storeu_ps(&c[i], vc);
    }
    for (; i < n; i++) c[i] = a[i] + b[i];  // Remainder
}
```

---

## Micro-optimizations

Use only after profiling confirms these are bottlenecks.

### Integer Tricks
```c
// Power of 2 operations
x >> n          // Divide by 2^n (signed: implementation-defined)
x << n          // Multiply by 2^n
x & (n - 1)     // Modulo n (when n is power of 2)
(x + n - 1) & ~(n - 1)  // Round up to multiple of n

// Branchless min/max (for predictable performance)
int min = y ^ ((x ^ y) & -(x < y));
int max = x ^ ((x ^ y) & -(x < y));

// Branchless abs
int abs_val = (x ^ (x >> 31)) - (x >> 31);
```

### Floating Point
```c
// Prefer multiplication over division
x * 0.5f          // Faster than x / 2.0f

// Use float instead of double when precision allows
float f = 1.0f;   // 'f' suffix for float literal

// Fused multiply-add (when available)
#include <math.h>
result = fmaf(a, b, c);  // a*b + c with one rounding
```

### Branch Elimination
```c
// Bad: Branch in hot path
if (condition) x = a; else x = b;

// Good: Branchless (if branch is unpredictable)
x = condition ? a : b;  // Compiler may use cmov

// Manual branchless
int mask = -condition;  // All 1s if true, all 0s if false
x = (a & mask) | (b & ~mask);
```

### Memory Tricks
```c
// Swap without temp
a ^= b; b ^= a; a ^= b;  // Warning: UB if a and b alias

// Clear memory (may be optimized away by compiler)
memset(buffer, 0, sizeof(buffer));

// Secure clear (not optimized away)
volatile char *p = buffer;
while (size--) *p++ = 0;
```
