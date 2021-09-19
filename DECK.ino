/**************************************************************************/
/*! 
    This example will wait for any ISO14443A card or tag, and
    depending on the size of the UID will attempt to read from it.
   
    If the card has a 4-byte UID it is probably a Mifare
    Classic card, and the following steps are taken:
   
    - Authenticate block 4 (the first block of Sector 1) using
      the default KEYA of 0XFF 0XFF 0XFF 0XFF 0XFF 0XFF
    - If authentication succeeds, we can then read any of the
      4 blocks in that sector (though only block 4 is read here)
	 
    If the card has a 7-byte UID it is probably a Mifare
    Ultralight card, and the 4 byte pages can be read directly.
    Page 4 is read by default since this is the first 'general-
    purpose' page on the tags.

    To enable debug message, define DEBUG in PN532/PN532_debug.h
*/
/**************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);	

//OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 1  // GPIO0
 
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 0
#define DELTAY 2
 
 
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

#define SCREEN_WIDTH 64 // OLED display width, in pixels
#define SCREEN_HEIGHT 48 // OLED display height, in pixels
Adafruit_SSD1306 display_oled(OLED_RESET);

// CONFIG
#include "DeckDatabase.h"
DeckDatabase deckDatabase;

#define PIN_BUTTON_OK D6
int buttonOkState = LOW;

// the current and previous readings from the input pin
int thisButtonOkState = LOW;
int lastButtonOkState = LOW;

unsigned long lastButtonOkDebounceTime = 0;
#define BUTTON_OK_DEBOUNCE_DELAY 50

void setup(void) {
  setupInputs();
  setupOled();
  
  Serial.begin(115200);
  Serial.println("Hello!");

  Serial.println(F(""));
  Serial.println(F(""));
  deckDatabase.mountFS();
  deckDatabase.listDir("/");
  deckDatabase.printJsonFile("/stim.json");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();

  while(! versiondata){
    display_oled.clearDisplay();
    Serial.println("Didn't find PN53x board");
    display_oled.println("Didn't find PN53x board");
    display_oled.display();
    delay(1000);
    Serial.println("Retrying ...");
    display_oled.println("Retrying ...");
    display_oled.display();
    nfc.begin();
  
    uint32_t versiondata = nfc.getFirmwareVersion();
  }

  //RFID INFO FOR DEBUG
  //printRfidReaderInfo(versiondata)

  // configure board to read RFID tags
  nfc.SAMConfig();
}

void setupInputs(void) {
  pinMode(PIN_BUTTON_OK, INPUT_PULLUP);
}

void setupOled(void) {
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display_oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)

  display_oled.clearDisplay();

  display_oled.setTextColor(WHITE);
  display_oled.setTextSize(1);
  display_oled.setCursor(0,0);
  display_oled.display();
}

void printRfidReaderInfo(uint32_t versiondata) {
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
}

void loop(void) {
  loopInput();
}

void loopInput(void) {
  thisButtonOkState = digitalRead(PIN_BUTTON_OK);
   if (thisButtonOkState != lastButtonOkState) {
    lastButtonOkDebounceTime = millis();
  }
  if ((millis() - lastButtonOkDebounceTime) > BUTTON_OK_DEBOUNCE_DELAY) {
    if (thisButtonOkState != buttonOkState) {
      buttonOkState = thisButtonOkState;
      if (buttonOkState == HIGH) {
        Serial.println("START SCAN");
        pn532ReadRfidLoop();
        Serial.println("END SCAN");
      }
    }
  }
  lastButtonOkState = thisButtonOkState;
}

String rfidUidBufferToString(uint8_t uid[]) {
  char uidCharArray[8] = {0};
  sprintf(uidCharArray,"%02X%02X%02X%02X",uid[0],uid[1],uid[2],uid[3]);
  String result = uidCharArray;
  return result;
}

void pn532ReadRfidLoop(void) {
  
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
  if (success) {
    // Display some basic information about the card

    Serial.print("Label from JSON: ");
    String stimLabel = deckDatabase.getLabelByUid("/stim.json", rfidUidBufferToString(uid));
    Serial.println(stimLabel);

    display_oled.clearDisplay();
    display_oled.setCursor(0,0);
    display_oled.println(stimLabel);
    display_oled.display();
    Serial.println("");
    
  }
}

//deprecated
String rfidGetLabelToDisplayFromKey(String key) {
  String result;

  if(key.equals("C09FC249")) {
    result = "Carte Blanche";
  } else if(key.equals("CE74D83F")) {
    result = "Tag Bleu";
  } else {
    result = "Inconnu";
  }

  return result;
}
