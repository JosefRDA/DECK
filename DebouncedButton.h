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
#define BUTTON_LONG_PRESS_DELAY 3000

// Read return value
//TODO : Move to struct/enum
#define BUTTON_NO_EVENT     0
#define BUTTON_SHORT_PRESS  1
#define BUTTON_LONG_PRESS   2




class DebouncedButton {
  public:
  
    // CONSTRUCTORS ------------------------------------------------------------
  
    DebouncedButton(uint8_t pin);
  
    // CLASS MEMBER FUNCTIONS ----------------------------------------------
    
    uint8_t read(void);

  
private:

    // PRIVATE PROPERTIES ----------------------------------------------
    uint8_t pin;
   
    // TODO : check init https://stackoverflow.com/a/3127603 - Define Only in Body : https://stackoverflow.com/a/15335287
    
    uint8_t state;

    // the current and previous readings from the input pin
    uint8_t currentState;
    uint8_t lastState;

    unsigned long lastDebounceTime;
    unsigned long lastPushedTime; //For future use like double press
    unsigned long lastHighTime;

    // CLASS PRIVATE FUNCTIONS ----------------------------------------------
    void debug(void);

};


#endif // end _DEBOUNCEDBUTTON_H_
