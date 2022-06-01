#include "includes.h"
#include "ScreenSelectorExtension.hpp"
#include "FullscreenManager.hpp"
#include "patcher.h"

GLFWmonitor** globalMonitors;
int* globalMonitorsCount;
GLFWmonitor** currentGlobalMonitor;

GLFWmonitor* getMonitor(int index) {
    auto maxMonitors = globalMonitorsCount ? *globalMonitorsCount : 1;
    if(index < 0 || index >= maxMonitors) {
        MessageBox(0, "Failed getting current screen!\nFalling back to primary monitor", "Screen Selector Error", MB_OK | MB_ICONERROR);
        return (GLFWmonitor*)globalMonitors;
    }
    return (GLFWmonitor*)((uintptr_t)globalMonitors + index * sizeof(uintptr_t));
}

// i'm not sure which class this is in, but it doesn't matter because i finally found out how this function works!!!!
GLFWmonitor* (__cdecl* findMonitor)(int* monitorCount);
GLFWmonitor* __cdecl findMonitor_H(int* monitorCount) {
    auto cocos2dBase = reinterpret_cast<uintptr_t>(GetModuleHandle("libcocos2d.dll"));
    // the original function just returns an array of all monitors (GLFWmonitor**) cast to GLFWmonitor* to get the first element
    uintptr_t monitor = (uintptr_t)findMonitor(monitorCount);
    globalMonitors = (GLFWmonitor**)monitor;
    globalMonitorsCount = monitorCount;
    currentGlobalMonitor = (GLFWmonitor**)(cocos2dBase + 0x1a1534);
    return getMonitor(0);
}

void (__stdcall* resetGlobalMonitor)();
void __stdcall resetGlobalMonitor_H() {
    // for some reason it crashes if the current monitor isn't the first one
    if(currentGlobalMonitor) *currentGlobalMonitor = getMonitor(0);
    resetGlobalMonitor();
}

void (__stdcall* getGlobalMonitor)();
void __stdcall getGlobalMonitor_H() {
    int screen = FullscreenManager::getScreen();
    // load the variable manually because it didn't load in FullscreenManager yet
    auto maxMonitors = globalMonitorsCount ? *globalMonitorsCount : 1;
    if(screen < 0 || screen >= maxMonitors) {
        auto gm = gd::GameManager::sharedState();
        if(gm && gm->m_pValueKeeper) screen = gm->getIntGameVariableDefault(FullscreenManager::screenGameVar, 0);
    }
    if(currentGlobalMonitor) *currentGlobalMonitor = getMonitor(screen);
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

    patch(base + 0x1e1f68, { 0xeb }, true); // don't check for changes when applying settings

    MH_CreateHook(reinterpret_cast<void*>(cocos2dBase + 0x114f20), findMonitor_H,
        reinterpret_cast<void**>(&findMonitor));

    MH_CreateHook(reinterpret_cast<void*>(cocos2dBase + 0x110760), resetGlobalMonitor_H,
        reinterpret_cast<void**>(&resetGlobalMonitor));

    MH_CreateHook(reinterpret_cast<void*>(cocos2dBase + 0x110cb0), getGlobalMonitor_H,
        reinterpret_cast<void**>(&getGlobalMonitor));

    MH_CreateHook(reinterpret_cast<void*>(base + 0xca230), GameManager_setIntGameVariable_H,
        reinterpret_cast<void**>(&GameManager_setIntGameVariable));

    MH_CreateHook(reinterpret_cast<void*>(cocos2dBase + 0xc48d0), CCEGLView_toggleFullScreen_H,
        reinterpret_cast<void**>(&CCEGLView_toggleFullScreen));

    MH_CreateHook(reinterpret_cast<void*>(base + 0x1e1e70), idk_applyGraphicsSettings_H,
        reinterpret_cast<void**>(&idk_applyGraphicsSettings));

    MH_CreateHook(reinterpret_cast<void*>(base + 0x1e0e10), VideoOptionsLayer_init_H,
        reinterpret_cast<void**>(&VideoOptionsLayer_init));

    MH_EnableHook(MH_ALL_HOOKS);

    while(!FullscreenManager::loadGameVariables()) Sleep(0);

    if(Hackpro::Initialize() && Hackpro::IsReady()) ScreenSelectorExtension::create();

    if(!currentGlobalMonitor) {
        // too lazy to split the string properly to make the line not be wide af
        MessageBox(0, "The mod was loaded too late!\nThis will cause issues while using the mod.\nMake sure you have Screen Selector in 'absolutedlls' if you're using MHv6 or in 'Startup' if you're using QuickLdr", "Screen Selector Warning", MB_OK | MB_ICONWARNING);
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE handle, DWORD reason, LPVOID reserved) {
    if(reason == DLL_PROCESS_ATTACH) {
        CreateThread(0, 0x100, mainThread, handle, 0, 0);
    }
    return TRUE;
}
