/*!
 * @file DebouncedButton.h
 *
 * Part TechnoLarp library. This class was written to  provide a generic 
 * debounce interface for simple push buttons.
 *
 * Written by Romain Féret for TechnoLarp, with contributions from the 
 * open source community.
 *
 * BSD license, all text here must be included in any redistribution.
 */
 
#ifndef _DEBOUNCEDBUTTON_H_
#define _DEBOUNCEDBUTTON_H_

#include <Arduino.h>

#define BUTTON_DEBOUNCE_DELAY 50

class DebouncedButton {
  public:
  
    // CONSTRUCTORS ------------------------------------------------------------
  
    DebouncedButton(uint8_t pin);
  
    // CLASS MEMBER FUNCTIONS ----------------------------------------------
    
    bool hasBeenPushed(void);
  
private:

    // PRIVATE PROPERTIES
    uint8_t pin;
   
    // TODO : check init https://stackoverflow.com/a/3127603 - Define Only in Body : https://stackoverflow.com/a/15335287
    
    uint8_t state;

    // the current and previous readings from the input pin
    uint8_t currentState;
    uint8_t lastState;

    unsigned long lastDebounceTime;

};


#endif // end _DEBOUNCEDBUTTON_H_
