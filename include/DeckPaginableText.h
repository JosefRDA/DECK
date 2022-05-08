#ifndef _DECKPAGINABLETEXT_H_
#define _DECKPAGINABLETEXT_H_

#define DECKPAGINABLETEXT_DEBUG false

#include <Arduino.h>
#include "CstmLinkedList.hpp"

//OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//CONFIG CONSTANTS
#define DECKPAGINABLETEXT_TEXT_MAX_LENGTH 62

class DeckPaginableText {
  public:
  
  // CONSTRUCTORS ------------------------------------------------------------
  
    DeckPaginableText(String text, Adafruit_SSD1306 oled);
    void render(void);
  
  // CLASS MEMBER FUNCTIONS ----------------------------------------------

  private:

    // PRIVATE PROPERTIES ----------------------------------------------
    Adafruit_SSD1306 _oled;
    CstmLinkedList<String> _text;

    // CLASS PRIVATE FUNCTIONS ----------------------------------------------

    int findNextWordEndClosestToScreenEnd(String text);
    void initMultiScreenText(String text);
};

#endif // end _DECKPAGINABLETEXT_H_
