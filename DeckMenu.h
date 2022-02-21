#ifndef _DECKMENU_H_
#define _DECKMENU_H_

#define DECKMENU_DIRECTION_UP false
#define DECKMENU_DIRECTION_DOWN true

//CONFIG CONSTANTS
#define DECKMENU_ITEMS_PER_PAGE 3
#define DECKMENU_CIRCULAR_SELECT true

#include "DeckMenuItem.h"

class DeckMenu {
  public:
  
    // CONSTRUCTORS ------------------------------------------------------------
    
    DeckMenu(DeckMenuItem *items, byte itemsLength);
    
    // CLASS MEMBER FUNCTIONS ----------------------------------------------

    void select(bool direction); //TODO : Bool to typedef ?
    DeckMenuItem getSelected();
    
  private:

    // PRIVATE PROPERTIES ----------------------------------------------
    DeckMenuItem _items[3]; //TODO : Dynamic table allocation : https://stackoverflow.com/a/37078742
    byte _itemsLength;
    uint8_t _currentPageId;

    // CLASS PRIVATE FUNCTIONS ----------------------------------------------
    uint8_t getSelectedId();
};

#endif // end _DECKMENU_H_
