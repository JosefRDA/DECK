#include "DeckChoiceNumberPopUp.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckChoiceNumberPopUp::DeckChoiceNumberPopUp(Adafruit_SSD1306 oled) { 
    this->_oled = oled;
    this->_values[0] = 0;
    this->_values[1] = 9;
    this->_values[2] = 7;
    //this->_values[0] = 0;
    //this->_values[1] = 0;
    //this->_values[2] = 0;
    this->_selectedField = 0;
    this->_editField = false;
}

// CLASS PRIVATE FUNCTIONS ----------------------------------------------

void DeckChoiceNumberPopUp::renderField(int id) {
    this->_oled.setCursor(DECKCHOICENUMBERPOPUP_RENDER_X_POS + DECKCHOICENUMBERPOPUP_RENDER_X_POS_OFSET*id, DECKCHOICENUMBERPOPUP_RENDER_Y_FIRST_POS);
    if(this->_selectedField == id && this->_editField) {
        this->_oled.setTextColor(BLACK, WHITE);
    }
    this->_oled.print(DECKCHOICENUMBERPOPUP_UP_ARROW_CHAR);
    if(this->_selectedField == id && this->_editField) {
        this->_oled.setTextColor(WHITE, BLACK);
    }

    this->_oled.setCursor(DECKCHOICENUMBERPOPUP_RENDER_X_POS + DECKCHOICENUMBERPOPUP_RENDER_X_POS_OFSET*id, DECKCHOICENUMBERPOPUP_RENDER_Y_FIRST_POS + DECKCHOICENUMBERPOPUP_RENDER_Y_POS_OFSET);
    if(this->_selectedField == id && !this->_editField) {
        this->_oled.setTextColor(BLACK, WHITE);
    }
    this->_oled.print(this->_values[id]);
    if(this->_selectedField == id && !this->_editField) {
        this->_oled.setTextColor(WHITE, BLACK);
    }

    this->_oled.setCursor(DECKCHOICENUMBERPOPUP_RENDER_X_POS + DECKCHOICENUMBERPOPUP_RENDER_X_POS_OFSET*id, DECKCHOICENUMBERPOPUP_RENDER_Y_FIRST_POS + DECKCHOICENUMBERPOPUP_RENDER_Y_POS_OFSET*2);
    if(this->_selectedField == id && this->_editField) {
        this->_oled.setTextColor(BLACK, WHITE);
    }
    this->_oled.print(DECKCHOICENUMBERPOPUP_DOWN_ARROW_CHAR);
    if(this->_selectedField == id && this->_editField) {
        this->_oled.setTextColor(WHITE, BLACK);
    }
}

// CLASS MEMBER FUNCTIONS --------------------------------------------------
void DeckChoiceNumberPopUp::increaseSelectedField(void) {
    if(this->_selectedField < DECKCHOICENUMBERPOPUP_FIELD_NUMBER) {
        uint8_t currentValue = this->_values[this->_selectedField];
        if(currentValue + 1 > DECKCHOICENUMBERPOPUP_FIELD_MAX_VALUE) {
            if(DECKCHOICENUMBERPOPUP_FIELD_CIRCULAR_SELECT) {
                this->_values[this->_selectedField] = DECKCHOICENUMBERPOPUP_FIELD_MIN_VALUE;
            } else {
                //DO nothing
            }
        } else {
            this->_values[this->_selectedField] = currentValue + 1;
        }
    } //Si le focus n'est pas sur un champ il ne fait pas sens de le modifier
}
void DeckChoiceNumberPopUp::decreaseSelectedField(void) {
    if(this->_selectedField < DECKCHOICENUMBERPOPUP_FIELD_NUMBER) {
        uint8_t currentValue = this->_values[this->_selectedField];
        if(currentValue - 1 < DECKCHOICENUMBERPOPUP_FIELD_MIN_VALUE) {
            if(DECKCHOICENUMBERPOPUP_FIELD_CIRCULAR_SELECT) {
                this->_values[this->_selectedField] = DECKCHOICENUMBERPOPUP_FIELD_MAX_VALUE;
            } else {
                //DO nothing
            }
        } else {
            this->_values[this->_selectedField] = currentValue - 1;
        }
    } //Si le focus n'est pas sur un champ il ne fait pas sens de le modifier
}
void DeckChoiceNumberPopUp::goToNextField(void) {
    if(this->_selectedField + 1 > DECKCHOICENUMBERPOPUP_CONTROLS_NUMBER-1) {
        if(DECKCHOICENUMBERPOPUP_FIELD_CIRCULAR_SELECT) { 
            this->_selectedField = 0;
        } else {
            //DO nothing
        }
    } else {
        this->_selectedField = this->_selectedField + 1;
    }
}
void DeckChoiceNumberPopUp::goToPrevField(void) {
    if(this->_selectedField - 1 < 0) {
        if(DECKCHOICENUMBERPOPUP_FIELD_CIRCULAR_SELECT) { 
            this->_selectedField = DECKCHOICENUMBERPOPUP_CONTROLS_NUMBER - 1;
        } else {
            //DO nothing
        }
    } else {
        this->_selectedField = this->_selectedField - 1;
    }
}
void DeckChoiceNumberPopUp::enterEditField(void) {
    this->_editField = true;
}
void DeckChoiceNumberPopUp::leaveEditField(void) {
    this->_editField = false;
}

void DeckChoiceNumberPopUp::render(void) {
    this->_oled.clearDisplay();
    this->_oled.setTextSize(1);
    this->_oled.setCursor(0,0);


   this->_oled.drawRoundRect(0, 5, 64, 43, 4, WHITE);

    //Menu Header
    this->_oled.setCursor(DECKCHOICENUMBERPOPUP_RENDER_X_HEADER_POS, DECKCHOICENUMBERPOPUP_RENDER_Y_HEADER_POS);
    this->_oled.setTextColor(WHITE, BLACK);
    this->_oled.print("  ID  ");

    this->renderField(0);
    this->renderField(1);
    this->renderField(2);

    this->_oled.display();
}
