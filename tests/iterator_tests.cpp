#include <cassert>
#include <iostream>

#include "RingBuffer.hpp"

void iterator_basic() {
  RingBuffer<int, 3> rb;

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
  RingBuffer<int, 3> rb;

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
  RingBuffer<int, 3> rb;

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
  RingBuffer<int, 3> rb;

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
  RingBuffer<int, 3> rb;

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
  RingBuffer<int, 3> rb;

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
  RingBuffer<int, 3> rb;

  assert(rb.begin() == rb.end());
  int count = 0;
  for (auto value : rb) {
    (void)value;
    ++count;
  }
  assert(count == 0);
}

void iterator_decrement_wrap() {
  RingBuffer<int, 3> rb;

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
  iterator_basic();
  iterator_wraparound();
  const_iteration();
  range_loop_modify();
  iterator_boundary();
  reverse_iteration();
  empty_iteration();
  iterator_decrement_wrap();

  std::cout << "Iterator correctness tests passed\n";
}
