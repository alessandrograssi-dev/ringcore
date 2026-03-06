# ringcore

Header-only, fixed-size ring buffer in C++20 with:

- fast bitmask wrap-around (`N` must be power of two)
- STL-style iterators (forward/reverse, const/non-const)
- throwing APIs and non-throwing `try_*` APIs
- tests, micro-benchmarks, and Doxygen docs generation

The main implementation is in `include/RingBuffer.hpp`.

## Strong points

- **Header-only integration**: no library build/link step, just include `RingBuffer.hpp`.
- **Deterministic memory usage**: fixed-size storage with no container-internal heap allocation.
- **Fast wrap-around indexing**: power-of-two storage enables bitmask-based index wrap.
- **Predictable O(1) core ops**: push/pop/front/back are constant-time operations.
- **Dual error-handling style**: throwing API and explicit non-throwing `try_*` API.
- **STL-friendly iteration**: bidirectional iterators, const iterators, and reverse iterators.
- **Good for real-time-ish paths**: bounded capacity and explicit behavior under full/empty conditions.
- **Easy to benchmark and audit**: small implementation surface and transparent semantics.

## Requirements

- C++20 compiler (GCC/Clang)
- CMake 3.31+
- `make`
- Optional: `doxygen` (for docs)

## Build

```bash
make
```

Equivalent manual commands:

```bash
cmake -S . -B build
cmake --build build
```

## Run tests

```bash
make test
```

This runs all test targets via CTest from `tests/`:

- `basic_tests.cpp`
- `iterator_tests.cpp`
- `stl_tests.cpp`

## Run benchmark

```bash
make run-bench
```

Benchmark source: `benchmarks/basic.cpp`.

### Benchmark methodology

The benchmark compares `RingBuffer` against:

- `std::deque` (queue-like push_back/pop_front)
- `std::vector` used as a stack (push_back/pop_back)

For each data type (`int`, `std::string`, `LargeObject`), it runs two push/pop modes plus one iteration test:

1. **Discard pop mode**
	- Fill the container with `fill_count` elements
	- Pop all elements without consuming their payload
	- Measures structural push/pop overhead

2. **Consume pop mode**
	- Fill the container with `fill_count` elements
	- Pop each element and feed it to `consume_value(...)`
	- Measures push/pop + payload move/copy/access cost

3. **Iteration mode (RingBuffer only)**
	- Fill once, then iterate repeatedly over logical elements
	- Measures iterator traversal cost

`RingBuffer` uses effective logical capacity `N-1`, so benchmark loops use `buffer.max_size()` for fair fill/drain counts across containers.

### Latest benchmark results

#### INT

| Mode | RingBuffer (s) | std::deque (s) | std::vector stack (s) |
|---|---:|---:|---:|
| Batch discard pop | 0.434571 | 0.902812 | 0.281144 |
| Batch consume pop | 1.42992 | 1.10755 | 0.684198 |
| Iteration (RingBuffer only) | 0.0758931 | - | - |

#### STRING

| Mode | RingBuffer (s) | std::deque (s) | std::vector stack (s) |
|---|---:|---:|---:|
| Batch discard pop | 1.98054 | 3.74455 | 1.71825 |
| Batch consume pop | 3.45806 | 5.32584 | 2.4731 |
| Iteration (RingBuffer only) | 0.0795547 | - | - |

#### LARGE_OBJECT

| Mode | RingBuffer (s) | std::deque (s) | std::vector stack (s) |
|---|---:|---:|---:|
| Batch discard pop | 6.76861 | 18.3302 | 9.25165 |
| Batch consume pop | 9.66389 | 21.1863 | 11.2808 |
| Iteration (RingBuffer only) | 0.0585418 | - | - |

## Generate documentation

```bash
make docs
```

What this does:

1. Generates a `Doxyfile`
2. Configures output to `doc/`
3. Runs Doxygen

Open generated HTML:

```bash
xdg-open doc/html/index.html
```

## API overview

Template:

```cpp
RingBuffer<T, N>
```

Constraints:

- `N > 0`
- `N` must be power of two
- effective logical capacity is `N - 1`

### Throwing operations

- `push(...)` throws if full
- `pop()` throws if empty
- `pop_discard()` throws if empty
- `emplace_back(...)` / `emplace_front(...)` throw if full
- `front()` / `back()` / `peek()` throw if empty
- `at(i)` throws if out of range

### Non-throwing operations

- `try_push(...)`
- `try_pop(T&)`
- `try_emplace_back(...)`
- `try_pop_discard()`
- `try_front(...)`, `try_back(...)`, `try_peek(...)` (pointer outputs)

These return `bool` to indicate success/failure.

### Iterators

Supported:

- `begin()` / `end()`
- `cbegin()` / `cend()`
- `rbegin()` / `rend()`
- `crbegin()` / `crend()`

Iterator category is bidirectional.

## Minimal usage

```cpp
#include "RingBuffer.hpp"

int main() {
	RingBuffer<int, 8> rb; // power-of-two storage size

	rb.push(1);
	rb.push(2);

	int value = rb.pop(); // 1

	int out{};
	if (rb.try_pop(out)) {
		// out == 2
	}

	return 0;
}
```

## Project layout

```text
include/
	RingBuffer.hpp
tests/
	basic_tests.cpp
	iterator_tests.cpp
	stl_tests.cpp
benchmarks/
	basic.cpp
```

