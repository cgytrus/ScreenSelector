#pragma once

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

        static BOOL CALLBACK monitorEnumCallback(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

        void setWindowedPreCallback();
        void setWindowedCallback();
        void setBorderlessFullscreenPreCallback();
        void setBorderlessFullscreenCallback();

    public:
        static const char* vanillaWindowedGameVar;
        static const char* fullscreenModeGameVar;
        static const char* screenGameVar;

        static bool saveGameVariables();
        static bool loadGameVariables();

        static void setFullscreenMode(FullscreenMode, UpdateMode = UpdateMode::OnChange);
        static FullscreenMode getFullscreenMode();

        static void setWindowed();
        static void setBorderlessFullscreen();
        static void setExclusiveFullscreen(bool);

        static void setScreen(int);
        static int getScreen();
        static HMONITOR getMonitor();
        static std::string getMonitorString();
        static const std::vector<HMONITOR>& getMonitors();
        static const std::vector<std::string>& getMonitorStrings();
        static void updateMonitors();

        static inline HWND getWindowHwnd(GLFWwindow* window) { return *(HWND*)((uintptr_t)window + 0x22c); }
        static inline HWND getWindowHwnd(CCEGLView* view) { return getWindowHwnd(view->getWindow()); }
        static inline CCSize getWindowedSize(CCEGLView* view) { return *(CCSize*)((uintptr_t)view + 0xa0); }

        static const char* modeToCstr(FullscreenMode);
};
