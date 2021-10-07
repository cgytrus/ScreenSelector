#ifndef __SCREEN_SELECTOR_EXTENSION_HPP
#define __SCREEN_SELECTOR_EXTENSION_HPP

#include "hackpro_ext.hpp"

class ScreenSelectorExtension : public Hackpro::Extension {
    private:
        void Initialize();

        static Hackpro::ComboBox* _fullscreenModeComboBox;
        static Hackpro::ComboBox* _screenComboBox;

    protected:
        void __stdcall ComboBoxFullscreenModeChanged(int, const char*);

        void __stdcall ComboBoxCurrentScreenChanged(int, const char*);

    public:
        static ScreenSelectorExtension* Create();

        static void UpdateFullscreenModeOption();
        static void UpdateScreenOption();

        static void InitVideoOptionsLayer(CCLayer*);

        void SelectNextFullscreenMode(CCObject*);
        void SelectPreviousFullscreenMode(CCObject*);

        void SelectNextMonitor(CCObject*);
        void SelectPreviousMonitor(CCObject*);

        static void ApplySelections();
};

#endif
