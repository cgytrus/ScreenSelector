#include "includes.h"
#include "FullscreenManager.hpp"
#include "ScreenSelectorExtension.hpp"
#include "patcher.h"

const char* FullscreenManager::vanillaWindowedGameVar = "0025";
const char* FullscreenManager::fullscreenModeGameVar = "ScreenSelector_fullscreenMode";
const char* FullscreenManager::screenGameVar = "ScreenSelector_screen";

std::vector<HMONITOR> FullscreenManager::_monitors;
std::vector<std::string> FullscreenManager::_monitorStrings;

int FullscreenManager::_screen = -1;
FullscreenMode FullscreenManager::_fullscreenMode = FullscreenMode::Windowed;

// the free window resize patch from mhv6
bool _freeWindowResize = false;
static void toggleFreeWindowResize(bool enabled) {
    // don't do it if we have mhv6
    // this should never happen anyway but just in case
    ////if(_extension) return; // nvm

    // we don't wanna patch/unpatch multiple times
    if(_freeWindowResize == enabled) return;
    _freeWindowResize = enabled;

    auto cocos2dBase = reinterpret_cast<uintptr_t>(GetModuleHandle("libcocos2d.dll"));
    if(enabled) {
        patch(cocos2dBase + 0x11388b, { 0x90, 0x90, 0x90, 0x90, 0x90 }, true);
        patch(cocos2dBase + 0x11389d, { 0xb9, 0xff, 0xff, 0xff, 0x7f, 0x90, 0x90 }, true);
        patch(cocos2dBase + 0x1138c0, { 0x48 }, true);
        patch(cocos2dBase + 0x1138c6, { 0x48 }, true);
        patch(cocos2dBase + 0x112536, { 0xeb, 0x11, 0x90 }, true);
    }
    else {
        unpatch(cocos2dBase + 0x11388b, true);
        unpatch(cocos2dBase + 0x11389d, true);
        unpatch(cocos2dBase + 0x1138c0, true);
        unpatch(cocos2dBase + 0x1138c6, true);
        unpatch(cocos2dBase + 0x112536, true);
    }
}

bool FullscreenManager::saveGameVariables() {
    auto gm = gd::GameManager::sharedState();
    if(!gm) return false;
    if(!gm->m_pValueKeeper) return false;

    gm->setIntGameVariable(screenGameVar, _screen);
    gm->setIntGameVariable(fullscreenModeGameVar, (int)_fullscreenMode);

    ScreenSelectorExtension::updateFullscreenModeOption();
    ScreenSelectorExtension::updateScreenOption();

    return true;
}

bool FullscreenManager::loadGameVariables() {
    auto gm = gd::GameManager::sharedState();
    if(!gm) return false;
    if(!gm->m_pValueKeeper) return false;
    if(!CCEGLView::sharedOpenGLView()) return false;

    updateMonitors();

    _fullscreenMode = (FullscreenMode)gm->getIntGameVariableDefault(fullscreenModeGameVar, !gm->getGameVariable(vanillaWindowedGameVar)); // default to vanilla setting
    _screen = gm->getIntGameVariableDefault(screenGameVar, 0);

    setFullscreenMode(_fullscreenMode, _fullscreenMode == FullscreenMode::Exclusive ? UpdateMode::None : UpdateMode::Force);
    setScreen(_screen);

    ScreenSelectorExtension::updateFullscreenModeOption();
    ScreenSelectorExtension::updateScreenOption();

    return true;
}

void FullscreenManager::setFullscreenMode(FullscreenMode mode, UpdateMode update) {
    if(_fullscreenMode == mode && update != UpdateMode::Force) return;

    if(update == UpdateMode::None) {
        _fullscreenMode = mode;
        saveGameVariables();
        return;
    }

    auto gm = gd::GameManager::sharedState();
    auto view = CCEGLView::sharedOpenGLView();
    if(!gm || !view) return;

    HWND windowHwnd = getWindowHwnd(view);

    // windowed and borderless are how robtop does it and i have to do this too,
    // or otherwise my functions get called befoe the switch from exclusive fullscreen happens and it breaks stuff
    switch(mode) {
        case FullscreenMode::Windowed: {
            setExclusiveFullscreen(false);

            CCCallFunc* callFunc = CCCallFunc::create(nullptr, callfunc_selector(FullscreenManager::setWindowedPreCallback));
            CCDelayTime* delayTime = CCDelayTime::create(0.f);
            CCSequence* sequence = CCSequence::create(delayTime, callFunc, delayTime);
            gm->getActionManager()->addAction((CCAction*)sequence, (CCNode*)gm, false);
            break;
        }
        case FullscreenMode::Borderless: {
            setExclusiveFullscreen(false);

            CCCallFunc* callFunc = CCCallFunc::create(nullptr, callfunc_selector(FullscreenManager::setBorderlessFullscreenPreCallback));
            CCDelayTime* delayTime = CCDelayTime::create(0.f);
            CCSequence* sequence = CCSequence::create(delayTime, callFunc, delayTime);
            gm->getActionManager()->addAction((CCAction*)sequence, (CCNode*)gm, false);
            break;
        }
        case FullscreenMode::Exclusive: {
            setExclusiveFullscreen(true);
            break;
        }
    }

    _fullscreenMode = mode;
    saveGameVariables();
}
FullscreenMode FullscreenManager::getFullscreenMode() {
    return _fullscreenMode;
}

void windowedModesShared(HWND windowHwnd, CCEGLView* view, int width, int height) {
    HMONITOR monitor = FullscreenManager::getMonitor();
    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(MONITORINFO);
    if(!GetMonitorInfo(monitor, &monitorInfo)) {
        // TODO: use FLAlertLayer instead
        MessageBox(windowHwnd, "Failed getting monitor info!", "Screen Selector Error", MB_OK | MB_ICONERROR);
    }

    RECT monitorRect = monitorInfo.rcMonitor;

    if(width <= 0) width = monitorRect.right - monitorRect.left;
    if(height <= 0) height = monitorRect.bottom - monitorRect.top;
    SetWindowPos(windowHwnd, HWND_TOP, monitorRect.left, monitorRect.top, width, height, SWP_NOREDRAW | SWP_NOACTIVATE | SWP_FRAMECHANGED);
    view->resizeWindow(width, height);
}

void FullscreenManager::setWindowedPreCallback() {
    auto gm = gd::GameManager::sharedState();
    CCCallFunc* callFunc = CCCallFunc::create(nullptr, callfunc_selector(FullscreenManager::setWindowedCallback));
    CCDelayTime* delayTime = CCDelayTime::create(0.f);
    CCSequence* sequence = CCSequence::create(delayTime, callFunc, delayTime);
    gm->getActionManager()->addAction((CCAction*)sequence, (CCNode*)gm, false);
}
void FullscreenManager::setWindowedCallback() { FullscreenManager::setWindowed(); }
void FullscreenManager::setWindowed() {
    auto gm = gd::GameManager::sharedState();
    auto view = CCEGLView::sharedOpenGLView();
    if(!gm || !view) return;
    bool currentWindowed = gm->getGameVariable(vanillaWindowedGameVar);
    if(!currentWindowed) return;

    HWND windowHwnd = getWindowHwnd(view);

    // i'm too lazy to understand which flags are actually set here,
    // but from looking at the decompilation it seems like it's the only possible set of flags that can be set
    // the 2 other ones are 0x86000000, which seems to be borderless fullscreen,
    // and 0x6ca0000, which is windowed but with small top bar thing and not resizeable
    // these modes are probably default in cocos and there was absolutely 0 point for rob to remove them
    // p.s. the function address i took this from is 0x1126e0 in cocos2d
    SetWindowLong(windowHwnd, GWL_STYLE, 0x6cf0000);

    toggleFreeWindowResize(false);
    CCSize clientSize = getWindowedSize(view);
    windowedModesShared(windowHwnd, view, (int)clientSize.width, (int)clientSize.height);
}
void FullscreenManager::setBorderlessFullscreenPreCallback() {
    auto gm = gd::GameManager::sharedState();
    CCCallFunc* callFunc = CCCallFunc::create(nullptr, callfunc_selector(FullscreenManager::setBorderlessFullscreenCallback));
    CCDelayTime* delayTime = CCDelayTime::create(0.f);
    CCSequence* sequence = CCSequence::create(delayTime, callFunc, delayTime);
    gm->getActionManager()->addAction((CCAction*)sequence, (CCNode*)gm, false);
}
void FullscreenManager::setBorderlessFullscreenCallback() { FullscreenManager::setBorderlessFullscreen(); }
void FullscreenManager::setBorderlessFullscreen() {
    auto gm = gd::GameManager::sharedState();
    auto view = CCEGLView::sharedOpenGLView();
    if(!gm || !view) return;
    bool currentWindowed = gm->getGameVariable(vanillaWindowedGameVar);
    if(!currentWindowed) return;

    HWND windowHwnd = getWindowHwnd(view);

    LONG windowStyle = GetWindowLong(windowHwnd, GWL_STYLE);
    SetWindowLong(windowHwnd, GWL_STYLE, windowStyle & ~(WS_CAPTION | WS_SIZEBOX | WS_SYSMENU));

    toggleFreeWindowResize(true);
    windowedModesShared(windowHwnd, view, 0, 0);
}
void FullscreenManager::setExclusiveFullscreen(bool fullscreen) {
    auto gm = gd::GameManager::sharedState();
    if(!gm) return;
    bool currentWindowed = gm->getGameVariable(vanillaWindowedGameVar);
    if(currentWindowed != fullscreen) return;
    gm->setGameVariable(vanillaWindowedGameVar, !fullscreen);
    gm->reloadAll(true, fullscreen, true);
}

void FullscreenManager::setScreen(int screen) {
    auto view = CCDirector::sharedDirector()->getOpenGLView();
    auto gm = gd::GameManager::sharedState();
    if(!view || !gm) return;

    screen = min(max(screen, 0), (int)_monitors.size() - 1);
    if(_screen == screen) return;
    _screen = screen;
    if(getFullscreenMode() == FullscreenMode::Exclusive) {
        // set m_bIsFullscreen to false directly to make the view->toggleFullScreen(bool) in reloadAll
        // think that we're in windowed and recreate the window for us
        *(bool*)((uintptr_t)view + 0xb0) = false; // view->m_bIsFullscreen = false
        gm->reloadAll(true, true, true);
    }
    else setFullscreenMode(getFullscreenMode(), UpdateMode::Force); // force an update without changing the fullscreen mode
    saveGameVariables();
}
int FullscreenManager::getScreen() {
    return _screen;
}
HMONITOR FullscreenManager::getMonitor() {
    return _monitors.at(_screen);
}
std::string FullscreenManager::getMonitorString() {
    return _monitorStrings.at(_screen);
}
const std::vector<HMONITOR>& FullscreenManager::getMonitors() {
    return _monitors;
}
const std::vector<std::string>& FullscreenManager::getMonitorStrings() {
    return _monitorStrings;
}
void FullscreenManager::updateMonitors() {
    EnumDisplayMonitors(NULL, NULL, monitorEnumCallback, NULL);
}
BOOL CALLBACK FullscreenManager::monitorEnumCallback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    _monitors.push_back(hMonitor);
    auto text = std::to_string(_monitorStrings.size()) + ": " +
        std::to_string(lprcMonitor->right - lprcMonitor->left) + "x" + std::to_string(lprcMonitor->bottom - lprcMonitor->top);
    _monitorStrings.push_back(text);
    return TRUE;
}

const char* FullscreenManager::modeToCstr(FullscreenMode mode) {
    switch(mode) {
        case FullscreenMode::Windowed:
            return "Windowed";
        case FullscreenMode::Borderless:
            return "Borderless";
        case FullscreenMode::Exclusive:
            return "Exclusive";
        default:
            return "Unknown";
    }
}
