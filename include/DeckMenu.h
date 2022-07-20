#ifndef _DECKMENU_H_
#define _DECKMENU_H_

#define DECKMENU_DEBUG_SERIAL false

#define DECKMENU_DIRECTION_UP false
#define DECKMENU_DIRECTION_DOWN true

//CONFIG CONSTANTS
#define DECKMENU_ITEMS_PER_PAGE 3
#define DECKMENU_CIRCULAR_SELECT true

//OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "DeckMenuItem.h"

//RENDER
#define DECKMENU_RENDER_X_POS         15
#define DECKMENU_RENDER_Y_HEADER_POS  2
#define DECKMENU_RENDER_Y_FIRST_POS   13
#define DECKMENU_RENDER_Y_POS_OFSET   11

class DeckMenu {
  public:
  
    // CONSTRUCTORS ------------------------------------------------------------
    
    DeckMenu(DeckMenuItem *items, byte itemsLength, Adafruit_SSD1306 oled);
    
    // CLASS MEMBER FUNCTIONS ----------------------------------------------

    void select(bool direction); //TODO : Bool to typedef ?
    DeckMenuItem getSelected();
    void render(void);
    
  private:

    // PRIVATE PROPERTIES ----------------------------------------------
    DeckMenuItem _items[6]; //TODO : Dynamic table allocation : https://stackoverflow.com/a/37078742
    byte _itemsLength;
    Adafruit_SSD1306 _oled;

    // CLASS PRIVATE FUNCTIONS ----------------------------------------------
    uint8_t getSelectedId();
    uint8_t getCurrentPage();
};

#endif // end _DECKMENU_H_
