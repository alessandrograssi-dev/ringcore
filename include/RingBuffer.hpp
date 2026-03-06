#ifndef RING_BUFFER_HPP
#define RING_BUFFER_HPP

/**
 * @file RingBuffer.hpp
 * @brief Fixed-capacity ring buffer and bidirectional iterator implementation.
 * @author Alessandro Grassi
 * @date 2026
 *
 * This header provides:
 * - `RingBufferIterator<RingBufferType>`: a lightweight bidirectional iterator
 *   that wraps around an underlying contiguous storage region.
 * - `RingBuffer<T, N>`: a fixed-size circular buffer with both throwing and
 *   non-throwing (`try_*`) APIs.
 *
 * Notes:
 * - Storage size `N` must be a power of two to enable bitmask index wrap.
 * - Effective logical capacity is `N - 1` in this implementation.
 */

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

/**
 * @brief Bidirectional iterator over ring-buffer-backed contiguous storage.
 *
 * @tparam RingBufferType Ring buffer type (const or non-const), used to derive
 *         value/reference/pointer qualifiers.
 *
 * This iterator tracks:
 * - `m_start_buffer`: start of physical storage,
 * - `m_ptr`: current position,
 * - `m_storage_size`: storage length for wrap-around.
 *
 * Increment/decrement operations wrap when the iterator crosses physical
 * boundaries, allowing traversal of circular storage with standard iterator
 * semantics required by bidirectional algorithms.
 *
 * Preconditions:
 * - Iterators compared with `operator==`/`operator!=` are expected to originate
 *   from the same underlying storage.
 */
template <typename RingBufferType> class RingBufferIterator {
public:
  /** @brief Iterator type aliases. */
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
  /**
   * @brief Constructs an iterator over the ring storage.
   * @param start_buffer Pointer to start of storage.
   * @param ptr Current iterator position.
   * @param storage Total storage size used for wrap-around.
   */
  constexpr RingBufferIterator(pointer start_buffer, pointer ptr, size_type storage)
      : m_start_buffer(start_buffer), m_ptr(ptr), m_storage_size(storage) {}

  /** @brief Prefix increment with wrap-around semantics. */
  RingBufferIterator& operator++() {
    assert(m_ptr != nullptr && m_start_buffer != nullptr);
    if (++m_ptr == m_start_buffer + m_storage_size)
      m_ptr = m_start_buffer;
    return *this;
  }

  /** @brief Postfix increment with wrap-around semantics. */
  RingBufferIterator operator++(int) {
    RingBufferIterator it = *this;
    ++(*this);
    return it;
  }

  /** @brief Prefix decrement with wrap-around semantics. */
  RingBufferIterator& operator--() {
    assert(m_ptr != nullptr && m_start_buffer != nullptr);
    if (--m_ptr < m_start_buffer)
      m_ptr = m_start_buffer + m_storage_size - 1;
    return *this;
  }

  /** @brief Postfix decrement with wrap-around semantics. */
  RingBufferIterator operator--(int) {
    RingBufferIterator it = *this;
    --(*this);
    return it;
  }

  /** @brief Equality comparison (same position in same storage). */
  bool operator==(const RingBufferIterator& other) const noexcept {
    assert(m_start_buffer == other.m_start_buffer);
    return m_ptr == other.m_ptr;
  }

  /** @brief Inequality comparison. */
  bool operator!=(const RingBufferIterator& other) const noexcept {
    return !((*this) == other);
  }

  /** @brief Dereference current iterator position. */
  reference operator*() const noexcept {
    assert(m_ptr != nullptr);
    return *m_ptr;
  }

  /** @brief Member-access for current iterator position. */
  pointer operator->() const noexcept {
    assert(m_ptr != nullptr);
    return m_ptr;
  }

private:
  pointer m_start_buffer;
  pointer m_ptr;
  size_type m_storage_size;
};

/**
 * @brief Fixed-size circular buffer with power-of-two storage.
 *
 * @tparam T Element type.
 * @tparam N Physical storage size (must be power of two).
 *
 * `RingBuffer` stores elements in a statically allocated array and tracks
 * logical head/tail indices with wrap-around via bitmasking (`index & (N - 1)`).
 *
 * Design highlights:
 * - Deterministic memory usage (no dynamic allocation in container internals).
 * - O(1) push/pop operations.
 * - Throwing APIs (`push`, `pop`, `emplace_*`) for strict error handling.
 * - Non-throwing APIs (`try_*`) returning boolean success flags.
 * - STL-friendly iterators and reverse iterators.
 *
 * Capacity model:
 * - Physical storage is `N`.
 * - Logical max size is `N - 1` in this implementation.
 */
template <typename T, std::size_t N> class RingBuffer {
  static_assert(N > 0, "RingBuffer: capacity must be bigger than 0");
  static_assert((N & (N - 1)) == 0, "RingBuffer: capacity must be a power of 2");

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
  /**
   * @brief Constructs an empty ring buffer.
   */
  explicit RingBuffer() noexcept {
    static_assert(N > 0, "RingBuffer capacity must be > 0");
  }

  /**
   * @brief Constructs a ring buffer pre-filled with @p size copies of @p value.
   * @throws std::out_of_range if @p size is greater than storage size N.
   */
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

  /** @brief Returns true if the buffer has no elements. */
  [[nodiscard]] inline constexpr bool empty() const noexcept {
    return m_size == 0;
  }

  /** @brief Returns the number of stored elements. */
  [[nodiscard]] inline constexpr size_type size() const noexcept {
    return m_size;
  }

  /** @brief Returns the maximum number of storable elements. */
  [[nodiscard]] constexpr size_type max_size() const noexcept {
    return sm_storage_size - 1;
  }

  /** @brief Returns true if the buffer reached its effective capacity. */
  [[nodiscard]] inline constexpr bool is_full() const noexcept {
    return m_size == max_size();
  }

  /**
   * @brief Returns the first logical element.
   * @throws std::out_of_range if the buffer is empty.
   */
  [[nodiscard]] constexpr reference front() {
    if (empty())
      throw std::out_of_range("The RingBuffer is empty");
    return m_data[m_head];
  }

  /**
   * @brief Returns the first logical element (const overload).
   * @throws std::out_of_range if the buffer is empty.
   */
  [[nodiscard]] constexpr const_reference front() const {
    if (empty())
      throw std::out_of_range("The RingBuffer is empty");
    return m_data[m_head];
  }

  /**
   * @brief Returns the last logical element.
   * @throws std::out_of_range if the buffer is empty.
   */
  [[nodiscard]] constexpr reference back() {
    if (empty())
      throw std::out_of_range("The RingBuffer is empty");

    index_type idx = (m_tail + sm_storage_size - 1) & sm_bitmask;
    return m_data[idx];
  }

  /**
   * @brief Returns the last logical element (const overload).
   * @throws std::out_of_range if the buffer is empty.
   */
  [[nodiscard]] constexpr const_reference back() const {
    if (empty())
      throw std::out_of_range("The RingBuffer is empty");

    index_type idx = (m_tail + sm_storage_size - 1) & sm_bitmask;
    return m_data[idx];
  }

  /**
   * @brief Returns the first logical element.
   * @throws std::out_of_range if the buffer is empty.
   */
  reference peek() {
    if (empty())
      throw std::out_of_range("RingBuffer is empty");

    return m_data[m_head];
  }

  /**
   * @brief Returns the first logical element (const overload).
   * @throws std::out_of_range if the buffer is empty.
   */
  const_reference peek() const {
    if (empty())
      throw std::out_of_range("RingBuffer is empty");

    return m_data[m_head];
  }

  /**
   * @brief Unchecked indexed access relative to logical head.
   * @pre i < size()
   */
  constexpr const_reference operator[](index_type i) const {
    assert(i < size());
    return m_data[(m_head + i) & sm_bitmask];
  }

  /**
   * @brief Unchecked indexed access relative to logical head.
   * @pre i < size()
   */
  constexpr reference operator[](index_type i) {
    assert(i < size());
    return m_data[(m_head + i) & sm_bitmask];
  }

  /**
   * @brief Checked indexed access relative to logical head.
   * @throws std::out_of_range if i >= size().
   */
  const_reference at(index_type i) const {
    if (i >= size())
      throw std::out_of_range("RingBuffer: index out of range");
    return m_data[(m_head + i) & sm_bitmask];
  }

  /**
   * @brief Checked indexed access relative to logical head.
   * @throws std::out_of_range if i >= size().
   */
  reference at(index_type i) {
    if (i >= size())
      throw std::out_of_range("RingBuffer: index out of range");
    return m_data[(m_head + i) & sm_bitmask];
  }

  /**
   * @brief Pushes one element at the logical tail.
   * @throws std::out_of_range if the buffer is full.
   */
  template <typename U>
    requires std::assignable_from<T&, U&&>
  void push(U&& value) {
    if (is_full())
      throw std::out_of_range("RingBuffer is full");

    m_data[m_tail] = std::forward<U>(value);
    _incr_tail();
  }

  /**
   * @brief Pushes one element, overwriting oldest one when full.
   */
  template <typename U>
    requires std::assignable_from<T&, U&&>
  void push_overwrite(U&& value) noexcept {
    const bool was_full = is_full();
    m_data[m_tail] = std::forward<U>(value);
    _incr_tail();
    if (was_full)
      _incr_head();
  }

  /**
   * @brief Constructs an element at the logical tail.
   * @throws std::out_of_range if the buffer is full.
   */
  template <typename... Args> void emplace_back(Args&&... args) {
    if (is_full())
      throw std::out_of_range("RingBuffer is full");

    m_data[m_tail] = value_type(std::forward<Args>(args)...);
    _incr_tail();
  }

  /**
   * @brief Constructs an element at the logical head (front insertion).
   * @throws std::out_of_range if the buffer is full.
   */
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

  /**
   * @brief Pops and returns the element at logical head.
   * @throws std::out_of_range if the buffer is empty.
   */
  value_type pop() {
    if (empty())
      throw std::out_of_range("RingBuffer is empty");

    T value = std::move(m_data[m_head]);
    _incr_head();
    return value;
  }

  /**
   * @brief Removes one element from logical head without returning it.
   * @throws std::out_of_range if the buffer is empty.
   */
  void pop_discard() {
    if (empty())
      throw std::out_of_range("RingBuffer is empty");

    _incr_head();
  }

  /** @brief Clears all elements. */
  void clear() noexcept {
    m_head = 0;
    m_tail = 0;
    m_size = 0;
  }

  /**
   * @brief Attempts to pop one element into @p out.
   * @return true if successful, false if empty.
   */
  bool try_pop(value_type& out) noexcept(noexcept(out = std::move(m_data[m_head]))) {
    if (empty())
      return false;

    out = std::move(m_data[m_head]);
    _incr_head();
    return true;
  }

  /**
   * @brief Attempts to push a copy of @p value.
   * @return true if successful, false if full.
   */
  bool try_push(const_reference value) noexcept(noexcept(m_data[m_tail] = value)) {
    if (is_full())
      return false;

    m_data[m_tail] = value;
    _incr_tail();
    return true;
  }

  /**
   * @brief Attempts to push a moved value.
   * @return true if successful, false if full.
   */
  bool try_push(value_type&& value) noexcept(noexcept(m_data[m_tail] = std::move(value))) {
    if (is_full())
      return false;

    m_data[m_tail] = std::move(value);
    _incr_tail();
    return true;
  }

  /**
   * @brief Attempts to emplace one element at tail.
   * @return true if successful, false if full.
   */
  template <typename... Args>
  bool try_emplace_back(Args&&... args) noexcept(
      noexcept(m_data[m_tail] = value_type(std::forward<Args>(args)...))) {
    if (is_full())
      return false;

    m_data[m_tail] = value_type(std::forward<Args>(args)...);
    _incr_tail();
    return true;
  }

  /**
   * @brief Attempts to discard one element from head.
   * @return true if successful, false if empty.
   */
  bool try_pop_discard() noexcept {
    if (empty())
      return false;

    _incr_head();
    return true;
  }

  /**
   * @brief Attempts to get pointer to head element.
   * @param[out] out Pointer to current head on success.
   * @return true if successful, false if empty.
   */
  bool try_peek(const_pointer& out) const noexcept {
    if (empty())
      return false;

    out = m_data.data() + m_head;
    return true;
  }

  /**
   * @brief Attempts to get mutable pointer to head element.
   * @return true if successful, false if empty.
   */
  bool try_front(pointer& out) noexcept {
    if (empty())
      return false;
    out = m_data.data() + m_head;
    return true;
  }

  /**
   * @brief Attempts to get const pointer to head element.
   * @return true if successful, false if empty.
   */
  bool try_front(const_pointer& out) const noexcept {
    if (empty())
      return false;
    out = m_data.data() + m_head;
    return true;
  }

  /**
   * @brief Attempts to get mutable pointer to tail-most element.
   * @return true if successful, false if empty.
   */
  bool try_back(pointer& out) noexcept {
    if (empty())
      return false;

    index_type idx = (m_tail + sm_storage_size - 1) & sm_bitmask;
    out = m_data.data() + idx;
    return true;
  }

  /**
   * @brief Attempts to get const pointer to tail-most element.
   * @return true if successful, false if empty.
   */
  bool try_back(const_pointer& out) const noexcept {
    if (empty())
      return false;

    index_type idx = (m_tail + sm_storage_size - 1) & sm_bitmask;
    out = m_data.data() + idx;
    return true;
  }

  /** @brief Returns iterator to first logical element. */
  [[nodiscard]] constexpr iterator begin() noexcept {
    return iterator(m_data.data(), &m_data[m_head], sm_storage_size);
  }

  /** @brief Returns iterator to one-past-last logical element. */
  [[nodiscard]] constexpr iterator end() noexcept {
    return iterator(m_data.data(), &m_data[m_tail], sm_storage_size);
  }

  /** @brief Returns const iterator to first logical element. */
  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return cbegin();
  }

  /** @brief Returns const iterator to one-past-last logical element. */
  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return cend();
  }

  /** @brief Returns const iterator to first logical element. */
  [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
    return const_iterator(m_data.data(), &m_data[m_head], sm_storage_size);
  }

  /** @brief Returns const iterator to one-past-last logical element. */
  [[nodiscard]] constexpr const_iterator cend() const noexcept {
    return const_iterator(m_data.data(), &m_data[m_tail], sm_storage_size);
  }

  /** @brief Returns reverse iterator to last logical element. */
  [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
    return std::reverse_iterator(end());
  }

  /** @brief Returns reverse iterator to before-first logical element. */
  [[nodiscard]] constexpr reverse_iterator rend() noexcept {
    return std::reverse_iterator(begin());
  }

  /** @brief Returns const reverse iterator to last logical element. */
  [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
    return std::reverse_iterator(cend());
  }

  /** @brief Returns const reverse iterator to before-first logical element. */
  [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
    return std::reverse_iterator(cbegin());
  }

  /** @brief Returns const reverse iterator to last logical element. */
  [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
    return std::reverse_iterator(cend());
  }

  /** @brief Returns const reverse iterator to before-first logical element. */
  [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
    return std::reverse_iterator(cbegin());
  }

  /** @brief Returns true if value exists in the logical sequence. */
  [[nodiscard]] constexpr bool contains(const T& value) const {
    for (const auto& v : *this)
      if (v == value)
        return true;

    return false;
  }

  /** @brief Swaps contents with another ring buffer. */
  void swap(RingBuffer& other) noexcept {
    std::swap(m_data, other.m_data);
    std::swap(m_head, other.m_head);
    std::swap(m_tail, other.m_tail);
    std::swap(m_size, other.m_size);
  }

  /** @brief ADL swap helper. */
  friend void swap(RingBuffer& a, RingBuffer& b) noexcept {
    a.swap(b);
  }

private:
  static constexpr size_type sm_storage_size = N;
  static constexpr size_type sm_bitmask = N - 1;
  std::array<T, sm_storage_size> m_data;
  index_type m_head = 0;
  index_type m_tail = 0;
  size_type m_size = 0;

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