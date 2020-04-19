# Source: https://github.com/microsoft/detours/tree/4.0.1
add_library(detours STATIC
    "src/creatwth.cpp"
    "src/detours.cpp"
    "src/detours.h"
    "src/detver.h"
    "src/disasm.cpp"
    "src/image.cpp"
    "src/modules.cpp"
)

target_compile_definitions(detours INTERFACE "_X86_")
target_include_directories(detours PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src")
