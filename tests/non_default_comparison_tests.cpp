#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include "RingBuffer.hpp"
#include "RingBufferNonDefaultConstructible.hpp"

namespace {

struct NonDefaultSmall {
  int value;

  explicit NonDefaultSmall(int v) noexcept : value(v) {}
  NonDefaultSmall() = delete;

  bool operator==(const NonDefaultSmall& other) const noexcept {
    return value == other.value;
  }
};

std::vector<int> parity_sequence_legacy() {
  RingBuffer<int, 8> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);
  rb.pop_discard();
  rb.push(4);
  rb.push_overwrite(5);
  rb.emplace_front(10);

  std::vector<int> out;
  while (!rb.empty()) {
    out.push_back(rb.pop());
  }
  return out;
}

std::vector<int> parity_sequence_non_default_impl() {
  RingBufferNonDefaultConstructible<int, 8> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);
  rb.pop_discard();
  rb.push(4);
  rb.push_overwrite(5);
  rb.emplace_front(10);

  std::vector<int> out;
  while (!rb.empty()) {
    out.push_back(rb.pop());
  }
  return out;
}

void parity_with_legacy_basic_behavior() {
  const auto old_values = parity_sequence_legacy();
  const auto new_values = parity_sequence_non_default_impl();

  assert(old_values == new_values);
  assert((new_values == std::vector<int>{10, 2, 3, 4, 5}));
}

void parity_with_legacy_iterator_behavior() {
  RingBuffer<int, 8> old_rb;
  RingBufferNonDefaultConstructible<int, 8> new_rb;

  for (int v : {7, 8, 9, 10}) {
    old_rb.push(v);
    new_rb.push(v);
  }
  old_rb.pop_discard();
  old_rb.push(11);
  new_rb.pop_discard();
  new_rb.push(11);

  std::vector<int> old_values(old_rb.begin(), old_rb.end());
  std::vector<int> new_values(new_rb.begin(), new_rb.end());

  assert(old_values == new_values);
  assert((new_values == std::vector<int>{8, 9, 10, 11}));

  std::vector<int> old_rev(old_rb.rbegin(), old_rb.rend());
  std::vector<int> new_rev(new_rb.rbegin(), new_rb.rend());
  assert(old_rev == new_rev);
}

void parity_with_legacy_try_api() {
  RingBuffer<int, 4> old_rb;
  RingBufferNonDefaultConstructible<int, 4> new_rb;

  assert(old_rb.try_push(1));
  assert(new_rb.try_push(1));
  assert(old_rb.try_push(2));
  assert(new_rb.try_push(2));
  assert(old_rb.try_push(3));
  assert(new_rb.try_push(3));

  assert(!old_rb.try_push(4));
  assert(!new_rb.try_push(4));

  int old_out = 0;
  int new_out = 0;
  while (old_rb.try_pop(old_out) && new_rb.try_pop(new_out)) {
    assert(old_out == new_out);
  }

  assert(old_rb.empty());
  assert(new_rb.empty());
}

void supports_non_default_constructible_types() {
  static_assert(!std::is_default_constructible_v<NonDefaultSmall>);

  RingBufferNonDefaultConstructible<NonDefaultSmall, 8> rb;

  rb.emplace_back(NonDefaultSmall{1});
  rb.emplace_back(NonDefaultSmall{2});
  rb.emplace_front(NonDefaultSmall{0});
  rb.push(NonDefaultSmall{3});

  assert(rb.size() == 4);
  assert(rb.front().value == 0);
  assert(rb.back().value == 3);
  assert(rb.contains(NonDefaultSmall{2}));

  NonDefaultSmall out{-1};
  assert(rb.try_pop(out));
  assert(out.value == 0);

  assert(rb.pop().value == 1);
  assert(rb.pop().value == 2);
  assert(rb.pop().value == 3);
  assert(rb.empty());
}

void non_default_wraparound_and_clear() {
  RingBufferNonDefaultConstructible<NonDefaultSmall, 4> rb;

  rb.emplace_back(NonDefaultSmall{10});
  rb.emplace_back(NonDefaultSmall{20});
  rb.emplace_back(NonDefaultSmall{30});

  assert(rb.pop().value == 10);
  rb.emplace_back(NonDefaultSmall{40});

  std::vector<int> values;
  for (const auto& item : rb) {
    values.push_back(item.value);
  }
  assert((values == std::vector<int>{20, 30, 40}));

  rb.clear();
  assert(rb.empty());

  rb.emplace_back(NonDefaultSmall{50});
  assert(rb.front().value == 50);
}

} // namespace

int main() {
  parity_with_legacy_basic_behavior();
  parity_with_legacy_iterator_behavior();
  parity_with_legacy_try_api();

  supports_non_default_constructible_types();
  non_default_wraparound_and_clear();

  std::cout << "Non-default and legacy comparison tests passed\n";
}
