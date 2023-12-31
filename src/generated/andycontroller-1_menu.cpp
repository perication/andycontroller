/*
    The code in this file uses open source libraries provided by thecoderscorner

    DO NOT EDIT THIS FILE, IT WILL BE GENERATED EVERY TIME YOU USE THE UI DESIGNER
    INSTEAD EITHER PUT CODE IN YOUR SKETCH OR CREATE ANOTHER SOURCE FILE.

    All the variables you may need access to are marked extern in this file for easy
    use elsewhere.
 */

#include <tcMenu.h>
#include "andycontroller-1_menu.h"
#include "../ThemeCoolBlueTraditional.h"

// Global variable declarations
const PROGMEM  ConnectorLocalInfo applicationInfo = { "Pere's Controller", "4cbb933d-ab5e-44b6-bf9b-87f3c0c0cfc1" };

TFT_eSPI gfx;
TfteSpiDrawable gfxDrawable(&gfx, 0);
GraphicsDeviceRenderer renderer(30, applicationInfo.name, &gfxDrawable);

// Global Menu Item declarations
const PROGMEM AnalogMenuInfo minfoLowbatlevel = { "Lowbatlevel", 5, 12, 100, onlowbatlevelchanged, 1200, 100, "V" };
AnalogMenuItem menuLowbatlevel(&minfoLowbatlevel, 50, nullptr, INFO_LOCATION_PGM);
const PROGMEM AnalogMenuInfo minfoTrim = { "Trim", 4, 10, 200, onTrimChanged, -100, 100, "V" };
AnalogMenuItem menuTrim(&minfoTrim, 100, &menuLowbatlevel, INFO_LOCATION_PGM);
const PROGMEM AnalogMenuInfo minfoThreshold = { "Threshold", 1, 4, 1370, onVoltageThresholdChange, 0, 100, "V" };
AnalogMenuItem menuThreshold(&minfoThreshold, 1370, &menuTrim, INFO_LOCATION_PGM);
const PROGMEM AnalogMenuInfo minfoInternal = { "Internal", 3, 8, 600, NO_CALLBACK, 0, 100, "V" };
AnalogMenuItem menuInternal(&minfoInternal, 0, &menuThreshold, INFO_LOCATION_PGM);
const PROGMEM AnalogMenuInfo minfoBattery = { "Battery", 2, 6, 2500, NO_CALLBACK, 0, 100, "V" };
AnalogMenuItem menuBattery(&minfoBattery, 0, &menuInternal, INFO_LOCATION_PGM);

void setupMenu() {
    // First we set up eeprom and authentication (if needed).
    setSizeBasedEEPROMStorageEnabled(true);
    // Now add any readonly, non-remote and visible flags.
    menuBattery.setReadOnly(true);
    menuInternal.setReadOnly(true);

    // Code generated by plugins.
    gfx.begin();
    gfx.setRotation(2);
    renderer.setUpdatesPerSecond(10);
    switches.init(internalDigitalIo(), SWITCHES_NO_POLLING, true);
    menuMgr.initForEncoder(&renderer, &menuBattery, 37, 38, 39);
    renderer.setTitleMode(BaseGraphicalRenderer::TITLE_FIRST_ROW);
    renderer.setUseSliderForAnalog(true);
    installCoolBlueTraditionalTheme(renderer, MenuFontDef(nullptr, 2), MenuFontDef(nullptr, 2), true);
}

