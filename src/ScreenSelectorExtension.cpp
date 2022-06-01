#include "includes.h"
#include "FullscreenManager.hpp"
#include "ScreenSelectorExtension.hpp"

MegaHackExt::ComboBox* ScreenSelectorExtension::_fullscreenModeComboBox;
MegaHackExt::ComboBox* ScreenSelectorExtension::_screenComboBox;

bool _initialized = false;

int _fullscreenModeSelectorIndex = 0;
int _screenSelectorIndex = 0;

int _fullscreenModeSelectorTextIndex;
int _screenSelectorTextIndex;

bool ScreenSelectorExtension::mhLoaded() { return GetModuleHandle(TEXT("hackpro.dll")); }

void MH_CALL comboBoxFullscreenModeChanged(MegaHackExt::ComboBox*, int, const char*);
void MH_CALL comboBoxCurrentScreenChanged(MegaHackExt::ComboBox*, int, const char*);

void ScreenSelectorExtension::create() {
    if(!mhLoaded())
        return;

    auto ext = MegaHackExt::Window::Create("Screen Selector");

    _screenComboBox = MegaHackExt::ComboBox::Create("", "");
    _screenComboBox->setCallback(comboBoxCurrentScreenChanged);

    auto monitorStrings = FullscreenManager::getMonitorStrings();
    std::vector<const char*> monitorCstrings;
    monitorCstrings.reserve(monitorStrings.size());
    for(int i = monitorStrings.size() - 1; i >= 0; i--)
        monitorCstrings.push_back(monitorStrings[i].c_str());
    monitorCstrings.push_back(0);

    _screenComboBox->setValues(&monitorCstrings[0], false);

    ext->add(_screenComboBox);

    _fullscreenModeComboBox = MegaHackExt::ComboBox::Create("", "");
    _fullscreenModeComboBox->setCallback(comboBoxFullscreenModeChanged);

    const char* fullscreenModes[] = { "Borderless Windowed", "Exclusive Fullscreen", "Windowed", 0 };
    _fullscreenModeComboBox->setValues(fullscreenModes, false);

    ext->add(_fullscreenModeComboBox);

    _initialized = true;
    updateScreenOption();
    updateFullscreenModeOption();

    MegaHackExt::Client::commit(ext);
}

void resetFullscreenModeSelection() {
    _fullscreenModeSelectorIndex = (int)FullscreenManager::getFullscreenMode();
}
void resetScreenSelection() {
    _screenSelectorIndex = FullscreenManager::getScreen();
}

void MH_CALL comboBoxFullscreenModeChanged(MegaHackExt::ComboBox* self, int index, const char* text) {
    if(!_initialized)
        return;
    FullscreenManager::setFullscreenMode((FullscreenMode)(2 - index));
}
void MH_CALL comboBoxCurrentScreenChanged(MegaHackExt::ComboBox* self, int index, const char* text) {
    if(!_initialized)
        return;
    FullscreenManager::setScreen(FullscreenManager::getMonitors().size() - index - 1);
}

void ScreenSelectorExtension::updateFullscreenModeOption() {
    if(!_initialized)
        return;

    if(_initialized && _fullscreenModeComboBox) {
        try { _fullscreenModeComboBox->set(2 - (int)FullscreenManager::getFullscreenMode(), false); }
        catch(...) { }
    }
}
void ScreenSelectorExtension::updateScreenOption() {
    if(_initialized && _screenComboBox) {
        try { _screenComboBox->set(FullscreenManager::getMonitorStrings().size() - 1 - FullscreenManager::getScreen(), false); }
        catch(...) { }
    }
}

void ScreenSelectorExtension::initVideoOptionsLayer(CCLayer* self) {
    auto layer = (CCNode*)self->getChildren()->objectAtIndex(0);
    auto children = layer->getChildren();
    auto vanillaFullscreenText = (CCNode*)children->objectAtIndex(3);
    auto menu = (CCNode*)children->objectAtIndex(2);
    auto menuChildren = menu->getChildren();
    auto vanillaFullscreenToggler = (CCNode*)menuChildren->objectAtIndex(0);

    vanillaFullscreenText->setVisible(false);
    vanillaFullscreenToggler->setVisible(false);

    resetFullscreenModeSelection();
    resetScreenSelection();

    // fullscreen
    auto fullscreenModeSelectorText = CCLabelBMFont::create(FullscreenManager::modeToCstr(FullscreenManager::getFullscreenMode()), "bigFont.fnt");
    fullscreenModeSelectorText->setPosition({ layer->getContentSize().width * 0.5f - 84.f, vanillaFullscreenText->getPositionY() - 0.75f });
    fullscreenModeSelectorText->setScale(vanillaFullscreenText->getScale() * 0.85f);
    layer->addChild(fullscreenModeSelectorText);

    auto fullscreenSelectorLabel = CCLabelBMFont::create("Fullscreen", "goldFont.fnt");
    fullscreenSelectorLabel->setPosition({ fullscreenModeSelectorText->getPositionX(), fullscreenModeSelectorText->getPositionY() + 17.f });
    fullscreenSelectorLabel->setScale(vanillaFullscreenText->getScale());
    layer->addChild(fullscreenSelectorLabel);

    auto selectFullscreenLeftSprite = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
    auto selectFullscreenLeftButton = gd::CCMenuItemSpriteExtra::create(selectFullscreenLeftSprite, fullscreenModeSelectorText,
        menu_selector(selectPreviousFullscreenMode));
    selectFullscreenLeftButton->setPosition(menu->convertToNodeSpace({ -70.f, 0.f }) + fullscreenModeSelectorText->getPosition());
    menu->addChild(selectFullscreenLeftButton);

    auto selectFullscreenRightSprite = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
    auto selectFullscreenRightButton = gd::CCMenuItemSpriteExtra::create(selectFullscreenRightSprite, fullscreenModeSelectorText,
        menu_selector(selectNextFullscreenMode));
    selectFullscreenRightButton->setPosition(menu->convertToNodeSpace({ 70.f, 0.f }) + fullscreenModeSelectorText->getPosition());
    menu->addChild(selectFullscreenRightButton);

    // screen
    auto screenSelectorText = CCLabelBMFont::create(FullscreenManager::getMonitorString().c_str(), "bigFont.fnt");
    screenSelectorText->setPosition({ layer->getContentSize().width * 0.5f + 84.f, vanillaFullscreenText->getPositionY() - 0.75f });
    screenSelectorText->setScale(vanillaFullscreenText->getScale() * 0.85f);
    layer->addChild(screenSelectorText);

    auto screenSelectorLabel = CCLabelBMFont::create("Screen", "goldFont.fnt");
    screenSelectorLabel->setPosition({ screenSelectorText->getPositionX(), screenSelectorText->getPositionY() + 17.f });
    screenSelectorLabel->setScale(vanillaFullscreenText->getScale());
    layer->addChild(screenSelectorLabel);

    auto selectScreenLeftSprite = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
    auto selectScreenLeftButton = gd::CCMenuItemSpriteExtra::create(selectScreenLeftSprite, screenSelectorText,
        menu_selector(selectPreviousMonitor));
    selectScreenLeftButton->setPosition(menu->convertToNodeSpace({ -70.f, 0.f }) + screenSelectorText->getPosition());
    menu->addChild(selectScreenLeftButton);

    auto selectScreenRightSprite = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
    auto selectScreenRightButton = gd::CCMenuItemSpriteExtra::create(selectScreenRightSprite, screenSelectorText,
        menu_selector(selectNextMonitor));
    selectScreenRightButton->setPosition(menu->convertToNodeSpace({ 70.f, 0.f }) + screenSelectorText->getPosition());
    menu->addChild(selectScreenRightButton);

    updateFullscreenModeOption();
    updateScreenOption();
}

void ScreenSelectorExtension::selectNextFullscreenMode(CCObject* object) {
    _fullscreenModeSelectorIndex++;
    while(_fullscreenModeSelectorIndex >= 3) _fullscreenModeSelectorIndex -= 3;
    while(_fullscreenModeSelectorIndex < 0) _fullscreenModeSelectorIndex += 3;
    ((CCLabelBMFont*)this)->setCString(FullscreenManager::modeToCstr((FullscreenMode)_fullscreenModeSelectorIndex));
}
void ScreenSelectorExtension::selectPreviousFullscreenMode(CCObject* object) {
    _fullscreenModeSelectorIndex--;
    while(_fullscreenModeSelectorIndex >= 3) _fullscreenModeSelectorIndex -= 3;
    while(_fullscreenModeSelectorIndex < 0) _fullscreenModeSelectorIndex += 3;
    ((CCLabelBMFont*)this)->setCString(FullscreenManager::modeToCstr((FullscreenMode)_fullscreenModeSelectorIndex));
}

void ScreenSelectorExtension::selectNextMonitor(CCObject* object) {
    _screenSelectorIndex++;
    int size = (int)FullscreenManager::getMonitors().size();
    while(_screenSelectorIndex >= size) _screenSelectorIndex -= size;
    while(_screenSelectorIndex < 0) _screenSelectorIndex += size;
    ((CCLabelBMFont*)this)->setCString(FullscreenManager::getMonitorStrings().at(_screenSelectorIndex).c_str());
}
void ScreenSelectorExtension::selectPreviousMonitor(CCObject* object) {
    _screenSelectorIndex--;
    int size = (int)FullscreenManager::getMonitors().size();
    while(_screenSelectorIndex >= size) _screenSelectorIndex -= size;
    while(_screenSelectorIndex < 0) _screenSelectorIndex += size;
    ((CCLabelBMFont*)this)->setCString(FullscreenManager::getMonitorStrings().at(_screenSelectorIndex).c_str());
}

void ScreenSelectorExtension::applySelections() {
    FullscreenManager::setFullscreenMode((FullscreenMode)_fullscreenModeSelectorIndex);
    FullscreenManager::setScreen(_screenSelectorIndex);

    resetFullscreenModeSelection();
    resetScreenSelection();
}
