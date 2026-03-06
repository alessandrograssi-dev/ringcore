#include <cassert>
#include <iostream>

#include "RingBuffer.hpp"

void iterator_basic() {
  RingBuffer<int, 4> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);

  int expected = 1;
  for (auto it = rb.begin(); it != rb.end(); ++it) {
    assert(*it == expected);
    ++expected;
  }
}

void iterator_wraparound() {
  RingBuffer<int, 4> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);
  rb.pop();
  rb.pop();
  rb.push(4);
  rb.push(5);

  int expected[] = {3, 4, 5};
  int i = 0;
  for (auto it = rb.begin(); it != rb.end(); ++it) {
    assert(*it == expected[i]);
    ++i;
  }
  assert(i == 3);
}

void const_iteration() {
  RingBuffer<int, 4> rb;

  rb.push(10);
  rb.push(20);

  const auto& crb = rb;
  int expected = 10;

  for (auto it = crb.cbegin(); it != crb.cend(); ++it) {
    assert(*it == expected);
    expected += 10;
  }
}

void range_loop_modify() {
  RingBuffer<int, 4> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);

  for (auto& v : rb) {
    v *= 2;
  }

  assert(rb.pop() == 2);
  assert(rb.pop() == 4);
  assert(rb.pop() == 6);
}

void iterator_boundary() {
  RingBuffer<int, 4> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);

  auto it = rb.begin();
  ++it;
  assert(*it == 2);
  ++it;
  assert(*it == 3);
  ++it;
  assert(it == rb.end());
}

void reverse_iteration() {
  RingBuffer<int, 4> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);

  auto it = rb.end();
  --it;
  assert(*it == 3);
  --it;
  assert(*it == 2);
  --it;
  assert(*it == 1);
}

void empty_iteration() {
  RingBuffer<int, 4> rb;

  assert(rb.begin() == rb.end());
  int count = 0;
  for (auto value : rb) {
    (void)value;
    ++count;
  }
  assert(count == 0);
}

void iterator_decrement_wrap() {
  RingBuffer<int, 4> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);

  rb.pop();
  rb.push(4);

  // logical: 2 3 4

  auto it = rb.end();

  --it;
  assert(*it == 4);

  --it;
  assert(*it == 3);

  --it;
  assert(*it == 2);
}

int main() {
  std::cout << "Test " << 1 << '\n';
  iterator_basic();
  std::cout << "Test " << 2 << '\n';
  iterator_wraparound();
  std::cout << "Test " << 3 << '\n';
  const_iteration();
  std::cout << "Test " << 4 << '\n';
  range_loop_modify();
  std::cout << "Test " << 5 << '\n';
  iterator_boundary();
  std::cout << "Test " << 6 << '\n';
  reverse_iteration();
  std::cout << "Test " << 7 << '\n';
  empty_iteration();
  std::cout << "Test " << 8 << '\n';
  iterator_decrement_wrap();

  std::cout << "Iterator correctness tests passed\n";
}
