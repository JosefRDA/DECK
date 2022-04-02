#ifndef _DECKMENUITEM_H_
#define _DECKMENUITEM_H_

#include <Arduino.h>

typedef struct {
  String label;
  String value;
  bool selected;
  void (*shortPressAction)(void);
  void (*longPressAction)(void);
} DeckMenuItem;

#endif // end _DECKMENUITEM_H_
