#define DECK_VERSION "v1.5.4"

// TODO : Refactor debug via services
#define DECKINO_DEBUG_SERIAL true
#define DECKINO_DEBUG_OLED false
#define DECKINO_DEBUG_SERIAL_OLED_MACHINE false

#ifdef DECKINO_DEBUG_SERIAL
  #define DECKINO_DEBUG_SERIAL_PRINTLN(x) Serial.println(x)
  #define DECKINO_DEBUG_SERIAL_PRINTLN_CST(x) Serial.println(F(x))
  #define DECKINO_DEBUG_SERIAL_PRINT(x) Serial.print(x)
  #define DECKINO_DEBUG_SERIAL_PRINT_CST(x) Serial.print(F(x))
  #define DECKINO_DEBUG_SERIAL_PRINTF(x,y) Serial.printf(x,y)
  #define DECKINO_DEBUG_SERIAL_SETUP() Serial.begin(115200);
#else
  #define DECKINO_DEBUG_SERIAL_PRINTLN(x)
  #define DECKINO_DEBUG_SERIAL_PRINTLN_CST(x)
  #define DECKINO_DEBUG_SERIAL_PRINT(x)
  #define DECKINO_DEBUG_SERIAL_PRINT_CST(x)
  #define DECKINO_DEBUG_SERIAL_PRINTF(x,y)
  #define DECKINO_DEBUG_SERIAL_SETUP()
#endif 

#include <SPI.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include "ESP8266WiFi.h"
PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

// OLED
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 1 // GPIO0

// Usage ?
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 0
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH 16

#define SCREEN_WIDTH 64  // OLED display width, in pixels
#define SCREEN_HEIGHT 48 // OLED display height, in pixels
Adafruit_SSD1306 display_oled(OLED_RESET);

#define USE_SCAN_ACTION_DEFAULT_MESSAGE "[SANS EFFET IMEDIAT]"

// CONFIG
#include "DeckDatabase.h"

#define PIN_BUTTON_OK D6
#define PIN_BUTTON_UP D7
#define PIN_BUTTON_DOWN D8
// D8 VCC other GND

#include "DebouncedButton.h"
DebouncedButton okButton(PIN_BUTTON_OK, HIGH);
DebouncedButton upButton(PIN_BUTTON_UP, HIGH);
DebouncedButton downButton(PIN_BUTTON_DOWN, LOW);

#include "DeckMenu.h"
DeckMenu *mainMenu;

#include "DeckConfirmationPopUp.h"
DeckConfirmationPopUp *confirmationPopUp;

#include "DeckChoiceNumberPopUp.h"
DeckChoiceNumberPopUp *choiceNumberPopUp;

#include "DeckDtodServer.h"
DeckDtodServer *dtodServer;

#include "DeckPaginableText.h"
DeckPaginableText *paginableText;

// Todo : Moove vibration motor to StateMachine
#define PIN_VIBRATION_MOTOR D5
unsigned long lastVibrationMotorStartTime = 0;
#define VIBRATION_MOTOR_DELAY 2000

#define SSID_PREFIX "DECK"
// #define SSID_PREFIX "TOTO"

#include "ClusterLogo.h"
#include "DeckScanResult.h"
#include "DeckMthrClient.h"

#include <LinkedList.h>
#include <StateMachine.h>

// BEGIN REGION navigationMachine
// TODO/REFACTOR : Move this StateMachine to it's own class
StateMachine navigationMachine = StateMachine();
State *StateMainMenu = navigationMachine.addState(&loopStateMainMenu);

State *StateScan = navigationMachine.addState(&loopStateScan);
State *StateConfirmBeforeUseScan = navigationMachine.addState(&loopStateConfirmBeforeUseScan);
State *StateUseScan = navigationMachine.addState(&loopStateUseScan);
State *StateSporulationEffectAfterUseScan = navigationMachine.addState(&loopStateSporulationEffectAfterUseScan);

State *StateGenericRemoteScan = navigationMachine.addState(&loopStateGenericRemoteScan);

State *StateConfirmBeforeEnterCharacterNumber = navigationMachine.addState(&loopStateConfirmBeforeEnterCharacterNumber);
State *StateEnterCharacterNumber = navigationMachine.addState(&loopStateEnterCharacterNumber);
State *StateTryToUpdateStim = navigationMachine.addState(&loopStateTryToUpdateStim);

State *StateDisplayDtodResult = navigationMachine.addState(&loopStateDisplayDtodResult);

bool scanHasBeenPressed = false;
bool genericActionRemoteScanHasBeenPressed = false;
bool confirmBeforeEnterCharacterNumberHasBeenPressed = false;
bool enterCharacterNumberHasBeenPressed = false;
bool enterUseScanHasBeenPressed = false;
bool returnToMainMenuHasBeenPressed = false;
bool confirmHasBeenPressed = false;
bool isScanUsable = false;
unsigned long lastNavigationStateChange = 0L;
bool hasDtodResultToDisplay = false;

// debug
int debugLastNavigationMachineState = -1;

// END REGION navigationMachine

// BEGIN REGION oledMachine
// TODO/REFACTOR : Move this StateMachine to it's own class
#define OLED_SMALL_DELAY 10L
#define OLED_MEDIUM_DELAY 30L
#define OLED_LONG_DELAY 60L

StateMachine oledMachine = StateMachine();
State *OledStateOff = oledMachine.addState(&loopOledStateOff);
State *OledStateOnForLongDelay = oledMachine.addState(&loopOledStateOnForLongDelay);
State *OledStateOnForMediumDelay = oledMachine.addState(&loopOledStateOnForMediumDelay);
State *OledStateOnForSmallDelay = oledMachine.addState(&loopOledStateOnForSmallDelay);
State *OledStateAlwaysOn = oledMachine.addState(&loopOledStateAlwaysOn);
bool oledOn = false;

bool oledRequestLong = false;
bool oledRequestMedium = false;
bool oledRequestSmall = false;
bool oledRequestAlways = false;
bool oledRequestOff = false;
unsigned long lastOledStateChange = 0L;

// debug
int debugLastOledMachineState = -1;

// END REGION oledMachine

unsigned long dtodServerUpSince = 0L;

String rfidUidBufferToStringLastValue = "";
String sporulationEffectAfterUseScanActionText = "";

#define CSV_LOG_PATH "/log.csv"

void setup(void)
{
  setupVibrationMotor();
  setupOled();

  display_oled.begin(SSD1306_SWITCHCAPVCC);
  display_oled.drawBitmap(0, 0, clusterLogo_data, clusterLogo_width, clusterLogo_height, 1);
  oledDisplay();
  delay(2000);

  display_oled.setCursor(27, 40);
  display_oled.println(DECK_VERSION);
  oledDisplay();
  delay(1000);

  oledDisplayBlackScreen();

  DECKINO_DEBUG_SERIAL_SETUP();

  DECKINO_DEBUG_SERIAL_PRINTLN_CST("DECK STARTUP");

  DeckDatabase::Instance()->mountFS();

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();

  while (!versiondata)
  {
    DECKINO_DEBUG_SERIAL_PRINTLN_CST("Didn't find PN53x board");
#if DECKINO_DEBUG_OLED
    oledClearDisplay();
    display_oled.println("Didn't find PN53x board");
    oledDisplay();
#endif
    delay(1000);

    DECKINO_DEBUG_SERIAL_PRINTLN_CST("Retrying ...");
#if DECKINO_DEBUG_OLED
    display_oled.println("Retrying ...");
    oledDisplay();
#endif
    nfc.begin();

    uint32_t versiondata = nfc.getFirmwareVersion();
  }

  // RFID INFO FOR DEBUG
  // printRfidReaderInfo(versiondata)

  // configure board to read RFID tags
  nfc.SAMConfig();

  dtodServer = NULL;
  paginableText = NULL;

  // navigationMachine transitions begin
  StateMainMenu->addTransition(&transitionStateMainMenuToStateScan, StateScan);
  StateMainMenu->addTransition(&transitionStateMainMenuToConfirmBeforeEnterCharacterNumber, StateConfirmBeforeEnterCharacterNumber);
  StateMainMenu->addTransition(&transitionStateMainMenuToDisplayDtodResult, StateDisplayDtodResult);
  StateMainMenu->addTransition(&transitionStateMainMenuToStateGenericRemoteScan, StateGenericRemoteScan);

  StateScan->addTransition(&transitionStateScanToStateMainMenu, StateMainMenu); // TODO : To modify
  StateScan->addTransition(&transitionStateScanToConfirmBeforeUseScan, StateConfirmBeforeUseScan);
  StateConfirmBeforeUseScan->addTransition(&transitionStateConfirmBeforeUseScanToStateMainMenu, StateMainMenu);
  StateConfirmBeforeUseScan->addTransition(&transitionStateConfirmBeforeUseScanToUseScan, StateUseScan);
  StateUseScan->addTransition(&transitionStateUseScanToStateMainMenu, StateMainMenu);
  StateUseScan->addTransition(&transitionStateUseScanToStateSporulationEffectAfterUseScan, StateSporulationEffectAfterUseScan);
  StateSporulationEffectAfterUseScan->addTransition(&transitionGenericReturnToMainMenu, StateMainMenu);

  StateDisplayDtodResult->addTransition(&transitionStateDisplayDtodResultToMainMenu, StateMainMenu);

  StateGenericRemoteScan->addTransition(&transitionGenericReturnToMainMenu, StateMainMenu);

  StateConfirmBeforeEnterCharacterNumber->addTransition(&transitionStateConfirmBeforeEnterCharacterNumberToEnterCharacterNumber, StateEnterCharacterNumber);
  StateConfirmBeforeEnterCharacterNumber->addTransition(&transitionStateConfirmBeforeEnterCharacterNumberToStateMainMenu, StateMainMenu);
  StateEnterCharacterNumber->addTransition(&transitionStateEnterCharacterNumberToTryToUpdateStim, StateTryToUpdateStim);
  StateTryToUpdateStim->addTransition(&transitionStateTryToUpdateStimToMainMenu, StateMainMenu);
  // navigationMachine transitions end

  // oledMachine transitions begin
  OledStateOff->addTransition(&transitionOledStateOffToOledStateOnForLongDelay, OledStateOnForLongDelay);
  OledStateOnForLongDelay->addTransition(&transitionOledStateOnForLongDelayToOledStateOff, OledStateOff);

  OledStateOff->addTransition(&transitionOledStateOffToOledStateOnForMediumDelay, OledStateOnForMediumDelay);
  OledStateOnForMediumDelay->addTransition(&transitionOledStateOnForMediumDelayToOledStateOff, OledStateOff);

  OledStateOff->addTransition(&transitionOledStateOffToOledStateOnForSmallDelay, OledStateOnForSmallDelay);
  OledStateOnForSmallDelay->addTransition(&transitionOledStateOnForSmallDelayToOledStateOff, OledStateOff);

  OledStateOff->addTransition(&transitionOledStateOffToOledStateAlwaysOn, OledStateAlwaysOn);
  OledStateAlwaysOn->addTransition(&transitionOledStateAlwaysOnToOledStateOff, OledStateOff);
  // oledMachine transitions end

#if DECKINO_DEBUG_SERIAL
  DeckDatabase::Instance()->printJsonFile("/pers.json");
#endif

  DeckDatabase::Instance()->appendCsvLog(CSV_LOG_PATH, "DECK STARTUP");

#if DECKINO_DEBUG_SERIAL
  DeckDatabase::Instance()->printCsvLog(CSV_LOG_PATH);
#endif

  oledRequestSmall = true;
}

void setUpMainMenu(void)
{
  // if(DeckDatabase::Instance()->getFirstLevelDataByKey("/pers.json", "can_dtod") == "true") {
  //   DeckMenuItem mainMenuItems[] = {
  //     { .label = "SCAN", .value = "SCAN", .selected = true, .shortPressAction = &mainMenuActionScan, .longPressAction = &mainMenuActionEnterCharacterNumber },
  //     { .label = "SERV", .value = "SERV", .selected = false, .shortPressAction = &mainMenuActionDtodServer },
  //     { .label = "DTOD", .value = "DTOD", .selected = false, .shortPressAction = &mainMenuActionDtod }
  //   };

  //   mainMenu = new DeckMenu(mainMenuItems, 3, display_oled);
  // }else{
  //   DeckMenuItem mainMenuItems[] = {
  //     { .label = "SCAN", .value = "SCAN", .selected = true, .shortPressAction = &mainMenuActionScan, .longPressAction = &mainMenuActionEnterCharacterNumber },
  //     { .label = "SERV", .value = "SERV", .selected = false, .shortPressAction = &mainMenuActionDtodServer }
  //   };

  //   mainMenu = new DeckMenu(mainMenuItems, 2, display_oled);
  // }

  const int maxMainMenuItems = 6;
  DeckMenuItem mainMenuItems[maxMainMenuItems];
  int currentMainMenuItem = 0;
  mainMenuItems[currentMainMenuItem++] = {.label = "SCAN", .value = "SCAN", .selected = true, .shortPressAction = &mainMenuActionScan, .longPressAction = &mainMenuActionEnterCharacterNumber};

  mainMenuItems[currentMainMenuItem++] = {.label = "ALOW", .value = "ALOW", .selected = false, .shortPressAction = &actionDtodServer, .longPressAction = &mainMenuActionEmptyLog};

  if (DeckDatabase::Instance()->getFirstLevelDataByKey("/pers.json", "can_dtod") == "true")
  {
    mainMenuItems[currentMainMenuItem++] = {.label = "DTOD", .value = "DTOD", .selected = false, .shortPressAction = &mainMenuActionDtod};
  }
  LinkedList<String> remoteScans = DeckDatabase::Instance()->getSubNodesOfAFirstLevelNode("/pers.json", "rmt_scan");

  for (int h = 0; h < remoteScans.size(); h++)
  {
    DECKINO_DEBUG_SERIAL_PRINTLN("[DEBUG BUILD MAIN MENU : REMOTE SCAN LOOP] " + String(remoteScans.get(h)));

    mainMenuItems[currentMainMenuItem++] = {.label = remoteScans.get(h), .value = remoteScans.get(h), .selected = false, .shortPressAction = &mainMenuActionRemoteScan}; // Action TODO
    if (currentMainMenuItem > maxMainMenuItems)
    {
      break;
    }
  }
  mainMenu = new DeckMenu(mainMenuItems, currentMainMenuItem, display_oled);
}

void setupVibrationMotor(void)
{
  pinMode(PIN_VIBRATION_MOTOR, OUTPUT);
  digitalWrite(PIN_VIBRATION_MOTOR, LOW);
}

void setupOled(void)
{
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display_oled.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize with the I2C addr 0x3C (for the 64x48)

  display_oled.setTextColor(WHITE);
  display_oled.setTextSize(1);
  display_oled.setCursor(0, 0);
  oledDisplayBlackScreen();
}

// Obselete : Keept for debug purpose only
// TODO : Refactor : Add in a RFID util class
// @obselete
void printRfidReaderInfo(uint32_t versiondata)
{
  // Got ok data, print it out!
  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);
}

void loop(void)
{
  loopVibrationMotor();
  loopDtodServer();
  navigationMachine.run();
  if (navigationMachine.currentState != NULL)
  {
    if (debugLastNavigationMachineState != navigationMachine.currentState)
    {
      DECKINO_DEBUG_SERIAL_PRINTLN("[STATE CHANGE] OLD:" + String(debugLastNavigationMachineState) + " NEW:" + String(navigationMachine.currentState));
      debugLastNavigationMachineState = navigationMachine.currentState;
    }
  }
  oledMachine.run();
  if (oledMachine.currentState != NULL)
  {
    if (debugLastOledMachineState != oledMachine.currentState)
    {
#if DECKINO_DEBUG_SERIAL_OLED_MACHINE
      Serial.println("[OLED CHANGE] OLD:" + String(debugLastOledMachineState) + " NEW:" + String(oledMachine.currentState));
#endif
      debugLastOledMachineState = oledMachine.currentState;
    }
  }
}

void loopDtodServer()
{
  if (dtodServer != NULL)
  {
    if (millis() - dtodServerUpSince > 1000 * 60)
    { // TODO : Check possible bug
      dtodServer->close();
      dtodServer = NULL;
      DeckDatabase::Instance()->appendCsvLog(CSV_LOG_PATH, "ALOW TIMEOUT");
    }
    else
    {
      dtodServer->handleClient();
    }
  }
}

void loopMainMenuOkButton(void)
{
  uint8_t buttonValue = okButton.read();
  if (buttonValue != BUTTON_NO_EVENT)
  {
    DeckMenuItem selectedMenuItem = mainMenu->getSelected();

    DECKINO_DEBUG_SERIAL_PRINTLN("[oledMachine.currentState] " + String(oledOn));

    if (!oledOn)
    {
      mainMenuRender();
      oledRequestSmall = true;
    }
    else
    {
      switch (buttonValue)
      {
      case BUTTON_SHORT_PRESS:
        if (*selectedMenuItem.shortPressAction != NULL)
        {
          selectedMenuItem.shortPressAction();
        }
        break;
      case BUTTON_LONG_PRESS:
        if (*selectedMenuItem.longPressAction != NULL)
        {
          selectedMenuItem.longPressAction();
        }
        break;
      }
    }
  }
}

void loopMainMenuUpButton(void)
{
  uint8_t buttonValue = upButton.read();
  if (buttonValue == BUTTON_SHORT_PRESS)
  {
    mainMenu->select(DECKMENU_DIRECTION_UP);
    mainMenuRender();
    oledRequestSmall = true;
  }
  else
  {
    if (buttonValue == BUTTON_LONG_PRESS)
    {
    }
  }
}

void loopMainMenuDownButton(void)
{
  uint8_t buttonValue = downButton.read();
  if (buttonValue == BUTTON_SHORT_PRESS)
  {
    // TODO : IF IN MAIN MENU // Still relevent ?
    mainMenu->select(DECKMENU_DIRECTION_DOWN);
    mainMenuRender();
    oledRequestSmall = true;
  }
  else
  {
    if (buttonValue == BUTTON_LONG_PRESS)
    {
    }
  }
}

void loopScanOkButton(void)
{
  uint8_t buttonValue = okButton.read();
  if (buttonValue != BUTTON_NO_EVENT)
  {
    if (oledMachine.currentState == OledStateOff->index)
    {
      oledDisplay();
      oledRequestSmall = true;
    }
    else
    {
      switch (buttonValue)
      {
      case BUTTON_SHORT_PRESS:
        returnToMainMenuHasBeenPressed = true;
        break;
      case BUTTON_LONG_PRESS:

        break;
      }
    }
  }
}

void loopScanUpButton(void)
{
  uint8_t buttonValue = upButton.read();
  if (buttonValue == BUTTON_SHORT_PRESS)
  {
    paginableText->prev();
    paginableTextRender();
    oledRequestSmall = true;
  }
  else
  {
    if (buttonValue == BUTTON_LONG_PRESS)
    {
    }
  }
}

void loopScanDownButton(void)
{
  uint8_t buttonValue = downButton.read();
  if (buttonValue == BUTTON_SHORT_PRESS)
  {
    paginableText->next();
    paginableTextRender();
    oledRequestSmall = true;
  }
  else
  {
    if (buttonValue == BUTTON_LONG_PRESS)
    {
    }
  }
}

void loopConfirmBeforeUseScanOkButton(void)
{
  uint8_t buttonValue = okButton.read();
  if (buttonValue != BUTTON_NO_EVENT)
  {
    if (oledMachine.currentState == OledStateOff->index)
    {
      oledDisplay();
      oledRequestSmall = true;
    }
    else
    {
      if (confirmationPopUp->isOkSelected())
      {
        enterUseScanHasBeenPressed = true;
      }
      else
      {
        returnToMainMenuHasBeenPressed = true;
      }
    }
  }
}

void loopConfirmBeforeEnterCharacterNumberOkButton(void)
{
  uint8_t buttonValue = okButton.read();
  if (buttonValue != BUTTON_NO_EVENT)
  {
    if (oledMachine.currentState == OledStateOff->index)
    {
      oledDisplay();
      oledRequestSmall = true;
    }
    else
    {
      if (confirmationPopUp->isOkSelected())
      {
        enterCharacterNumberHasBeenPressed = true;
      }
      else
      {
        returnToMainMenuHasBeenPressed = true;
      }
    }
  }
}

void loopConfirmUpButton(void)
{
  uint8_t buttonValue = upButton.read();
  if (buttonValue == BUTTON_SHORT_PRESS)
  {
    confirmationPopUp->toggleSelection();
    confirmationPopUp->render();
  }
  else
  {
    if (buttonValue == BUTTON_LONG_PRESS)
    {
    }
  }
}

void loopConfirmDownButton(void)
{
  uint8_t buttonValue = downButton.read();
  if (buttonValue == BUTTON_SHORT_PRESS)
  {
    confirmationPopUp->toggleSelection();
    confirmationPopUp->render();
  }
  else
  {
    if (buttonValue == BUTTON_LONG_PRESS)
    {
    }
  }
}

void loopEnterCharacterNumberOkButton(void)
{
  uint8_t buttonValue = okButton.read();
  if (buttonValue != BUTTON_NO_EVENT)
  {
    if (oledMachine.currentState == OledStateOff->index)
    {
      oledDisplay();
      oledRequestSmall = true;
    }
    else
    {
      if (choiceNumberPopUp->isCurrentControlButton())
      {

        confirmHasBeenPressed = true;
      }
      else
      {
        choiceNumberPopUp->toggleEditField();
        choiceNumberPopUp->render();
      }
    }
  }
}

String utilZeroPadPlayerId(String inputPlayerId)
{
  char playerIdBuffer[3];
  sprintf(playerIdBuffer, "%03d", inputPlayerId.toInt());
  return String(playerIdBuffer);
}

void loopEnterCharacterNumberUpButton(void)
{
  uint8_t buttonValue = upButton.read();
  if (buttonValue == BUTTON_SHORT_PRESS)
  {
    if (choiceNumberPopUp->getEditField())
    {
      choiceNumberPopUp->increaseSelectedField();
    }
    else
    {
      choiceNumberPopUp->goToNextControl();
    }
    choiceNumberPopUp->render();
  }
  else
  {
    if (buttonValue == BUTTON_LONG_PRESS)
    {
    }
  }
}

void loopEnterCharacterNumberDownButton(void)
{
  uint8_t buttonValue = downButton.read();
  if (buttonValue == BUTTON_SHORT_PRESS)
  {
    if (choiceNumberPopUp->getEditField())
    {
      choiceNumberPopUp->decreaseSelectedField();
    }
    else
    {
      choiceNumberPopUp->goToPrevControl();
    }
    choiceNumberPopUp->render();
  }
  else
  {
    if (buttonValue == BUTTON_LONG_PRESS)
    {
    }
  }
}

void loopTryToUpdateStimOkButton(void)
{
  uint8_t buttonValue = okButton.read();
  if (buttonValue != BUTTON_NO_EVENT)
  {
    if (oledMachine.currentState == OledStateOff->index)
    {
      oledDisplay();
      oledRequestSmall = true;
    }
    else
    {
      returnToMainMenuHasBeenPressed = true;
    }
  }
}

// ACTIONS ----------------------------------------------

void actionDtodServer(void)
{
  if (dtodServer == NULL)
  {
    dtodServer = new DeckDtodServer(display_oled);
  }
  dtodServerUpSince = millis();
  paginableText = new DeckPaginableText("En attente d'être scanné par un autre DECK", display_oled);
  paginableTextRender();
  DeckDatabase::Instance()->appendCsvLog(CSV_LOG_PATH, "ALOW");
}

void mainMenuActionEnterCharacterNumber(void)
{
  confirmBeforeEnterCharacterNumberHasBeenPressed = true;
}

void mainMenuActionEmptyLog(void)
{
  // TODO : Move to navigation state ?
  DeckDatabase::Instance()->emptyCsvLog(CSV_LOG_PATH);
  oledClearDisplay();
  display_oled.setCursor(0, 0);
  display_oled.println("Log du deck effacé avec succès.");
  oledDisplay();
  oledRequestSmall = true;
}

void mainMenuActionScan(void)
{
  scanHasBeenPressed = true;
  DeckDatabase::Instance()->appendCsvLog(CSV_LOG_PATH, "SCAN");
}

void mainMenuGenericActionRemoteScan(void)
{
  genericActionRemoteScanHasBeenPressed = true;
}

void mainMenuActionDtod()
{
  mainMenuActionOrRemoteScan(false);
}

void mainMenuActionRemoteScan()
{
  mainMenuActionOrRemoteScan(true);
}

void mainMenuActionOrRemoteScan(bool isRemoteScan)
{
  oledClearDisplay();
  display_oled.setCursor(0, 0);
  display_oled.println("Scan des deck à portée en cours .");
  oledDisplay();
  oledRequestSmall = true;

  // INIT
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  oledClearDisplay();
  display_oled.setCursor(0, 0);
  display_oled.println("Scan des deck à portée en cours ...");
  oledDisplay();
  oledRequestSmall = true;

  String csvLogString = (isRemoteScan ? mainMenu->getSelected().value : "DTOD");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  if (n == 0)
  {
    oledClearDisplay();
    display_oled.setCursor(0, 0);
    display_oled.println("Aucun DECK a scanner");
    oledDisplay();
    oledRequestSmall = true;
    DeckDatabase::Instance()->appendCsvLog(CSV_LOG_PATH, csvLogString + " Aucun DECK a scanner");
  }
  else
  {
    int maxForce = -999;
    String closestDeckName = "";
    String closestDeckSsid = "";
    for (int i = 0; i < n; ++i)
    {

      // TODO ajouter ici le filtre par seuil minimum de force
      if (WiFi.SSID(i).substring(0, 4) == SSID_PREFIX)
      {
        DECKINO_DEBUG_SERIAL_PRINTLN_CST("DECK FOUND :");
        DECKINO_DEBUG_SERIAL_PRINT_CST("NAME : ");
        DECKINO_DEBUG_SERIAL_PRINTLN(WiFi.SSID(i).substring(5, 8));
        DECKINO_DEBUG_SERIAL_PRINT_CST("SPORE : ");
        DECKINO_DEBUG_SERIAL_PRINTLN(WiFi.SSID(i).substring(9));
        DECKINO_DEBUG_SERIAL_PRINT_CST("FORCE : ");
        DECKINO_DEBUG_SERIAL_PRINTLN(WiFi.RSSI(i));

        int force = WiFi.RSSI(i);
        if (force > maxForce)
        {
          maxForce = force;
          closestDeckSsid = WiFi.SSID(i);
          closestDeckName = WiFi.SSID(i).substring(4);
        }
      }
    }
    oledClearDisplay();
    display_oled.setCursor(0, 0);

    String remoteDeckData = "";
    if (maxForce > -999)
    {

      DeckDatabase::Instance()->appendCsvLog(CSV_LOG_PATH, csvLogString + " DECK " + closestDeckSsid + " SCAN");

      String labelToDisplay;
      if (isRemoteScan)
      {
        String paddedPlayerId = closestDeckSsid.substring(5, 8);
        labelToDisplay = getRemoteScanLabelFromRemoteData(paddedPlayerId);
      }
      else
      {
        String playerSporePercent = closestDeckSsid.substring(9);
        long playerSporePercentLng = strtol(playerSporePercent.c_str(), NULL, 16);
        playerSporePercentLng -= 7;
        labelToDisplay = getDtodLabelFromRemoteData(String(playerSporePercentLng));
      }

      paginableText = new DeckPaginableText(labelToDisplay, display_oled);
      paginableTextRender();
      oledRequestAlways = true;
      hasDtodResultToDisplay = true;

      // Vibration motor
      digitalWrite(PIN_VIBRATION_MOTOR, HIGH);
      lastVibrationMotorStartTime = millis();
    }
  }
  WiFi.disconnect();
}

String getDtodLabelFromRemoteData(String playerSporePercent)
{
  int playerSporePercentInt = playerSporePercent.toInt();
  if (playerSporePercentInt == 0)
  {
    playerSporePercentInt = 1; // Si sporulation 0% on affiche le message de 1%
  }
  String result = DeckDatabase::Instance()->getMatchingLabelByRange("/pers.json", "dtod_ranges", playerSporePercentInt);

  DECKINO_DEBUG_SERIAL_PRINTLN("[getDtodLabelFromRemoteData] Label found for " + playerSporePercent + "% : \"" + result + "\"");

  return result;
}

String getRemoteScanLabelFromRemoteData(String remotePlayerId)
{
  DeckMenuItem selectedMenuItem = mainMenu->getSelected();
  selectedMenuItem.value;

  String result = DeckDatabase::Instance()->getThirdLevelDataByKeys("/pers.json", "rmt_scan", selectedMenuItem.value, remotePlayerId);
  return result;
}

//
[[deprecated]]
String mainMenuActionDtodGetRemoteData(String closestDeckSsid)
{
  String result = "";

  ESP8266WiFiMulti WiFiMulti;

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(closestDeckSsid.c_str(), "aaaaaaaa");

  WiFiClient client;
  HTTPClient http;

  while ((WiFiMulti.run() != WL_CONNECTED))
  {
    DECKINO_DEBUG_SERIAL_PRINTLN("[mainMenuActionDtodGetRemoteData] Waiting for connexion on SSID:" + closestDeckSsid + " pass:" + "aaaaaaaa");

  }

  DECKINO_DEBUG_SERIAL_PRINTLN_CST("[mainMenuActionDtodGetRemoteData][HTTP] begin...");
  if (http.begin(client, "http://192.168.4.1/"))
  { // HTTP

    DECKINO_DEBUG_SERIAL_PRINTLN_CST("[mainMenuActionDtodGetRemoteData][HTTP] GET...");

    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
// HTTP header has been send and Server response header has been handled
      DECKINO_DEBUG_SERIAL_PRINTF("[mainMenuActionDtodGetRemoteData][HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        result = http.getString();
        DECKINO_DEBUG_SERIAL_PRINTLN_CST("[mainMenuActionDtodGetRemoteData][DATA RECIEVED]");
        DECKINO_DEBUG_SERIAL_PRINTLN(result);
      }
    }
    else
    {
      DECKINO_DEBUG_SERIAL_PRINTF("[mainMenuActionDtodGetRemoteData][HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  else
  {

    DECKINO_DEBUG_SERIAL_PRINTLN_CST("[mainMenuActionDtodGetRemoteData][HTTP] Unable to connect");
  }

  return result;
}

void loopVibrationMotor(void)
{
  if (lastVibrationMotorStartTime != 0 && millis() > lastVibrationMotorStartTime + VIBRATION_MOTOR_DELAY)
  {
    digitalWrite(PIN_VIBRATION_MOTOR, LOW);
    lastVibrationMotorStartTime = 0;
  }
}

String rfidUidBufferToString(uint8_t uid[])
{
  char uidCharArray[8] = {0};
  sprintf(uidCharArray, "%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
  String result = uidCharArray;
  return result;
}

void pn532ReadRfidLoop(void)
{

  paginableText = new DeckPaginableText("En attente de SCAN", display_oled);
  paginableTextRender();
  oledRequestAlways = true;

  bool success = false;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
  uint8_t uidLength;                     // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  int rfidScanTryCpt = 0;
  while (success == false)
  {
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
    rfidScanTryCpt++;
    String displayMessage = "En attente de SCAN    ";
    for (int cpt = 0; cpt <= (rfidScanTryCpt % 3); cpt++)
    {
      displayMessage += ".";
    }
    paginableText = new DeckPaginableText(displayMessage, display_oled);
    paginableTextRender();
    oledRequestAlways = true;
  }

  if (success)
  {
    // Display some basic information about the card


    String scanResultId = rfidUidBufferToString(uid);
    DeckScanResult scanResult = DeckDatabase::Instance()->getStimResultByUid(scanResultId);
    isScanUsable = scanResult.usable;
    
    DECKINO_DEBUG_SERIAL_PRINT_CST("Label from JSON: ");
    DECKINO_DEBUG_SERIAL_PRINTLN(scanResult.label);
    DECKINO_DEBUG_SERIAL_PRINTLN_CST("");

    // Vibration motor
    digitalWrite(PIN_VIBRATION_MOTOR, HIGH);
    lastVibrationMotorStartTime = millis();

    paginableText = new DeckPaginableText(scanResult.label, display_oled);
    paginableTextRender();
    oledRequestSmall = true;

    DeckDatabase::Instance()->appendCsvLog(CSV_LOG_PATH, "SCAN RESULT = " + scanResult.label + "(" + scanResultId + " - usable " + (scanResult.usable ? "true" : "false") + ")");

    rfidUidBufferToStringLastValue = rfidUidBufferToString(uid);
  }
}

void useScanAction(void)
{
  // Vibration motor
  digitalWrite(PIN_VIBRATION_MOTOR, HIGH);
  lastVibrationMotorStartTime = millis();

  DECKINO_DEBUG_SERIAL_PRINT_CST("[SPORE](Before use) : ");
  DECKINO_DEBUG_SERIAL_PRINT_CST("- Spore Max : ");
  DECKINO_DEBUG_SERIAL_PRINT(DeckDatabase::Instance()->getFirstLevelDataByKey("/pers.json", "spore_max"));
  DECKINO_DEBUG_SERIAL_PRINT_CST(" - Spore Actuel : ");
  DECKINO_DEBUG_SERIAL_PRINT(
    DeckDatabase::Instance()->getFirstLevelDataByKey("/spor.json", "spore_actuel", 
    DeckDatabase::Instance()->getFirstLevelDataByKey("/pers.json", "spore_actuel")
  ));
  DECKINO_DEBUG_SERIAL_PRINTLN_CST("");

  int sporeMaxInt = DeckDatabase::Instance()->getFirstLevelDataByKey("/pers.json", "spore_max").toInt();
  if (sporeMaxInt == 0)
  {
    sporeMaxInt = 10;
  }
  DeckDatabase::Instance()->persistSporeActuel(
      String(constrain(
        DeckDatabase::Instance()->getFirstLevelDataByKey("/spor.json", "spore_actuel", 
        DeckDatabase::Instance()->getFirstLevelDataByKey("/pers.json", "spore_actuel")).toInt() 
        + DeckDatabase::Instance()->getFieldValueByUid(rfidUidBufferToStringLastValue, "spore").toInt()
        , 0
        ,sporeMaxInt
      )));

  String useScanActionTextToDisplay = DeckDatabase::Instance()->getFieldValueByUid(rfidUidBufferToStringLastValue, "effect");
  if (useScanActionTextToDisplay == "")
  {
    useScanActionTextToDisplay = USE_SCAN_ACTION_DEFAULT_MESSAGE;
  }

  paginableText = new DeckPaginableText(useScanActionTextToDisplay, display_oled);
  paginableTextRender();
  oledRequestSmall = true;

  rfidUidBufferToStringLastValue = "";

  sporulationEffectAfterUseScanActionText = DeckDatabase::Instance()->getMatchingLabelByRange("/pers.json", "spore_ranges", utilGetCurrentSporePercent());

  DECKINO_DEBUG_SERIAL_PRINT_CST("[SPORE](After use) : ");
  DECKINO_DEBUG_SERIAL_PRINT_CST("- Spore Max : ");
  DECKINO_DEBUG_SERIAL_PRINT(String(sporeMaxInt));
  DECKINO_DEBUG_SERIAL_PRINT_CST(" - Spore Actuel : ");
  DECKINO_DEBUG_SERIAL_PRINT(DeckDatabase::Instance()->getFirstLevelDataByKey(
    "/spor.json", 
    "spore_actuel", 
    DeckDatabase::Instance()->getFirstLevelDataByKey("/pers.json", "spore_actuel")
  ));
  DECKINO_DEBUG_SERIAL_PRINT_CST("");
  DECKINO_DEBUG_SERIAL_PRINT_CST("Range Label : ");
  DECKINO_DEBUG_SERIAL_PRINT(sporulationEffectAfterUseScanActionText);
  DECKINO_DEBUG_SERIAL_PRINT_CST("");
  DeckDatabase::Instance()->appendCsvLog(CSV_LOG_PATH, "SCAN USED");
}

int utilGetCurrentSporePercent(void)
{
  String sporeActuelStr = DeckDatabase::Instance()->getFirstLevelDataByKey(
    "/spor.json", 
    "spore_actuel", 
    DeckDatabase::Instance()->getFirstLevelDataByKey("/pers.json", "spore_actuel")
  );
  int sporeActuel = 0;
  if (sporeActuelStr.length() > 0)
  {
    sporeActuel = sporeActuelStr.toInt();
  }
  String sporeMaxStr = DeckDatabase::Instance()->getFirstLevelDataByKey("/pers.json", "spore_max");
  int sporeMax = 0; // default 10
  if (sporeMaxStr.length() > 0)
  {
    sporeMax = sporeMaxStr.toInt();
  }
  if (sporeMax == 0)
  {
    sporeMax = 10; // default 10
  }
  return round(sporeActuel * 100 / sporeMax);
}

void sporulationEffectAfterUseScanAction(void)
{
  // Vibration motor
  digitalWrite(PIN_VIBRATION_MOTOR, HIGH);
  lastVibrationMotorStartTime = millis();

  paginableText = new DeckPaginableText(sporulationEffectAfterUseScanActionText, display_oled);
  paginableTextRender();
  oledRequestSmall = true;

  sporulationEffectAfterUseScanActionText = "";
}

void confirmBeforeEnterCharacterNumberAction(void)
{
  String confirmBeforeEnterCharacterNumberActionMessage = 
    "ID_PERS=" 
    + utilZeroPadPlayerId(DeckDatabase::Instance()->getFirstLevelDataByKey("/config.json", "player_id")) 
    + " changer ?";
  confirmationPopUp = new DeckConfirmationPopUp(confirmBeforeEnterCharacterNumberActionMessage, display_oled);
  confirmationPopUp->render();
  oledRequestAlways = true; // necessary ?
}

void confirmBeforeUseScanAction(void)
{
  confirmationPopUp = new DeckConfirmationPopUp("Utiliser l'objet ?", display_oled);
  confirmationPopUp->render();
  oledRequestAlways = true;
}

void enterCharacterNumberAction(void)
{
  choiceNumberPopUp = new DeckChoiceNumberPopUp(display_oled, DeckDatabase::Instance()->getFirstLevelDataByKey("/config.json", "player_id").toInt());
  choiceNumberPopUp->render();
}

void tryToUpdateStimOkButtonAction(void)
{
  String oldPaddedPlayerId = utilZeroPadPlayerId(DeckDatabase::Instance()->getFirstLevelDataByKey("/config.json", "player_id"));

  DeckDatabase::Instance()->persistFirstLevelDataByKeyValue(
    "/config.json", 
    "player_id", 
    String(choiceNumberPopUp->getFinalValue())
  );

  String paddedPlayerId = utilZeroPadPlayerId(DeckDatabase::Instance()->getFirstLevelDataByKey("/config.json", "player_id"));

  paginableText = new DeckPaginableText("DOWN ID" + paddedPlayerId + "...", display_oled);
  paginableTextRender();

  DECKINO_DEBUG_SERIAL_PRINTLN_CST("[tryToUpdateStimOkButtonAction] before mthrClient construct");
  DECKINO_DEBUG_SERIAL_PRINTLN_CST("[tryToUpdateStimOkButtonAction] begin print /wifi.json");
  DeckDatabase::Instance()->printJsonFile("/wifi.json");
  DECKINO_DEBUG_SERIAL_PRINTLN_CST("[tryToUpdateStimOkButtonAction] end print /wifi.json");

  DeckMthrClient *mthrClient = new DeckMthrClient(
    DeckDatabase::Instance()->getFirstLevelDataByKey("/wifi.json", "mthr_ssid"), 
    DeckDatabase::Instance()->getFirstLevelDataByKey("/wifi.json", "mthr_password"), 
    DeckDatabase::Instance()->getFirstLevelDataByKey("/wifi.json", "mthr_uri")
  );

  DECKINO_DEBUG_SERIAL_PRINTLN_CST("[tryToUpdateStimOkButtonAction] after mthrClient construct");


  // DOWNLOAD STIM.JSON

  UpdateStimsResponse updateStimsResponse = mthrClient->updateStims(paddedPlayerId, true);

  String userDisplayMessage = updateStimsResponse.userMessage;
  
  // DOWNLOAD PERS.JSON

  RessourceResponse motherResponse = mthrClient->DownloadRessource("/HTTP/pers/" + paddedPlayerId + "/pers.json");

  bool persSucces = false;
  if (motherResponse.httpCode == 404)
  {
    userDisplayMessage += " - ECHEC PERS : PERSONNAGE NOT FOUND";
  }
  else if (motherResponse.httpCode != 200)
  {
    userDisplayMessage += " - ECHEC PERS : NETWORK ERROR : " + String(motherResponse.httpCode);
  }
  else
  {

    DeckDatabase::Instance()->persistFullFile("/pers.json", motherResponse.payload);
    userDisplayMessage += " - SUCCESS PERS : DATA UPDATED";
    persSucces = true;
  }
  paginableText = new DeckPaginableText("DOWN ID" + paddedPlayerId + "..." + userDisplayMessage, display_oled);
  paginableTextRender();

  String csvLogMessage = "TRY TO CHANGE PLAYER_ID (old=" + oldPaddedPlayerId + " - new=" + paddedPlayerId;
  csvLogMessage += " - STIM.JSON=";
  csvLogMessage += (!updateStimsResponse.error ? "OK" : "KO");
  csvLogMessage += " - PERS.JSON=";
  csvLogMessage += (persSucces ? "OK" : "KO");
  csvLogMessage += " )";
  DeckDatabase::Instance()->appendCsvLog(CSV_LOG_PATH, csvLogMessage);
}


//-------------------------

void loopStateMainMenu()
{
  if (navigationMachine.executeOnce)
  {
    lastNavigationStateChange = millis();
    setUpMainMenu();
    mainMenuRender();
    oledRequestMedium = true;
  }
  loopMainMenuOkButton();
  loopMainMenuUpButton();
  loopMainMenuDownButton();
}

void loopStateScan()
{
  if (navigationMachine.executeOnce)
  {
    lastNavigationStateChange = millis();
    pn532ReadRfidLoop();
  }
  loopScanOkButton();
  loopScanUpButton();
  loopScanDownButton();
}

void loopStateConfirmBeforeUseScan()
{
  if (navigationMachine.executeOnce)
  {
    lastNavigationStateChange = millis();
    confirmBeforeUseScanAction();
  }
  loopConfirmBeforeUseScanOkButton();
  loopConfirmUpButton();
  loopConfirmDownButton();
}

void loopStateUseScan()
{
  if (navigationMachine.executeOnce)
  {
    lastNavigationStateChange = millis();
    useScanAction();
  }
  loopScanOkButton();
  loopScanUpButton();
  loopScanDownButton();
}

void loopStateSporulationEffectAfterUseScan()
{
  if (navigationMachine.executeOnce)
  {
    lastNavigationStateChange = millis();
    sporulationEffectAfterUseScanAction();
  }
  loopScanOkButton();
  loopScanUpButton();
  loopScanDownButton();
}

void loopStateConfirmBeforeEnterCharacterNumber()
{
  if (navigationMachine.executeOnce)
  {
    lastNavigationStateChange = millis();
    confirmBeforeEnterCharacterNumberAction();
  }
  loopConfirmBeforeEnterCharacterNumberOkButton();
  loopConfirmUpButton();
  loopConfirmDownButton();
}

void loopStateEnterCharacterNumber()
{
  if (navigationMachine.executeOnce)
  {
    lastNavigationStateChange = millis();
    enterCharacterNumberAction();
  }
  loopEnterCharacterNumberOkButton();
  loopEnterCharacterNumberUpButton();
  loopEnterCharacterNumberDownButton();
}

void loopStateTryToUpdateStim()
{
  if (navigationMachine.executeOnce)
  {
    lastNavigationStateChange = millis();
    tryToUpdateStimOkButtonAction();
  }
  loopTryToUpdateStimOkButton();
}

void loopStateDisplayDtodResult()
{
  if (navigationMachine.executeOnce)
  {
    lastNavigationStateChange = millis();
    DECKINO_DEBUG_SERIAL_PRINTLN_CST("[loopStateDisplayDtodResult] Hello");
    // Nothing ?
  }
  loopScanOkButton();
  loopScanUpButton();
  loopScanDownButton();
}

void loopStateGenericRemoteScan()
{
  if (navigationMachine.executeOnce)
  {
    lastNavigationStateChange = millis();
    DECKINO_DEBUG_SERIAL_PRINTLN_CST("[loopStateGenericRemoteScan] Hello");
    // Nothing ?
  }
  loopScanOkButton();
  loopScanUpButton();
  loopScanDownButton();
}

bool transitionStateMainMenuToStateScan()
{
  if (scanHasBeenPressed)
  {
    scanHasBeenPressed = false;
    return true;
  }
  return false;
}

bool transitionStateMainMenuToStateGenericRemoteScan()
{
  if (genericActionRemoteScanHasBeenPressed)
    if (genericActionRemoteScanHasBeenPressed)
    {
      genericActionRemoteScanHasBeenPressed = false;
      return true;
    }
  return false;
}

bool transitionStateMainMenuToDisplayDtodResult()
{
  if (hasDtodResultToDisplay)
  {
    hasDtodResultToDisplay = false;
    DECKINO_DEBUG_SERIAL_PRINTLN_CST("[transitionStateMainMenuToDisplayDtodResult] Hello");
    returnToMainMenuHasBeenPressed = false; // better safe than sorry
    return true;
  }
  return false;
}

bool transitionStateDisplayDtodResultToMainMenu()
{
  return transitionGenericReturnToMainMenu();
}

bool transitionStateMainMenuToConfirmBeforeEnterCharacterNumber()
{
  if (confirmBeforeEnterCharacterNumberHasBeenPressed)
  {
    confirmBeforeEnterCharacterNumberHasBeenPressed = false;
    return true;
  }
  return false;
}

bool transitionStateConfirmBeforeEnterCharacterNumberToEnterCharacterNumber()
{
  if (enterCharacterNumberHasBeenPressed)
  {
    enterCharacterNumberHasBeenPressed = false;
    return true;
  }
  return false;
}

bool transitionStateConfirmBeforeUseScanToUseScan()
{
  if (enterUseScanHasBeenPressed)
  {
    enterUseScanHasBeenPressed = false;
    return true;
  }
  return false;
}

bool transitionStateConfirmBeforeEnterCharacterNumberToStateMainMenu()
{
  return transitionGenericReturnToMainMenu();
}

bool transitionStateConfirmBeforeUseScanToStateMainMenu()
{
  return transitionGenericReturnToMainMenu();
}

bool transitionNavigationGenericSecond(long second)
{
  return millis() - lastNavigationStateChange > 1000 * second;
}

bool transitionNavigationGeneric10Seconds()
{
  return transitionNavigationGenericSecond(10);
}

bool transitionStateScanToStateMainMenu()
{
  if (!isScanUsable)
  {
    return transitionGenericReturnToMainMenu();
  }
  else
  {
    return false;
  }
}

bool transitionStateScanToConfirmBeforeUseScan()
{
  if (isScanUsable)
  {
    return transitionGenericReturnToMainMenu();
  }
  else
  {
    return false;
  }
}

bool transitionStateUseScanToStateMainMenu()
{
  if (sporulationEffectAfterUseScanActionText.length() == 0)
  {
    return transitionGenericReturnToMainMenu();
  }
  else
  {
    return false;
  }
}

bool transitionStateUseScanToStateSporulationEffectAfterUseScan()
{
  if (sporulationEffectAfterUseScanActionText.length() > 0)
  {
    return transitionGenericReturnToMainMenu();
  }
  else
  {
    return false;
  }
}

bool transitionStateEnterCharacterNumberToTryToUpdateStim()
{
  if (confirmHasBeenPressed)
  {
    confirmHasBeenPressed = false;
    return true;
  }
  return false;
}

bool transitionStateTryToUpdateStimToMainMenu()
{
  return transitionGenericReturnToMainMenu();
}

bool transitionGenericReturnToMainMenu()
{
  if (returnToMainMenuHasBeenPressed)
  {
    returnToMainMenuHasBeenPressed = false;
    return true;
  }
  return false;
}

//--- OLED STATES LOOP

void loopOledStateOnForSmallDelay()
{
  if (oledMachine.executeOnce)
  {
    // TODO
    lastOledStateChange = millis();
  }
  // TODO
}

void loopOledStateOnForMediumDelay()
{
  if (oledMachine.executeOnce)
  {
    // TODO
    lastOledStateChange = millis();
  }
  // TODO
}

void loopOledStateOnForLongDelay()
{
  if (oledMachine.executeOnce)
  {
    // TODO
    lastOledStateChange = millis();
  }
  // TODO
}

void loopOledStateAlwaysOn()
{
  if (oledMachine.executeOnce)
  {
    // TODO
    lastOledStateChange = millis();
  }
  // TODO
}

void loopOledStateOff()
{
  if (oledMachine.executeOnce)
  {
    oledDisplayBlackScreen();
    DECKINO_DEBUG_SERIAL_PRINTLN_CST("loopOledStateOff!");
  }
}

//---

bool transitionOledStateOffToOledStateOnForLongDelay()
{
  if (oledRequestLong)
  {
    oledRequestLong = false;
    return true;
  }
  return false;
}

bool transitionOledStateOnForLongDelayToOledStateOff()
{
  return transitionOledGenericSecond(OLED_LONG_DELAY);
}

bool transitionOledStateOffToOledStateOnForMediumDelay()
{
  if (oledRequestMedium)
  {
    oledRequestMedium = false;
    return true;
  }
  return false;
}

bool transitionOledStateOnForMediumDelayToOledStateOff()
{
  return transitionOledGenericSecond(OLED_MEDIUM_DELAY);
}

bool transitionOledStateOffToOledStateOnForSmallDelay()
{
  if (oledRequestSmall)
  {
    oledRequestSmall = false;
    return true;
  }
  return false;
}

bool transitionOledStateOnForSmallDelayToOledStateOff()
{
  return transitionOledGenericSecond(OLED_SMALL_DELAY);
}

bool transitionOledStateOffToOledStateAlwaysOn()
{
  if (oledRequestAlways)
  {
    oledRequestAlways = false;
    return true;
  }
  return false;
}

bool transitionOledStateAlwaysOnToOledStateOff()
{
  if (oledRequestOff)
  {
    oledRequestOff = false;
    return true;
  }
  return false;
}

bool transitionOledGenericSecond(long second)
{
  return millis() - lastOledStateChange > 1000 * second;
}

//----------

void oledClearDisplay(void)
{
  display_oled.clearDisplay();
  oledOn = false;
}

void oledDisplay(void)
{
  display_oled.display();
  oledOn = true;
}

void oledDisplayBlackScreen(void)
{
  display_oled.clearDisplay();
  display_oled.display();
  oledOn = false;
}

void mainMenuRender(void)
{
  mainMenu->render();
  oledOn = true;
}

void paginableTextRender(void)
{
  paginableText->render();
  oledOn = true;
}