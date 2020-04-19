#include "san-andreas.h"
#include "exception-filter.h"
#include <cstdio>
#include <windows.h>

HMODULE runtime_library_handle = nullptr;

static DWORD WINAPI ThreadEntrypoint(LPVOID);

BOOL WINAPI DllMain(HINSTANCE handle, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    runtime_library_handle = handle;
    CreateThread(NULL, 0, &ThreadEntrypoint, NULL, 0, NULL);
  }

  return TRUE;
}

static DWORD WINAPI ThreadEntrypoint(LPVOID) {
  SetExceptionFilter();
  AllocConsole();
  SetConsoleTitleW(L"GTA:SA Console");
  FILE* unused_file;
  freopen_s(&unused_file, "CONOUT$", "wt", stdout);
  freopen_s(&unused_file, "CONOUT$", "wt", stderr);
  PreProcessStart();
  return 0;
}
