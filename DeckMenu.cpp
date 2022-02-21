#include "DeckMenu.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckMenu::DeckMenu(DeckMenuItem *items, byte itemsLength) { 
  _itemsLength = itemsLength;
  for (uint8_t i = 0; i < itemsLength; i++) {
    _items[i] = items[i];
    _items[i].selected = false;
  }
  _items[0].selected = true;
}

// CLASS PRIVATE FUNCTIONS ----------------------------------------------
uint8_t DeckMenu::getSelectedId() {
  for (uint8_t i = 0; i < _itemsLength; i++) {
    if(_items[i].selected == true) {
      return i;
    }
  }
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
