#ifndef __FULLSCREEN_MANAGER_HPP
#define __FULLSCREEN_MANAGER_HPP

#include <windows.h>
#include <memory>
#include <cocos2d.h>

enum class UpdateMode { None, OnChange, Force };
enum class FullscreenMode { Windowed, Exclusive, Borderless };

class FullscreenManager : public CCObject {
    private:
        FullscreenManager() {}

        static int _screen;
        static FullscreenMode _fullscreenMode;

        static std::vector<HMONITOR> _monitors;
        static std::vector<std::string> _monitorStrings;

        static BOOL CALLBACK MonitorEnumCallback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

        void SetWindowedPreCallback();
        void SetWindowedCallback();
        void SetBorderlessFullscreenPreCallback();
        void SetBorderlessFullscreenCallback();

    public:
        static const char* vanillaWindowedGameVar;
        static const char* fullscreenModeGameVar;
        static const char* screenGameVar;

        static bool SaveGameVariables();
        static bool LoadGameVariables();

        static void SetFullscreenMode(FullscreenMode, UpdateMode = UpdateMode::OnChange);
        static FullscreenMode GetFullscreenMode();

        static void SetWindowed();
        static void SetBorderlessFullscreen();
        static void SetExclusiveFullscreen(bool);

        static void SetScreen(int);
        static void SetMonitor(HMONITOR);
        static int GetScreen();
        static HMONITOR GetMonitor();
        static std::string GetMonitorString();
        static const std::vector<HMONITOR>& GetMonitors();
        static const std::vector<std::string>& GetMonitorStrings();
        static void UpdateMonitors();

        static inline HWND getWindowHwnd(GLFWwindow* window) { return *(HWND*)((uintptr_t)window + 0x22c); }
        static inline HWND getWindowHwnd(CCEGLView* view) { return getWindowHwnd(view->getWindow()); }
        static inline CCSize getWindowedSize(CCEGLView* view) { return *(CCSize*)((uintptr_t)view + 0xa0); }

        static const char* ModeToCstr(FullscreenMode);
};

#endif
