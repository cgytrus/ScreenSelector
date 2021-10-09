#include "includes.h"
#include "ScreenSelectorExtension.hpp"
#include "FullscreenManager.hpp"
#include "patcher.h"

GLFWmonitor** globalMonitors;
int* globalMonitorsCount;
GLFWmonitor** currentGlobalMonitor;

GLFWmonitor* getMonitor(int index) {
    if(index < 0 || index >= *globalMonitorsCount) {
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
    *currentGlobalMonitor = getMonitor(0);
    resetGlobalMonitor();
}

void (__stdcall* getGlobalMonitor)();
void __stdcall getGlobalMonitor_H() {
    int screen = FullscreenManager::GetScreen();
    // load the variable manually because it didn't load in FullscreenManager yet
    if(screen < 0 || screen >= *globalMonitorsCount) {
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
