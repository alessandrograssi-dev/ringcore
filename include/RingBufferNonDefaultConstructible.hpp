#ifndef RING_BUFFER_NON_DEFAULT_CONSTRUCTIBLE_HPP
#define RING_BUFFER_NON_DEFAULT_CONSTRUCTIBLE_HPP

/**
 * @file RingBufferNonDefaultConstructible.hpp
 * @brief Fixed-capacity ring buffer and bidirectional iterator implementation.
 * @author Alessandro Grassi
 * @date 2026
 *
 * This header provides:
 * - `RingBufferNonDefaultConstructibleIterator<RingBufferType>`: a lightweight
 *   bidirectional iterator
 *   that wraps around an underlying contiguous storage region.
 * - `RingBufferNonDefaultConstructible<T, N>`: a fixed-size circular buffer
 *   with both throwing and
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
#include <cassert>
#include <iterator>

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
template <typename RingBufferType> class RingBufferNonDefaultConstructibleIterator {
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
  /** @brief Default constructor required by iterator concepts used by std adapters. */
  constexpr RingBufferNonDefaultConstructibleIterator() noexcept
      : m_start_buffer(nullptr), m_ptr(nullptr), m_storage_size(0) {}

  /**
   * @brief Constructs an iterator over the ring storage.
   * @param start_buffer Pointer to start of storage.
   * @param ptr Current iterator position.
   * @param storage Total storage size used for wrap-around.
   */
  constexpr RingBufferNonDefaultConstructibleIterator(pointer start_buffer, pointer ptr,
                                                      size_type storage)
      : m_start_buffer(start_buffer), m_ptr(ptr), m_storage_size(storage) {}

  /** @brief Prefix increment with wrap-around semantics. */
  RingBufferNonDefaultConstructibleIterator& operator++() {
    assert(m_ptr != nullptr && m_start_buffer != nullptr);
    if (++m_ptr == m_start_buffer + m_storage_size)
      m_ptr = m_start_buffer;
    return *this;
  }

  /** @brief Postfix increment with wrap-around semantics. */
  RingBufferNonDefaultConstructibleIterator operator++(int) {
    RingBufferNonDefaultConstructibleIterator it = *this;
    ++(*this);
    return it;
  }

  /** @brief Prefix decrement with wrap-around semantics. */
  RingBufferNonDefaultConstructibleIterator& operator--() {
    assert(m_ptr != nullptr && m_start_buffer != nullptr);
    if (m_ptr == m_start_buffer) {
      m_ptr = m_start_buffer + m_storage_size - 1;
    } else {
      --m_ptr;
    }
    return *this;
  }

  /** @brief Postfix decrement with wrap-around semantics. */
  RingBufferNonDefaultConstructibleIterator operator--(int) {
    RingBufferNonDefaultConstructibleIterator it = *this;
    --(*this);
    return it;
  }

  /** @brief Equality comparison (same position in same storage). */
  bool operator==(const RingBufferNonDefaultConstructibleIterator& other) const noexcept {
    assert(m_start_buffer == other.m_start_buffer);
    return m_ptr == other.m_ptr;
  }

  /** @brief Inequality comparison. */
  bool operator!=(const RingBufferNonDefaultConstructibleIterator& other) const noexcept {
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
template <typename T, std::size_t N> class RingBufferNonDefaultConstructible {
  static_assert(N > 0, "RingBufferNonDefaultConstructible: capacity must be bigger than 0");
  static_assert((N & (N - 1)) == 0,
                "RingBufferNonDefaultConstructible: capacity must be a power of 2");

public:
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using iterator =
      RingBufferNonDefaultConstructibleIterator<RingBufferNonDefaultConstructible<T, N>>;
  using const_iterator =
      RingBufferNonDefaultConstructibleIterator<const RingBufferNonDefaultConstructible<T, N>>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using index_type = std::size_t;

public:
  /**
   * @brief Constructs an empty ring buffer.
   */
  explicit RingBufferNonDefaultConstructible() noexcept {
    static_assert(N > 0, "RingBuffer capacity must be > 0");
  }

  /**
   * @brief Constructs a ring buffer pre-filled with @p size copies of @p value.
   * @param size Number of elements to insert initially.
   * @param value Value copied into each inserted slot.
   * @throws std::out_of_range if @p size is greater than storage size N.
   */
  explicit RingBufferNonDefaultConstructible(size_type size, const_reference value)
      : RingBufferNonDefaultConstructible() {
    if (size > N)
      throw std::out_of_range("The size cannot be bigger than the capacity");

    for (index_type i = 0; i < size; ++i) {
      push(value);
    }
  }

  RingBufferNonDefaultConstructible(const RingBufferNonDefaultConstructible&) = default;
  RingBufferNonDefaultConstructible& operator=(const RingBufferNonDefaultConstructible&) = default;
  RingBufferNonDefaultConstructible(RingBufferNonDefaultConstructible&&) noexcept = default;
  RingBufferNonDefaultConstructible&
  operator=(RingBufferNonDefaultConstructible&&) noexcept = default;

  ~RingBufferNonDefaultConstructible() {
    clear();
  }

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
    return N - 1;
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
    return *ptr(m_head);
  }

  /**
   * @brief Returns the first logical element (const overload).
   * @throws std::out_of_range if the buffer is empty.
   */
  [[nodiscard]] constexpr const_reference front() const {
    if (empty())
      throw std::out_of_range("The RingBuffer is empty");
    return *ptr(m_head);
  }

  /**
   * @brief Returns the last logical element.
   * @throws std::out_of_range if the buffer is empty.
   */
  [[nodiscard]] constexpr reference back() {
    if (empty())
      throw std::out_of_range("The RingBuffer is empty");

    index_type idx = (m_tail + N - 1) & sm_bitmask;
    return *ptr(idx);
  }

  /**
   * @brief Returns the last logical element (const overload).
   * @throws std::out_of_range if the buffer is empty.
   */
  [[nodiscard]] constexpr const_reference back() const {
    if (empty())
      throw std::out_of_range("The RingBuffer is empty");

    index_type idx = (m_tail + N - 1) & sm_bitmask;
    return *ptr(idx);
  }

  /**
   * @brief Returns the first logical element.
   * @throws std::out_of_range if the buffer is empty.
   */
  reference peek() {
    if (empty())
      throw std::out_of_range("RingBuffer is empty");

    return *ptr(m_head);
  }

  /**
   * @brief Returns the first logical element (const overload).
   * @throws std::out_of_range if the buffer is empty.
   */
  const_reference peek() const {
    if (empty())
      throw std::out_of_range("RingBuffer is empty");

    return *ptr(m_head);
  }

  /**
   * @brief Unchecked indexed access relative to logical head.
   * @param i Logical index in the range [0, size()).
   * @return Const reference to the element at logical index @p i.
   * @pre i < size()
   */
  constexpr const_reference operator[](index_type i) const {
    assert(i < size());
    return *ptr((m_head + i) & sm_bitmask);
  }

  /**
   * @brief Unchecked indexed access relative to logical head.
   * @param i Logical index in the range [0, size()).
   * @return Mutable reference to the element at logical index @p i.
   * @pre i < size()
   */
  constexpr reference operator[](index_type i) {
    assert(i < size());
    return *ptr((m_head + i) & sm_bitmask);
  }

  /**
   * @brief Checked indexed access relative to logical head.
   * @param i Logical index in the range [0, size()).
   * @return Const reference to the element at logical index @p i.
   * @throws std::out_of_range if i >= size().
   */
  const_reference at(index_type i) const {
    if (i >= size())
      throw std::out_of_range("RingBuffer: index out of range");
    return *ptr((m_head + i) & sm_bitmask);
  }

  /**
   * @brief Checked indexed access relative to logical head.
   * @param i Logical index in the range [0, size()).
   * @return Mutable reference to the element at logical index @p i.
   * @throws std::out_of_range if i >= size().
   */
  reference at(index_type i) {
    if (i >= size())
      throw std::out_of_range("RingBuffer: index out of range");
    return *ptr((m_head + i) & sm_bitmask);
  }

  /**
   * @brief Pushes one element at the logical tail.
   * @tparam U Source value category/type.
   * @param value Value to copy/move into the tail slot.
   * @throws std::out_of_range if the buffer is full.
   */
  template <typename U>
    requires std::assignable_from<T&, U&&>
  void push(U&& value) {
    if (is_full())
      throw std::out_of_range("RingBuffer is full");

    construct(m_tail, std::forward<U>(value));
    _incr_tail();
  }

  /**
   * @brief Pushes one element, overwriting oldest one when full.
   * @tparam U Source value category/type.
   * @param value Value to copy/move into the tail slot.
   */
  template <typename U>
    requires std::assignable_from<T&, U&&>
  void push_overwrite(U&& value) noexcept {
    const bool was_full = is_full();
    construct(m_tail, std::forward<U>(value));
    _incr_tail();
    if (was_full) {
      destroy(m_head);
      _incr_head();
    }
  }

  /**
   * @brief Constructs an element at the logical tail.
   * @tparam Args Constructor argument types for T.
   * @param args Constructor arguments forwarded to T.
   * @throws std::out_of_range if the buffer is full.
   */
  template <typename... Args> void emplace_back(Args&&... args) {
    if (is_full())
      throw std::out_of_range("RingBuffer is full");

    construct(m_tail, std::forward<Args>(args)...);
    _incr_tail();
  }

  /**
   * @brief Constructs an element at the logical head (front insertion).
   * @tparam Args Constructor argument types for T.
   * @param args Constructor arguments forwarded to T.
   * @throws std::out_of_range if the buffer is full.
   */
  template <typename... Args> void emplace_front(Args&&... args) {
    if (is_full())
      throw std::out_of_range("RingBuffer is full");

    if (m_head == 0) {
      m_head = N - 1;
    } else {
      --m_head;
    }
    construct(m_head, std::forward<Args>(args)...);
    ++m_size;
  }

  /**
   * @brief Pops and returns the element at logical head.
   * @throws std::out_of_range if the buffer is empty.
   */
  value_type pop() {
    if (empty())
      throw std::out_of_range("RingBuffer is empty");

    T value = std::move(*ptr(m_head));
    destroy(m_head);
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

    destroy(m_head);
    _incr_head();
  }

  /** @brief Clears all elements. */
  void clear() noexcept {
    while (m_size)
      pop_discard();
    m_head = 0;
    m_tail = 0;
  }

  /**
   * @brief Attempts to pop one element into @p out.
   * @param[out] out Destination for the popped value when successful.
   * @return true if successful, false if empty.
   */
  bool try_pop(value_type& out) noexcept(noexcept(out = std::move(*ptr(m_head)))) {
    if (empty())
      return false;

    out = std::move(*ptr(m_head));
    destroy(m_head);
    _incr_head();
    return true;
  }

  /**
   * @brief Attempts to push a copy of @p value.
   * @param value Value to copy into the tail slot.
   * @return true if successful, false if full.
   */
  bool try_push(const_reference value) noexcept(noexcept(construct(m_tail, value))) {
    if (is_full())
      return false;

    construct(m_tail, value);
    _incr_tail();
    return true;
  }

  /**
   * @brief Attempts to push a moved value.
   * @param value Value to move into the tail slot.
   * @return true if successful, false if full.
   */
  bool try_push(value_type&& value) noexcept(noexcept(construct(m_tail, std::move(value)))) {
    if (is_full())
      return false;

    construct(m_tail, std::move(value));
    _incr_tail();
    return true;
  }

  /**
   * @brief Attempts to emplace one element at tail.
   * @tparam Args Constructor argument types for T.
   * @param args Constructor arguments forwarded to T.
   * @return true if successful, false if full.
   */
  template <typename... Args>
  bool try_emplace_back(Args&&... args) noexcept(noexcept(construct(m_tail,
                                                                    std::forward<Args>(args)...))) {
    if (is_full())
      return false;

    construct(m_tail, std::forward<Args>(args)...);
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

    destroy(m_head);
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

    out = ptr(m_head);
    return true;
  }

  /**
   * @brief Attempts to get mutable pointer to head element.
   * @param[out] out Pointer to head element on success.
   * @return true if successful, false if empty.
   */
  bool try_front(pointer& out) noexcept {
    if (empty())
      return false;
    out = ptr(m_head);
    return true;
  }

  /**
   * @brief Attempts to get const pointer to head element.
   * @param[out] out Pointer to head element on success.
   * @return true if successful, false if empty.
   */
  bool try_front(const_pointer& out) const noexcept {
    if (empty())
      return false;
    out = ptr(m_head);
    return true;
  }

  /**
   * @brief Attempts to get mutable pointer to tail-most element.
   * @param[out] out Pointer to last logical element on success.
   * @return true if successful, false if empty.
   */
  bool try_back(pointer& out) noexcept {
    if (empty())
      return false;

    index_type idx = (m_tail + N - 1) & sm_bitmask;
    out = ptr(idx);
    return true;
  }

  /**
   * @brief Attempts to get const pointer to tail-most element.
   * @param[out] out Pointer to last logical element on success.
   * @return true if successful, false if empty.
   */
  bool try_back(const_pointer& out) const noexcept {
    if (empty())
      return false;

    index_type idx = (m_tail + N - 1) & sm_bitmask;
    out = ptr(idx);
    return true;
  }

  /** @brief Returns iterator to first logical element. */
  [[nodiscard]] constexpr iterator begin() noexcept {
    return iterator(ptr(0), ptr(m_head), N);
  }

  /** @brief Returns iterator to one-past-last logical element. */
  [[nodiscard]] constexpr iterator end() noexcept {
    return iterator(ptr(0), ptr(m_tail), N);
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
    return const_iterator(ptr(0), ptr(m_head), N);
  }

  /** @brief Returns const iterator to one-past-last logical element. */
  [[nodiscard]] constexpr const_iterator cend() const noexcept {
    return const_iterator(ptr(0), ptr(m_tail), N);
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
  void swap(RingBufferNonDefaultConstructible& other) noexcept {
    std::swap(m_data, other.m_data);
    std::swap(m_head, other.m_head);
    std::swap(m_tail, other.m_tail);
    std::swap(m_size, other.m_size);
  }

  /** @brief ADL swap helper. */
  friend void swap(RingBufferNonDefaultConstructible& a,
                   RingBufferNonDefaultConstructible& b) noexcept {
    a.swap(b);
  }

private:
  static constexpr size_type sm_bitmask = N - 1;
  // std::array<T, N> m_data;
  alignas(T) std::byte m_data[N][sizeof(T)];
  index_type m_head = 0;
  index_type m_tail = 0;
  size_type m_size = 0;

private:
  [[nodiscard]] constexpr static size_type storage() noexcept {
    return N;
  }

  constexpr void _incr_tail() noexcept {
    m_tail = (m_tail + 1) & sm_bitmask;
    ++m_size;
  }

  constexpr void _incr_head() noexcept {
    m_head = (m_head + 1) & sm_bitmask;
    --m_size;
  }

  inline T* ptr(size_t i) {
    return std::launder(reinterpret_cast<T*>(m_data[i]));
  }

  inline const T* ptr(size_t i) const {
    return std::launder(reinterpret_cast<const T*>(m_data[i]));
  }

  inline void construct(size_t i, const T& value) {
    std::construct_at(ptr(i), value);
  }

  inline void construct(size_t i, T&& value) {
    std::construct_at(ptr(i), std::move(value));
  }

  template <typename... Args>
  inline void construct(size_t i, Args&&... args)
    requires(sizeof...(Args) != 1)
  {
    std::construct_at(ptr(i), std::forward<Args>(args)...);
  }

  inline constexpr void destroy(size_t i) {
    if constexpr (!std::is_trivially_destructible_v<T>) {
      std::destroy_at(ptr(i));
    }
  }
};

#endif // RING_BUFFER_NON_DEFAULT_CONSTRUCTIBLE_HPP