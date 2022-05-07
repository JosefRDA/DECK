#ifndef _DECKCONFIRMATIONPOPUP_
#define _DECKCONFIRMATIONPOPUP_


//OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//RENDER
#define DECKCONFIRMATIONPOPUP_RENDER_X_POS                        4
#define DECKCONFIRMATIONPOPUP_RENDER_Y_FIRST_POS                  2
#define DECKCONFIRMATIONPOPUP_RENDER_Y_POS_OFSET                  10
#define DECKCONFIRMATIONPOPUP_RENDER_QUESTION_LABEL_LINE_LENGTH   9
#define DECKCONFIRMATIONPOPUP_RENDER_BUTTON_LABELS_MAX_LENGTH     3
#define DECKCONFIRMATIONPOPUP_LABELS_OK_DEFAULT                   "Oui"
#define DECKCONFIRMATIONPOPUP_LABELS_KO_DEFAULT                   "Non"

class DeckConfirmationPopUp {
  public:
  
    // CONSTRUCTORS ------------------------------------------------------------
    
    DeckConfirmationPopUp(String questionLabel, String okLabel, String koLabel, Adafruit_SSD1306 oled);
    DeckConfirmationPopUp(String questionLabel, Adafruit_SSD1306 oled) : DeckConfirmationPopUp(questionLabel, DECKCONFIRMATIONPOPUP_LABELS_OK_DEFAULT, DECKCONFIRMATIONPOPUP_LABELS_KO_DEFAULT, oled){};
    
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

#endif // end _DECKCONFIRMATIONPOPUP_
