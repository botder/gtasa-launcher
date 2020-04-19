#pragma once

#include <cstdint>
#include <cstddef>
#include <initializer_list>

namespace memory
{
  void hook(std::intptr_t from, std::intptr_t to);

  template <typename FnFrom, typename FnTo>
  void hook(FnFrom from, FnTo to) {
    hook(reinterpret_cast<std::intptr_t>(from), reinterpret_cast<std::intptr_t>(to));
  }

  void nop(std::intptr_t address, std::size_t count);
  void set(std::intptr_t address, std::uint8_t byte, std::size_t count);
  void copy(std::intptr_t address, const std::uint8_t bytes[], std::size_t count);

  template <typename T, std::size_t N>
  void copy(T address, const std::uint8_t (&bytes)[N]) {
    copy(std::intptr_t(address), bytes, N);
  }

  template <typename T>
  void copy(T address, std::initializer_list<std::uint8_t> data) {
    copy(std::intptr_t(address), data.begin(), data.size());
  }

  template <typename T, typename DataT>
  void copy(T address, const DataT& data) {
    copy(std::intptr_t(address), (const std::uint8_t*)&data, sizeof(DataT));
  }
}
