#include "DeckPaginableText.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckPaginableText::DeckPaginableText(String text, Adafruit_SSD1306 oled) {
  this->_oled = oled;

  if(text.length() <= DECKPAGINABLETEXT_TEXT_MAX_LENGTH) {
    this->_text.Append(text);
  } else {
    this->initMultiScreenText(text);
  }
  
  this->_text.moveToStart();
}

// CLASS PRIVATE FUNCTIONS ----------------------------------------------

void DeckPaginableText::initMultiScreenText(String text) {
  #if DECKPAGINABLETEXT_DEBUG
  Serial.print("[DECKPAGINABLETEXT][initMultiScreenText] BEGIN text=" +  + "\n");
  #endif
  String processingText = text;
  
  while(processingText.length() >= DECKPAGINABLETEXT_TEXT_MAX_LENGTH) {

    #if DECKPAGINABLETEXT_DEBUG
    Serial.print("[DECKPAGINABLETEXT][initMultiScreenText] processingText.length() =" + String(processingText.length()) + "\n");
    #endif

    int nextWordEndClosestToScreenEnd = this->findNextWordEndClosestToScreenEnd(processingText);

    #if DECKPAGINABLETEXT_DEBUG
    Serial.print("[DECKPAGINABLETEXT][initMultiScreenText] nextWordEndClosestToScreenEnd =" + String(nextWordEndClosestToScreenEnd) + "\n");
    #endif

    if(nextWordEndClosestToScreenEnd < 1 ) {
      break; //Never suppose to happend
    }

    this->_text.Append(processingText.substring(0, nextWordEndClosestToScreenEnd)); //(nextWordEndClosestToScreenEnd-1) ?
    processingText = processingText.substring(nextWordEndClosestToScreenEnd, processingText.length());   //(nextWordEndClosestToScreenEnd+1) ?
    #if DECKPAGINABLETEXT_DEBUG
    Serial.print("[DECKPAGINABLETEXT][initMultiScreenText] end loop =" + String(nextWordEndClosestToScreenEnd) + "\n");
    #endif
  }
  #if DECKPAGINABLETEXT_DEBUG
  Serial.print("[DECKPAGINABLETEXT][initMultiScreenText] all lines : \n");
  this->_text.moveToStart();
  int cpt = 1;
  while (this->_text.hasNext())
  {
    Serial.print("SCREEN " + String(cpt++) + ":" + this->_text.getCurrent() + "\n");
    this->_text.next();
  }
  
  Serial.print("[DECKPAGINABLETEXT][initMultiScreenText] END\n");
  #endif
}

int DeckPaginableText::findNextWordEndClosestToScreenEnd(String text) {
  if(text.length() <= DECKPAGINABLETEXT_TEXT_MAX_LENGTH) {
    return 0;
  } else {
    for(int i = (DECKPAGINABLETEXT_TEXT_MAX_LENGTH - 1); i >= 0; i--) {
      #if DECKPAGINABLETEXT_DEBUG
      Serial.print("[DECKPAGINABLETEXT][findNextWordEndClosestToScreenEnd] currentchar =" + String(text.charAt(i)) + "\n");
      #endif
      if(text.charAt(i) == ' ') {
        return i;
      }
    }
    return DECKPAGINABLETEXT_TEXT_MAX_LENGTH;
  }
}


// CLASS MEMBER FUNCTIONS --------------------------------------------------

void DeckPaginableText::render(void) {
  this->_oled.clearDisplay();
  this->_oled.setCursor(0,0);
  this->_oled.println(_text.getCurrent());
  if(this->_text.hasPrev()) {
    this->_oled.setCursor(55,0);
    this->_oled.setTextColor(BLACK, WHITE);
    this->_oled.println("A");
    this->_oled.setTextColor(WHITE, BLACK);
  }
  if(this->_text.hasNext()) {
    this->_oled.setCursor(55,40);
    this->_oled.setTextColor(BLACK, WHITE);
    this->_oled.println("V");
    this->_oled.setTextColor(WHITE, BLACK);
  }
  this->_oled.display();
}

void DeckPaginableText::next() {
  if(this->_text.hasNext()) {
    this->_text.next();
  }
}

void DeckPaginableText::prev() {
  if(this->_text.hasPrev()) {
    bool retval = this->_text.prev();
  }
}