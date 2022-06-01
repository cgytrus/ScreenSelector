#ifndef __SCREEN_SELECTOR_EXTENSION_HPP
#define __SCREEN_SELECTOR_EXTENSION_HPP

#include "hackpro_ext.hpp"

class ScreenSelectorExtension : public Hackpro::Extension {
    private:
        void initialize();

        static Hackpro::ComboBox* _fullscreenModeComboBox;
        static Hackpro::ComboBox* _screenComboBox;

    protected:
        void __stdcall comboBoxFullscreenModeChanged(int, const char*);

        void __stdcall comboBoxCurrentScreenChanged(int, const char*);

    public:
        static ScreenSelectorExtension* create();

        static void updateFullscreenModeOption();
        static void updateScreenOption();

        static void initVideoOptionsLayer(CCLayer*);

        void selectNextFullscreenMode(CCObject*);
        void selectPreviousFullscreenMode(CCObject*);

        void selectNextMonitor(CCObject*);
        void selectPreviousMonitor(CCObject*);

        static void applySelections();
};

#endif
