#include "DeckMenu.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckMenu::DeckMenu(DeckMenuItem *items, byte itemsLength, Adafruit_SSD1306 oled) { 
  _itemsLength = itemsLength;
  for (uint8_t i = 0; i < itemsLength; i++) {
    _items[i] = items[i];
    _items[i].selected = false;
  }
  _items[0].selected = true;

  _oled = oled;
}

// CLASS PRIVATE FUNCTIONS ----------------------------------------------
uint8_t DeckMenu::getSelectedId() {
  for (uint8_t i = 0; i < _itemsLength; i++) {
    if(_items[i].selected == true) {
      return i;
    }
  }
  return 99;
}

// CLASS MEMBER FUNCTIONS --------------------------------------------------

void DeckMenu::select(bool direction) {
  uint8_t selectedId = getSelectedId();
  if(direction == DECKMENU_DIRECTION_UP) {
    if(selectedId > 0) {
      _items[selectedId].selected = false;
      _items[selectedId-1].selected = true;
    } else {
      if(DECKMENU_CIRCULAR_SELECT) {
        _items[selectedId].selected = false;
        _items[_itemsLength-1].selected = true;
      } //else do nothing
    }
  } else { //direction == DECKMENU_DIRECTION_DOWN
    if(selectedId < _itemsLength-1) {
      _items[selectedId].selected = false;
      _items[selectedId+1].selected = true;
    } else {
      if(DECKMENU_CIRCULAR_SELECT) {
        _items[selectedId].selected = false;
        _items[0].selected = true;
      }
    }
  }
}

DeckMenuItem DeckMenu::getSelected() {
  return _items[getSelectedId()];
}

void DeckMenu::render(void) {
  
  _oled.clearDisplay();
  _oled.setTextSize(1);
  _oled.setCursor(0,0);

  _oled.drawRoundRect(0, 5, 64, 43, 4, WHITE);

  //Menu Header
  _oled.setCursor(DECKMENU_RENDER_X_POS, DECKMENU_RENDER_Y_HEADER_POS);
  _oled.setTextColor(WHITE, BLACK);
  _oled.print(" DECK ");
  
  //TODO : Handle more than 3 menu items
  uint8_t page = this->getCurrentPage();

  uint8_t displayCpt = 0;
  #if DECKMENU_DEBUG_SERIAL
  Serial.println("[DECKMENU_DEBUG_SERIAL] Starting menu build for page " + String(page) + ".");
  #endif
  for (uint8_t i = page * DECKMENU_ITEMS_PER_PAGE; i < _itemsLength &&  displayCpt < 3; i++) {
    _oled.setCursor(DECKMENU_RENDER_X_POS, DECKMENU_RENDER_Y_FIRST_POS + i%3 * DECKMENU_RENDER_Y_POS_OFSET);
    if(_items[i].selected) {
      _oled.setTextColor(BLACK, WHITE);
    } else {
      _oled.setTextColor(WHITE, BLACK);
    }
    _oled.print(_items[i].label);
    #if DECKMENU_DEBUG_SERIAL
    Serial.println("[DECKMENU_DEBUG_SERIAL] Item : " + String(i) + ", label : " + _items[i].label);
    #endif
    displayCpt++;
  }
  _oled.display();
}

uint8_t DeckMenu::getCurrentPage(void) { //commence a 0
  return this->getSelectedId() / DECKMENU_ITEMS_PER_PAGE;
}
