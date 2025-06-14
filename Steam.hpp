#pragma once
#include "Mod.hpp"
#define STEAM_API_NODLL 1

#if defined(_AMD64_)
#define STEAM_DLL_NAME "steam_api64.dll"
#elif defined(_X86_)
#define STEAM_DLL_NAME "steam_api.dll"
#elif defined(__APPLE__)
#define STEAM_DLL_NAME "libsteam_api.dylib"
#endif

namespace Steam
{
    bool InitAPI();
    void ShutdownAPI();
    void RunCallbacks();
    bool OpenDLCStore(uint8 id);
    const char* GetProfileName();
    void UnlockAchievement(const char* id);
    void SetPresence(const char* text);
    bool CheckDLCOwnership(uint8 id);
}
