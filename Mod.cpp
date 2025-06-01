#include "Mod.hpp"
#include "Steam.hpp"

bool32 EnabledDLC[DLC_COUNT];
void* ModHandle = nullptr;

// Hooks
install_hook_name(ShowExtensionOverlay, bool32, uint8 overlay)
{
    return Steam::OpenDLCStore(overlay);
}
install_hook_name(GetUsername, bool32, String* userName)
{
    const char* name = Steam::GetProfileName();
    if (name)
    {
        RSDK.SetString(userName, name);
        return true;
    }
    return orig_GetUsername(userName);
}
install_hook_name(TryUnlockAchievement, void, AchievementID *id)
{
    Steam::UnlockAchievement(id->id);
}
install_hook_name(SetRichPresence, void, int32 id, String* message)
{
    char buffer[256];
    RSDK.GetCString(buffer, message);

    Steam::SetPresence(buffer);
}

install_hook_name(CheckDLC, bool32, uint8 id)
{
    if (id >= 0 && id <= 8)
        return EnabledDLC[id];
    return false;
}

void CheckDLCs()
{
    for (int i = 0; i < DLC_COUNT; ++i)
        EnabledDLC[i] = Steam::CheckDLCOwnership(i);
}

void OnUpdate(void* data)
{
    Steam::RunCallbacks();
}

void OnGameStartUp(void* data)
{
    CheckDLCs();
}

void InitModAPI()
{
    if (Steam::InitAPI())
    {
        install_hook_ShowExtensionOverlay(reinterpret_cast<void *>(API.ShowExtensionOverlay));
        install_hook_GetUsername(reinterpret_cast<void *>(API.GetUsername));
        install_hook_TryUnlockAchievement(reinterpret_cast<void *>(API.TryUnlockAchievement));
        install_hook_SetRichPresence(reinterpret_cast<void *>(API.SetRichPresence));
        install_hook_CheckDLC(reinterpret_cast<void *>(API.CheckDLC));

        memset(EnabledDLC, 0, sizeof(EnabledDLC));

        Mod.AddModCallback(MODCB_ONUPDATE, OnUpdate);
        Mod.AddModCallback(MODCB_ONGAMESTARTUP, OnGameStartUp);
    }
}

extern "C" DLLExport void UnloadMod()
{
    DobbyDestroy(reinterpret_cast<void *>(API.ShowExtensionOverlay));
    DobbyDestroy(reinterpret_cast<void *>(API.GetUsername));
    DobbyDestroy(reinterpret_cast<void *>(API.TryUnlockAchievement));
    DobbyDestroy(reinterpret_cast<void *>(API.SetRichPresence));
    DobbyDestroy(reinterpret_cast<void *>(API.CheckDLC));
    Steam::ShutdownAPI();
}

extern "C" DLLExport bool32 LinkModLogic(EngineInfo* info, const char* id)
{
#if !RETRO_REV01
    LinkGameLogicDLL(info);
#else
    LinkGameLogicDLL(*info);
#endif
#ifdef GAME_IS_MANIA
    InitModAPI();
    return true;
#else
    return false;
#endif
}

