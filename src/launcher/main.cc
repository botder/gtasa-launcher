#include <iostream>
#include <filesystem>
#include <sstream>
#include <Windows.h>

bool StartGame(std::filesystem::path const& executable_path, std::filesystem::path const& current_dir_path);
bool CreateGameProcess(std::filesystem::path const& executable_path, PROCESS_INFORMATION& process_info);
bool CreateDebuggerProcess(unsigned long process_id);
bool InjectLibrary(HANDLE process, std::filesystem::path const& library_path);
bool CallRemoteFunction(HANDLE process, FARPROC function, void const* argument, std::size_t argument_size);

int wmain(int argc, wchar_t* argv[]) {
  std::filesystem::path application{argv[0]};

  if (argc < 2) {
    std::wcerr << "Usage: " << application.filename().wstring() << " <path to gta_sa.exe>\n";
    return EXIT_FAILURE;
  }

  StartGame(std::filesystem::path(argv[1]), application.parent_path());
  return 0;
}

bool StartGame(std::filesystem::path const& executable_path, std::filesystem::path const& current_dir_path) {
  std::filesystem::path runtime_library_path = current_dir_path / "launcher.runtime.dll";

  if (std::error_code ec; !std::filesystem::is_regular_file(runtime_library_path, ec) || ec) {
    std::wcerr << "Runtime library for launcher not found\n";
    return false;
  }

  PROCESS_INFORMATION process_info{};

  if (!CreateGameProcess(executable_path, process_info)) {
    std::wcerr << "Failed to create the game process\n";
    return false;
  }

  std::wcout << "Game has started (process: " << process_info.dwProcessId << ", thread: " << process_info.dwThreadId << ")\n";

  if (IsDebuggerPresent()) {
    std::wcout << "Debugger is present. Launching JIT debugger for game process\n";
    CreateDebuggerProcess(process_info.dwProcessId);
  }

  if (!InjectLibrary(process_info.hProcess, runtime_library_path)) {
    std::wcout << "Failed to inject the runtime library\n";
    return false;
  }

  ResumeThread(process_info.hThread);
  CloseHandle(process_info.hThread);
  CloseHandle(process_info.hProcess);
  return true;
}

bool CreateGameProcess(std::filesystem::path const& executable_path, PROCESS_INFORMATION& process_info) {
  std::wstring application_name = executable_path.wstring();
  std::wstring current_directory = executable_path.parent_path().wstring();
  DWORD creation_flags = CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED | CREATE_UNICODE_ENVIRONMENT;
  STARTUPINFO startup_info{};
  startup_info.cb = sizeof(startup_info);
  return CreateProcess(application_name.c_str(), nullptr, nullptr, nullptr, false, creation_flags, nullptr, current_directory.c_str(),
                       &startup_info, &process_info) != 0;
}

bool CreateDebuggerProcess(unsigned long process_id) {
  // Create path to vsjitdebugger.exe (usually in C:/Windows/system32)
  std::wstring system_dir_path(MAX_PATH + 1, '\0');
  UINT system_dir_path_length = GetSystemDirectoryW(&system_dir_path[0], system_dir_path.size());

  if (!system_dir_path_length) {
    return false;
  }

  if (system_dir_path_length > system_dir_path.size()) {
    system_dir_path.resize(system_dir_path_length + 1, '\0');
    system_dir_path_length = GetSystemDirectoryW(&system_dir_path[0], system_dir_path.size());

    if (!system_dir_path_length) {
      return false;
    }
  }

  system_dir_path.resize(system_dir_path_length);

  std::wostringstream command_line_buffer;
  command_line_buffer << system_dir_path << L"\\vsjitdebugger.exe -p " << process_id;
  std::wstring command_line = command_line_buffer.str();

  // Start Visual Studio JIT Debugger
  STARTUPINFO startup_info{};
  startup_info.cb = sizeof(startup_info);

  PROCESS_INFORMATION process_info{};

  if (!CreateProcess(nullptr, command_line.data(), nullptr, nullptr, false, 0, nullptr, nullptr, &startup_info, &process_info))
      return false;

  WaitForSingleObject(process_info.hProcess, INFINITE);
  CloseHandle(process_info.hThread);
  CloseHandle(process_info.hProcess);
  return true;
}

bool InjectLibrary(HANDLE process, std::filesystem::path const& library_path) {
  HMODULE kernel32 = GetModuleHandle(L"kernel32");

  if (!kernel32) {
    return false;
  }

  FARPROC function = GetProcAddress(kernel32, "LoadLibraryA");

  if (!function) {
    return false;
  }

  std::string argument = library_path.string();
  return CallRemoteFunction(process, function, argument.data(), argument.size() + 1);
}

bool CallRemoteFunction(HANDLE process, FARPROC function, void const* argument, std::size_t argument_size) {
  void* remote_base_address = nullptr;

  if (argument_size > 0) {
    remote_base_address = VirtualAllocEx(process, nullptr, argument_size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (!remote_base_address) {
      return false;
    }

    SIZE_T num_bytes_written = 0;

    if (BOOL write_success = WriteProcessMemory(process, remote_base_address, argument, argument_size, &num_bytes_written);
        !write_success || num_bytes_written != argument_size) {
      VirtualFreeEx(process, remote_base_address, argument_size, MEM_RELEASE);
      return false;
    }
  }

  HANDLE remote_thread = CreateRemoteThread(process, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(function),
                                            remote_base_address, 0, nullptr);

  if (!remote_thread) {
    if (remote_base_address) {
      VirtualFreeEx(process, remote_base_address, argument_size, MEM_RELEASE);
    }

    return false;
  }

  SetThreadPriority(remote_thread, THREAD_PRIORITY_TIME_CRITICAL);
  WaitForSingleObject(remote_thread, INFINITE);
  CloseHandle(remote_thread);

  if (remote_base_address) {
    VirtualFreeEx(process, remote_base_address, argument_size, MEM_RELEASE);
  }

  return true;
}
