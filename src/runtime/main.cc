#include <Windows.h>

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
{
    switch (reason) 
    { 
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}
