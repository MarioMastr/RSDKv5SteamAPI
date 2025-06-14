#pragma once
extern "C"
{
#include <Game.h>
}
#include <dobby.h>

#define DLC_COUNT 8

#ifdef _AMD64_
#define DEFAULT_CALL_TYPE __fastcall
#else
#ifdef _X86_
#define DEFAULT_CALL_TYPE __cdecl
#endif
#endif

#ifdef _WIN32
#define DisplayError(message) MessageBoxA(NULL, message, "RSDKv5SteamAPI Error", MB_ICONERROR);
#endif

extern void* ModHandle;

void CheckDLCs();

#if defined(_WIN32)
#include "framework.h"
#define PROC_TYPE HMODULE
#elif defined(__APPLE__)
#include <dlfcn.h>
#define PROC_TYPE void *
#endif

PROC_TYPE getLibrary(const char *fileName);
void *getSymbol(PROC_TYPE handle, const char *symbolName);