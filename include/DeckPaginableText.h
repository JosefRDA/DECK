#ifndef _DECKPAGINABLETEXT_H_
#define _DECKPAGINABLETEXT_H_

#define DECKPAGINABLETEXT_U8G2SCREEN_X_OFSET              0
#define DECKPAGINABLETEXT_U8G2SCREEN_Y_OFSET              8
#define DECKPAGINABLETEXT_U8G2SCREEN_LINE_LENGTH_MAX      11
#define DECKPAGINABLETEXT_U8G2SCREEN_LINE_HIGH            8
#define DECKPAGINABLETEXT_U8G2SCREEN_BASE_FONT            u8g2_font_5x8_tf
#define DECKPAGINABLETEXT_U8G2SCREEN_CURSORS_FONT         u8g2_font_open_iconic_arrow_1x_t
#define DECKPAGINABLETEXT_U8G2SCREEN_CURSORS_X_OFSET      58
#define DECKPAGINABLETEXT_U8G2SCREEN_CURSOR_PREV_Y_OFSET  DECKPAGINABLETEXT_U8G2SCREEN_Y_OFSET
#define DECKPAGINABLETEXT_U8G2SCREEN_CURSOR_PREV_CHAR     "\x43"
#define DECKPAGINABLETEXT_U8G2SCREEN_CURSOR_NEXT_Y_OFSET  47
#define DECKPAGINABLETEXT_U8G2SCREEN_CURSOR_NEXT_CHAR     "\x40"

#define DECKPAGINABLETEXT_DEBUG false

#include <Arduino.h>
#include "CstmLinkedList.hpp"

//OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <U8g2_for_Adafruit_GFX.h>

//CONFIG CONSTANTS
#define DECKPAGINABLETEXT_TEXT_MAX_LENGTH 66

class DeckPaginableText {
  public:
  
  // CONSTRUCTORS ------------------------------------------------------------
  
    DeckPaginableText(String text, Adafruit_SSD1306 oled);
    void render(void);
    void next(void);
    void prev(void);
  
  // CLASS MEMBER FUNCTIONS ----------------------------------------------

  private:

    // PRIVATE PROPERTIES ----------------------------------------------
    Adafruit_SSD1306 _oled;
    U8G2_FOR_ADAFRUIT_GFX _oledU8g2;
    CstmLinkedList<String> _text;

    // CLASS PRIVATE FUNCTIONS ----------------------------------------------

    int findNextWordEndClosestToScreenEnd(String text);
    void initMultiScreenText(String text);
    String splitTextToDisplay(String text);
    void printTextScreen(String text);

};
#endif // end _DECKPAGINABLETEXT_H_
