#pragma once

#include "extensions2.h"

class ScreenSelectorExtension {
    private:
        static MegaHackExt::ComboBox* _fullscreenModeComboBox;
        static MegaHackExt::ComboBox* _screenComboBox;

    public:
        static bool mhLoaded();

        static void create();

        static void updateFullscreenModeOption();
        static void updateScreenOption();

        static void initVideoOptionsLayer(CCLayer*);

        void selectNextFullscreenMode(CCObject*);
        void selectPreviousFullscreenMode(CCObject*);

        void selectNextMonitor(CCObject*);
        void selectPreviousMonitor(CCObject*);

        static void applySelections();
};
