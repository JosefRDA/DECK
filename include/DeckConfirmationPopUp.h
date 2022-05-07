#ifndef _DECKCONFIRMATIONPOPUP_
#define _DECKCONFIRMATIONPOPUP_


//OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//RENDER
#define DECKCONFIRMATIONPOPUP_RENDER_X_POS         15
#define DECKCONFIRMATIONPOPUP_RENDER_Y_HEADER_POS  2
#define DECKCONFIRMATIONPOPUP_RENDER_Y_FIRST_POS   13
#define DECKCONFIRMATIONPOPUP_RENDER_Y_POS_OFSET   11
#define DECKCONFIRMATIONPOPUP_RENDER_QUESTION_LABEL_LINE_LENGTH  9

class DeckConfirmationPopUp {
  public:
  
    // CONSTRUCTORS ------------------------------------------------------------
    
    DeckConfirmationPopUp(String questionLabel, String okLabel, String koLabel, Adafruit_SSD1306 oled);
    DeckConfirmationPopUp(String questionLabel, Adafruit_SSD1306 oled) : DeckConfirmationPopUp(questionLabel, "Oui", "Non", oled){};
    
    // CLASS MEMBER FUNCTIONS ----------------------------------------------

    void toggleSelection();
    bool isOkSelected();
    void render(void);
    
  private:
    String _questionLabel;
    String _okLabel;
    String _koLabel;

    bool _okSelected;

    Adafruit_SSD1306 _oled;
};

#endif // end _DECKMENU_H_
