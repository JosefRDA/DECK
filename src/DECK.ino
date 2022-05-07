
#include <SPI.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include "ESP8266WiFi.h"
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
#define PIN_BUTTON_UP D7
#define PIN_BUTTON_DOWN D8
//D8 VCC other GND  

#include "DebouncedButton.h"
DebouncedButton okButton(PIN_BUTTON_OK, HIGH);
DebouncedButton upButton(PIN_BUTTON_UP, HIGH);
DebouncedButton downButton(PIN_BUTTON_DOWN, LOW);

#include "DeckMenu.h"
DeckMenu* mainMenu;

#include "DeckDtodServer.h"
DeckDtodServer* dtodServer;

#include "DeckPaginableText.h"
DeckPaginableText* paginableText;

unsigned long lastDisplayOledTime = 0;
#define OLED_CLS_DELAY 10000

#define PIN_VIBRATION_MOTOR D5
unsigned long lastVibrationMotorStartTime = 0;
#define VIBRATION_MOTOR_DELAY 2000

#define SSID_PREFIX "Bbox"
//#define SSID_PREFIX "TOTO"

#include "ClusterLogo.h"
#include "DeckScanResult.h"

#include <StateMachine.h>
const int STATE_DELAY = 1000;

StateMachine machine = StateMachine();
State* StateMainMenu = machine.addState(&loopStateMainMenu);
State* StateScan = machine.addState(&loopStateScan);

bool scanHasBeenPressed = false;

void setup(void) {
  setupVibrationMotor();
  setupOled();

  display_oled.drawBitmap(0, 0, clusterLogo_data, clusterLogo_width, clusterLogo_height, 1);
  display_oled.display();

  delay(2000);


  display_oled.clearDisplay();
  display_oled.display();
  
  Serial.begin(115200);
  Serial.println("Hello!");

  Serial.println(F(""));
  Serial.println(F(""));
  deckDatabase.mountFS();
  //deckDatabase.listDir("/");
  //deckDatabase.printJsonFile("/stim.json");

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

  setUpMainMenu();
  mainMenu->render();

  dtodServer = NULL;
  paginableText = NULL;

  
  StateMainMenu->addTransition(&transitionStateMainMenuToStateScan,StateScan);
  StateScan->addTransition(&transitionStateScanToStateMainMenu,StateMainMenu);
}

void setUpMainMenu(void) {
  DeckMenuItem mainMenuItems[] = { 
        { .label = "SCAN", .value = "SCAN", .selected = true, .shortPressAction = &mainMenuActionScan },
        { .label = "STIM", .value = "STIM", .selected = false, .shortPressAction = &mainMenuActionStim },
        { .label = "DTOD", .value = "DTOD", .selected = false, .shortPressAction = &mainMenuActionDtod }
      };
    
  mainMenu = new DeckMenu(mainMenuItems, 3, display_oled);
}

void setupVibrationMotor(void) {
  pinMode(PIN_VIBRATION_MOTOR, OUTPUT);
  digitalWrite(PIN_VIBRATION_MOTOR, LOW);
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
  loopOkButton();
  loopUpButton();
  loopDownButton();
  loopCleanOled();
  loopVibrationMotor();
  loopDtodServer();
  machine.run();
}

void loopDtodServer() {
  if(dtodServer != NULL) {
    dtodServer->handleClient();
  }
}

void loopOkButton(void){
  uint8_t buttonValue = okButton.read();
  if(buttonValue != BUTTON_NO_EVENT) {
    DeckMenuItem selectedMenuItem = mainMenu->getSelected();
    
    switch(buttonValue) {
      case BUTTON_SHORT_PRESS:
        if(*selectedMenuItem.shortPressAction != NULL) {
          selectedMenuItem.shortPressAction();
        }
        break;
      case BUTTON_LONG_PRESS:
        if(*selectedMenuItem.longPressAction != NULL) {
          selectedMenuItem.longPressAction();
        }
        break;
    }
  }
}

void loopUpButton(void){
  uint8_t buttonValue = upButton.read();
  if(buttonValue == BUTTON_SHORT_PRESS) {
    //Serial.println("SHORT UP BUTTON");
    mainMenu->select(DECKMENU_DIRECTION_UP);
    mainMenu->render();
  } else { 
    if(buttonValue == BUTTON_LONG_PRESS) {
      //Serial.println("LONG UP BUTTON : WIFI");
    } 
  }
}

void loopDownButton(void){
  uint8_t buttonValue = downButton.read();
  if(buttonValue == BUTTON_SHORT_PRESS) {
    //Serial.println("SHORT DOWN BUTTON");
    //TODO : IF IN MAIN MENU 
    mainMenu->select(DECKMENU_DIRECTION_DOWN);
    mainMenu->render();
  } else { 
    if(buttonValue == BUTTON_LONG_PRESS) {
      //Serial.println("LONG DOWN BUTTON");
    } 
  }
}

// ACTIONS ----------------------------------------------

void mainMenuActionStim(void) {
  dtodServer = new DeckDtodServer(display_oled);
}

void mainMenuActionScan(void) {
  Serial.println("START SCAN");
  scanHasBeenPressed = true;
  Serial.println("END SCAN");
}

void mainMenuActionDtod(void) {
  display_oled.clearDisplay();
  display_oled.setCursor(0,0);
  display_oled.println("Scan des deck à portée en cours .");
  display_oled.display();
  lastDisplayOledTime = millis();

  
  //INIT
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  
  display_oled.clearDisplay();
  display_oled.setCursor(0,0);
  display_oled.println("Scan des deck à portée en cours ...");
  display_oled.display();
  lastDisplayOledTime = millis();

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  if (n == 0) {
    display_oled.clearDisplay();
    display_oled.setCursor(0,0);
    display_oled.println("Aucun DECK a scanner");
    display_oled.display();
    lastDisplayOledTime = millis();
  } else {
    int maxForce = -999;
    String closestDeckName = ""; 
    for (int i = 0; i < n; ++i) {

      //TODO ajouter ici le filtre par seuil minimum de force
      if(WiFi.SSID(i).substring(0, 4) == SSID_PREFIX) {
        Serial.println("DECK FOUND :");
        Serial.print("NAME : ");
        Serial.println(WiFi.SSID(i).substring(4));
        Serial.print("FORCE : ");
        Serial.println(WiFi.RSSI(i));

        int force = WiFi.RSSI(i);
        if(force > maxForce) {
          maxForce = force;
          closestDeckName = WiFi.SSID(i).substring(4);
        }
      }
    }
    display_oled.clearDisplay();
    display_oled.setCursor(0,0);
    
    String dtodLabel = "Aucun DECK a scanner";
    if(maxForce >  -999) {
      dtodLabel = deckDatabase.getLabelByUid("/dtod.json", closestDeckName).label;
    }
    display_oled.println(dtodLabel);
    
    display_oled.display();
    lastDisplayOledTime = millis();
    if(maxForce >  -999) {
      //Vibration motor
      digitalWrite(PIN_VIBRATION_MOTOR, HIGH);
      lastVibrationMotorStartTime = millis();
    }
  }
  WiFi.disconnect();
}

void loopCleanOled(void) {
  if(lastDisplayOledTime != 0 && millis() > lastDisplayOledTime + OLED_CLS_DELAY) {
    display_oled.clearDisplay();
    display_oled.display();
    lastDisplayOledTime = 0;
  }
}

void loopVibrationMotor(void) {
  if(lastVibrationMotorStartTime != 0 && millis() > lastVibrationMotorStartTime + VIBRATION_MOTOR_DELAY) {
    digitalWrite(PIN_VIBRATION_MOTOR, LOW);
    lastVibrationMotorStartTime = 0;
  }
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
    DeckScanResult scanResult = deckDatabase.getLabelByUid("/stim.json", rfidUidBufferToString(uid));
    if(deckDatabase.getFieldValueByUid("/stim.json", rfidUidBufferToString(uid), "value") == "true") {
      Serial.println("[USABLE]");
    }
    Serial.println(scanResult.label);
    Serial.println("");

    paginableText = new DeckPaginableText(scanResult.label, display_oled);
    paginableText->render();
    lastDisplayOledTime = millis();

    

    //

    //Vibration motor
    digitalWrite(PIN_VIBRATION_MOTOR, HIGH);
    lastVibrationMotorStartTime = millis();
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

void loopStateMainMenu(){
}

//-------------------------
void loopStateScan(){
  if(machine.executeOnce) {
    pn532ReadRfidLoop();
  }
}

bool transitionStateMainMenuToStateScan(){
  if(scanHasBeenPressed) {
    scanHasBeenPressed = false;
    return true;
  } 
  return false;
}

bool transitionStateScanToStateMainMenu(){
  return false;
}
