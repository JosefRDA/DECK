#include "DebouncedButton.h"

// CONSTRUCTORS ------------------------------------------------------------

DebouncedButton::DebouncedButton(uint8_t pin, uint8_t pressedState) : state(LOW), currentState(LOW), lastState(LOW), lastDebounceTime(0) {
  this->pin = pin;
  pinMode(pin, INPUT_PULLUP);

  this->_pressedState = pressedState;
  
  this->lastPushedTime = millis();
  this->lastHighTime = 0;
}

// CLASS PRIVATE FUNCTIONS ----------------------------------------------
void DebouncedButton::debug(void){
  Serial.print("DebouncedButton::hasBeenPushed not has been pushied since ");
  Serial.print((millis() - this->lastPushedTime)/1000-(millis() - this->lastHighTime)/1000);
  Serial.print(" s and been pressed for ");
  Serial.print((millis() - this->lastHighTime)/1000);
  Serial.println(" s.");
}

// CLASS MEMBER FUNCTIONS --------------------------------------------------

uint8_t DebouncedButton::read(void){
  unsigned long currentTime = millis();
  
  uint8_t result = BUTTON_NO_EVENT;

  this->currentState = digitalRead(this->pin);
  
  if(this->currentState != this->lastState) {
    this->lastDebounceTime = currentTime;
  }
  if((currentTime - this->lastDebounceTime) > BUTTON_DEBOUNCE_DELAY) {
    if (this->currentState != this->state) {
      this->state = this->currentState;
      if (this->lastHighTime != 0 && this->state == this->_pressedState) {
        
        this->debug();
        
        if((currentTime - this->lastHighTime) < BUTTON_LONG_PRESS_DELAY) {
          result = BUTTON_SHORT_PRESS;
        } else {
          result = BUTTON_LONG_PRESS;
        }
       
        this->lastPushedTime = currentTime;
      } else {
        this->lastHighTime = currentTime;
      }
    }
  }
  this->lastState = this->currentState;
  return result;
}
