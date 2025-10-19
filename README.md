
# C Threads

A `C` *library combining multiple packages for cross-platform development:*

- custom malloc with [rpmalloc](https://github.com/zelang-dev/rpmalloc).
- emulated C11 [thread](https://en.cppreference.com/w/c/thread) using [Pthreads](https://en.wikipedia.org/wiki/Pthreads), or [Pthreads4w](http://sourceforge.net/projects/pthreads4w/).
- emulated C11 [Atomic](https://en.cppreference.com/w/c/atomic) with [c-atomic](https://github.com/zelang-dev/c-atomic).
- general `Linux` compatibility *headers* for **Windows**, including a [mmap](https://man7.org/linux/man-pages/man2/mmap.2.html) compilation.

The **deps** folder has *Windows* **build/fork** of [pthread-win32](https://github.com/GerHobbelt/pthread-win32).

This library is a minimal, portable implementation of basic *threading* and *atomics* for `C`. They closely mimic the functionality and naming of the standard **C11 Thread**, and **C11 Atomic**, should be easily replaceable with the corresponding standard variants.

Custom **malloc** is a fork [rpmalloc](https://github.com/zelang-dev/rpmalloc) has no dependency on `thread_local` and can be compiled using [Tiny C compiler](https://github.com/zelang-dev/tinycc). All `malloc` operations are [lockless](https://preshing.com/20120612/an-introduction-to-lock-free-programming/), and implements a emulated **Thread-local storage** via macro `tls_storage(type, variable)` as:

_example.h_

```h
#include <cthreads.h>
#include <rpmalloc.h>
#include <stdio.h>
#include <assert.h>
tls_storage_extern(int, gLocalVar);
```

_example.c_

```c
#include "example.h"
#define THREAD_COUNT 5

tls_storage(int, gLocalVar)

static int thread_local_storage(void *aArg) {
    int thread = *(int *)aArg;
    free(aArg);

    int data = thread + rand();
    *gLocalVar() = data;
    usleep(10);
    printf("thread #%d, gLocalVar is: %d\n", thread, *gLocalVar());
    assert(*gLocalVar() == data);
    return 0;
}

void emulated_tls(void) {
    thrd_t t[THREAD_COUNT];
    *gLocalVar() = 1;

    for (int i = 0; i < THREAD_COUNT; i++) {
        int *n = malloc(sizeof * n);
        *n = i;
        thrd_create(t + i, thread_local_storage, n);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        thrd_join(t[i], NULL);
    }

    assert(*gLocalVar() == 1);
}

int main(void) {
    emulated_tls();
    return 0;
}
```

> **cthreads.h** implements `thrd_local(type, variable)` macro, same behavior as above, but
> *will not emulate* if feature is *available* in your compiler, should be used for cross compatibility calling.

## Emulate/create/control `Thread Local Storage` via macros

The design of these *macros* follow normal behaviors when using `thread local storage`.

### In `rpmalloc.h`

The macro `tls_storage_extern(type, variable)` make function *proto*, and macro `tls_storage(type, variable)` will create functions as:

```h
C_API int rpmalloc_variable_tls;
C_API tls_storage_t rpmalloc_variable_tss;
C_API type* variable(void);
C_API void variable_delete(void);
```

The macro `tls_storage(type, variable)` will create the functions using:

```h
C_API int rpmalloc_tls_create(tls_storage_t *key, tls_dtor_t dtor);
C_API void rpmalloc_tls_delete(tls_storage_t key);
C_API void *rpmalloc_tls_get(tls_storage_t key);
C_API int rpmalloc_tls_set(tls_storage_t key, void *val);
```

### In `cthreads.h`

The macro `thrd_local_extern(type, variable)` make functions *proto*, and `thrd_local(type, variable)` create functions as:

**If compiler support *native* `thread_local`.**

```h
C_API thread_local type* thrd_variable_tls;
C_API void variable_del(void);
C_API void variable_update(type* value);
C_API bool is_variable_empty(void);
C_API type* variable(void);
```

Macro `thrd_local(type, variable)` also add.

```c
static thread_local type thrd_variable_buffer;
```

**If emulating `thread_local` is *detected/required*, will use platform `thread library` routines.**

```h
C_API int thrd_variable_tls;
C_API tss_t thrd_variable_tss;
C_API void variable_del(void);
C_API void variable_update(type* value);
C_API bool is_variable_empty(void);
C_API type* variable(void);
```

Macro `thrd_local(type, variable)` also add when *emulating*.

```c
static type thrd_variable_buffer;
```

- The macro `thrd_local_external(type, variable)` makes **non-pointer** *version* of above.
- The macro `thrd_static(type, variable)` makes local **non-extern** static function *version* of above.

## Installation

Any **commit** with an **tag** is considered *stable* for **release** at that *version* point.

If there are no *binary* available for your platform under **Releases** then build using **cmake**,
which produces **static** libraries by default.

You will need the *binary* stored under `built`, and `*.h` headers, or complete `include` *folder* if **Windows**.

### Linux

```shell
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug/Release -D BUILD_TESTS=OFF -D BUILD_EXAMPLES=OFF # use to not build tests and examples
cmake --build .
```

### Windows

```shell
mkdir build
cd build
cmake .. -D BUILD_TESTS=OFF -D BUILD_EXAMPLES=OFF # use to not build tests and examples
cmake --build . --config Debug/Release
```

### As cmake project dependency

> For **CMake** versions earlier than `3.14`, see <https://cmake.org/cmake/help/v3.14/module/FetchContent.html>

Add to **CMakeLists.txt**

```c
find_package(cthreads QUIET)
FetchContent_Declare(cthreads
 URL https://github.com/zelang-dev/c-threads/archive/refs/tags/1.0.0.zip
 URL_MD5 8e7a1260dc13bf1bd44ad2fba6404436
)
FetchContent_MakeAvailable(cthreads)

target_include_directories(${PROJECT_NAME}
  AFTER PUBLIC $<BUILD_INTERFACE:${CTHREADS_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:${CTHREADS_INCLUDE_DIR}>)
target_link_libraries(${PROJECT_NAME} PUBLIC cthreads)
```
