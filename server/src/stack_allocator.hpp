#pragma once
#include <memory>
#include <iostream>


struct stack_state
{
  char * const _buffer;
  const size_t _buffer_size;
  size_t       _bytes_left;
  const bool   _deallocate = false;
};

template <class T>
struct stack_allocator
{


  using value_type                             = T;
  using size_type                              = size_t;
  using difference_type                        = std::ptrdiff_t;
  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal                        = std::true_type;

  stack_allocator() = delete;

  stack_allocator(stack_state & ss) : _stack_state(ss) {}

  template <class U>
  constexpr stack_allocator(const stack_allocator<U> & other) noexcept : _stack_state(other._stack_state)
  {
  }

  value_type * allocate(std::size_t n)
  {
    const size_t nbytes = sizeof(value_type) * n;

    if(nbytes > _stack_state._bytes_left)
    {
      throw std::bad_alloc();
    }

    auto result =
      reinterpret_cast<value_type *>(_stack_state._buffer + (_stack_state._buffer_size - _stack_state._bytes_left));
    _stack_state._bytes_left -= nbytes;
    return result;
  }
  void deallocate(T *, std::size_t n) noexcept
  {
    const size_t nbytes = sizeof(value_type) * n;
    if (_stack_state._deallocate)
      _stack_state._bytes_left += nbytes;
  }
  stack_state & _stack_state;
};


template <class T, class U>
bool operator==(const stack_allocator<T> &, const stack_allocator<U> &)
{
  return true;
}
template <class T, class U>
bool operator!=(const stack_allocator<T> &, const stack_allocator<U> &)
{
  return true;
}
