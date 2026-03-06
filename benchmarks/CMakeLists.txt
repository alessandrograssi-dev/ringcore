add_executable(bench_basic
	basic.cpp
)

target_include_directories(bench_basic
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
)

target_compile_features(bench_basic PRIVATE cxx_std_20)
target_compile_options(bench_basic PRIVATE -O3 -march=native)
