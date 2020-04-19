#include "san-andreas.h"
#include "memory.h"
#include <cstdio>

// 0xC8D4C0 ; unsigned int gGameState
static auto& gGameState = *reinterpret_cast<std::uint32_t*>(0xC8D4C0);

// 0xC920D0 ; const char **aVideoModeList
static auto& aVideoModeList = *reinterpret_cast<const char***>(0xC920D0);

// 0x8CDE56 ; static unsigned char CStats::bShowUpdateStats
static auto& CStats__bShowUpdateStats = *reinterpret_cast<bool*>(0x8CDE56);

// 0x8D093C ; static int CLoadingScreen::m_currDisplayedSplash
static auto& CLoadingScreen__m_currDisplayedSplash = *reinterpret_cast<std::int32_t*>(0x8D093C);

// 0x5900B0 ; static void __cdecl CLoadingScreen::LoadSplashes(unsigned char, unsigned char)
using CLoadingScreen__LoadSplashes_t = void (__cdecl *)(std::uint8_t, std::uint8_t);
static auto CLoadingScreen__LoadSplashes = reinterpret_cast<CLoadingScreen__LoadSplashes_t>(0x5900B0);

// 0x5904D0 ; static void __cdecl CLoadingScreen::DisplayNextSplash()
using CLoadingScreen__DisplayNextSplash_t = void (__cdecl *)();
static auto CLoadingScreen__DisplayNextSplash = reinterpret_cast<CLoadingScreen__DisplayNextSplash_t>(0x5904D0);

// 0x5BA160 ; static bool __cdecl CGame::InitialiseEssentialsAfterRW()
using CGame__InitialiseEssentialsAfterRW_t = bool (__cdecl *)();
static auto CGame__InitialiseEssentialsAfterRW = reinterpret_cast<CGame__InitialiseEssentialsAfterRW_t>(0x5BA160);

// 0x5BFA90 ; static bool __cdecl CGame::InitialiseCoreDataAfterRW()
using CGame__InitialiseCoreDataAfterRW_t = bool (__cdecl *)();
static auto CGame__InitialiseCoreDataAfterRW = reinterpret_cast<CGame__InitialiseCoreDataAfterRW_t>(0x5BFA90);

// 0x506E50 ; void __thiscall CAudioEngine::SetEffectsFaderScalingFactor(float)
using CAudioEngine__SetEffectsFaderScalingFactor_t = void (__thiscall *)(void*, float);
static auto CAudioEngine__SetEffectsFaderScalingFactor = reinterpret_cast<CAudioEngine__SetEffectsFaderScalingFactor_t>(0x506E50);

// 0x5078F0 ; void __thiscall CAudioEngine::InitialisePostLoading()
using CAudioEngine__InitialisePostLoading_t = void (__thiscall *)(void*);
static auto CAudioEngine__InitialisePostLoading = reinterpret_cast<CAudioEngine__InitialisePostLoading_t>(0x5078F0);

// 0x53E580 ; void __cdecl InitialiseGame()
using InitialiseGame_t = bool (__cdecl *)();
static auto InitialiseGame = reinterpret_cast<InitialiseGame_t>(0x53E580);

// 0x747180 ; void __thiscall CGamma::Init()
using CGamma__Init_t = void (__thiscall *)(void*);
static auto CGamma__Init = reinterpret_cast<CGamma__Init_t>(0x747180);

// 0x746190 ; bool __cdecl psSelectDevice()
using psSelectDevice_t = bool (__cdecl *)();
static auto psSelectDevice = reinterpret_cast<psSelectDevice_t>(0x746190);

// 0x7F2CC0 ; bool __cdecl RwEngineGetNumVideoModes()
using RwEngineGetNumVideoModes_t = bool (__cdecl *)();
static auto RwEngineGetNumVideoModes = reinterpret_cast<RwEngineGetNumVideoModes_t>(0x7F2CC0);

// 0x7F2CF0 ; bool __cdecl RwEngineGetVideoModeInfo(RwVideoMode*, std::int32_t)
struct RwVideoMode {
  std::int32_t width;
  std::int32_t height;
  std::int32_t depth;
  std::int32_t flags;
  std::int32_t refreshRate;
  std::int32_t format;
};
using RwEngineGetVideoModeInfo_t = bool (__cdecl *)(RwVideoMode*, std::int32_t);
static auto RwEngineGetVideoModeInfo = reinterpret_cast<RwEngineGetVideoModeInfo_t>(0x7F2CF0);

// 0x7F2D50 ; bool __cdecl RwEngineSetVideoMode(std::int32_t)
using RwEngineSetVideoMode_t = bool (__cdecl *)(std::int32_t);
static auto RwEngineSetVideoMode = reinterpret_cast<RwEngineSetVideoMode_t>(0x7F2D50);

// 0x402012 ; const char** __cdecl GetAllVideoModes(bool)
using GetAllVideoModes_t = const char** (__cdecl *)(bool);
static auto GetAllVideoModes = reinterpret_cast<GetAllVideoModes_t>(0x402012);

// 0x745A80 ; void __cdecl FreeAllVideoModesStr()
using FreeAllVideoModesStr_t = const char** (__cdecl *)(bool);
static auto FreeAllVideoModesStr = reinterpret_cast<FreeAllVideoModesStr_t>(0x745A80);

static void PrepareGameStart();

static void __fastcall Hook_CGamma__Init(void* self) {
  CGamma__Init(self);
  PrepareGameStart();
}

static void __cdecl Hook_psSelectDevice() {
  const std::int32_t numVideoModes = RwEngineGetNumVideoModes();

  for (std::int32_t i = 0; i < numVideoModes; ++i) {
    RwVideoMode videoMode;
    RwEngineGetVideoModeInfo(&videoMode, i);

    if (videoMode.flags != 0)
      continue;

    RwEngineSetVideoMode(i);
    break;
  }
}

static const char** __cdecl Hook_GetAllVideoModes(bool) {
  if (aVideoModeList)
    return aVideoModeList;

  std::int32_t numVideoModes = RwEngineGetNumVideoModes();
  aVideoModeList = new const char*[numVideoModes];

  for (std::int32_t i = 0; i < numVideoModes; ++i) {
    RwVideoMode videoMode;
    RwEngineGetVideoModeInfo(&videoMode, i);

    if (videoMode.flags != 0) {
      aVideoModeList[i] = nullptr;
      continue;
    }

    char* videoModeName = new char[64];
    sprintf_s(videoModeName, 64, "%lu X %lu X %lu", videoMode.width, videoMode.height, videoMode.depth);
    aVideoModeList[i] = videoModeName;
  }

  return aVideoModeList;
}

static void __cdecl Hook_FreeAllVideoModesStr() {
  if (aVideoModeList) {
    std::int32_t numVideoModes = RwEngineGetNumVideoModes();

    for (std::int32_t i = 0; i < numVideoModes; ++i) {
      if (aVideoModeList[i])
        delete[] aVideoModeList[i];
    }

    delete[] aVideoModeList;
    aVideoModeList = nullptr;
  }
}

void PreProcessStart() {
  // Set default window size to 1024x768
  {
    // 0x61960C | C7 05 44 70 C1 00 80 02 00 00 | mov ds:dword_C17044, 280h ; 640
    // 0x619616 | C7 05 48 70 C1 00 E0 01 00 00 | mov ds:dword_C17048, 1E0h ; 480
    std::uint32_t width = 1368, height = 768;
    memory::copy(0x61960C + 0x6, width);
    memory::copy(0x619616 + 0x6, height);
  }

  // Disable loading of dpnhpast.dll (DirectPlay)
  {
    // >>> 0x745701 | 68 DC 48 87 00 | push offset "dpnhpast.dll"
    // >>> 0x745706 | FF D5          | call ebp ; LoadLibraryA(x)
    memory::nop(0x745701, 7);

    // Before: 0x74570C | 75 25 | jnz short loc_745733
    // After:  0x74570C | EB 25 | jmp short loc_745733
    memory::set(0x74570C, 0xEB, 1);

    // >>> 0x74573A | 56 | push esi ; dpnhpast.dll handle
    memory::nop(0x74573A, 1);

    // >>> 0x745743 | FF D6 | call esi ; FreeLibrary(x)
    memory::nop(0x745743, 2);
  }

  // Disable main menu
  // (Outcommented because of the GetAllVideoModes/FreeAllVideoModesStr hooks)
  {
    // >>> 0x53BF44 | E8 F7 F4 03 00 | call CMenuManager::Process(void)
    // memory::nop(0x53BF44, 5);
  }

  // Disable forced fullscreen
  {
    // Before: 0x74887B | 39 1D 18 21 C9 00 | cmp ds:bMultiAdapter, ebx
    // After:  0x74887B | EB 5D 90 90 90 90 | jmp 07488DAh (short, relative)
    memory::copy(0x74887B, {0xEB, 0x5D, 0x90, 0x90, 0x90, 0x90});
  }

  // Disable foreground check in WinMain loop
  {
    //     0x748719 | 33 DB             | xor ebx, ebx
    //     ...
    //     0x748A87 | 39 1D 1C 62 8D 00 | cmp ds:ForegroundApp, ebx
    // >>> 0x748A8D | 0F 84 20 03 00 00 | jz loc_748DB3
    memory::nop(0x748A8D, 6);
  }

  // Automatically select a windowed video mode
  memory::hook(&psSelectDevice, &Hook_psSelectDevice);

  // Patch GetAllVideoModes/FreeAllVideoModesStr to show windowed video modes
  memory::hook(&GetAllVideoModes, &Hook_GetAllVideoModes);
  memory::hook(&FreeAllVideoModesStr, &Hook_FreeAllVideoModesStr);

  // Force start game
  {
    // Before the main game-loop in WinMain, there is a 'CGamma::Init(void)' call, which we will hook.
    // In the hook we execute the hooked function and run our initialization code, which in turn
    // starts the game without any movie, splash or the main menu.

    // 0x748710 = int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
    // 0x748A23 | B9 34 21 C9 00 | mov ecx, offset g_GameGamma
    // 0x748A28 | ...
    // 0x748A30 | E8 4B E7 FF FF | call CGamma::Init()
    memory::hook(&CGamma__Init, &Hook_CGamma__Init);
  }
}

void PrepareGameStart() {
  if (gGameState != 0)
    return;

  // Show a random loading screen
  CLoadingScreen__m_currDisplayedSplash = 1;
  CLoadingScreen__LoadSplashes(0, 0);
  CLoadingScreen__DisplayNextSplash();

  // Initialise essentials and core data
  CGame__InitialiseEssentialsAfterRW();
  CGame__InitialiseCoreDataAfterRW();

  // Force start game
  CAudioEngine__SetEffectsFaderScalingFactor((void*)0xB6BC90, 1.0f);
  CStats__bShowUpdateStats = false;
  InitialiseGame();
  CAudioEngine__InitialisePostLoading((void*)0xB6BC90);

  // Main menu doesn't show the ingame-menu if we don't set this
  *reinterpret_cast<bool *>(0xBA6831) = 0;

  gGameState = 9;
}
