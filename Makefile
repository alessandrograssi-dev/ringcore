
.PHONY: all configure build test bench run-bench clean

all: build

configure:
	mkdir -p build
	cmake -S . -B build

build: configure
	cmake --build build

test: build
	ctest --test-dir build --output-on-failure

bench: configure
	cmake --build build --target bench_basic

run-bench: bench
	./build/benchmarks/bench_basic

clean:
	rm -rf build