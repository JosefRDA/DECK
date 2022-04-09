#include "DeckPaginableText.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckPaginableText::DeckPaginableText(String text, Adafruit_SSD1306 oled) {
  _oled = oled;

  if(text.length() <= DECKPAGINABLETEXT_TEXT_MAX_LENGTH) {
    _text.Append(text);
  } else {
    this->initMultiScreenText(text);
  }
}

// CLASS PRIVATE FUNCTIONS ----------------------------------------------

void DeckPaginableText::initMultiScreenText(String text) {
  String processingText = text;
  while(processingText.length() <= DECKPAGINABLETEXT_TEXT_MAX_LENGTH) {
    int nextWordEndClosestToScreenEnd = this->findNextWordEndClosestToScreenEnd(processingText);
    if(nextWordEndClosestToScreenEnd < 1 ) {
      break; //Never suppose to happend
    }
    _text.Append(processingText.substring(0, nextWordEndClosestToScreenEnd)); //(nextWordEndClosestToScreenEnd-1) ?
    processingText = processingText.substring(nextWordEndClosestToScreenEnd, processingText.length());   //(nextWordEndClosestToScreenEnd+1) ?
  }
}

int DeckPaginableText::findNextWordEndClosestToScreenEnd(String text) {
  if(text.length() <= DECKPAGINABLETEXT_TEXT_MAX_LENGTH) {
    return 0;
  } else {
    for(int i = (DECKPAGINABLETEXT_TEXT_MAX_LENGTH - 1); i >= 0; i--) {
      if(text.charAt(i) == ' ') {
        return i;
      }
    }
    return -1;
  }
}


// CLASS MEMBER FUNCTIONS --------------------------------------------------

void DeckPaginableText::render(void) {
  _oled.clearDisplay();
  _oled.setCursor(0,0);
  _text.moveToStart();
  _oled.println(_text.getCurrent());
  _oled.display();
}
