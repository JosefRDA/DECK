#include "DeckPaginableText.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckPaginableText::DeckPaginableText(String text, Adafruit_SSD1306 oled) {
  this->_oled = oled;
  this->_oledU8g2.begin(this->_oled);

  this->_oledU8g2.setFontMode(1);                                   // use u8g2 transparent mode (this is default)
  this->_oledU8g2.setFontDirection(0);                              // left to right (this is default)
  this->_oledU8g2.setFont(DECKPAGINABLETEXT_U8G2SCREEN_BASE_FONT);  // select u8g2 font

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
  Serial.print("[DECKPAGINABLETEXT][initMultiScreenText] BEGIN text(" + String(text.length()) + ")=" + text + "\n");
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
  this->_text.Append(processingText); //Reliquat de texte

  #if DECKPAGINABLETEXT_DEBUG
  Serial.print("[DECKPAGINABLETEXT][initMultiScreenText] all lines : \n");
  this->_text.moveToStart();
  int cpt = 1;
  while (this->_text.hasNext())
  {
    Serial.print("SCREEN " + String(cpt++) + ":" + this->_text.getCurrent() + "\n");
    this->_text.next();
  }
  Serial.print("SCREEN " + String(cpt++) + ":" + this->_text.getCurrent() + "\n");
  
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

//deprecated
String DeckPaginableText::splitTextToDisplay(String text) {
  String result;
  if(text.length() > DECKPAGINABLETEXT_U8G2SCREEN_LINE_LENGTH_MAX) {
    result = text.substring(0, DECKPAGINABLETEXT_U8G2SCREEN_LINE_LENGTH_MAX) + "\n" + this->splitTextToDisplay(text.substring(DECKPAGINABLETEXT_U8G2SCREEN_LINE_LENGTH_MAX));
  } else {
    result = text;
  }
  result.trim();
  return result;
}

void DeckPaginableText::printTextScreen(String text) {


  String remainingText = text;
  int yOffset = DECKPAGINABLETEXT_U8G2SCREEN_Y_OFSET;
  do {
    this->_oledU8g2.setCursor(DECKPAGINABLETEXT_U8G2SCREEN_X_OFSET, yOffset);
    String textToPrint = remainingText.substring(0, DECKPAGINABLETEXT_U8G2SCREEN_LINE_LENGTH_MAX);
    textToPrint.trim();
    this->_oledU8g2.print(textToPrint);
    
    remainingText = remainingText.substring(min((unsigned int)DECKPAGINABLETEXT_U8G2SCREEN_LINE_LENGTH_MAX, remainingText.length()));

    yOffset += DECKPAGINABLETEXT_U8G2SCREEN_LINE_HIGH;
  } while(remainingText.length() > 0);
};

// CLASS MEMBER FUNCTIONS --------------------------------------------------

void DeckPaginableText::render(void) {
  this->_oled.clearDisplay();
  this->_oledU8g2.setForegroundColor(WHITE); 
  this->_oledU8g2.setBackgroundColor(BLACK); 

  //this->_oledU8g2.println(this->splitTextToDisplay(this->_text.getCurrent()));
  this->printTextScreen(this->_text.getCurrent());
  if(this->_text.hasPrev()) {
    this->_oledU8g2.setCursor(DECKPAGINABLETEXT_U8G2SCREEN_CURSORS_X_OFSET, DECKPAGINABLETEXT_U8G2SCREEN_Y_OFSET);
    this->_oledU8g2.setFont(DECKPAGINABLETEXT_U8G2SCREEN_CURSORS_FONT);
    this->_oledU8g2.println(DECKPAGINABLETEXT_U8G2SCREEN_CURSOR_PREV_CHAR);
    this->_oledU8g2.setFont(DECKPAGINABLETEXT_U8G2SCREEN_BASE_FONT);
  }
  if(this->_text.hasNext()) {
    this->_oledU8g2.setCursor(DECKPAGINABLETEXT_U8G2SCREEN_CURSORS_X_OFSET, DECKPAGINABLETEXT_U8G2SCREEN_CURSOR_NEXT_Y_OFSET);
    this->_oledU8g2.setFont(DECKPAGINABLETEXT_U8G2SCREEN_CURSORS_FONT);
    this->_oledU8g2.println(DECKPAGINABLETEXT_U8G2SCREEN_CURSOR_NEXT_CHAR);
    this->_oledU8g2.setFont(DECKPAGINABLETEXT_U8G2SCREEN_BASE_FONT);
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