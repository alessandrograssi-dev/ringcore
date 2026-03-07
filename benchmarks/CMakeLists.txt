add_executable(bench_basic
	basic.cpp
)

add_executable(bench_non_default_compare
	non_default_compare.cpp
)

target_include_directories(bench_basic
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
)

target_include_directories(bench_non_default_compare
	PRIVATE
		${PROJECT_SOURCE_DIR}/include
)

target_compile_features(bench_basic PRIVATE cxx_std_20)
target_compile_options(bench_basic PRIVATE -O3 -march=native)
target_compile_definitions(bench_basic PRIVATE NDEBUG)

target_compile_features(bench_non_default_compare PRIVATE cxx_std_20)
target_compile_options(bench_non_default_compare PRIVATE -O3 -march=native)
target_compile_definitions(bench_non_default_compare PRIVATE NDEBUG)
