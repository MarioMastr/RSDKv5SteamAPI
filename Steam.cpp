#define _CRT_SECURE_NO_WARNINGS
#include "Steam.hpp"
// Workaround
#undef false
#undef true
#include <string>
#pragma warning(push)
#pragma warning(disable : 4819)
#include <steam_api.h>
#include <filesystem>
#pragma warning(pop)

namespace fs = std::filesystem;

class SteamCallbacks
{
public:
    STEAM_CALLBACK(SteamCallbacks, OnUserStatsReceived, UserStatsReceived_t);
    STEAM_CALLBACK(SteamCallbacks, GameOverlayActivated, GameOverlayActivated_t);
};


SteamCallbacks* SteamCallbacksInstance;
bool32 SteamUserStatsReceived = false;

void SteamCallbacks::OnUserStatsReceived(UserStatsReceived_t* pCallback)
{
    SteamUserStatsReceived = pCallback->m_eResult == k_EResultOK;
    RSDK.PrintLog(PRINT_NORMAL, "Logged into Steam account %lld", pCallback->m_steamIDUser.GetAccountID());
}

void SteamCallbacks::GameOverlayActivated(GameOverlayActivated_t* pCallback)
{
    // Check for DLC when the overlay closes
    if (!pCallback->m_bActive)
        CheckDLCs();
}

#if STEAM_API_NODLL
PROC_TYPE steamAPIHandle = nullptr;

S_API ESteamAPIInitResult S_CALLTYPE SteamAPI_InitFlat(SteamErrMsg *errMsg)
{
    if (steamAPIHandle) {
        void* address = getSymbol(steamAPIHandle, "SteamAPI_InitFlat");
        return ((ESteamAPIInitResult(S_CALLTYPE *)(SteamErrMsg *errMsg))(address))(errMsg);
    }
    return k_ESteamAPIInitResult_FailedGeneric;
}

S_API void S_CALLTYPE SteamAPI_Shutdown()
{
    if (steamAPIHandle) {
        void* address = getSymbol(steamAPIHandle, "SteamAPI_Shutdown");
        return ((void(S_CALLTYPE*)())(address))();
    }
}

S_API void S_CALLTYPE SteamAPI_RunCallbacks()
{
    if (steamAPIHandle) {
        void* address = getSymbol(steamAPIHandle, "SteamAPI_RunCallbacks");
        return ((void(S_CALLTYPE*)())(address))();
    }
}

S_API bool S_CALLTYPE SteamAPI_IsSteamRunning()
{
    if (steamAPIHandle) {
        void* address = getSymbol(steamAPIHandle, "SteamAPI_IsSteamRunning");
        return ((bool(S_CALLTYPE*)())(address))();
    }
    return false;
}

S_API HSteamPipe SteamAPI_GetHSteamPipe()
{
    if (steamAPIHandle) {
        void* address = getSymbol(steamAPIHandle, "SteamAPI_GetHSteamPipe");
        return ((HSteamPipe(S_CALLTYPE*)())(address))();
    }
    return 0;
}

S_API HSteamUser SteamAPI_GetHSteamUser()
{
    if (steamAPIHandle) {
        void* address = getSymbol(steamAPIHandle, "SteamAPI_GetHSteamUser");
        return ((HSteamUser(S_CALLTYPE*)())(address))();
    }
    return 0;
}

S_API void* S_CALLTYPE SteamInternal_ContextInit(void* pContextInitData)
{
    if (steamAPIHandle) {
        void* address = getSymbol(steamAPIHandle, "SteamInternal_ContextInit");
        return ((void* (S_CALLTYPE*)(void* pContextInitData))(address))(pContextInitData);
    }
    return nullptr;
}

S_API void* S_CALLTYPE SteamInternal_CreateInterface(const char* ver)
{
    if (steamAPIHandle) {
        void* address = getSymbol(steamAPIHandle, "SteamInternal_CreateInterface");
        return ((void* (S_CALLTYPE*)(const char* ver))(address))(ver);
    }
    return nullptr;
}

S_API void S_CALLTYPE SteamAPI_RegisterCallback(class CCallbackBase* pCallback, int iCallback)
{
    if (steamAPIHandle) {
        void* address = getSymbol(steamAPIHandle, "SteamAPI_RegisterCallback");
        ((void (S_CALLTYPE*)(class CCallbackBase* pCallback, int iCallback))(address))(pCallback, iCallback);
    }
}

S_API void S_CALLTYPE SteamAPI_UnregisterCallback(class CCallbackBase* pCallback)
{
    if (steamAPIHandle) {
        void* address = getSymbol(steamAPIHandle, "SteamAPI_UnregisterCallback");
        ((void (S_CALLTYPE*)(class CCallbackBase* pCallback))(address))(pCallback);
    }
}

#endif

bool Steam::OpenDLCStore(uint8 id)
{
    switch (id)
    {
    case 0:
        SteamFriends()->ActivateGameOverlayToWebPage("https://store.steampowered.com/app/845640/Sonic_Mania__Encore_DLC/");
        return true;
    default:
        return false;
    }
}

const char* Steam::GetProfileName()
{
    return SteamFriends()->GetPersonaName();
}

void Steam::UnlockAchievement(const char* id)
{
    if (SteamUserStatsReceived && API.GetAchievementsEnabled())
    {
        SteamUserStats()->SetAchievement(id);
        SteamUserStats()->StoreStats();
    }
}

void Steam::SetPresence(const char* text)
{
    SteamFriends()->SetRichPresence("status", text);
}

bool Steam::CheckDLCOwnership(uint8 id)
{
    switch (id)
    {
    case 0:
        return SteamApps()->BIsSubscribedApp(845640);

    default:
        return false;
    }

}

bool Steam::InitAPI()
{
#if STEAM_API_NODLL
    auto oldWorkingDirectory = fs::current_path();

    String pathAsString;
    Mod.GetModPath("RSDKv5SteamAPI", &pathAsString);

    char newWorkingDirectory[256];
    RSDK.GetCString(newWorkingDirectory, &pathAsString);

    fs::current_path(newWorkingDirectory);
    if (!(steamAPIHandle = getLibrary(STEAM_DLL_NAME)))
    {
        RSDK.PrintLog(PRINT_ERROR, "Failed to load SteamAPI.");
        steamAPIHandle = nullptr;
        fs::current_path(oldWorkingDirectory);
        return false;
    }
#endif

    SteamErrMsg errMsg;
    if (SteamAPI_InitFlat(&errMsg) != k_ESteamAPIInitResult_OK)
    {
        RSDK.PrintLog(PRINT_ERROR, "Failed to initialise the Steam API: %s", errMsg);
#if STEAM_API_NODLL
        fs::current_path(oldWorkingDirectory);
#endif
        return false;
    }
    else
    {
        SteamCallbacksInstance = new SteamCallbacks();
        if (!SteamUserStats()->RequestUserStats(SteamUser()->GetSteamID()))
            RSDK.PrintLog(PRINT_NORMAL, "Failed to request current stats from Steam.");
    }
#if STEAM_API_NODLL
    fs::current_path(oldWorkingDirectory);
#endif
    return true;
}

void Steam::ShutdownAPI()
{
    SteamAPI_Shutdown();
}

void Steam::RunCallbacks()
{
    SteamAPI_RunCallbacks();
}
