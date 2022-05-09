//TODO : Refactor debug via services
#define DECKINO_DEBUG_SERIAL  true
#define DECKINO_DEBUG_OLED    true

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

//Usage ?
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

#include "DeckConfirmationPopUp.h"
DeckConfirmationPopUp* confirmationPopUp;

#include "DeckChoiceNumberPopUp.h"
DeckChoiceNumberPopUp* choiceNumberPopUp;

#include "DeckDtodServer.h"
DeckDtodServer* dtodServer;

#include "DeckPaginableText.h"
DeckPaginableText* paginableText;

//Todo : Moove vibration motor to StateMachine
#define PIN_VIBRATION_MOTOR D5
unsigned long lastVibrationMotorStartTime = 0;
#define VIBRATION_MOTOR_DELAY 2000

#define SSID_PREFIX "Bbox"
//#define SSID_PREFIX "TOTO"

#include "ClusterLogo.h"
#include "DeckScanResult.h"
#include "DeckMthrClient.h"

#include <StateMachine.h>

//BEGIN REGION navigationMachine
//TODO/REFACTOR : Move this StateMachine to it's own class
StateMachine navigationMachine = StateMachine();
State* StateMainMenu = navigationMachine.addState(&loopStateMainMenu);

State* StateScan = navigationMachine.addState(&loopStateScan);
State* StateConfirmBeforeUseScan = navigationMachine.addState(&loopStateConfirmBeforeUseScan);
State* StateUseScan = navigationMachine.addState(&loopStateUseScan);

State* StateConfirmBeforeEnterCharacterNumber = navigationMachine.addState(&loopStateConfirmBeforeEnterCharacterNumber);
State* StateEnterCharacterNumber = navigationMachine.addState(&loopStateEnterCharacterNumber);
State* StateTryToUpdateStim = navigationMachine.addState(&loopStateTryToUpdateStim);

bool scanHasBeenPressed = false;
bool confirmBeforeEnterCharacterNumberHasBeenPressed = false;
bool enterCharacterNumberHasBeenPressed = false;
bool enterUseScanHasBeenPressed = false;
bool returnToMainMenuHasBeenPressed = false;
bool confirmHasBeenPressed = false;
bool isScanUsable = false;
unsigned long lastNavigationStateChange = 0L;

//debug
int debugLastNavigationMachineState = -1;

//END REGION navigationMachine

//BEGIN REGION oledMachine
//TODO/REFACTOR : Move this StateMachine to it's own class
#define OLED_SMALL_DELAY  10L
#define OLED_MEDIUM_DELAY 30L
#define OLED_LONG_DELAY   60L

StateMachine oledMachine = StateMachine();
State* OledStateOff = oledMachine.addState(&loopOledStateOff);
State* OledStateOnForLongDelay = oledMachine.addState(&loopOledStateOnForLongDelay);
State* OledStateOnForMediumDelay = oledMachine.addState(&loopOledStateOnForMediumDelay);
State* OledStateOnForSmallDelay = oledMachine.addState(&loopOledStateOnForSmallDelay);
State* OledStateAlwaysOn = oledMachine.addState(&loopOledStateAlwaysOn);

bool oledRequestLong = false;
bool oledRequestMedium = false;
bool oledRequestSmall = false;
bool oledRequestAlways = false;
bool oledRequestOff = false;
unsigned long lastOledStateChange = 0L;
//END REGION oledMachine

String rfidUidBufferToStringLastValue = "";

void setup(void) {
  setupVibrationMotor();
  setupOled();

  display_oled.begin(SSD1306_SWITCHCAPVCC);
  display_oled.drawBitmap(0, 0, clusterLogo_data, clusterLogo_width, clusterLogo_height, 1);
  display_oled.display();

  delay(2000);


  display_oled.clearDisplay();
  display_oled.display();
  
  Serial.begin(115200);

  #if DECKINO_DEBUG_SERIAL
  Serial.println("Hello!");
  #endif

  deckDatabase.mountFS();

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();

  while(! versiondata){
    #if DECKINO_DEBUG_SERIAL
    Serial.println("Didn't find PN53x board");
    #endif
    #if DECKINO_DEBUG_OLED
    display_oled.clearDisplay();
    display_oled.println("Didn't find PN53x board");
    display_oled.display();
    #endif
    delay(1000);
    
    #if DECKINO_DEBUG_SERIAL
    Serial.println("Retrying ...");
    #endif
    #if DECKINO_DEBUG_OLED
    display_oled.println("Retrying ...");
    display_oled.display();
    #endif
    nfc.begin();
  
    uint32_t versiondata = nfc.getFirmwareVersion();
  }

  //RFID INFO FOR DEBUG
  //printRfidReaderInfo(versiondata)

  // configure board to read RFID tags
  nfc.SAMConfig();

  dtodServer = NULL;
  paginableText = NULL;

  //navigationMachine transitions begin
  StateMainMenu->addTransition(&transitionStateMainMenuToStateScan, StateScan);
  StateMainMenu->addTransition(&transitionStateMainMenuToConfirmBeforeEnterCharacterNumber, StateConfirmBeforeEnterCharacterNumber);
  
  StateScan->addTransition(&transitionStateScanToStateMainMenu, StateMainMenu);//TODO : To modify
  StateScan->addTransition(&transitionStateScanToConfirmBeforeUseScan, StateConfirmBeforeUseScan);
  StateConfirmBeforeUseScan->addTransition(&transitionStateConfirmBeforeUseScanToStateMainMenu, StateMainMenu);
  StateConfirmBeforeUseScan->addTransition(&transitionStateConfirmBeforeUseScanToUseScan, StateUseScan);
  StateUseScan->addTransition(&transitionStateUseScanToStateMainMenu, StateMainMenu);
  
  StateConfirmBeforeEnterCharacterNumber->addTransition(&transitionStateConfirmBeforeEnterCharacterNumberToEnterCharacterNumber, StateEnterCharacterNumber);
  StateConfirmBeforeEnterCharacterNumber->addTransition(&transitionStateConfirmBeforeEnterCharacterNumberToStateMainMenu, StateMainMenu); 
  StateEnterCharacterNumber->addTransition(&transitionStateEnterCharacterNumberToTryToUpdateStim, StateTryToUpdateStim);
  StateTryToUpdateStim->addTransition(&transitionStateTryToUpdateStimToMainMenu, StateMainMenu);
  //navigationMachine transitions end

  //oledMachine transitions begin
  OledStateOff->addTransition(&transitionOledStateOffToOledStateOnForLongDelay, OledStateOnForLongDelay);
  OledStateOnForLongDelay->addTransition(&transitionOledStateOnForLongDelayToOledStateOff, OledStateOff);

  OledStateOff->addTransition(&transitionOledStateOffToOledStateOnForMediumDelay, OledStateOnForMediumDelay);
  OledStateOnForMediumDelay->addTransition(&transitionOledStateOnForMediumDelayToOledStateOff, OledStateOff);

  OledStateOff->addTransition(&transitionOledStateOffToOledStateOnForSmallDelay, OledStateOnForSmallDelay);
  OledStateOnForSmallDelay->addTransition(&transitionOledStateOnForSmallDelayToOledStateOff, OledStateOff);

  OledStateOff->addTransition(&transitionOledStateOffToOledStateAlwaysOn, OledStateAlwaysOn);
  OledStateAlwaysOn->addTransition(&transitionOledStateAlwaysOnToOledStateOff, OledStateOff);
  //oledMachine transitions end

  deckDatabase.printJsonFile("/used_stim_log.json");
  
  oledRequestSmall = true;
}

void setUpMainMenu(void) {
  DeckMenuItem mainMenuItems[] = { 
        { .label = "SCAN", .value = "SCAN", .selected = true, .shortPressAction = &mainMenuActionScan, .longPressAction = &mainMenuActionEnterCharacterNumber },
        { .label = "STIM", .value = "STIM", .selected = false, .shortPressAction = &mainMenuActionStim, .longPressAction = &mainMenuActionTestPaginableText},
        { .label = "DTOD", .value = "DTOD", .selected = false, .shortPressAction = &mainMenuActionDtod }
      };
    
  mainMenu = new DeckMenu(mainMenuItems, 3, display_oled);
}

void mainMenuActionTestPaginableText(void) {
  paginableText = new DeckPaginableText("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris in facilisis ante. Vestibulum est massa, laoreet at metus a, aliquam rhoncus libero. Sed rhoncus nulla ornare nisl imperdiet dignissim. Etiam sit amet tristique risus, a cursus orci. Nam non ex ac est mattis pharetra at eu orci. Etiam non quam mattis, aliquet turpis et, fermentum orci.", display_oled);
  paginableText->render();
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

//Obselete : Keept for debug purpose only
//TODO : Refactor : Add in a RFID util class 
void printRfidReaderInfo(uint32_t versiondata) {
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
}

void loop(void) {
  loopVibrationMotor();
  loopDtodServer();
  navigationMachine.run();
  if(navigationMachine.currentState != NULL) {
    if(debugLastNavigationMachineState != navigationMachine.currentState) {
      #if DECKINO_DEBUG_SERIAL
      Serial.println("[STATE CHANGE] OLD:" + String(debugLastNavigationMachineState) + " NEW:" + String(navigationMachine.currentState));
      #endif
      debugLastNavigationMachineState = navigationMachine.currentState;
    }
  }
  oledMachine.run();
}

void loopDtodServer() {
  if(dtodServer != NULL) {
    dtodServer->handleClient();
  }
}

void loopMainMenuOkButton(void){
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

void loopMainMenuUpButton(void){
  uint8_t buttonValue = upButton.read();
  if(buttonValue == BUTTON_SHORT_PRESS) {
    mainMenu->select(DECKMENU_DIRECTION_UP);
    mainMenu->render();
    oledRequestSmall = true;
  } else { 
    if(buttonValue == BUTTON_LONG_PRESS) {
    } 
  }
}

void loopMainMenuDownButton(void){
  uint8_t buttonValue = downButton.read();
  if(buttonValue == BUTTON_SHORT_PRESS) {
    //TODO : IF IN MAIN MENU // Still relevent ?
    mainMenu->select(DECKMENU_DIRECTION_DOWN);
    mainMenu->render();
    oledRequestSmall = true;
  } else { 
    if(buttonValue == BUTTON_LONG_PRESS) {
    } 
  }
}

void loopScanOkButton(void){
  uint8_t buttonValue = okButton.read();
  if(buttonValue != BUTTON_NO_EVENT) {    
    switch(buttonValue) {
      case BUTTON_SHORT_PRESS:
        returnToMainMenuHasBeenPressed = true;
        break;
      case BUTTON_LONG_PRESS:

        break;
    }
  }
}

void loopScanUpButton(void){
  uint8_t buttonValue = upButton.read();
  if(buttonValue == BUTTON_SHORT_PRESS) {
    paginableText->prev();
    paginableText->render();
    oledRequestSmall = true;
  } else { 
    if(buttonValue == BUTTON_LONG_PRESS) {
    } 
  }
}

void loopScanDownButton(void){
  uint8_t buttonValue = downButton.read();
  if(buttonValue == BUTTON_SHORT_PRESS) {
    paginableText->next();
    paginableText->render();
    oledRequestSmall = true;
  } else { 
    if(buttonValue == BUTTON_LONG_PRESS) {
    } 
  }
}

void loopConfirmBeforeUseScanOkButton(void){
  uint8_t buttonValue = okButton.read();
  if(buttonValue != BUTTON_NO_EVENT) {
    if(confirmationPopUp->isOkSelected()) {
      enterUseScanHasBeenPressed = true;
    } else {
      returnToMainMenuHasBeenPressed = true;
    }
  }
}

void loopConfirmBeforeEnterCharacterNumberOkButton(void){
  uint8_t buttonValue = okButton.read();
  if(buttonValue != BUTTON_NO_EVENT) {
    if(confirmationPopUp->isOkSelected()) {
      enterCharacterNumberHasBeenPressed = true;
    } else {
      returnToMainMenuHasBeenPressed = true;
    }
  }
}

void loopConfirmUpButton(void){
  uint8_t buttonValue = upButton.read();
  if(buttonValue == BUTTON_SHORT_PRESS) {
    confirmationPopUp->toggleSelection();
    confirmationPopUp->render();
  } else { 
    if(buttonValue == BUTTON_LONG_PRESS) {
    } 
  }
}

void loopConfirmDownButton(void){
  uint8_t buttonValue = downButton.read();
  if(buttonValue == BUTTON_SHORT_PRESS) {
    confirmationPopUp->toggleSelection();
    confirmationPopUp->render();
  } else { 
    if(buttonValue == BUTTON_LONG_PRESS) {
    } 
  }
}

void loopEnterCharacterNumberOkButton(void){
  uint8_t buttonValue = okButton.read();
  if(buttonValue != BUTTON_NO_EVENT) {
    if(choiceNumberPopUp->isCurrentControlButton()) {
      
      confirmHasBeenPressed = true;
    } else {
      choiceNumberPopUp->toggleEditField();
      choiceNumberPopUp->render();
    }
  }
}

String utilZeroPadPlayerId(String inputPlayerId) {
  char playerIdBuffer[3];
  sprintf(playerIdBuffer, "%03d", inputPlayerId.toInt());
  return String(playerIdBuffer);
}

void loopEnterCharacterNumberUpButton(void){
  uint8_t buttonValue = upButton.read();
  if(buttonValue == BUTTON_SHORT_PRESS) {
    if(choiceNumberPopUp->getEditField()) {
      choiceNumberPopUp->increaseSelectedField();
    } else {
      choiceNumberPopUp->goToNextControl();
    }
    choiceNumberPopUp->render();
  } else { 
    if(buttonValue == BUTTON_LONG_PRESS) {

    } 
  }
}

void loopEnterCharacterNumberDownButton(void){
  uint8_t buttonValue = downButton.read();
  if(buttonValue == BUTTON_SHORT_PRESS) {
    if(choiceNumberPopUp->getEditField()) {
      choiceNumberPopUp->decreaseSelectedField();
    } else {
      choiceNumberPopUp->goToPrevControl();
    }
    choiceNumberPopUp->render();
  } else { 
    if(buttonValue == BUTTON_LONG_PRESS) {

    } 
  }
}

void loopTryToUpdateStimOkButton(void) {
  uint8_t buttonValue = okButton.read();
  if(buttonValue != BUTTON_NO_EVENT) {
    returnToMainMenuHasBeenPressed = true;
  }
}

// ACTIONS ----------------------------------------------

void mainMenuActionStim(void) {
  dtodServer = new DeckDtodServer(display_oled);
}

void mainMenuActionEnterCharacterNumber(void) {
  confirmBeforeEnterCharacterNumberHasBeenPressed = true;
}

void mainMenuActionScan(void) {
  scanHasBeenPressed = true;
}

void mainMenuActionDtod(void) {
  display_oled.clearDisplay();
  display_oled.setCursor(0,0);
  display_oled.println("Scan des deck à portée en cours .");
  display_oled.display();
  oledRequestSmall = true;

  
  //INIT
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  
  display_oled.clearDisplay();
  display_oled.setCursor(0,0);
  display_oled.println("Scan des deck à portée en cours ...");
  display_oled.display();
  oledRequestSmall = true;

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  if (n == 0) {
    display_oled.clearDisplay();
    display_oled.setCursor(0,0);
    display_oled.println("Aucun DECK a scanner");
    display_oled.display();
    oledRequestSmall = true;
  } else {
    int maxForce = -999;
    String closestDeckName = ""; 
    for (int i = 0; i < n; ++i) {

      //TODO ajouter ici le filtre par seuil minimum de force
      if(WiFi.SSID(i).substring(0, 4) == SSID_PREFIX) {
  
        #if DECKINO_DEBUG_SERIAL
        Serial.println("DECK FOUND :");
        Serial.print("NAME : ");
        Serial.println(WiFi.SSID(i).substring(4));
        Serial.print("FORCE : ");
        Serial.println(WiFi.RSSI(i));
        #endif

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
    oledRequestSmall = true;
    if(maxForce >  -999) {
      //Vibration motor
      digitalWrite(PIN_VIBRATION_MOTOR, HIGH);
      lastVibrationMotorStartTime = millis();
    }
  }
  WiFi.disconnect();
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
    
    #if DECKINO_DEBUG_SERIAL
    Serial.print("Label from JSON: ");
    #endif

    DeckScanResult scanResult = deckDatabase.getLabelByUid("/stim.json",rfidUidBufferToString(uid));
    if(deckDatabase.getFieldValueByUid("/stim.json", rfidUidBufferToString(uid), "usable") == "true") {
      isScanUsable = true;
    }else{
      isScanUsable = false;
    }

    #if DECKINO_DEBUG_SERIAL
    Serial.println(scanResult.label);
    Serial.println("");
    #endif
    
    //Vibration motor
    digitalWrite(PIN_VIBRATION_MOTOR, HIGH);
    lastVibrationMotorStartTime = millis();

    paginableText = new DeckPaginableText(scanResult.label, display_oled);
    paginableText->render();
    oledRequestSmall = true;

    
    rfidUidBufferToStringLastValue =  rfidUidBufferToString(uid);

  }
}

void useScanAction(void){
  //Vibration motor
  digitalWrite(PIN_VIBRATION_MOTOR, HIGH);
  lastVibrationMotorStartTime = millis();

  deckDatabase.appendUsedStimLog("/used_stim_log.json", deckDatabase.getFieldValueByUid("/stim.json", rfidUidBufferToStringLastValue, "stim_code"));

  rfidUidBufferToStringLastValue = "";

  paginableText = new DeckPaginableText("Objet utilisé !", display_oled);
  paginableText->render();
  oledRequestSmall = true;
}

void confirmBeforeEnterCharacterNumberAction(void) {
  confirmationPopUp = new DeckConfirmationPopUp("Definir id personnage ?", display_oled);
  confirmationPopUp->render();
  bool oledRequestAlways = true; //necessary ?
}

void confirmBeforeUseScanAction(void) {
  confirmationPopUp = new DeckConfirmationPopUp("Utiliser l'objet ?", display_oled);
  confirmationPopUp->render();
  bool oledRequestAlways = true;
}

void enterCharacterNumberAction(void) {
  choiceNumberPopUp = new DeckChoiceNumberPopUp(display_oled, deckDatabase.getFirstLevelDataByKey("/config.json", "player_id").toInt());
  choiceNumberPopUp->render();
}

void tryToUpdateStimOkButtonAction(void) {
  deckDatabase.persistFirstLevelDataByKeyValue("/config.json", "player_id", String(choiceNumberPopUp->getFinalValue()));

  String paddedPlayerId = utilZeroPadPlayerId(deckDatabase.getFirstLevelDataByKey("/config.json", "player_id"));

  paginableText = new DeckPaginableText("DOWN ID" + paddedPlayerId + "...", display_oled);
  paginableText->render();

  DeckMthrClient* mthrClient = new DeckMthrClient(deckDatabase.getFirstLevelDataByKey("/wifi.json", "mthr_ssid"), deckDatabase.getFirstLevelDataByKey("/wifi.json", "mthr_password"), "http://192.168.0.8:8080");
  RessourceResponse motherResponse = mthrClient->DownloadRessource("/HTTP/JSON/" + paddedPlayerId +  "/STIM.JSON");

  String userDisplayMessage = "";
  if(motherResponse.httpCode == 404) {
    userDisplayMessage = "PERSONNAGE NOT FOUND";
  } else if(motherResponse.httpCode != 200) {
    userDisplayMessage = "NETWORK ERROR : " + String(motherResponse.httpCode);
  } else {
    deckDatabase.persistFullFile("/stim.json", motherResponse.payload);
    userDisplayMessage = "DATA UPDATED";
  }
  paginableText = new DeckPaginableText("DOWN ID" + paddedPlayerId + "...\n" + userDisplayMessage, display_oled);
  paginableText->render();
}

//-------------------------

void loopStateMainMenu(){
  if(navigationMachine.executeOnce) {
    lastNavigationStateChange = millis();
    setUpMainMenu();
    mainMenu->render();
    oledRequestMedium = true;
  }
  loopMainMenuOkButton();
  loopMainMenuUpButton();
  loopMainMenuDownButton();
}

void loopStateScan(){
  if(navigationMachine.executeOnce) {
    lastNavigationStateChange = millis();
    pn532ReadRfidLoop();
  }
  loopScanOkButton();
  loopScanUpButton();
  loopScanDownButton();
}

void loopStateConfirmBeforeUseScan() {
  if(navigationMachine.executeOnce) {
    lastNavigationStateChange = millis();
    confirmBeforeUseScanAction();
  }
  loopConfirmBeforeUseScanOkButton();
  loopConfirmUpButton();
  loopConfirmDownButton();
}

void loopStateUseScan(){
  if(navigationMachine.executeOnce) {
    lastNavigationStateChange = millis();
    useScanAction();
  }
  loopScanOkButton();
  loopScanUpButton();
  loopScanDownButton();
}

void loopStateConfirmBeforeEnterCharacterNumber() {
  if(navigationMachine.executeOnce) {
    lastNavigationStateChange = millis();
    confirmBeforeEnterCharacterNumberAction();
  }
  loopConfirmBeforeEnterCharacterNumberOkButton();
  loopConfirmUpButton();
  loopConfirmDownButton();
}

void loopStateEnterCharacterNumber() {
  if(navigationMachine.executeOnce) {
    lastNavigationStateChange = millis();
    enterCharacterNumberAction();
  }
  loopEnterCharacterNumberOkButton();
  loopEnterCharacterNumberUpButton();
  loopEnterCharacterNumberDownButton();
}

void loopStateTryToUpdateStim() {
  if(navigationMachine.executeOnce) {
    lastNavigationStateChange = millis();
    tryToUpdateStimOkButtonAction();
  }
  loopTryToUpdateStimOkButton();
}


bool transitionStateMainMenuToStateScan(){
  if(scanHasBeenPressed) {
    scanHasBeenPressed = false;
    return true;
  } 
  return false;
}

bool transitionStateMainMenuToConfirmBeforeEnterCharacterNumber(){
  if(confirmBeforeEnterCharacterNumberHasBeenPressed) {
    confirmBeforeEnterCharacterNumberHasBeenPressed = false;
    return true;
  } 
  return false;
}

bool transitionStateConfirmBeforeEnterCharacterNumberToEnterCharacterNumber() {
  if(enterCharacterNumberHasBeenPressed) {
    enterCharacterNumberHasBeenPressed = false;
    return true;
  } 
  return false;
}

bool transitionStateConfirmBeforeUseScanToUseScan() {
  if(enterUseScanHasBeenPressed) {
    enterUseScanHasBeenPressed = false;
    return true;
  } 
  return false;
}

bool transitionStateConfirmBeforeEnterCharacterNumberToStateMainMenu() {
  return transitionGenericReturnToMainMenu();
}

bool transitionStateConfirmBeforeUseScanToStateMainMenu() {
  return transitionGenericReturnToMainMenu();
}

bool transitionNavigationGenericSecond(long second){
  return millis() - lastNavigationStateChange > 1000 * second;
}

bool transitionNavigationGeneric10Seconds(){
  return transitionNavigationGenericSecond(10);
}

bool transitionStateScanToStateMainMenu(){
  if(!isScanUsable) {
    return transitionGenericReturnToMainMenu();
  } else {
    return false;
  }
}

bool transitionStateScanToConfirmBeforeUseScan(){
  if(isScanUsable) {
    return transitionGenericReturnToMainMenu();
  } else {
    return false;
  }
}

bool transitionStateUseScanToStateMainMenu(){
  return transitionGenericReturnToMainMenu();
}

bool transitionStateEnterCharacterNumberToTryToUpdateStim(){
  if(confirmHasBeenPressed) {
    confirmHasBeenPressed = false;
    return true;
  } 
  return false;
}

bool transitionStateTryToUpdateStimToMainMenu(){
  return transitionGenericReturnToMainMenu();
}

bool transitionGenericReturnToMainMenu(){
  if(returnToMainMenuHasBeenPressed) {
    returnToMainMenuHasBeenPressed = false;
    return true;
  } 
  return false;
}

//--- OLED STATES LOOP

void loopOledStateOnForSmallDelay(){
  if(oledMachine.executeOnce) {
    //TODO
    lastOledStateChange = millis();
  }
  //TODO
}

void loopOledStateOnForMediumDelay(){
  if(oledMachine.executeOnce) {
    //TODO
    lastOledStateChange = millis();
  }
  //TODO
}

void loopOledStateOnForLongDelay(){
  if(oledMachine.executeOnce) {
    //TODO
    lastOledStateChange = millis();
  }
  //TODO
}

void loopOledStateAlwaysOn(){
  if(oledMachine.executeOnce) {
    //TODO
    lastOledStateChange = millis();
  }
  //TODO
}

void loopOledStateOff(){
  if(oledMachine.executeOnce) {
    display_oled.clearDisplay();
    display_oled.display();
  }
}

//---

bool transitionOledStateOffToOledStateOnForLongDelay() {
  if(oledRequestLong) {
    oledRequestLong = false;
    return true;
  } 
  return false;
}

bool transitionOledStateOnForLongDelayToOledStateOff() {
  return transitionOledGenericSecond(OLED_LONG_DELAY);
}

bool transitionOledStateOffToOledStateOnForMediumDelay() {
  if(oledRequestMedium) {
    oledRequestMedium = false;
    return true;
  } 
  return false;
}

bool transitionOledStateOnForMediumDelayToOledStateOff() {
  return transitionOledGenericSecond(OLED_MEDIUM_DELAY);
}

bool transitionOledStateOffToOledStateOnForSmallDelay() {
  if(oledRequestSmall) {
    oledRequestSmall = false;
    return true;
  } 
  return false;
}

bool transitionOledStateOnForSmallDelayToOledStateOff() {
  return transitionOledGenericSecond(OLED_SMALL_DELAY);
}

bool transitionOledStateOffToOledStateAlwaysOn() {
  if(oledRequestAlways) {
    oledRequestAlways = false;
    return true;
  } 
  return false;
}

bool transitionOledStateAlwaysOnToOledStateOff() {
  if(oledRequestOff) {
    oledRequestOff = false;
    return true;
  } 
  return false;
}

bool transitionOledGenericSecond(long second){
  return millis() - lastOledStateChange > 1000 * second;
}