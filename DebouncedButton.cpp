#include "DebouncedButton.h"

// CONSTRUCTORS ------------------------------------------------------------

DebouncedButton::DebouncedButton(uint8_t pin){
  this->pin = pin;
  pinMode(pin, INPUT_PULLUP);
}

// CLASS MEMBER FUNCTIONS --------------------------------------------------

bool DebouncedButton::hasBeenPushed(void){
  bool result = false;
  this->thisButtonOkState = digitalRead(this->pin);
  if(this->thisButtonOkState != this->lastButtonOkState) {
    this->lastButtonOkDebounceTime = millis();
  }
  if((millis() - this->lastButtonOkDebounceTime) > BUTTON_OK_DEBOUNCE_DELAY) {
    if (this->thisButtonOkState != this->buttonOkState) {
      this->buttonOkState = this->thisButtonOkState;
      if (this->buttonOkState == HIGH) {
       result = true;
      }
    }
  }
  this->lastButtonOkState = this->thisButtonOkState;
  return result;
}
