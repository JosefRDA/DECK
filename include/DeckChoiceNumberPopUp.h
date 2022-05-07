#ifndef _DECKCHOICENUMBERPOPUP_
#define _DECKCHOICENUMBERPOPUP_


//OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//RENDER
#define DECKCHOICENUMBERPOPUP_RENDER_X_POS                      5
#define DECKCHOICENUMBERPOPUP_RENDER_X_HEADER_POS               15
#define DECKCHOICENUMBERPOPUP_RENDER_Y_HEADER_POS               2
#define DECKCHOICENUMBERPOPUP_RENDER_Y_FIRST_POS                13
#define DECKCHOICENUMBERPOPUP_RENDER_Y_POS_OFSET                11
#define DECKCHOICENUMBERPOPUP_RENDER_X_POS_OFSET                7
#define DECKCHOICENUMBERPOPUP_RENDER_QUESTION_LABEL_LINE_LENGTH 9
#define DECKCHOICENUMBERPOPUP_UP_ARROW_CHAR                     "A"
#define DECKCHOICENUMBERPOPUP_DOWN_ARROW_CHAR                   "V"

#define DECKCHOICENUMBERPOPUP_FIELD_NUMBER                      3
#define DECKCHOICENUMBERPOPUP_FIELD_MIN_VALUE                   0
#define DECKCHOICENUMBERPOPUP_FIELD_MAX_VALUE                   9
#define DECKCHOICENUMBERPOPUP_FIELD_CIRCULAR_SELECT             true

#define DECKCHOICENUMBERPOPUP_BUTTON_NUMBER                     1
#define DECKCHOICENUMBERPOPUP_CONTROLS_NUMBER                   DECKCHOICENUMBERPOPUP_FIELD_NUMBER + DECKCHOICENUMBERPOPUP_BUTTON_NUMBER
#define DECKCHOICENUMBERPOPUP_CONTROLS_MAX_ID                   DECKCHOICENUMBERPOPUP_CONTROLS_NUMBER - 1


class DeckChoiceNumberPopUp {
  public:
  
    // CONSTRUCTORS ------------------------------------------------------------
    
    DeckChoiceNumberPopUp(Adafruit_SSD1306 oled);
    
    // CLASS MEMBER FUNCTIONS ----------------------------------------------

    void increaseSelectedField(void);
    void decreaseSelectedField(void);
    void goToNextControl(void);
    void goToPrevControl(void);
    void toggleEditField(void);

    bool getEditField(void);
    bool isCurrentControlButton(void);
    uint8_t getFinalValue(void);

    void render(void);
    
  private:
    void renderField(int id);
    void renderButton(void);
    uint8_t _values[DECKCHOICENUMBERPOPUP_FIELD_NUMBER];
    uint8_t _selectedField;
    bool _editField;


    Adafruit_SSD1306 _oled;
};

#endif // end _DECKCHOICENUMBERPOPUP_
