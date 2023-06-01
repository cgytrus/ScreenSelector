#pragma once

// we don't have glfw bindings in geode (yet) and i dont wanna find addresses for each func so
// instead i'll just define the internal structs here and use them directly lmao
// they're cut to exactly those fields that i actually need
// update: ended up needing some funcs anyway .-.

struct GLFWmonitor {
    char* name;
    int widthMM;
    int heightMM;
    GLFWvidmode* modes;
    int modeCount;
    GLFWvidmode currentMode;
};

struct _GLFWlibrary {
    struct {
        int redBits;
        int greenBits;
        int blueBits;
        int alphaBits;
        int depthBits;
        int stencilBits;
        int accumRedBits;
        int accumGreenBits;
        int accumBlueBits;
        int accumAlphaBits;
        int auxBuffers;
        int stereo;
        int resizable;
        int visible;
        int decorated;
        int focused;
        int autoIconify;
        int floating;
        int samples;
        int sRGB;
        int refreshRate;
        int doublebuffer;
        int api;
        int major;
        int minor;
        int forward;
        int debug;
        int profile;
        int robustness;
        int release;
    } hints;
};

/*struct GLFWwindow {
    struct GLFWwindow* next;

    // Window settings and state
    GLboolean resizable;
    GLboolean decorated;
    GLboolean autoIconify;
    GLboolean floating;
    GLboolean closed;
    void* userPointer;
    GLFWvidmode videoMode;
    GLFWmonitor* monitor;
    GLFWcursor* cursor;

    // Window input state
    GLboolean stickyKeys;
    GLboolean stickyMouseButtons;
    double cursorPosX, cursorPosY;
    int cursorMode;
    char mouseButtons[GLFW_MOUSE_BUTTON_LAST + 1];
    char keys[GLFW_KEY_LAST + 1];

    // OpenGL extensions and context attributes
    struct {
        int api;
        int major, minor, revision;
        GLboolean forward, debug;
        int profile;
        int robustness;
        int release;
    } context;

    struct {
        GLFWwindowposfun pos;
        GLFWwindowsizefun size;
        GLFWwindowclosefun close;
        GLFWwindowrefreshfun refresh;
        GLFWwindowfocusfun focus;
        GLFWwindowiconifyfun iconify;
        GLFWframebuffersizefun fbsize;
        GLFWmousebuttonfun mouseButton;
        GLFWcursorposfun cursorPos;
        GLFWcursorenterfun cursorEnter;
        GLFWscrollfun scroll;
        GLFWkeyfun key;
        GLFWcharfun character;
        GLFWcharmodsfun charmods;
        GLFWdropfun drop;
    } callbacks;

    struct {
        HWND handle;
        DWORD dwStyle;
        DWORD dwExStyle;

        GLboolean cursorInside;
        GLboolean iconified;

        // The last received cursor position, regardless of source
        int cursorPosX, cursorPosY;
    } win32;
};*/

_GLFWlibrary* _glfw = nullptr;
GLFWvidmode* (*_glfwGetVideoMode)(GLFWmonitor* handle) = nullptr;
//void (*_glfwSetWindowPos)(GLFWwindow* handle, int xpos, int ypos) = nullptr;
void (*_glfwGetMonitorPos)(GLFWmonitor* handle, int* xpos, int* ypos) = nullptr;
GLFWmonitor** (*_glfwGetMonitors)(int* count) = nullptr;

void findGlfw() {
    if(!_glfw || !_glfwGetVideoMode || /*!_glfwSetWindowPos || */!_glfwGetMonitorPos || !_glfwGetMonitors) {
#ifdef GEODE_IS_WINDOWS
        _glfw = reinterpret_cast<_GLFWlibrary*>(geode::base::getCocos() + 0x1a14a0);
        _glfwGetVideoMode = reinterpret_cast<GLFWvidmode*(*)(GLFWmonitor*)>(geode::base::getCocos() + 0x110ce0);
        // please fucking kill me already i cant set the window position glfwsetwindowpos doesnt work winapi doesnt work
        // does anything in gd or cocos ever fucking work
        //_glfwSetWindowPos = reinterpret_cast<void(*)(GLFWwindow*, int, int)>(geode::base::getCocos() + 0x111630);
        _glfwGetMonitorPos = reinterpret_cast<void(*)(GLFWmonitor*, int*, int*)>(geode::base::getCocos() + 0x114ea0);
        _glfwGetMonitors = reinterpret_cast<GLFWmonitor**(*)(int*)>(geode::base::getCocos() + 0x114f20);
#else
        log::error("Not supported on non-Windows platforms!");
        Mod::get()->disable();
        return;
#endif
    }
}
