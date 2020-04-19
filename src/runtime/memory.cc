#include "memory.h"
#include <windows.h>
#include <detours.h>
#include <cstring>

void memory::hook(std::intptr_t from, std::intptr_t to) {
  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());
  DetourAttach(reinterpret_cast<void**>(from), reinterpret_cast<void*>(to));
  DetourTransactionCommit();
}

void memory::nop(std::intptr_t address, std::size_t count) {
  memory::set(address, 0x90, count);
}

void memory::set(std::intptr_t address, std::uint8_t byte, std::size_t count) {
  DWORD dwOldProtect;
  auto pAddress = reinterpret_cast<void*>(address);
  VirtualProtect(pAddress, count, PAGE_READWRITE, &dwOldProtect);
  std::memset(pAddress, byte, count);
  VirtualProtect(pAddress, count, dwOldProtect, &dwOldProtect);
}

void memory::copy(std::intptr_t address, const std::uint8_t bytes[], std::size_t count) {
  DWORD dwOldProtect;
  auto pAddress = reinterpret_cast<void*>(address);
  VirtualProtect(pAddress, count, PAGE_READWRITE, &dwOldProtect);
  std::memcpy(pAddress, reinterpret_cast<const void*>(bytes), count);
  VirtualProtect(pAddress, count, dwOldProtect, &dwOldProtect);
}
