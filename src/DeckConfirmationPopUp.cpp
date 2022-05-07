#include "DeckConfirmationPopUp.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckConfirmationPopUp::DeckConfirmationPopUp(String questionLabel, String okLabel, String koLabel, Adafruit_SSD1306 oled) { 
    questionLabel.trim();

    this->_questionLabel = questionLabel;
    this->_okLabel = okLabel;
    this->_koLabel = koLabel;
    this->_oled = oled;

    this->_okSelected = true;
}

// CLASS PRIVATE FUNCTIONS ----------------------------------------------

// CLASS MEMBER FUNCTIONS --------------------------------------------------

void DeckConfirmationPopUp::toggleSelection() {
    this->_okSelected = !this->_okSelected;
}

bool DeckConfirmationPopUp::isOkSelected() {
    return this->_okSelected;
}

void DeckConfirmationPopUp::render(void) {
    int questionLabelRemainingLength = this->_questionLabel.length();


    this->_oled.clearDisplay();
    this->_oled.setTextSize(1);
    this->_oled.setCursor(0,0);


    this->_oled.drawRoundRect(0, 0, 64, 48, 4, WHITE);
    
    //Question Label, Line 1
    this->_oled.setCursor(DECKCONFIRMATIONPOPUP_RENDER_X_POS, DECKCONFIRMATIONPOPUP_RENDER_Y_FIRST_POS);
    int lineMaxLength = min(DECKCONFIRMATIONPOPUP_RENDER_QUESTION_LABEL_LINE_LENGTH, questionLabelRemainingLength);
    this->_oled.print(this->_questionLabel.substring(0, lineMaxLength));
    questionLabelRemainingLength = questionLabelRemainingLength - DECKCONFIRMATIONPOPUP_RENDER_QUESTION_LABEL_LINE_LENGTH;


    //Question Label, Line 2
    if(questionLabelRemainingLength > 0) {
        this->_oled.setCursor(DECKCONFIRMATIONPOPUP_RENDER_X_POS, DECKCONFIRMATIONPOPUP_RENDER_Y_FIRST_POS + DECKCONFIRMATIONPOPUP_RENDER_Y_POS_OFSET);
        lineMaxLength = min(DECKCONFIRMATIONPOPUP_RENDER_QUESTION_LABEL_LINE_LENGTH, questionLabelRemainingLength);
        this->_oled.print(this->_questionLabel.substring(DECKCONFIRMATIONPOPUP_RENDER_QUESTION_LABEL_LINE_LENGTH, DECKCONFIRMATIONPOPUP_RENDER_QUESTION_LABEL_LINE_LENGTH + lineMaxLength));
        questionLabelRemainingLength = questionLabelRemainingLength - DECKCONFIRMATIONPOPUP_RENDER_QUESTION_LABEL_LINE_LENGTH;
        
        //Question Label, Line 3
        if(questionLabelRemainingLength > 0) {
            this->_oled.setCursor(DECKCONFIRMATIONPOPUP_RENDER_X_POS, DECKCONFIRMATIONPOPUP_RENDER_Y_FIRST_POS + DECKCONFIRMATIONPOPUP_RENDER_Y_POS_OFSET*2);
            lineMaxLength = min(DECKCONFIRMATIONPOPUP_RENDER_QUESTION_LABEL_LINE_LENGTH, questionLabelRemainingLength);
            this->_oled.print(this->_questionLabel.substring(DECKCONFIRMATIONPOPUP_RENDER_QUESTION_LABEL_LINE_LENGTH *2 , DECKCONFIRMATIONPOPUP_RENDER_QUESTION_LABEL_LINE_LENGTH *2 + lineMaxLength));
        
        }
        
    }
    
    this->_oled.setCursor(DECKCONFIRMATIONPOPUP_RENDER_X_POS,DECKCONFIRMATIONPOPUP_RENDER_Y_FIRST_POS + DECKCONFIRMATIONPOPUP_RENDER_Y_POS_OFSET*3);
    if(this->isOkSelected()) {
        this->_oled.setTextColor(BLACK, WHITE);
    }
    this->_oled.print(this->_okLabel.substring(0,DECKCONFIRMATIONPOPUP_RENDER_BUTTON_LABELS_MAX_LENGTH));
    this->_oled.setTextColor(WHITE, BLACK);
    this->_oled.print("   ");
    if(!this->isOkSelected()) {
        this->_oled.setTextColor(BLACK, WHITE);
    }
    this->_oled.print(this->_koLabel.substring(0,DECKCONFIRMATIONPOPUP_RENDER_BUTTON_LABELS_MAX_LENGTH));
    this->_oled.setTextColor(WHITE, BLACK);
    this->_oled.display();
}
