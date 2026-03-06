#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

#include <memory>
#include <optional>
#include <stdexcept>
#include <array>
#include <concepts>
#include <type_traits>
#include <utility>
#include <iostream>
#include <cassert>
#include <iterator>
#include <cmath>

template <typename RingBufferType> class RingBufferIterator {
public:
  using size_type = std::size_t;
  using index_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using value_type = typename RingBufferType::value_type;
  using pointer =
      std::conditional_t<std::is_const_v<RingBufferType>, const value_type*, value_type*>;
  using reference =
      std::conditional_t<std::is_const_v<RingBufferType>, const value_type&, value_type&>;
  using iterator_concept = std::bidirectional_iterator_tag;
  using iterator_category = std::bidirectional_iterator_tag;

public:
  constexpr RingBufferIterator(pointer start_buffer, pointer ptr, size_type storage)
      : m_start_buffer(start_buffer), m_ptr(ptr), m_storage_size(storage) {}

  RingBufferIterator& operator++() {
    assert(m_ptr != nullptr && m_start_buffer != nullptr);
    if (++m_ptr == m_start_buffer + m_storage_size)
      m_ptr = m_start_buffer;
    return *this;
  }

  RingBufferIterator operator++(int) {
    RingBufferIterator it = *this;
    ++(*this);
    return it;
  }

  RingBufferIterator& operator--() {
    assert(m_ptr != nullptr && m_start_buffer != nullptr);
    if (--m_ptr < m_start_buffer)
      m_ptr = m_start_buffer + m_storage_size - 1;
    return *this;
  }

  RingBufferIterator operator--(int) {
    RingBufferIterator it = *this;
    --(*this);
    return it;
  }

  bool operator==(const RingBufferIterator& other) const noexcept {
    assert(m_start_buffer == other.m_start_buffer);
    return m_ptr == other.m_ptr;
  }

  bool operator!=(const RingBufferIterator& other) const noexcept {
    return !((*this) == other);
  }

  reference operator*() const noexcept {
    assert(m_ptr != nullptr);
    return *m_ptr;
  }

  pointer operator->() const noexcept {
    assert(m_ptr != nullptr);
    return m_ptr;
  }

private:
  pointer m_start_buffer;
  pointer m_ptr;
  size_type m_storage_size;
};

template <typename T, std::size_t N> class RingBuffer {
  static_assert(N > 0, "RingBuffer: capacity must be bigger than 0");
  static_assert((N & (N-1)) == 0, "RingBuffer: capacity must be a power of 2");

public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using iterator = RingBufferIterator<RingBuffer<T, N>>;
  using const_iterator = RingBufferIterator<const RingBuffer<T, N>>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using index_type = std::size_t;

public:
  explicit RingBuffer() noexcept {
    static_assert(N > 0, "RingBuffer capacity must be > 0");
  }

  explicit RingBuffer(size_type size, const_reference value) : RingBuffer() {
    if (size > N)
      throw std::out_of_range("The size cannot be bigger than the capacity");

    for (index_type i = 0; i < size; ++i) {
      push(value);
    }
  }

  RingBuffer(const RingBuffer&) = default;
  RingBuffer& operator=(const RingBuffer&) = default;
  RingBuffer(RingBuffer&&) noexcept = default;
  RingBuffer& operator=(RingBuffer&&) noexcept = default;

  ~RingBuffer() = default;

  [[nodiscard]] inline constexpr bool empty() const noexcept {
    return m_size == 0;
  }

  [[nodiscard]] inline constexpr bool is_full() const noexcept {
    return m_size == max_size();
  }

  template <typename U>
    requires std::assignable_from<T&, U&&>
  void push(U&& value) {
    if (is_full())
      throw std::out_of_range("RingBuffer is full");

    m_data[m_tail] = std::forward<U>(value);
    _incr_tail();
  }

  template <typename U>
    requires std::assignable_from<T&, U&&>
  void push_overwrite(U&& value) noexcept {
    const bool was_full = is_full();
    m_data[m_tail] = std::forward<U>(value);
    _incr_tail();
    if (was_full) 
      _incr_head();
  }

  value_type pop() {
    if (empty())
      throw std::out_of_range("RingBuffer is empty");

    T value = std::move(m_data[m_head]);
    _incr_head();
    return value;
  }

  void pop_discard() {
    if (empty())
      throw std::out_of_range("RingBuffer is empty");

    _incr_head();
  }

  // try_* API
  bool try_pop(value_type& out) noexcept(noexcept(out = std::move(m_data[m_head]))) {
    if (empty())
      return false;

    out = std::move(m_data[m_head]);
    _incr_head();
    return true;
  }

  bool try_push(const_reference value) noexcept(noexcept(m_data[m_tail] = value)) {
    if (is_full())
      return false;

    m_data[m_tail] = value;
    _incr_tail();
    return true;
  }

  bool try_push(value_type&& value) noexcept(noexcept(m_data[m_tail] = std::move(value))) {
    if (is_full())
      return false;

    m_data[m_tail] = std::move(value);
    _incr_tail();
    return true;
  }

  template <typename... Args>
  bool try_emplace_back(Args&&... args)
      noexcept(noexcept(m_data[m_tail] = value_type(std::forward<Args>(args)...))) {
    if (is_full())
      return false;

    m_data[m_tail] = value_type(std::forward<Args>(args)...);
    _incr_tail();
    return true;
  }

  bool try_pop_discard() noexcept {
    if (empty())
      return false;

    _incr_head();
    return true;
  }

  bool try_peek(const_pointer& out) const noexcept {
    if (empty())
      return false;

    out = m_data.data() + m_head;
    return true;
  }

  bool try_front(pointer& out) noexcept {
    if (empty())
      return false;
    out = m_data.data() + m_head;
    return true;
  }

  bool try_front(const_pointer& out) const noexcept {
    if (empty())
      return false;
    out = m_data.data() + m_head;
    return true;
  }
  bool try_back(pointer& out) noexcept {
    if (empty())
      return false;

    index_type idx = (m_tail + sm_storage_size - 1) & sm_bitmask;
    out = m_data.data() + idx;
    return true;
  }

  bool try_back(const_pointer& out) const noexcept {
    if (empty())
      return false;

    index_type idx = (m_tail + sm_storage_size - 1) & sm_bitmask;
    out = m_data.data() + idx;
    return true;
  }

  reference peek() {
    if (empty())
      throw std::out_of_range("RingBuffer is empty");      

    return m_data[m_head];
  }

  const_reference peek() const {
    if (empty())
      throw std::out_of_range("RingBuffer is empty");      

    return m_data[m_head];
  }

  [[nodiscard]] inline constexpr size_type size() const noexcept {
    return m_size;
  }

  [[nodiscard]] constexpr size_type max_size() const noexcept {
    return sm_storage_size-1;
  }

  [[nodiscard]] constexpr iterator begin() noexcept {
    return iterator(m_data.data(), &m_data[m_head], sm_storage_size);
  }

  [[nodiscard]] constexpr iterator end() noexcept {
    return iterator(m_data.data(), &m_data[m_tail], sm_storage_size);
  }

  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return cbegin();
  }

  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return cend();
  }

  [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
    return const_iterator(m_data.data(), &m_data[m_head], sm_storage_size);
  }

  [[nodiscard]] constexpr const_iterator cend() const noexcept {
    return const_iterator(m_data.data(), &m_data[m_tail], sm_storage_size);
  }

  [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
    return std::reverse_iterator(end());
  }

  [[nodiscard]] constexpr reverse_iterator rend() noexcept {
    return std::reverse_iterator(begin());
  }

  [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
    return std::reverse_iterator(cend());
  }

  [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
    return std::reverse_iterator(cbegin());
  }

  [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
    return std::reverse_iterator(cend());
  }

  [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
    return std::reverse_iterator(cbegin());
  }

  template <typename... Args> void emplace_back(Args&&... args) {
    if (is_full())
      throw std::out_of_range("RingBuffer is full");

    m_data[m_tail] = value_type(std::forward<Args>(args)...);
    _incr_tail();
  }

  template <typename... Args> void emplace_front(Args&&... args) {
    if (is_full())
      throw std::out_of_range("RingBuffer is full");

    if (m_head == 0) {
      m_head = sm_storage_size - 1;
    } else {
      --m_head;
    }
    m_data[m_head] = value_type(std::forward<Args>(args)...);
    ++m_size;
  }

  void clear() noexcept {
    m_head = 0;
    m_tail = 0;
    m_size = 0;
  }

  [[nodiscard]] constexpr reference front() {
    if (empty())
      throw std::out_of_range("The RingBuffer is empty");
    return m_data[m_head];
  }

  [[nodiscard]] constexpr const_reference front() const {
    if (empty())
      throw std::out_of_range("The RingBuffer is empty");
    return m_data[m_head];
  }

  [[nodiscard]] constexpr reference back() {
    if (empty())
      throw std::out_of_range("The RingBuffer is empty");

    index_type idx = (m_tail + sm_storage_size - 1) & sm_bitmask;
    return m_data[idx];
  }

  [[nodiscard]] constexpr const_reference back() const {
    if (empty())
      throw std::out_of_range("The RingBuffer is empty");

    index_type idx = (m_tail + sm_storage_size - 1) & sm_bitmask;
    return m_data[idx];
  }

  constexpr const_reference operator[](index_type i) const {
    assert(i < size());
    return m_data[(m_head + i) & sm_bitmask];
  }

  constexpr reference operator[](index_type i) {
    assert(i < size());
    return m_data[(m_head + i) & sm_bitmask];
  }

  const_reference at(index_type i) const {
    if (i >= size())
      throw std::out_of_range("RingBuffer: index out of range");
    return m_data[(m_head + i) & sm_bitmask];
  }

  reference at(index_type i) {
    if (i >= size())
      throw std::out_of_range("RingBuffer: index out of range");
    return m_data[(m_head + i) & sm_bitmask];
  }

  [[nodiscard]] constexpr bool contains(const T& value) const {
    for (const auto& v : *this)
      if (v == value)
        return true;

    return false;
  }

  void swap(RingBuffer& other) noexcept {
    std::swap(m_data, other.m_data);
    std::swap(m_head, other.m_head);
    std::swap(m_tail, other.m_tail);
    std::swap(m_size, other.m_size);
  }

  friend void swap(RingBuffer& a, RingBuffer& b) noexcept {
    a.swap(b);
  }

private:
  static constexpr size_type sm_storage_size = N;
  static constexpr size_type sm_bitmask = N-1;
  std::array<T, sm_storage_size> m_data;
  alignas(64) index_type m_head = 0;
  alignas(64) index_type m_tail = 0;
  alignas(64) size_type  m_size = 0;

private:
  [[nodiscard]] constexpr static size_type storage() noexcept {
    return sm_storage_size;
  }

  constexpr void _incr_tail() noexcept {
    m_tail = (m_tail + 1) & sm_bitmask;
    ++m_size;
  }

  constexpr void _incr_head() noexcept {
    m_head = (m_head + 1) & sm_bitmask;
    --m_size;
  }
};

#endif // RING_BUFFER_HPP