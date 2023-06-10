#include <Geode/Geode.hpp>
using namespace geode::prelude;

// i absolutely hate this and never wanna touch glfw nor cocos nor winapi ever again in my entire  fucknig  life.

#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/VideoOptionsLayer.hpp>

#include "glfw_internal_stuff.h"

bool isBorderless() { return Mod::get()->getSettingValue<bool>("borderless"); }
void isBorderless(bool x) { Mod::get()->setSettingValue<bool>("borderless", x); }

int screen() { return Mod::get()->getSettingValue<int64_t>("screen"); }
void screen(int x) { Mod::get()->setSettingValue<int64_t>("screen", x); }

struct CustomCCEGLView : geode::Modify<CustomCCEGLView, CCEGLView> {
    void startForceToggleFullScreen(bool fullscreen) {
        m_bIsFullscreen = !fullscreen;
    }

    // https://discourse.glfw.org/t/clarification-of-borderless-fullscreen-and-borderless-windowed-modes/1235
    void setupWindow(CCRect rect) {
        findGlfw();

        int monitorCount;
        auto monitor = _glfwGetMonitors(&monitorCount)[screen()];
        m_pPrimaryMonitor = m_bIsFullscreen ? monitor : nullptr;

        auto vidmode = _glfwGetVideoMode(monitor);

        _glfw->hints.redBits = vidmode->redBits;
        _glfw->hints.greenBits = vidmode->greenBits;
        _glfw->hints.blueBits = vidmode->blueBits;
        _glfw->hints.refreshRate = vidmode->refreshRate;
        _glfw->hints.decorated = m_bIsFullscreen || isBorderless() ? 0 : 1;

        if(m_bIsFullscreen || isBorderless()) {
            m_obWindowedSize = CCSize{static_cast<float>(vidmode->width), static_cast<float>(vidmode->height)};
            rect.size = m_obWindowedSize;
        }
        CCEGLView::setupWindow(rect);
    }

    void toggleFullScreen(bool fullscreen) {
        CCEGLView::toggleFullScreen(fullscreen);
        m_pMainWindow->width = (int)m_obWindowedSize.width;
        m_pMainWindow->height = (int)m_obWindowedSize.height;
        updateWindow(m_pMainWindow->width, m_pMainWindow->height);
        centerWindow();
    }
};

void centerWindowOrSmth(GLFWwindow* window) {
    if(!window)
        return;
    RECT windowRect;
    GetWindowRect(window->win32.handle, &windowRect);
    int monitorCount;
    auto monitor = _glfwGetMonitors(&monitorCount)[screen()];
    auto vidmode = _glfwGetVideoMode(monitor);
    int x = 0;
    int y = 0;
    _glfwGetMonitorPos(monitor, &x, &y);
    x += vidmode->width / 2 - (windowRect.right - windowRect.left) / 2;
    y += vidmode->height / 2 - (windowRect.bottom - windowRect.top) / 2;
    SetWindowPos(window->win32.handle, HWND_TOP, x, y, 0, 0,
        SWP_NOREDRAW | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOSIZE);
}

$execute {
    if(!Mod::get()->addHook(
        reinterpret_cast<void*>(base::getCocos() + 0x111f80),
        &centerWindowOrSmth,
        "centerWindow",
        tulip::hook::TulipConvention::Cdecl
    ))
        log::error("failed to hook centerWindow");
}

struct CustomVideoOptionsLayer : geode::Modify<CustomVideoOptionsLayer, VideoOptionsLayer> {
    bool m_isBorderless = isBorderless();
    CCLabelBMFont* m_modeText;
    int m_screen = screen();
    CCLabelBMFont* m_screenText;

    const char* getModeCstr() {
        if(m_fields->m_isBorderless)
            return "Borderless";
        if(m_isFullscreen)
            return "Fullscreen";
        return "Windowed";
    }
    void setWindowed() {
        m_fields->m_isBorderless = false;
        m_isFullscreen = false;
    }
    void setFullscreen() {
        m_fields->m_isBorderless = false;
        m_isFullscreen = true;
    }
    void setBorderless() {
        m_fields->m_isBorderless = true;
        m_isFullscreen = false;
    }

    const char* getScreenCstr() {
        int monitorCount;
        return _glfwGetMonitors(&monitorCount)[m_fields->m_screen]->name;
    }

    void selectNextFullscreenMode(CCObject*) {
        if(m_fields->m_isBorderless)
            setWindowed();
        else if(m_isFullscreen)
            setBorderless();
        else
            setFullscreen();
        m_fields->m_modeText->setCString(getModeCstr());
        toggleResolution();
    }
    void selectPreviousFullscreenMode(CCObject*) {
        if(m_fields->m_isBorderless)
            setFullscreen();
        else if(m_isFullscreen)
            setWindowed();
        else
            setBorderless();
        m_fields->m_modeText->setCString(getModeCstr());
        toggleResolution();
    }

    void selectPreviousMonitor(CCObject*) {
        m_fields->m_screen++;
        int size;
        _glfwGetMonitors(&size);
        while(m_fields->m_screen >= size) m_fields->m_screen -= size;
        while(m_fields->m_screen < 0) m_fields->m_screen += size;
        m_fields->m_screenText->setCString(getScreenCstr());
    }
    void selectNextMonitor(CCObject*) {
        m_fields->m_screen--;
        int size;
        _glfwGetMonitors(&size);
        while(m_fields->m_screen >= size) m_fields->m_screen -= size;
        while(m_fields->m_screen < 0) m_fields->m_screen += size;
        m_fields->m_screenText->setCString(getScreenCstr());
    }

    bool init() {
        findGlfw();

        m_fields->m_isBorderless = isBorderless();
        m_fields->m_screen = screen();

        if(!VideoOptionsLayer::init())
            return false;

        auto fullscreenToggleLabel = reinterpret_cast<CCNode*>(m_mainLayer->getChildren()->objectAtIndex(3));
        auto fullscreenToggle = reinterpret_cast<CCNode*>(m_buttonMenu->getChildren()->objectAtIndex(0));

        fullscreenToggleLabel->setVisible(false);
        fullscreenToggle->setVisible(false);

        // fullscreen
        m_fields->m_modeText = CCLabelBMFont::create(getModeCstr(), "bigFont.fnt");
        m_fields->m_modeText->setPosition({ m_mainLayer->getContentSize().width * 0.5f - 84.f, fullscreenToggleLabel->getPositionY() - 0.75f });
        m_fields->m_modeText->setScale(fullscreenToggleLabel->getScale() * 0.85f);
        m_mainLayer->addChild(m_fields->m_modeText);

        auto fullscreenSelectorLabel = CCLabelBMFont::create("Mode", "goldFont.fnt");
        fullscreenSelectorLabel->setPosition({ m_fields->m_modeText->getPositionX(), m_fields->m_modeText->getPositionY() + 17.f });
        fullscreenSelectorLabel->setScale(fullscreenToggleLabel->getScale());
        m_mainLayer->addChild(fullscreenSelectorLabel);

        auto selectFullscreenLeftSprite = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
        auto selectFullscreenLeftButton = CCMenuItemSpriteExtra::create(selectFullscreenLeftSprite, this,
            menu_selector(CustomVideoOptionsLayer::selectPreviousFullscreenMode));
        selectFullscreenLeftButton->setPosition(m_buttonMenu->convertToNodeSpace({ -70.f, 0.f }) + m_fields->m_modeText->getPosition());
        m_buttonMenu->addChild(selectFullscreenLeftButton);

        auto selectFullscreenRightSprite = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
        auto selectFullscreenRightButton = CCMenuItemSpriteExtra::create(selectFullscreenRightSprite, this,
            menu_selector(CustomVideoOptionsLayer::selectNextFullscreenMode));
        selectFullscreenRightButton->setPosition(m_buttonMenu->convertToNodeSpace({ 70.f, 0.f }) + m_fields->m_modeText->getPosition());
        m_buttonMenu->addChild(selectFullscreenRightButton);

        // screen
        m_fields->m_screenText = CCLabelBMFont::create(getScreenCstr(), "bigFont.fnt");
        m_fields->m_screenText->setPosition({ m_mainLayer->getContentSize().width * 0.5f + 84.f, fullscreenToggleLabel->getPositionY() - 0.75f });
        m_fields->m_screenText->setScale(fullscreenToggleLabel->getScale() * 0.85f);
        m_mainLayer->addChild(m_fields->m_screenText);

        auto screenSelectorLabel = CCLabelBMFont::create("Screen", "goldFont.fnt");
        screenSelectorLabel->setPosition({ m_fields->m_screenText->getPositionX(), m_fields->m_screenText->getPositionY() + 17.f });
        screenSelectorLabel->setScale(fullscreenToggleLabel->getScale());
        m_mainLayer->addChild(screenSelectorLabel);

        auto selectScreenLeftSprite = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
        auto selectScreenLeftButton = CCMenuItemSpriteExtra::create(selectScreenLeftSprite, this,
            menu_selector(CustomVideoOptionsLayer::selectPreviousMonitor));
        selectScreenLeftButton->setPosition(m_buttonMenu->convertToNodeSpace({ -70.f, 0.f }) + m_fields->m_screenText->getPosition());
        m_buttonMenu->addChild(selectScreenLeftButton);

        auto selectScreenRightSprite = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
        auto selectScreenRightButton = CCMenuItemSpriteExtra::create(selectScreenRightSprite, this,
            menu_selector(CustomVideoOptionsLayer::selectNextMonitor));
        selectScreenRightButton->setPosition(m_buttonMenu->convertToNodeSpace({ 70.f, 0.f }) + m_fields->m_screenText->getPosition());
        m_buttonMenu->addChild(selectScreenRightButton);

        toggleResolution();

        return true;
    }

    void toggleResolution() {
        auto prev = m_isFullscreen;
        m_isFullscreen |= m_fields->m_isBorderless;
        VideoOptionsLayer::toggleResolution();
        m_isFullscreen = prev;
    }

    void onApply(CCObject*) {
        auto gm = GameManager::sharedState();
        auto egl = reinterpret_cast<CustomCCEGLView*>(CCEGLView::sharedOpenGLView());
        auto director = CCDirector::sharedDirector();

        int resolution = m_resolutions->stringAtIndex(m_currentResolution)->intValue();

        CCSize size = m_fields->m_isBorderless ? egl->getDisplaySize() : gm->resolutionForKey(resolution);

        bool resourcesChanged = gm->m_quality != m_quality;
        bool windowChanged = gm->getGameVariable("0025") == m_isFullscreen ||
            isBorderless() != m_fields->m_isBorderless || screen() != m_fields->m_screen || egl->getWindowedSize() != size;

        gm->m_quality = m_quality;
        gm->setGameVariable("0025", !m_isFullscreen);
        gm->m_resolution = resolution;
        isBorderless(m_fields->m_isBorderless);
        screen(m_fields->m_screen);

        director->updateContentScale(m_quality);
        egl->setWindowedSize(size);

        if(!resourcesChanged && !windowChanged)
            return;

        if(windowChanged)
            egl->startForceToggleFullScreen(m_isFullscreen);
        gm->reloadAll(windowChanged, m_isFullscreen, true);
    }
};
