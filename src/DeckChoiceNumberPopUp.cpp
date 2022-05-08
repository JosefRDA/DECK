#include "DeckChoiceNumberPopUp.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckChoiceNumberPopUp::DeckChoiceNumberPopUp(Adafruit_SSD1306 oled, uint8_t presentValue) { 
    this->_oled = oled;
    this->_values[0] = presentValue/100;
    this->_values[1] = presentValue%100/10;
    this->_values[2] = presentValue%10;
    //this->_values[0] = 0;
    //this->_values[1] = 0;
    //this->_values[2] = 0;
    this->_selectedField = 3;
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

void DeckChoiceNumberPopUp::renderButton(void) {
    this->_oled.setCursor(DECKCHOICENUMBERPOPUP_RENDER_X_POS + DECKCHOICENUMBERPOPUP_RENDER_X_POS_OFSET*4, DECKCHOICENUMBERPOPUP_RENDER_Y_FIRST_POS + DECKCHOICENUMBERPOPUP_RENDER_Y_POS_OFSET);
    if(this->_selectedField == 3) {
        this->_oled.setTextColor(BLACK, WHITE);
    }
    this->_oled.print("OK");
    if(this->_selectedField == 3) {
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

void DeckChoiceNumberPopUp::goToNextControl(void) {
    if(this->_selectedField + 1 > DECKCHOICENUMBERPOPUP_CONTROLS_MAX_ID) {
        if(DECKCHOICENUMBERPOPUP_FIELD_CIRCULAR_SELECT) { 
            this->_selectedField = 0;
        } else {
            //DO nothing
        }
    } else {
        this->_selectedField = this->_selectedField + 1;
    }
}

void DeckChoiceNumberPopUp::goToPrevControl(void) {
    if(this->_selectedField - 1 < 0) {
        if(DECKCHOICENUMBERPOPUP_FIELD_CIRCULAR_SELECT) { 
            this->_selectedField = DECKCHOICENUMBERPOPUP_CONTROLS_MAX_ID;
        } else {
            //DO nothing
        }
    } else {
        this->_selectedField = this->_selectedField - 1;
    }
}

void DeckChoiceNumberPopUp::toggleEditField(void) {
    this->_editField = !this->_editField;
}

bool DeckChoiceNumberPopUp::getEditField(void) {
    return this->_editField;
}

bool DeckChoiceNumberPopUp::isCurrentControlButton(void) {
    return this->_selectedField == DECKCHOICENUMBERPOPUP_CONTROLS_MAX_ID;
}

uint8_t DeckChoiceNumberPopUp::getFinalValue(void) {
    return this->_values[0] * 100 + this->_values[1] * 10 + this->_values[2];
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
    this->renderButton();

    this->_oled.display();
}
