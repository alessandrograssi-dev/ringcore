[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/std/the-standard)
[![CMake](https://img.shields.io/badge/CMake-3.31%2B-064F8C.svg)](https://cmake.org/)
[![Header-only](https://img.shields.io/badge/library-header--only-success.svg)](https://github.com/alessandrograssi-dev/ringcore)
[![GitHub stars](https://img.shields.io/github/stars/alessandrograssi-dev/ringcore?style=social)](https://github.com/alessandrograssi-dev/ringcore/stargazers)
[![GitHub last commit](https://img.shields.io/github/last-commit/alessandrograssi-dev/ringcore)](https://github.com/alessandrograssi-dev/ringcore/commits/main)
[![GitHub repo size](https://img.shields.io/github/repo-size/alessandrograssi-dev/ringcore)](https://github.com/alessandrograssi-dev/ringcore)
[![Build and Validate](https://github.com/alessandrograssi-dev/ringcore/actions/workflows/build.yml/badge.svg)](https://github.com/alessandrograssi-dev/ringcore/actions/workflows/build.yml)
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

## Compile on Linux / macOS / Windows

### Linux

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

### macOS

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

### Windows (PowerShell)

Use generator-agnostic CMake commands (no `make` required):

```powershell
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure
```

If Ninja is not installed, omit `-G Ninja` and use your default Visual Studio generator.

To build and run benchmark executable:

```bash
cmake --build build --target bench_basic
./build/benchmarks/bench_basic
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

### Benchmark configuration

From `benchmarks/basic.cpp`:

- `BUFFER_SIZE = 1024`
- Effective ring `fill_count = buffer.max_size() = 1023`
- `ROUNDS = 1'000'000` for batch push/pop benchmarks
- `LOOPS = 200'000` for iterator benchmark
- Clock source: `std::chrono::steady_clock`
- Data types tested:
	- `int`
	- `std::string`
	- `LargeObject` (`int data[64]`)

The benchmark includes two push/pop modes:

- **Discard pop**: pops without payload consumption (container overhead focus)
- **Consume pop**: pops and passes values to `consume_value(...)` to keep payload work observable

### Benchmark environment

Measured on this machine:

| Item | Value |
|---|---|
| OS | Linux 6.14.0-37-generic x86_64 |
| CPU | 13th Gen Intel(R) Core(TM) i9-13900H |
| Cores / Threads | 14 cores, 20 threads |
| Max CPU freq | 5400 MHz |
| RAM | 30 GiB |
| Compiler | `c++ (Ubuntu 14.2.0-19ubuntu2) 14.2.0` |

Because this is a micro-benchmark, results can vary with thermal/power state, background load, and CPU scaling.

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

## Memory layout

Ring buffer storage is fixed and contiguous (size `N`).

### Base storage view

```text
Storage indices:
[0][1][2][3][4][5][6][7]
```

- `head` points to the first logical element.
- `tail` points to the next insertion position.
- Logical active range is `[head ... tail)` (with wrap-around).

### Non-wrapped example

```text
Storage:
[0][1][2][3][4][5][6][7]
	 ^        ^
	head     tail

Logical elements: [1, 2, 3]
```

### Wrapped example

```text
Storage:
[0][1][2][3][4][5][6][7]
		 ^              ^
		tail           head

Logical elements: [6, 7, 0, 1]
```

In wrapped state, iteration still follows logical order starting at `head` and continuing circularly until `tail`.

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

## STL algorithm examples

`RingBuffer` iterators are compatible with common STL algorithms.

```cpp
#include <algorithm>
#include <numeric>
#include <vector>
#include "RingBuffer.hpp"

int main() {
	RingBuffer<int, 8> rb;

	rb.push(10);
	rb.push(42);
	rb.push(5);
	rb.push(7);

	// std::find
	auto it = std::find(rb.begin(), rb.end(), 42);
	if (it != rb.end()) {
		// found 42
	}

	// std::accumulate
	int sum = std::accumulate(rb.begin(), rb.end(), 0);

	// std::sort on a copied range
	std::vector<int> sorted(rb.begin(), rb.end());
	std::sort(sorted.begin(), sorted.end());

	return sum;
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

