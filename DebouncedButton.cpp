#include "DebouncedButton.h"

// CONSTRUCTORS ------------------------------------------------------------

DebouncedButton::DebouncedButton(uint8_t pin) : state(LOW), currentState(LOW), lastState(LOW), lastDebounceTime(0) {
  this->pin = pin;
  pinMode(pin, INPUT_PULLUP);
}

// CLASS MEMBER FUNCTIONS --------------------------------------------------

bool DebouncedButton::hasBeenPushed(void){
  bool result = false;
  this->currentState = digitalRead(this->pin);
  if(this->currentState != this->lastState) {
    this->lastDebounceTime = millis();
  }
  if((millis() - this->lastDebounceTime) > BUTTON_DEBOUNCE_DELAY) {
    if (this->currentState != this->state) {
      this->state = this->currentState;
      if (this->state == HIGH) {
       result = true;
      }
    }
  }
  this->lastState = this->currentState;
  return result;
}
