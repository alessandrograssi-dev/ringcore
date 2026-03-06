#include <cassert>
#include <iostream>

#include "RingBuffer.hpp"

void basic_push_pop() {
  RingBuffer<int, 4> rb;

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
  RingBuffer<int, 4> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);

  bool thrown = false;
  try {
    rb.push(4);
  } catch (const std::out_of_range&) {
    thrown = true;
  }

  assert(thrown);
  assert(rb.size() == 3);
}

void pop_empty() {
  RingBuffer<int, 4> rb;
  bool thrown = false;
  try {
    rb.pop();
  } catch (const std::out_of_range&) {
    thrown = true;
  }
  assert(thrown);
}

void peek_front_back() {
  RingBuffer<int, 4> rb;

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
  RingBuffer<int, 4> rb;

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
  RingBuffer<int, 8> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);
  rb.clear();

  assert(rb.empty());

  rb.push(10);
  assert(rb.front() == 10);
}

void stress_cycles() {
  RingBuffer<int, 8> rb;

  for (int i = 0; i < 10000; ++i) {
    rb.push(i);
    rb.pop();
  }

  assert(rb.empty());
}

void index_wraparound() {
  RingBuffer<int, 4> rb;

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
  RingBuffer<int, 4> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);

  rb.pop();
  rb.push(4); // tail becomes 0 internally

  assert(rb.back() == 4);
}

void emplace_front_basic() {
  RingBuffer<int, 4> rb;

  rb.emplace_front(1);
  rb.emplace_front(2);
  rb.emplace_front(3);

  assert(rb.pop() == 3);
  assert(rb.pop() == 2);
  assert(rb.pop() == 1);
}

void emplace_front_wraparound() {
  RingBuffer<int, 4> rb;

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
  RingBuffer<int, 4> rb;

  rb.push(10);
  rb.push(20);

  assert(rb.at(0) == 10);
  assert(rb.at(1) == 20);
}

void at_out_of_range() {
  RingBuffer<int, 4> rb;

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
  RingBuffer<int, 4> a;
  RingBuffer<int, 4> b;

  a.push(1);
  a.push(2);

  b.push(10);

  swap(a, b);

  assert(a.pop() == 10);

  assert(b.pop() == 1);
  assert(b.pop() == 2);
}

void max_size_test() {
  RingBuffer<int, 8> rb;

  assert(rb.max_size() == 7);
}

void index_access() {
  RingBuffer<int, 4> rb;

  rb.push(5);
  rb.push(6);
  rb.push(7);

  assert(rb[0] == 5);
  assert(rb[1] == 6);
  assert(rb[2] == 7);
}

int main() {
  std::cout << "Test " << 1 << '\n';
  basic_push_pop();
  std::cout << "Test " << 2 << '\n';
  push_overflow();
  std::cout << "Test " << 3 << '\n';
  pop_empty();
  std::cout << "Test " << 4 << '\n';
  peek_front_back();
  std::cout << "Test " << 5 << '\n';
  wraparound_push_pop();
  std::cout << "Test " << 6 << '\n';
  clear_test();
  std::cout << "Test " << 7 << '\n';
  stress_cycles();
  std::cout << "Test " << 8 << '\n';
  index_wraparound();
  std::cout << "Test " << 9 << '\n';
  back_wraparound_edge();
  std::cout << "Test " << 10 << '\n';
  emplace_front_basic();
  std::cout << "Test " << 11 << '\n';
  emplace_front_wraparound();
  std::cout << "Test " << 12 << '\n';
  at_valid();
  std::cout << "Test " << 13 << '\n';
  at_out_of_range();
  std::cout << "Test " << 14 << '\n';
  swap_test();
  std::cout << "Test " << 15 << '\n';
  max_size_test();
  std::cout << "Test " << 16 << '\n';
  index_access();

  std::cout << "Container behavior tests passed\n";
}