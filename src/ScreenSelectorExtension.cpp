#include "includes.h"
#include "FullscreenManager.hpp"
#include "ScreenSelectorExtension.hpp"

Hackpro::ComboBox* ScreenSelectorExtension::_fullscreenModeComboBox;
Hackpro::ComboBox* ScreenSelectorExtension::_screenComboBox;

bool _initialized = false;

int _fullscreenModeSelectorIndex = 0;
int _screenSelectorIndex = 0;

CCLabelBMFont* _fullscreenModeSelectorText;
CCLabelBMFont* _screenSelectorText;

ScreenSelectorExtension* ScreenSelectorExtension::Create() {
    auto pRet = static_cast<ScreenSelectorExtension*>(Extension::Create("Screen Selector"));
    pRet->Initialize();
    return pRet;
}

void ScreenSelectorExtension::Initialize() {
    _screenComboBox = AddComboBox(combobox_callback_func(ScreenSelectorExtension::ComboBoxCurrentScreenChanged));

    auto monitorStrings = FullscreenManager::GetMonitorStrings();
    std::vector<const char*> monitorCstrings;
    monitorCstrings.reserve(monitorStrings.size());
    for(int i = monitorStrings.size() - 1; i >= 0; i--)
        monitorCstrings.push_back(monitorStrings[i].c_str());
    monitorCstrings.push_back(0);

    _screenComboBox->SetStrings(&monitorCstrings[0]);

    _fullscreenModeComboBox = AddComboBox(combobox_callback_func(ScreenSelectorExtension::ComboBoxFullscreenModeChanged));

    const char* fullscreenModes[] = { "Borderless Windowed", "Exclusive Fullscreen", "Windowed", 0 };
    _fullscreenModeComboBox->SetStrings(fullscreenModes);

    _initialized = true;
    UpdateScreenOption();
    UpdateFullscreenModeOption();

    Commit();
}

void resetFullscreenModeSelection() {
    _fullscreenModeSelectorIndex = (int)FullscreenManager::GetFullscreenMode();
    _screenSelectorIndex = FullscreenManager::GetScreen();
}
void resetScreenSelection() {
    _fullscreenModeSelectorIndex = (int)FullscreenManager::GetFullscreenMode();
    _screenSelectorIndex = FullscreenManager::GetScreen();
}

void __stdcall ScreenSelectorExtension::ComboBoxFullscreenModeChanged(int index, const char* text) {
    if(!_initialized) return;
    FullscreenManager::SetFullscreenMode((FullscreenMode)(2 - index));
}

void __stdcall ScreenSelectorExtension::ComboBoxCurrentScreenChanged(int index, const char* text) {
    if(!_initialized) return;
    FullscreenManager::SetScreen(FullscreenManager::GetMonitors().size() - index - 1);
}

void ScreenSelectorExtension::UpdateFullscreenModeOption() {
    if(_fullscreenModeSelectorText != nullptr && *(uintptr_t*)_fullscreenModeSelectorText != NULL) {
        try { _fullscreenModeSelectorText->setCString(FullscreenManager::ModeToCstr(FullscreenManager::GetFullscreenMode())); }
        catch(...) { }
    }
    if(_initialized && _fullscreenModeComboBox != nullptr && *(uintptr_t*)_fullscreenModeComboBox != NULL) {
        try { _fullscreenModeComboBox->SetIndex(2 - (int)FullscreenManager::GetFullscreenMode()); }
        catch(...) { }
    }
}
void ScreenSelectorExtension::UpdateScreenOption() {
    if(_screenSelectorText != nullptr && *(uintptr_t*)_screenSelectorText != NULL) {
        try { _screenSelectorText->setCString(FullscreenManager::GetMonitorString().c_str()); }
        catch(...) { }
    }
    if(_initialized && _screenComboBox != nullptr && *(uintptr_t*)_screenComboBox != NULL) {
        try { _screenComboBox->SetIndex(FullscreenManager::GetMonitorStrings().size() - 1 - FullscreenManager::GetScreen()); }
        catch(...) { }
    }
}

void ScreenSelectorExtension::InitVideoOptionsLayer(CCLayer* self) {
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
    _fullscreenModeSelectorText = CCLabelBMFont::create("Unknown", "bigFont.fnt");
    _fullscreenModeSelectorText->setPosition({ layer->getContentSize().width * 0.5f - 84.f, vanillaFullscreenText->getPositionY() - 0.75f });
    _fullscreenModeSelectorText->setScale(vanillaFullscreenText->getScale() * 0.85f);
    layer->addChild(_fullscreenModeSelectorText);

    auto fullscreenSelectorLabel = CCLabelBMFont::create("Fullscreen", "goldFont.fnt");
    fullscreenSelectorLabel->setPosition({ _fullscreenModeSelectorText->getPositionX(), _fullscreenModeSelectorText->getPositionY() + 17.f });
    fullscreenSelectorLabel->setScale(vanillaFullscreenText->getScale());
    layer->addChild(fullscreenSelectorLabel);

    auto selectFullscreenLeftSprite = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
    auto selectFullscreenLeftButton = gd::CCMenuItemSpriteExtra::create(selectFullscreenLeftSprite, _fullscreenModeSelectorText,
        menu_selector(SelectPreviousFullscreenMode));
    selectFullscreenLeftButton->setPosition(menu->convertToNodeSpace({ -70.f, 0.f }) + _fullscreenModeSelectorText->getPosition());
    menu->addChild(selectFullscreenLeftButton);

    auto selectFullscreenRightSprite = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
    auto selectFullscreenRightButton = gd::CCMenuItemSpriteExtra::create(selectFullscreenRightSprite, _fullscreenModeSelectorText,
        menu_selector(SelectNextFullscreenMode));
    selectFullscreenRightButton->setPosition(menu->convertToNodeSpace({ 70.f, 0.f }) + _fullscreenModeSelectorText->getPosition());
    menu->addChild(selectFullscreenRightButton);

    // screen
    _screenSelectorText = CCLabelBMFont::create("9: 9999x9999", "bigFont.fnt");
    _screenSelectorText->setPosition({ layer->getContentSize().width * 0.5f + 84.f, vanillaFullscreenText->getPositionY() - 0.75f });
    _screenSelectorText->setScale(vanillaFullscreenText->getScale() * 0.85f);
    layer->addChild(_screenSelectorText);

    auto screenSelectorLabel = CCLabelBMFont::create("Screen", "goldFont.fnt");
    screenSelectorLabel->setPosition({ _screenSelectorText->getPositionX(), _screenSelectorText->getPositionY() + 17.f });
    screenSelectorLabel->setScale(vanillaFullscreenText->getScale());
    layer->addChild(screenSelectorLabel);

    auto selectScreenLeftSprite = CCSprite::createWithSpriteFrameName("edit_leftBtn_001.png");
    auto selectScreenLeftButton = gd::CCMenuItemSpriteExtra::create(selectScreenLeftSprite, _screenSelectorText,
        menu_selector(SelectPreviousMonitor));
    selectScreenLeftButton->setPosition(menu->convertToNodeSpace({ -70.f, 0.f }) + _screenSelectorText->getPosition());
    menu->addChild(selectScreenLeftButton);

    auto selectScreenRightSprite = CCSprite::createWithSpriteFrameName("edit_rightBtn_001.png");
    auto selectScreenRightButton = gd::CCMenuItemSpriteExtra::create(selectScreenRightSprite, _screenSelectorText,
        menu_selector(SelectNextMonitor));
    selectScreenRightButton->setPosition(menu->convertToNodeSpace({ 70.f, 0.f }) + _screenSelectorText->getPosition());
    menu->addChild(selectScreenRightButton);

    UpdateFullscreenModeOption();
    UpdateScreenOption();
}

void ScreenSelectorExtension::SelectNextFullscreenMode(CCObject* object) {
    _fullscreenModeSelectorIndex++;
    while(_fullscreenModeSelectorIndex >= 3) _fullscreenModeSelectorIndex -= 3;
    while(_fullscreenModeSelectorIndex < 0) _fullscreenModeSelectorIndex += 3;
    ((CCLabelBMFont*)this)->setCString(FullscreenManager::ModeToCstr((FullscreenMode)_fullscreenModeSelectorIndex));
}
void ScreenSelectorExtension::SelectPreviousFullscreenMode(CCObject* object) {
    _fullscreenModeSelectorIndex--;
    while(_fullscreenModeSelectorIndex >= 3) _fullscreenModeSelectorIndex -= 3;
    while(_fullscreenModeSelectorIndex < 0) _fullscreenModeSelectorIndex += 3;
    ((CCLabelBMFont*)this)->setCString(FullscreenManager::ModeToCstr((FullscreenMode)_fullscreenModeSelectorIndex));
}

void ScreenSelectorExtension::SelectNextMonitor(CCObject* object) {
    _screenSelectorIndex++;
    int size = (int)FullscreenManager::GetMonitors().size();
    while(_screenSelectorIndex >= size) _screenSelectorIndex -= size;
    while(_screenSelectorIndex < 0) _screenSelectorIndex += size;
    ((CCLabelBMFont*)this)->setCString(FullscreenManager::GetMonitorStrings().at(_screenSelectorIndex).c_str());
}
void ScreenSelectorExtension::SelectPreviousMonitor(CCObject* object) {
    _screenSelectorIndex--;
    int size = (int)FullscreenManager::GetMonitors().size();
    while(_screenSelectorIndex >= size) _screenSelectorIndex -= size;
    while(_screenSelectorIndex < 0) _screenSelectorIndex += size;
    ((CCLabelBMFont*)this)->setCString(FullscreenManager::GetMonitorStrings().at(_screenSelectorIndex).c_str());
}

void ScreenSelectorExtension::ApplySelections() {
    FullscreenManager::SetFullscreenMode((FullscreenMode)_fullscreenModeSelectorIndex);
    FullscreenManager::SetScreen(_screenSelectorIndex);

    resetFullscreenModeSelection();
    resetScreenSelection();
}
