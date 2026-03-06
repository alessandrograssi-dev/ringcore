#include <cassert>
#include <iostream>

#include "RingBuffer.hpp"

void basic_push_pop() {
  RingBuffer<int, 3> rb;

  assert(rb.empty());
  assert(!rb.is_full());

  rb.push(1);
  rb.push(2);
  rb.push(3);

  assert(rb.is_full());
  assert(rb.size() == 3);

  auto v1 = rb.pop();
  auto v2 = rb.pop();
  auto v3 = rb.pop();

  assert(v1 == 1);
  assert(v2 == 2);
  assert(v3 == 3);

  assert(rb.empty());
}

void push_overflow() {
  RingBuffer<int, 2> rb;

  assert(rb.push(1));
  assert(rb.push(2));
  assert(!rb.push(3));
  assert(rb.size() == 2);
}

void pop_empty() {
  RingBuffer<int, 2> rb;
  bool thrown = false;
  try {
    rb.pop();
  } catch (const std::out_of_range&) {
    thrown = true;
  }
  assert(thrown);
}

void peek_front_back() {
  RingBuffer<int, 3> rb;

  rb.push(10);
  rb.push(20);

  auto p = rb.peek();
  auto f = rb.front();
  auto b = rb.back();

  assert(p == 10);
  assert(f == 10);
  assert(b == 20);
}

void wraparound_push_pop() {
  RingBuffer<int, 3> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);

  rb.pop();
  rb.pop();

  rb.push(4);
  rb.push(5);

  assert(rb.size() == 3);
  assert(rb.pop() == 3);
  assert(rb.pop() == 4);
  assert(rb.pop() == 5);
  assert(rb.empty());
}

void clear_test() {
  RingBuffer<int, 4> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);
  rb.clear();

  assert(rb.empty());

  rb.push(10);
  assert(rb.front() == 10);
}

void stress_cycles() {
  RingBuffer<int, 4> rb;

  for (int i = 0; i < 10000; ++i) {
    rb.push(i);
    rb.pop();
  }

  assert(rb.empty());
}

void index_wraparound() {
  RingBuffer<int, 3> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);

  rb.pop();
  rb.push(4);

  // buffer now logically: 2,3,4

  assert(rb[0] == 2);
  assert(rb[1] == 3);
  assert(rb[2] == 4);
}

void back_wraparound_edge() {
  RingBuffer<int, 3> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);

  rb.pop();
  rb.push(4); // tail becomes 0 internally

  assert(rb.back() == 4);
}

void emplace_front_basic() {
  RingBuffer<int, 3> rb;

  rb.emplace_front(1);
  rb.emplace_front(2);
  rb.emplace_front(3);

  assert(rb.pop() == 3);
  assert(rb.pop() == 2);
  assert(rb.pop() == 1);
}

void emplace_front_wraparound() {
  RingBuffer<int, 3> rb;

  rb.push(1);
  rb.push(2);

  rb.emplace_front(3);

  assert(rb.front() == 3);
  assert(rb.back() == 2);

  assert(rb.pop() == 3);
  assert(rb.pop() == 1);
  assert(rb.pop() == 2);
}

void at_valid() {
  RingBuffer<int, 3> rb;

  rb.push(10);
  rb.push(20);

  assert(rb.at(0) == 10);
  assert(rb.at(1) == 20);
}

void at_out_of_range() {
  RingBuffer<int, 3> rb;

  rb.push(10);

  bool thrown = false;

  try {
    rb.at(1);
  } catch (const std::out_of_range&) {
    thrown = true;
  }

  assert(thrown);
}

void swap_test() {
  RingBuffer<int, 3> a;
  RingBuffer<int, 3> b;

  a.push(1);
  a.push(2);

  b.push(10);

  swap(a, b);

  assert(a.pop() == 10);

  assert(b.pop() == 1);
  assert(b.pop() == 2);
}

void max_size_test() {
  RingBuffer<int, 5> rb;

  assert(rb.max_size() == 5);
}

void index_access() {
  RingBuffer<int, 3> rb;

  rb.push(5);
  rb.push(6);
  rb.push(7);

  assert(rb[0] == 5);
  assert(rb[1] == 6);
  assert(rb[2] == 7);
}

int main() {
  basic_push_pop();
  push_overflow();
  pop_empty();
  peek_front_back();
  wraparound_push_pop();
  clear_test();
  stress_cycles();
  index_wraparound();
  back_wraparound_edge();
  emplace_front_basic();
  emplace_front_wraparound();
  at_valid();
  at_out_of_range();
  swap_test();
  max_size_test();
  index_access();

  std::cout << "Container behavior tests passed\n";
}