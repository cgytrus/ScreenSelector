#include "includes.h"
#include "ScreenSelectorExtension.hpp"
#include "FullscreenManager.hpp"
#include "patcher.h"
#include "extensions2.h"

GLFWmonitor** globalMonitors;
int globalMonitorsCount;
GLFWmonitor** currentGlobalMonitor;

GLFWmonitor* getMonitor(int index) {
    auto maxMonitors = globalMonitorsCount;
    if(index < 0 || index >= maxMonitors) {
        MessageBox(0, "Failed getting current screen!\nFalling back to primary monitor", "Screen Selector Error", MB_OK | MB_ICONERROR);
        return (GLFWmonitor*)globalMonitors;
    }
    return (GLFWmonitor*)((uintptr_t)globalMonitors + index * sizeof(uintptr_t));
}

GLFWmonitor** (__cdecl* findMonitors)(int* monitorCount);

void (__stdcall* resetGlobalMonitor)();
void __stdcall resetGlobalMonitor_H() {
    // for some reason it crashes if the current monitor isn't the first one
    *currentGlobalMonitor = getMonitor(0);
    resetGlobalMonitor();
}

void (__stdcall* getGlobalMonitor)();
void __stdcall getGlobalMonitor_H() {
    int screen = FullscreenManager::getScreen();
    // load the variable manually because it didn't load in FullscreenManager yet
    auto maxMonitors = globalMonitorsCount;
    if(screen < 0 || screen >= maxMonitors) {
        auto gm = gd::GameManager::sharedState();
        if(gm && gm->m_pValueKeeper) screen = gm->getIntGameVariableDefault(FullscreenManager::screenGameVar, 0);
    }
    *currentGlobalMonitor = getMonitor(screen);
    getGlobalMonitor();
}

void (__thiscall* CCEGLView_toggleFullScreen)(CCEGLView* self, bool fullscreen);
void __fastcall CCEGLView_toggleFullScreen_H(CCEGLView* self, void*, bool fullscreen) {
    CCEGLView_toggleFullScreen(self, fullscreen);
    if(!fullscreen) return;
    FullscreenManager::setFullscreenMode(FullscreenMode::Exclusive, UpdateMode::None);
    FullscreenManager::saveGameVariables();
}

void (__thiscall* GameManager_setIntGameVariable)(gd::GameManager* self, const char* key, int value);
void __fastcall GameManager_setIntGameVariable_H(gd::GameManager* self, void*, const char* key, int value) {
    GameManager_setIntGameVariable(self, key, value);
    if(strcmp(key, FullscreenManager::fullscreenModeGameVar) == 0)
        self->setGameVariable(FullscreenManager::vanillaWindowedGameVar, (FullscreenMode)value != FullscreenMode::Exclusive);
}

void (__cdecl* CCEGLView_onGLFWwindowPosCallback)(GLFWwindow* window, int x, int y);
void __cdecl CCEGLView_onGLFWwindowPosCallback_H(GLFWwindow* window, int x, int y) {
    CCEGLView_onGLFWwindowPosCallback(window, x, y);
    if(FullscreenManager::getFullscreenMode() != FullscreenMode::Windowed)
        return;
    FullscreenManager::setMonitor(MonitorFromWindow(FullscreenManager::getWindowHwnd(window), MONITOR_DEFAULTTONEAREST));
}

void (__thiscall* idk_applyGraphicsSettings)(void* self, CCObject* obj);
void __fastcall idk_applyGraphicsSettings_H(void* self, void*, CCObject* obj) {
    idk_applyGraphicsSettings(self, obj);
    ScreenSelectorExtension::applySelections();
}

bool (__thiscall* VideoOptionsLayer_init)(CCLayer* self);
bool __fastcall VideoOptionsLayer_init_H(CCLayer* self) {
    if(!VideoOptionsLayer_init(self)) return false;
    ScreenSelectorExtension::initVideoOptionsLayer(self);
    return true;
}

DWORD WINAPI mainThread(void* hModule) {
    MH_Initialize();

    auto base = gd::base;
    auto cocos2dBase = reinterpret_cast<uintptr_t>(GetModuleHandle("libcocos2d.dll"));

    *reinterpret_cast<void**>(&findMonitors) = reinterpret_cast<void*>(cocos2dBase + 0x114f20);
    currentGlobalMonitor = (GLFWmonitor**)(cocos2dBase + 0x1a1534);
    globalMonitors = findMonitors(&globalMonitorsCount);

    patch(base + 0x1e1f68, { 0xeb }, true); // don't check for changes when applying settings

    MH_CreateHook(reinterpret_cast<void*>(cocos2dBase + 0x110760), resetGlobalMonitor_H,
        reinterpret_cast<void**>(&resetGlobalMonitor));

    MH_CreateHook(reinterpret_cast<void*>(cocos2dBase + 0x110cb0), getGlobalMonitor_H,
        reinterpret_cast<void**>(&getGlobalMonitor));

    MH_CreateHook(reinterpret_cast<void*>(base + 0xca230), GameManager_setIntGameVariable_H,
        reinterpret_cast<void**>(&GameManager_setIntGameVariable));

    MH_CreateHook(reinterpret_cast<void*>(cocos2dBase + 0xc48d0), CCEGLView_toggleFullScreen_H,
        reinterpret_cast<void**>(&CCEGLView_toggleFullScreen));

    MH_CreateHook(reinterpret_cast<void*>(cocos2dBase + 0xc42b0), CCEGLView_onGLFWwindowPosCallback_H,
        reinterpret_cast<void**>(&CCEGLView_onGLFWwindowPosCallback));

    MH_CreateHook(reinterpret_cast<void*>(base + 0x1e1e70), idk_applyGraphicsSettings_H,
        reinterpret_cast<void**>(&idk_applyGraphicsSettings));

    MH_CreateHook(reinterpret_cast<void*>(base + 0x1e0e10), VideoOptionsLayer_init_H,
        reinterpret_cast<void**>(&VideoOptionsLayer_init));

    MH_EnableHook(MH_ALL_HOOKS);

    while(!FullscreenManager::loadGameVariables())
        Sleep(0);

    ScreenSelectorExtension::create();

    return 0;
}

BOOL APIENTRY DllMain(HMODULE handle, DWORD reason, LPVOID reserved) {
    if(reason == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0x100, mainThread, handle, 0, 0);
    }
    return TRUE;
}
