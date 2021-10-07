#include "includes.h"
#include "ScreenSelectorExtension.hpp"
#include "FullscreenManager.hpp"
#include "patcher.h"

void (__thiscall* CCEGLView_toggleFullScreen)(CCEGLView* self, bool fullscreen);
void __fastcall CCEGLView_toggleFullScreen_H(CCEGLView* self, void*, bool fullscreen) {
    CCEGLView_toggleFullScreen(self, fullscreen);
    if(!fullscreen) return;
    FullscreenManager::SetFullscreenMode(FullscreenMode::Exclusive, UpdateMode::None);
    FullscreenManager::SaveGameVariables();
}

void (__thiscall* GameManager_setIntGameVariable)(gd::GameManager* self, const char* key, int value);
void __fastcall GameManager_setIntGameVariable_H(gd::GameManager* self, void*, const char* key, int value) {
    GameManager_setIntGameVariable(self, key, value);
    if(strcmp(key, FullscreenManager::fullscreenModeGameVar) == 0)
        self->setGameVariable(FullscreenManager::vanillaWindowedGameVar, (FullscreenMode)value != FullscreenMode::Exclusive);
}

void (__thiscall* idk_applyGraphicsSettings)(void* self, CCObject* obj);
void __fastcall idk_applyGraphicsSettings_H(void* self, void*, CCObject* obj) {
    idk_applyGraphicsSettings(self, obj);
    ScreenSelectorExtension::ApplySelections();
}

void (__cdecl* CCEGLView_onGLFWwindowPosCallback)(GLFWwindow* window, int x, int y);
void __cdecl CCEGLView_onGLFWwindowPosCallback_H(GLFWwindow* window, int x, int y) {
    CCEGLView_onGLFWwindowPosCallback(window, x, y);
    FullscreenManager::SetMonitor(MonitorFromWindow(FullscreenManager::getWindowHwnd(window), MONITOR_DEFAULTTONEAREST));
}

bool (__thiscall* VideoOptionsLayer_init)(CCLayer* self);
bool __fastcall VideoOptionsLayer_init_H(CCLayer* self) {
    if(!VideoOptionsLayer_init(self)) return false;
    ScreenSelectorExtension::InitVideoOptionsLayer(self);
    return true;
}

DWORD WINAPI MainThread(void* hModule) {
    /*AllocConsole();
    std::ofstream conout("CONOUT$", std::ios::out);
    std::ifstream conin("CONIN$", std::ios::in);
    std::cout.rdbuf(conout.rdbuf());
    std::cin.rdbuf(conin.rdbuf());*/

    MH_Initialize();

    auto base = gd::base;
    auto cocos2dBase = reinterpret_cast<uintptr_t>(GetModuleHandle("libcocos2d.dll"));

    patch(base + 0x1e1f68, { 0xeb }, true); // don't check for changes when applying settings

    MH_CreateHook(reinterpret_cast<void*>(base + 0xca230), GameManager_setIntGameVariable_H,
        reinterpret_cast<void**>(&GameManager_setIntGameVariable));

    MH_CreateHook(reinterpret_cast<void*>(cocos2dBase + 0xc48d0), CCEGLView_toggleFullScreen_H,
        reinterpret_cast<void**>(&CCEGLView_toggleFullScreen));

    MH_CreateHook(reinterpret_cast<void*>(base + 0x1e1e70), idk_applyGraphicsSettings_H,
        reinterpret_cast<void**>(&idk_applyGraphicsSettings));

    MH_CreateHook(reinterpret_cast<void*>(cocos2dBase + 0xc42b0), CCEGLView_onGLFWwindowPosCallback_H,
        reinterpret_cast<void**>(&CCEGLView_onGLFWwindowPosCallback));

    MH_CreateHook(reinterpret_cast<void*>(base + 0x1e0e10), VideoOptionsLayer_init_H,
        reinterpret_cast<void**>(&VideoOptionsLayer_init));

    MH_EnableHook(MH_ALL_HOOKS);

    while(!FullscreenManager::LoadGameVariables()) Sleep(0);

    if(Hackpro::Initialize() && Hackpro::IsReady()) ScreenSelectorExtension::Create();

    /*std::getline(std::cin, std::string());

    conout.close();
    conin.close();
    FreeConsole();
    FreeLibraryAndExitThread(cast<HMODULE>(hModule), 0);*/

    return 0;
}

BOOL APIENTRY DllMain(HMODULE handle, DWORD reason, LPVOID reserved) {
    if(reason == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0x100, MainThread, handle, 0, 0);
    }
    return TRUE;
}
