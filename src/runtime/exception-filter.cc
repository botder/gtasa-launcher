#include "exception-filter.h"
#include "memory.h"
#include <sstream>
#include <windows.h>
#include <filesystem>
#include <dbghelp.h>

extern HMODULE runtime_library_handle;

static LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo);
static std::filesystem::path GetCurrentModulePath();

void SetExceptionFilter() {
  // Set our unhandled exception filter
  SetUnhandledExceptionFilter(&CustomUnhandledExceptionFilter);

  // Disable the function SetUnhandledExceptionFilter
  memory::copy(&SetUnhandledExceptionFilter, {0xC2, 0x04, 0x00});
}

static LONG WINAPI CustomUnhandledExceptionFilter(PEXCEPTION_POINTERS exception_pointers) {
  // Create the 'dumps' directory
  std::filesystem::path module_path = GetCurrentModulePath();
  std::filesystem::path dumps_dir_path = module_path.parent_path() / "dumps";
  
  if (!std::filesystem::is_directory(dumps_dir_path)) {
    CreateDirectoryW(dumps_dir_path.wstring().c_str(), NULL);
  }

  // Generate an appropriate filename
  std::wstring datetime_text(64, L'\0');
  std::time_t current_time = std::time(nullptr);
  tm time{};
  localtime_s(&time, &current_time);
  std::wcsftime(datetime_text.data(), datetime_text.size(), L"%Y-%m-%d_%H-%M-%S.dmp", &time);

  // Create dump file
  std::wstring dumpfile_path = (dumps_dir_path / std::filesystem::path(datetime_text)).wstring();
  HANDLE file_handle = CreateFileW(dumpfile_path.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if (file_handle == NULL || file_handle == INVALID_HANDLE_VALUE) {
    return EXCEPTION_EXECUTE_HANDLER;
  }

  MINIDUMP_EXCEPTION_INFORMATION dump_exception_info{};
  dump_exception_info.ThreadId = GetCurrentThreadId();
  dump_exception_info.ExceptionPointers = exception_pointers;
  dump_exception_info.ClientPointers = TRUE;

  auto dump_type = (MINIDUMP_TYPE)(MiniDumpWithDataSegs | MiniDumpWithCodeSegs | MiniDumpWithIndirectlyReferencedMemory);

  if (MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file_handle, dump_type, &dump_exception_info, NULL, NULL)) {
    std::wstringstream message_buffer;
    message_buffer << "Unhandled exception caught and written as dump to " << dumpfile_path;
    MessageBoxW(NULL, message_buffer.str().c_str(), L"Unhandled exception handler", MB_ICONERROR | MB_OK);
    ShowCursor(TRUE);
  }

  CloseHandle(file_handle);
  return EXCEPTION_EXECUTE_HANDLER;
}

static std::filesystem::path GetCurrentModulePath() {
  std::wstring module_path(1024, L'\0');
  GetModuleFileNameW(runtime_library_handle, module_path.data(), module_path.size());
  return std::filesystem::path(module_path);
}
