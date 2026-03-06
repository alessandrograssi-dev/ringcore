#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <numeric>
#include <vector>

#include "RingBuffer.hpp"

void copy_algorithm() {
  RingBuffer<int, 5> rb;
  rb.push(1);
  rb.push(2);
  rb.push(3);

  std::vector<int> v(3);
  std::copy(rb.begin(), rb.end(), v.begin());

  assert(v[0] == 1);
  assert(v[1] == 2);
  assert(v[2] == 3);
}

void distance_algorithm() {
  RingBuffer<int, 5> rb;
  rb.push(10);
  rb.push(20);
  rb.push(30);

  auto d = std::distance(rb.begin(), rb.end());
  assert(d == 3);
}

void accumulate_algorithm() {
  RingBuffer<int, 5> rb;
  rb.push(1);
  rb.push(2);
  rb.push(3);

  int sum = std::accumulate(rb.begin(), rb.end(), 0);
  assert(sum == 6);
}

void find_algorithm() {
  RingBuffer<int, 5> rb;
  rb.push(1);
  rb.push(2);
  rb.push(3);

  auto it = std::find(rb.begin(), rb.end(), 2);
  assert(it != rb.end());
  assert(*it == 2);
}

void equal_algorithm() {
  RingBuffer<int, 5> rb;
  rb.push(4);
  rb.push(5);
  rb.push(6);

  std::vector<int> ref = {4, 5, 6};
  assert(std::equal(rb.begin(), rb.end(), ref.begin()));
}

void iterator_traits_check() {
  RingBuffer<int, 5> rb;
  using it = decltype(rb.begin());

  static_assert(std::is_same_v<typename std::iterator_traits<it>::iterator_category,
                               std::bidirectional_iterator_tag>);
}

void range_construct_and_sort_copy() {
  RingBuffer<int, 5> rb;
  rb.push(5);
  rb.push(1);
  rb.push(3);

  std::vector<int> v(rb.begin(), rb.end());
  std::sort(v.begin(), v.end());

  assert(v[0] == 1);
  assert(v[1] == 3);
  assert(v[2] == 5);
}

void iterator_distance_loop() {
  RingBuffer<int, 5> rb;

  rb.push(1);
  rb.push(2);
  rb.push(3);

  int count = 0;

  for (auto it = rb.begin(); it != rb.end(); ++it)
    ++count;

  assert(count == rb.size());
}

int main() {
  copy_algorithm();
  distance_algorithm();
  accumulate_algorithm();
  find_algorithm();
  equal_algorithm();
  iterator_traits_check();
  range_construct_and_sort_copy();
  iterator_distance_loop();

  std::cout << "STL compatibility tests passed\n";
}
