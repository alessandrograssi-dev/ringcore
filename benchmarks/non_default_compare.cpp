#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include "RingBufferNonDefaultConstructible.hpp"

using bench_clock = std::chrono::steady_clock;

constexpr std::size_t BUFFER_SIZE = 1024;
constexpr std::size_t ROUNDS = 1'000'000;

template <typename T> inline void do_not_optimize(const T& value) {
#if defined(__clang__) || defined(__GNUC__)
  asm volatile("" : : "m"(value) : "memory");
#else
  (void)value;
#endif
}

template <std::size_t PayloadSize> struct NonDefaultBlob {
  std::array<std::byte, PayloadSize> payload{};
  std::uint64_t id;

  explicit NonDefaultBlob(std::uint64_t seed) noexcept : id(seed) {
    for (std::size_t i = 0; i < payload.size(); ++i) {
      payload[i] = std::byte((seed + i) & 0xFFu);
    }
  }

  NonDefaultBlob() = delete;

  bool operator==(const NonDefaultBlob& other) const noexcept {
    return id == other.id;
  }
};

template <typename T> inline void consume_value(const T& value) {
  auto id = value.id;
  auto first = static_cast<unsigned>(value.payload[0]);
  do_not_optimize(id);
  do_not_optimize(first);
}

void separator() {
  std::cout << "-----------------------------\n";
}

template <typename T> void bench_ringbuffer_batch_discard() {
  RingBufferNonDefaultConstructible<T, BUFFER_SIZE> buffer;
  const std::size_t fill_count = buffer.max_size();
  std::size_t sink = 0;

  auto start = bench_clock::now();
  for (std::size_t r = 0; r < ROUNDS; ++r) {
    for (std::size_t i = 0; i < fill_count; ++i)
      buffer.push(T{r + i});

    for (std::size_t i = 0; i < fill_count; ++i) {
      buffer.pop_discard();
      ++sink;
    }
  }
  auto end = bench_clock::now();

  do_not_optimize(sink);
  std::cout << "RingBufferNonDefaultConstructible batch discard pop: "
            << std::chrono::duration<double>(end - start).count() << " s\n";
}

template <typename T> void bench_deque_batch_discard() {
  std::deque<T> dq;
  const std::size_t fill_count = RingBufferNonDefaultConstructible<T, BUFFER_SIZE>{}.max_size();
  std::size_t sink = 0;

  auto start = bench_clock::now();
  for (std::size_t r = 0; r < ROUNDS; ++r) {
    for (std::size_t i = 0; i < fill_count; ++i)
      dq.push_back(T{r + i});

    for (std::size_t i = 0; i < fill_count; ++i) {
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
  const std::size_t fill_count = RingBufferNonDefaultConstructible<T, BUFFER_SIZE>{}.max_size();
  vec.reserve(fill_count);
  std::size_t sink = 0;

  auto start = bench_clock::now();
  for (std::size_t r = 0; r < ROUNDS; ++r) {
    for (std::size_t i = 0; i < fill_count; ++i)
      vec.push_back(T{r + i});

    for (std::size_t i = 0; i < fill_count; ++i) {
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
  RingBufferNonDefaultConstructible<T, BUFFER_SIZE> buffer;
  const std::size_t fill_count = buffer.max_size();
  std::size_t sink = 0;

  auto start = bench_clock::now();
  for (std::size_t r = 0; r < ROUNDS; ++r) {
    for (std::size_t i = 0; i < fill_count; ++i)
      buffer.push(T{r + i});

    for (std::size_t i = 0; i < fill_count; ++i) {
      T value = buffer.pop();
      consume_value(value);
      ++sink;
    }
  }
  auto end = bench_clock::now();

  do_not_optimize(sink);
  std::cout << "RingBufferNonDefaultConstructible batch consume pop: "
            << std::chrono::duration<double>(end - start).count() << " s\n";
}

template <typename T> void bench_deque_batch_consume() {
  std::deque<T> dq;
  const std::size_t fill_count = RingBufferNonDefaultConstructible<T, BUFFER_SIZE>{}.max_size();
  std::size_t sink = 0;

  auto start = bench_clock::now();
  for (std::size_t r = 0; r < ROUNDS; ++r) {
    for (std::size_t i = 0; i < fill_count; ++i)
      dq.push_back(T{r + i});

    for (std::size_t i = 0; i < fill_count; ++i) {
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
  const std::size_t fill_count = RingBufferNonDefaultConstructible<T, BUFFER_SIZE>{}.max_size();
  vec.reserve(fill_count);
  std::size_t sink = 0;

  auto start = bench_clock::now();
  for (std::size_t r = 0; r < ROUNDS; ++r) {
    for (std::size_t i = 0; i < fill_count; ++i)
      vec.push_back(T{r + i});

    for (std::size_t i = 0; i < fill_count; ++i) {
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
  RingBufferNonDefaultConstructible<T, BUFFER_SIZE> buffer;
  const std::size_t fill_count = buffer.max_size();

  for (std::size_t i = 0; i < fill_count; ++i)
    buffer.push(T{i});

  std::size_t sink = 0;

  auto start = bench_clock::now();
  constexpr std::size_t LOOPS = 200'000;
  for (std::size_t l = 0; l < LOOPS; ++l) {
    for (const auto& v : buffer) {
      ++sink;
      consume_value(v);
    }
  }
  auto end = bench_clock::now();

  do_not_optimize(sink);
  std::cout << "RingBufferNonDefaultConstructible iteration: "
            << std::chrono::duration<double>(end - start).count() << " s\n";
}

template <std::size_t PayloadSize> void run_suite(const std::string& name) {
  using T = NonDefaultBlob<PayloadSize>;
  static_assert(!std::is_default_constructible_v<T>);

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
  std::cout << "=== RingBufferNonDefaultConstructible vs std containers ===\n";

  run_suite<16>("NON-DEFAULT BLOB16");
  run_suite<64>("NON-DEFAULT BLOB64");
  run_suite<256>("NON-DEFAULT BLOB256");

  return 0;
}
