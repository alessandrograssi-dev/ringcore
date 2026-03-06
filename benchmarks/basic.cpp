#include <iostream>
#include <chrono>
#include <vector>
#include <deque>
#include <string>
#include <type_traits>
#include <cstddef>

#include "RingBuffer.hpp"

using bench_clock = std::chrono::steady_clock;

constexpr size_t BUFFER_SIZE = 1024;
constexpr size_t ROUNDS = 1'000'000;

struct LargeObject {
  int data[64];

  LargeObject() {
    for (int i = 0; i < 64; ++i)
      data[i] = i;
  }
};

void separator() {
  std::cout << "-----------------------------\n";
}

template <typename T> inline void do_not_optimize(const T& value) {
#if defined(__clang__) || defined(__GNUC__)
  asm volatile("" : : "m"(value) : "memory");
#else
  (void)value;
#endif
}

template <typename T> inline void consume_value(const T& value) {
  if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
    do_not_optimize(value);
  } else if constexpr (requires { value.size(); }) {
    auto n = value.size();
    do_not_optimize(n);
  } else {
    do_not_optimize(value);
  }
}

template <typename T> void bench_ringbuffer_batch_discard() {
  RingBuffer<T, BUFFER_SIZE> buffer;
  const size_t fill_count = buffer.max_size();
  size_t sink = 0;

  auto start = bench_clock::now();

  for (size_t r = 0; r < ROUNDS; ++r) {
    // fill buffer
    for (size_t i = 0; i < fill_count; ++i)
      buffer.push(T{});

    // drain buffer
    for (size_t i = 0; i < fill_count; ++i) {
      buffer.pop_discard();
      ++sink;
    }
  }

  auto end = bench_clock::now();

  do_not_optimize(sink);

  std::cout << "RingBuffer batch discard pop: "
            << std::chrono::duration<double>(end - start).count() << " s\n";
}

template <typename T> void bench_deque_batch_discard() {
  std::deque<T> dq;
  const size_t fill_count = RingBuffer<T, BUFFER_SIZE>{}.max_size();
  size_t sink = 0;

  auto start = bench_clock::now();

  for (size_t r = 0; r < ROUNDS; ++r) {
    for (size_t i = 0; i < fill_count; ++i)
      dq.push_back(T{});

    for (size_t i = 0; i < fill_count; ++i) {
      dq.pop_front();
      ++sink;
    }
  }

  auto end = bench_clock::now();

  do_not_optimize(sink);

  std::cout << "std::deque batch discard pop: "
            << std::chrono::duration<double>(end - start).count() << " s\n";
}

template <typename T> void bench_vector_stack_discard() {
  std::vector<T> vec;
  const size_t fill_count = RingBuffer<T, BUFFER_SIZE>{}.max_size();
  vec.reserve(fill_count);

  size_t sink = 0;

  auto start = bench_clock::now();

  for (size_t r = 0; r < ROUNDS; ++r) {
    for (size_t i = 0; i < fill_count; ++i)
      vec.push_back(T{});

    for (size_t i = 0; i < fill_count; ++i) {
      vec.pop_back();
      ++sink;
    }
  }

  auto end = bench_clock::now();

  do_not_optimize(sink);

  std::cout << "std::vector stack discard pop: "
            << std::chrono::duration<double>(end - start).count() << " s\n";
}

template <typename T> void bench_ringbuffer_batch_consume() {
  RingBuffer<T, BUFFER_SIZE> buffer;
  const size_t fill_count = buffer.max_size();
  size_t sink = 0;

  auto start = bench_clock::now();

  for (size_t r = 0; r < ROUNDS; ++r) {
    for (size_t i = 0; i < fill_count; ++i)
      buffer.push(T{});

    T value{};
    for (size_t i = 0; i < fill_count; ++i) {
      if (buffer.try_pop(value)) {
        consume_value(value);
        ++sink;
      }
    }
  }

  auto end = bench_clock::now();

  do_not_optimize(sink);

  std::cout << "RingBuffer batch consume pop: "
            << std::chrono::duration<double>(end - start).count() << " s\n";
}

template <typename T> void bench_deque_batch_consume() {
  std::deque<T> dq;
  const size_t fill_count = RingBuffer<T, BUFFER_SIZE>{}.max_size();
  size_t sink = 0;

  auto start = bench_clock::now();

  for (size_t r = 0; r < ROUNDS; ++r) {
    for (size_t i = 0; i < fill_count; ++i)
      dq.push_back(T{});

    for (size_t i = 0; i < fill_count; ++i) {
      T value = std::move(dq.front());
      dq.pop_front();
      consume_value(value);
      ++sink;
    }
  }

  auto end = bench_clock::now();

  do_not_optimize(sink);

  std::cout << "std::deque batch consume pop: "
            << std::chrono::duration<double>(end - start).count() << " s\n";
}

template <typename T> void bench_vector_stack_consume() {
  std::vector<T> vec;
  const size_t fill_count = RingBuffer<T, BUFFER_SIZE>{}.max_size();
  vec.reserve(fill_count);
  size_t sink = 0;

  auto start = bench_clock::now();

  for (size_t r = 0; r < ROUNDS; ++r) {
    for (size_t i = 0; i < fill_count; ++i)
      vec.push_back(T{});

    for (size_t i = 0; i < fill_count; ++i) {
      T value = std::move(vec.back());
      vec.pop_back();
      consume_value(value);
      ++sink;
    }
  }

  auto end = bench_clock::now();

  do_not_optimize(sink);

  std::cout << "std::vector stack consume pop: "
            << std::chrono::duration<double>(end - start).count() << " s\n";
}

template <typename T> void bench_ringbuffer_iteration() {
  RingBuffer<T, BUFFER_SIZE> buffer;
  const size_t fill_count = buffer.max_size();

  for (size_t i = 0; i < fill_count; ++i)
    buffer.push(T{});

  size_t sink = 0;

  constexpr size_t LOOPS = 200'000;

  auto start = bench_clock::now();

  for (size_t l = 0; l < LOOPS; ++l) {
    for (auto& v : buffer) {
      sink += 1;
      consume_value(v);
    }
  }

  auto end = bench_clock::now();

  do_not_optimize(sink);

  std::cout << "RingBuffer iteration: " << std::chrono::duration<double>(end - start).count()
            << " s\n";
}

template <typename T> void run_suite(const std::string& name) {
  std::cout << "\n===== " << name << " =====\n";

  bench_ringbuffer_batch_discard<T>();
  bench_deque_batch_discard<T>();
  bench_vector_stack_discard<T>();

  std::cout << "(consume value variants)\n";
  bench_ringbuffer_batch_consume<T>();
  bench_deque_batch_consume<T>();
  bench_vector_stack_consume<T>();

  bench_ringbuffer_iteration<T>();

  separator();
}

int main() {
  run_suite<int>("INT");
  run_suite<std::string>("STRING");
  run_suite<LargeObject>("LARGE OBJECT");

  return 0;
}