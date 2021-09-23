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

#define BUTTON_OK_DEBOUNCE_DELAY 50

class DebouncedButton {
  public:
  
    // CONSTRUCTORS ------------------------------------------------------------
  
    DebouncedButton(uint8_t pin);
  
    // CLASS MEMBER FUNCTIONS ----------------------------------------------
    
    bool hasBeenPushed(void);
  
private:

    // PRIVATE PROPERTIES
    // TODO : check init https://stackoverflow.com/a/3127603
    
    int buttonOkState = LOW;

    // the current and previous readings from the input pin
    int thisButtonOkState = LOW;
    int lastButtonOkState = LOW;

    unsigned long lastButtonOkDebounceTime = 0;

};


#endif // end _DEBOUNCEDBUTTON_H_
