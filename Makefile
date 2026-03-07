
.PHONY: all configure build test bench run-bench docs clean delete-docs

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

docs:
	@if ! command -v doxygen >/dev/null 2>&1; then \
		echo "doxygen not found. Install it first (e.g. sudo apt install doxygen)."; \
		exit 1; \
	fi
	@mkdir -p docs
	@echo "Generating Doxyfile for doc output..."
	@doxygen -g Doxyfile >/dev/null
	@sed -i 's|^PROJECT_NAME .*|PROJECT_NAME = "ringcore"|' Doxyfile
	@sed -i 's|^OUTPUT_DIRECTORY .*|OUTPUT_DIRECTORY = docs|' Doxyfile
	@sed -i 's|^INPUT .*|INPUT = include README.md|' Doxyfile
	@sed -i 's|^USE_MDFILE_AS_MAINPAGE .*|USE_MDFILE_AS_MAINPAGE = README.md|' Doxyfile
	@sed -i 's|^MARKDOWN_SUPPORT .*|MARKDOWN_SUPPORT = YES|' Doxyfile
	@sed -i 's|^RECURSIVE .*|RECURSIVE = YES|' Doxyfile
	@sed -i 's|^GENERATE_LATEX .*|GENERATE_LATEX = NO|' Doxyfile
	doxygen Doxyfile

format:
	clang-format -i $$(find . -name "*.cpp" -o -name "*.hpp")

clean:
	rm -rf build

delete-docs:
	rm -rf docs