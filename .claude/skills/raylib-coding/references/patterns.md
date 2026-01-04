# Patterns & Idioms

## Table of Contents
1. [Error Handling](#error-handling)
2. [Resource Management](#resource-management)
3. [Data Structures](#data-structures)
4. [API Design](#api-design)
5. [Concurrency Patterns](#concurrency-patterns)
6. [Common Idioms](#common-idioms)

---

## Error Handling

### Return Codes Pattern
```c
typedef enum {
    ERR_OK = 0,
    ERR_NOMEM = -1,
    ERR_INVALID = -2,
    ERR_IO = -3,
    ERR_TIMEOUT = -4
} ErrorCode;

const char *error_string(ErrorCode err) {
    switch (err) {
        case ERR_OK:      return "Success";
        case ERR_NOMEM:   return "Out of memory";
        case ERR_INVALID: return "Invalid argument";
        case ERR_IO:      return "I/O error";
        case ERR_TIMEOUT: return "Timeout";
        default:          return "Unknown error";
    }
}

// Usage
ErrorCode result = do_operation();
if (result != ERR_OK) {
    fprintf(stderr, "Error: %s\n", error_string(result));
    return result;
}
```

### Output Parameters Pattern
```c
// Return status, output via pointer
bool parse_int(const char *str, int *out) {
    char *end;
    long val = strtol(str, &end, 10);
    if (end == str || *end != '\0') return false;
    if (val < INT_MIN || val > INT_MAX) return false;
    *out = (int)val;
    return true;
}

// Usage
int value;
if (!parse_int(input, &value)) {
    // Handle error
}
```

### Cleanup Goto Pattern
```c
int process_file(const char *path) {
    int result = -1;
    FILE *f = NULL;
    char *buffer = NULL;
    
    f = fopen(path, "r");
    if (!f) goto cleanup;
    
    buffer = malloc(BUFFER_SIZE);
    if (!buffer) goto cleanup;
    
    // Do work...
    result = 0;
    
cleanup:
    free(buffer);
    if (f) fclose(f);
    return result;
}
```

### Error Context Pattern
```c
typedef struct {
    int code;
    char message[256];
    const char *file;
    int line;
} Error;

#define ERROR_SET(err, c, msg) do { \
    (err)->code = (c); \
    snprintf((err)->message, sizeof((err)->message), "%s", (msg)); \
    (err)->file = __FILE__; \
    (err)->line = __LINE__; \
} while(0)

bool read_config(const char *path, Config *cfg, Error *err) {
    FILE *f = fopen(path, "r");
    if (!f) {
        ERROR_SET(err, ERR_IO, "Failed to open config file");
        return false;
    }
    // ...
}
```

---

## Resource Management

### Constructor/Destructor Pattern
```c
typedef struct Connection {
    int socket;
    char *buffer;
    size_t buffer_size;
} Connection;

Connection *connection_create(const char *host, int port) {
    Connection *conn = calloc(1, sizeof(*conn));
    if (!conn) return NULL;
    
    conn->socket = connect_to_host(host, port);
    if (conn->socket < 0) {
        free(conn);
        return NULL;
    }
    
    conn->buffer_size = 4096;
    conn->buffer = malloc(conn->buffer_size);
    if (!conn->buffer) {
        close(conn->socket);
        free(conn);
        return NULL;
    }
    
    return conn;
}

void connection_destroy(Connection *conn) {
    if (!conn) return;
    close(conn->socket);
    free(conn->buffer);
    free(conn);
}
```

### Handle Pattern (Opaque Pointer)
```c
// In header (public)
typedef struct Handle Handle;
Handle *handle_create(void);
void handle_destroy(Handle *h);
int handle_operation(Handle *h, int arg);

// In source (private)
struct Handle {
    int internal_state;
    void *private_data;
};

Handle *handle_create(void) {
    Handle *h = calloc(1, sizeof(*h));
    // Initialize...
    return h;
}
```

### RAII-like Macro Pattern
```c
#define SCOPED_FILE(var, path, mode) \
    for (FILE *var = fopen(path, mode), *_once = (void*)1; \
         _once && var; \
         _once = NULL, fclose(var))

// Usage
SCOPED_FILE(f, "data.txt", "r") {
    // Use f here
    // Automatically closed when scope exits
}
```

---

## Data Structures

### Dynamic Array
```c
typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} IntArray;

void intarray_init(IntArray *arr) {
    arr->data = NULL;
    arr->size = 0;
    arr->capacity = 0;
}

bool intarray_push(IntArray *arr, int value) {
    if (arr->size >= arr->capacity) {
        size_t new_cap = arr->capacity ? arr->capacity * 2 : 8;
        int *new_data = realloc(arr->data, new_cap * sizeof(int));
        if (!new_data) return false;
        arr->data = new_data;
        arr->capacity = new_cap;
    }
    arr->data[arr->size++] = value;
    return true;
}

void intarray_free(IntArray *arr) {
    free(arr->data);
    intarray_init(arr);
}
```

### Intrusive Linked List
```c
// Node is embedded in data structure
typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *prev;
} ListNode;

typedef struct {
    ListNode node;  // Embed list node
    int data;
    char name[32];
} MyItem;

// Get container from node pointer
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

void process_list(ListNode *head) {
    for (ListNode *n = head; n; n = n->next) {
        MyItem *item = container_of(n, MyItem, node);
        printf("Item: %s = %d\n", item->name, item->data);
    }
}
```

### Hash Table (Simple)
```c
#define HASH_SIZE 256

typedef struct Entry {
    char *key;
    int value;
    struct Entry *next;
} Entry;

typedef struct {
    Entry *buckets[HASH_SIZE];
} HashMap;

static unsigned hash(const char *key) {
    unsigned h = 0;
    while (*key) h = h * 31 + (unsigned char)*key++;
    return h % HASH_SIZE;
}

void hashmap_set(HashMap *map, const char *key, int value) {
    unsigned idx = hash(key);
    for (Entry *e = map->buckets[idx]; e; e = e->next) {
        if (strcmp(e->key, key) == 0) {
            e->value = value;
            return;
        }
    }
    Entry *e = malloc(sizeof(*e));
    e->key = strdup(key);
    e->value = value;
    e->next = map->buckets[idx];
    map->buckets[idx] = e;
}
```

---

## API Design

### Init/Cleanup Pair
```c
// Always pair init with cleanup
int module_init(ModuleConfig *cfg);
void module_cleanup(void);

// For objects
Object *object_create(const char *name);
void object_destroy(Object *obj);
```

### Builder Pattern
```c
typedef struct {
    int width;
    int height;
    bool fullscreen;
    const char *title;
} WindowConfig;

WindowConfig window_config_default(void) {
    return (WindowConfig){
        .width = 800,
        .height = 600,
        .fullscreen = false,
        .title = "Window"
    };
}

// Usage
WindowConfig cfg = window_config_default();
cfg.width = 1920;
cfg.height = 1080;
cfg.fullscreen = true;
Window *w = window_create(&cfg);
```

### Callback Pattern
```c
typedef void (*EventCallback)(void *context, int event_type, void *event_data);

typedef struct {
    EventCallback callback;
    void *context;
} EventHandler;

void register_handler(EventHandler *handler) {
    // Store handler...
}

void emit_event(int type, void *data) {
    // For each registered handler:
    handler->callback(handler->context, type, data);
}

// Usage
void my_callback(void *ctx, int type, void *data) {
    MyApp *app = ctx;
    // Handle event...
}

EventHandler h = { .callback = my_callback, .context = &my_app };
register_handler(&h);
```

---

## Concurrency Patterns

### Mutex Protected Data
```c
#include <pthread.h>

typedef struct {
    pthread_mutex_t lock;
    int value;
} AtomicCounter;

void counter_init(AtomicCounter *c) {
    pthread_mutex_init(&c->lock, NULL);
    c->value = 0;
}

int counter_increment(AtomicCounter *c) {
    pthread_mutex_lock(&c->lock);
    int result = ++c->value;
    pthread_mutex_unlock(&c->lock);
    return result;
}
```

### Producer-Consumer Queue
```c
typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    int *buffer;
    int capacity;
    int size;
    int head;
    int tail;
} Queue;

void queue_push(Queue *q, int item) {
    pthread_mutex_lock(&q->lock);
    while (q->size == q->capacity) {
        pthread_cond_wait(&q->not_full, &q->lock);
    }
    q->buffer[q->tail] = item;
    q->tail = (q->tail + 1) % q->capacity;
    q->size++;
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

int queue_pop(Queue *q) {
    pthread_mutex_lock(&q->lock);
    while (q->size == 0) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }
    int item = q->buffer[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->size--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->lock);
    return item;
}
```

---

## Common Idioms

### Sizeof on Variable
```c
// Prefer sizeof on variable, not type
// If type of ptr changes, this still works
ptr = malloc(sizeof(*ptr));

// For arrays
int arr[100];
memset(arr, 0, sizeof(arr));
```

### Compile-Time Assert
```c
#define STATIC_ASSERT(cond, msg) \
    typedef char static_assert_##msg[(cond) ? 1 : -1]

STATIC_ASSERT(sizeof(int) == 4, int_must_be_4_bytes);
STATIC_ASSERT(sizeof(MyStruct) <= 64, struct_must_fit_cacheline);
```

### X-Macro Pattern
```c
// Define data once, generate code multiple times
#define ERROR_CODES(X) \
    X(ERR_NONE,    "No error") \
    X(ERR_NOMEM,   "Out of memory") \
    X(ERR_IO,      "I/O error") \
    X(ERR_INVALID, "Invalid argument")

// Generate enum
enum ErrorCode {
    #define X(code, msg) code,
    ERROR_CODES(X)
    #undef X
};

// Generate string table
const char *error_messages[] = {
    #define X(code, msg) msg,
    ERROR_CODES(X)
    #undef X
};
```

### Flexible Array Member
```c
typedef struct {
    size_t length;
    char data[];  // Flexible array member (C99)
} String;

String *string_create(const char *str) {
    size_t len = strlen(str);
    String *s = malloc(sizeof(String) + len + 1);
    if (!s) return NULL;
    s->length = len;
    memcpy(s->data, str, len + 1);
    return s;
}
```

### Designated Initializers
```c
// Initialize specific fields (C99)
struct Point {
    int x, y, z;
};

struct Point p = { .x = 10, .y = 20 };  // z is 0

// Array initializers
int sparse[100] = {
    [0] = 1,
    [50] = 2,
    [99] = 3
};
```

### Compound Literals
```c
// Create temporary struct/array
void draw_rect(struct Rect r);

draw_rect((struct Rect){ .x = 0, .y = 0, .w = 100, .h = 50 });

// Array compound literal
int sum(int *arr, int n);
int result = sum((int[]){1, 2, 3, 4, 5}, 5);
```
