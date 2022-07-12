#define DECK_VERSION "v1.4.0"

// TODO : Refactor debug via services
#define DECKINO_DEBUG_SERIAL true
#define DECKINO_DEBUG_OLED true

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
DeckDatabase deckDatabase;

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
//#define SSID_PREFIX "TOTO"

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

bool oledRequestLong = false;
bool oledRequestMedium = false;
bool oledRequestSmall = false;
bool oledRequestAlways = false;
bool oledRequestOff = false;
unsigned long lastOledStateChange = 0L;
// END REGION oledMachine

// BEGIN REGION dtodServerMachine
StateMachine DtodServerMachine = StateMachine();
State *DtodServerStateOn = DtodServerMachine.addState(&loopDtodServerStateOn);
State *DtodServerStateOff = DtodServerMachine.addState(&loopDtodServerStateOff);

bool dtodServerRequest = false;

unsigned long lastDtodServerStateChange = 0L;
// END REGION dtodServerMachine

String rfidUidBufferToStringLastValue = "";
String sporulationEffectAfterUseScanActionText = "";

void setup(void)
{
  setupVibrationMotor();
  setupOled();

  display_oled.begin(SSD1306_SWITCHCAPVCC);
  display_oled.drawBitmap(0, 0, clusterLogo_data, clusterLogo_width, clusterLogo_height, 1);
  display_oled.display();
  delay(2000);

  display_oled.setCursor(27, 40);
  display_oled.println(DECK_VERSION);
  display_oled.display();
  delay(1000);

  display_oled.clearDisplay();
  display_oled.display();

  Serial.begin(115200);

#if DECKINO_DEBUG_SERIAL
  Serial.println("Hello!");
#endif

  deckDatabase.mountFS();

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();

  while (!versiondata)
  {
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

  // dtodServerMachine transitions begin
  DtodServerStateOn->addTransition(&transitionDtodServerStateOnToDtodServerStateOff, DtodServerStateOff);
  DtodServerStateOff->addTransition(&transitionDtodServerStateOffToDtodServerStateOn, DtodServerStateOn);
  // dtodServerMachine transitions end

  deckDatabase.printJsonFile("/pers.json");

  oledRequestSmall = true;
  dtodServerRequest = true;
}

void setUpMainMenu(void)
{
  // if(deckDatabase.getFirstLevelDataByKey("/pers.json", "can_dtod") == "true") {
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

  const int maxMainMenuItems = 3;
  DeckMenuItem mainMenuItems[maxMainMenuItems];
  int currentMainMenuItem = 0;
  mainMenuItems[currentMainMenuItem++] = {.label = "SCAN", .value = "SCAN", .selected = true, .shortPressAction = &mainMenuActionScan, .longPressAction = &mainMenuActionEnterCharacterNumber};
  if (deckDatabase.getFirstLevelDataByKey("/pers.json", "can_dtod") == "true")
  {
    mainMenuItems[currentMainMenuItem++] = {.label = "DTOD", .value = "DTOD", .selected = false, .shortPressAction = &mainMenuActionDtod};
  }
  LinkedList<String> remoteScans = deckDatabase.getSubNodesOfAFirstLevelNode("/pers.json", "rmt_scan");

  for (int h = 0; h < remoteScans.size(); h++)
  {
#if DECKINO_DEBUG_SERIAL
    Serial.println("[DEBUG BUILD MAIN MENU : REMOTE SCAN LOOP] " + String(remoteScans.get(h)));
#endif
    mainMenuItems[currentMainMenuItem++] = {.label = remoteScans.get(h), .value = remoteScans.get(h), .selected = false, .shortPressAction = &mainMenuActionRemoteScan}; // Action TODO
    if (currentMainMenuItem >= 3)
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

  display_oled.clearDisplay();

  display_oled.setTextColor(WHITE);
  display_oled.setTextSize(1);
  display_oled.setCursor(0, 0);
  display_oled.display();
}

// Obselete : Keept for debug purpose only
// TODO : Refactor : Add in a RFID util class
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
#if DECKINO_DEBUG_SERIAL
      Serial.println("[STATE CHANGE] OLD:" + String(debugLastNavigationMachineState) + " NEW:" + String(navigationMachine.currentState));
#endif
      debugLastNavigationMachineState = navigationMachine.currentState;
    }
  }
  oledMachine.run();
}

void loopDtodServer()
{
  if (dtodServer != NULL)
  {
    dtodServer->handleClient();
  }
}

void loopMainMenuOkButton(void)
{
  uint8_t buttonValue = okButton.read();
  if (buttonValue != BUTTON_NO_EVENT)
  {
    DeckMenuItem selectedMenuItem = mainMenu->getSelected();

    if (oledMachine.currentState == OledStateOff->index)
    {
      display_oled.display();
      oledRequestSmall = true;
      dtodServerRequest = true;
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
    mainMenu->render();
    oledRequestSmall = true;
    dtodServerRequest = true;
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
    mainMenu->render();
    oledRequestSmall = true;
    dtodServerRequest = true;
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
      display_oled.display();
      oledRequestSmall = true;
      dtodServerRequest = true;
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
    paginableText->render();
    oledRequestSmall = true;
    dtodServerRequest = true;
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
    paginableText->render();
    oledRequestSmall = true;
    dtodServerRequest = true;
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
      display_oled.display();
      oledRequestSmall = true;
      dtodServerRequest = true;
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
      display_oled.display();
      oledRequestSmall = true;
      dtodServerRequest = true;
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
      display_oled.display();
      oledRequestSmall = true;
      dtodServerRequest = true;
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
      display_oled.display();
      oledRequestSmall = true;
      dtodServerRequest = true;
    }
    else
    {
      returnToMainMenuHasBeenPressed = true;
    }
  }
}

// ACTIONS ----------------------------------------------

void mainMenuActionDtodServer(void)
{
  dtodServer = new DeckDtodServer(display_oled, deckDatabase);
  oledRequestAlways = true;
  dtodServerRequest = true;
  paginableText = new DeckPaginableText("SERVEUR DEMARRE", display_oled);
  paginableText->render();
}

void mainMenuActionEnterCharacterNumber(void)
{
  confirmBeforeEnterCharacterNumberHasBeenPressed = true;
}

void mainMenuActionScan(void)
{
  scanHasBeenPressed = true;
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
  display_oled.clearDisplay();
  display_oled.setCursor(0, 0);
  display_oled.println("Scan des deck à portée en cours .");
  display_oled.display();
  oledRequestSmall = true;
  dtodServerRequest = true;

  // INIT
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  display_oled.clearDisplay();
  display_oled.setCursor(0, 0);
  display_oled.println("Scan des deck à portée en cours ...");
  display_oled.display();
  oledRequestSmall = true;
  dtodServerRequest = true;

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  if (n == 0)
  {
    display_oled.clearDisplay();
    display_oled.setCursor(0, 0);
    display_oled.println("Aucun DECK a scanner");
    display_oled.display();
    oledRequestSmall = true;
    dtodServerRequest = true;
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

#if DECKINO_DEBUG_SERIAL
        Serial.println("DECK FOUND :");
        Serial.print("NAME : ");
        Serial.println(WiFi.SSID(i).substring(4));
        Serial.print("FORCE : ");
        Serial.println(WiFi.RSSI(i));
#endif

        int force = WiFi.RSSI(i);
        if (force > maxForce)
        {
          maxForce = force;
          closestDeckSsid = WiFi.SSID(i);
          closestDeckName = WiFi.SSID(i).substring(4);
        }
      }
    }
    display_oled.clearDisplay();
    display_oled.setCursor(0, 0);

    String remoteDeckData = "";
    if (maxForce > -999)
    {
      // Old info from deck name
      // remoteData = deckDatabase.getLabelByUid("/dtod.json", closestDeckName).label;
      remoteDeckData = mainMenuActionDtodGetRemoteData(closestDeckSsid);

      deckDatabase.persistFullFile("/temp.json", remoteDeckData);

      String labelToDisplay;
      if (isRemoteScan)
      {
        labelToDisplay = getRemoteScanLabelFromRemoteData();
      }
      else
      {
        labelToDisplay = getDtodLabelFromRemoteData();
      }

      paginableText = new DeckPaginableText(labelToDisplay, display_oled);
      paginableText->render();
      oledRequestAlways = true;
      dtodServerRequest = true;
      hasDtodResultToDisplay = true;

      // Vibration motor
      digitalWrite(PIN_VIBRATION_MOTOR, HIGH);
      lastVibrationMotorStartTime = millis();
    }
  }
  WiFi.disconnect();
}

String getDtodLabelFromRemoteData(void)
{
  String result = deckDatabase.getMatchingLabelByRange("/pers.json", "dtod_ranges", deckDatabase.getFirstLevelDataByKey("/temp.json", "spore_percent").toInt());
  return result;
}

String getRemoteScanLabelFromRemoteData()
{
  DeckMenuItem selectedMenuItem = mainMenu->getSelected();
  selectedMenuItem.value;

  String remotePlayerId = deckDatabase.getFirstLevelDataByKey("/temp.json", "player_id");
  String result = deckDatabase.getThirdLevelDataByKeys("/pers.json", "rmt_scan", selectedMenuItem.value, remotePlayerId);
  return result;
}

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
#if DECKINO_DEBUG_SERIAL
    Serial.println("[mainMenuActionDtodGetRemoteData] Waiting for connexion on SSID:" + closestDeckSsid + " pass:" + "aaaaaaaa");
#endif
  }

#if DECKINO_DEBUG_SERIAL
  Serial.println("[mainMenuActionDtodGetRemoteData][HTTP] begin...");
#endif
  if (http.begin(client, "http://192.168.4.1/"))
  { // HTTP

#if DECKINO_DEBUG_SERIAL
    Serial.println("[mainMenuActionDtodGetRemoteData][HTTP] GET...");
#endif
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
// HTTP header has been send and Server response header has been handled
#if DECKINO_DEBUG_SERIAL
      Serial.printf("[mainMenuActionDtodGetRemoteData][HTTP] GET... code: %d\n", httpCode);
#endif

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        result = http.getString();
#if DECKINO_DEBUG_SERIAL
        Serial.println("[mainMenuActionDtodGetRemoteData][DATA RECIEVED]" + result);
#endif
      }
    }
    else
    {
#if DECKINO_DEBUG_SERIAL
      Serial.printf("[mainMenuActionDtodGetRemoteData][HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
#endif
    }

    http.end();
  }
  else
  {

#if DECKINO_DEBUG_SERIAL
    Serial.printf("[mainMenuActionDtodGetRemoteData][HTTP] Unable to connect\n");
#endif
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
  paginableText->render();
  oledRequestAlways = true;
  dtodServerRequest = true;

  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
  uint8_t uidLength;                     // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success)
  {
    // Display some basic information about the card

#if DECKINO_DEBUG_SERIAL
    Serial.print("Label from JSON: ");
#endif

    DeckScanResult scanResult = deckDatabase.getLabelByUid("/stim.json", rfidUidBufferToString(uid));
    if (deckDatabase.getFieldValueByUid("/stim.json", rfidUidBufferToString(uid), "usable") == "true")
    {
      isScanUsable = true;
    }
    else
    {
      isScanUsable = false;
    }

#if DECKINO_DEBUG_SERIAL
    Serial.println(scanResult.label);
    Serial.println("");
#endif

    // Vibration motor
    digitalWrite(PIN_VIBRATION_MOTOR, HIGH);
    lastVibrationMotorStartTime = millis();

    paginableText = new DeckPaginableText(scanResult.label, display_oled);
    paginableText->render();
    oledRequestSmall = true;
    dtodServerRequest = true;

    rfidUidBufferToStringLastValue = rfidUidBufferToString(uid);
  }
}

void useScanAction(void)
{
  // Vibration motor
  digitalWrite(PIN_VIBRATION_MOTOR, HIGH);
  lastVibrationMotorStartTime = millis();

  // Old behaviour. Log obselete.
  // deckDatabase.appendUsedStimLog("/used_stim_log.json", deckDatabase.getFieldValueByUid("/stim.json", rfidUidBufferToStringLastValue, "stim_code"));

#if DECKINO_DEBUG_SERIAL
  Serial.print("[SPORE](Before use) : ");
  Serial.print("- Spore Max : ");
  Serial.print(deckDatabase.getFirstLevelDataByKey("/pers.json", "spore_max"));
  Serial.print(" - Spore Actuel : ");
  Serial.print(deckDatabase.getFirstLevelDataByKey("/pers.json", "spore_actuel"));
  Serial.println("");
#endif

  deckDatabase.persistFirstLevelDataByKeyValue("/pers.json", "spore_actuel",
                                               String(constrain(deckDatabase.getFirstLevelDataByKey("/pers.json", "spore_actuel").toInt() + deckDatabase.getFieldValueByUid("/stim.json", rfidUidBufferToStringLastValue, "spore").toInt(), 0,
                                                                deckDatabase.getFirstLevelDataByKey("/pers.json", "spore_max").toInt())));

  String useScanActionTextToDisplay = deckDatabase.getFieldValueByUid("/stim.json", rfidUidBufferToStringLastValue, "effect");
  if (useScanActionTextToDisplay == "")
  {
    useScanActionTextToDisplay = USE_SCAN_ACTION_DEFAULT_MESSAGE;
  }

  paginableText = new DeckPaginableText(useScanActionTextToDisplay, display_oled);
  paginableText->render();
  oledRequestSmall = true;
  dtodServerRequest = true;

  rfidUidBufferToStringLastValue = "";

  sporulationEffectAfterUseScanActionText = deckDatabase.getMatchingLabelByRange("/pers.json", "spore_ranges", utilGetCurrentSporePercent());

#if DECKINO_DEBUG_SERIAL
  Serial.print("[SPORE](After use) : ");
  Serial.print("- Spore Max : ");
  Serial.print(deckDatabase.getFirstLevelDataByKey("/pers.json", "spore_max"));
  Serial.print(" - Spore Actuel : ");
  Serial.print(deckDatabase.getFirstLevelDataByKey("/pers.json", "spore_actuel"));
  Serial.println("");
  Serial.print("Range Label : ");
  Serial.print(sporulationEffectAfterUseScanActionText);
  Serial.println("");
#endif
}

int utilGetCurrentSporePercent(void)
{
  return round(deckDatabase.getFirstLevelDataByKey("/pers.json", "spore_actuel").toInt() * 100 / deckDatabase.getFirstLevelDataByKey("/pers.json", "spore_max").toInt());
}

void sporulationEffectAfterUseScanAction(void)
{
  // Vibration motor
  digitalWrite(PIN_VIBRATION_MOTOR, HIGH);
  lastVibrationMotorStartTime = millis();

  paginableText = new DeckPaginableText(sporulationEffectAfterUseScanActionText, display_oled);
  paginableText->render();
  oledRequestSmall = true;
  dtodServerRequest = true;

  sporulationEffectAfterUseScanActionText = "";
}

void confirmBeforeEnterCharacterNumberAction(void)
{
  confirmationPopUp = new DeckConfirmationPopUp("Definir id personnage ?", display_oled);
  confirmationPopUp->render();
  oledRequestAlways = true; // necessary ?
  dtodServerRequest = true;
}

void confirmBeforeUseScanAction(void)
{
  confirmationPopUp = new DeckConfirmationPopUp("Utiliser l'objet ?", display_oled);
  confirmationPopUp->render();
  oledRequestAlways = true;
  dtodServerRequest = true;
}

void enterCharacterNumberAction(void)
{
  choiceNumberPopUp = new DeckChoiceNumberPopUp(display_oled, deckDatabase.getFirstLevelDataByKey("/config.json", "player_id").toInt());
  choiceNumberPopUp->render();
}

void tryToUpdateStimOkButtonAction(void)
{
  deckDatabase.persistFirstLevelDataByKeyValue("/config.json", "player_id", String(choiceNumberPopUp->getFinalValue()));

  String paddedPlayerId = utilZeroPadPlayerId(deckDatabase.getFirstLevelDataByKey("/config.json", "player_id"));

  paginableText = new DeckPaginableText("DOWN ID" + paddedPlayerId + "...", display_oled);
  paginableText->render();

  DeckMthrClient *mthrClient = new DeckMthrClient(deckDatabase.getFirstLevelDataByKey("/wifi.json", "mthr_ssid"), deckDatabase.getFirstLevelDataByKey("/wifi.json", "mthr_password"), "http://192.168.0.8:8080");

  // DOWNLOAD STIM.JSON

  RessourceResponse motherResponse = mthrClient->DownloadRessource("/HTTP/JSON/" + paddedPlayerId + "/STIM.JSON");

  String userDisplayMessage = "";
  if (motherResponse.httpCode == 404)
  {
    userDisplayMessage = "STIM : PERSONNAGE NOT FOUND";
  }
  else if (motherResponse.httpCode != 200)
  {
    userDisplayMessage = "STIM : NETWORK ERROR : " + String(motherResponse.httpCode);
  }
  else
  {
    deckDatabase.persistFullFile("/stim.json", motherResponse.payload);
    userDisplayMessage = "STIM : DATA UPDATED";
  }

  // DOWNLOAD PERS.JSON

  motherResponse = mthrClient->DownloadRessource("/HTTP/JSON/" + paddedPlayerId + "/PERS.JSON");

  if (motherResponse.httpCode == 404)
  {
    userDisplayMessage += "\nPERS : PERSONNAGE NOT FOUND";
  }
  else if (motherResponse.httpCode != 200)
  {
    userDisplayMessage += "\nPERS : NETWORK ERROR : " + String(motherResponse.httpCode);
  }
  else
  {
    deckDatabase.persistFullFile("/pers.json", motherResponse.payload);
    userDisplayMessage += "\nPERS : DATA UPDATED";
  }
  paginableText = new DeckPaginableText("DOWN ID" + paddedPlayerId + "...\n" + userDisplayMessage, display_oled);
  paginableText->render();
}

//-------------------------

void loopStateMainMenu()
{
  if (navigationMachine.executeOnce)
  {
    lastNavigationStateChange = millis();
    setUpMainMenu();
    mainMenu->render();
    oledRequestMedium = true;
    dtodServerRequest = true;
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
#if DECKINO_DEBUG_SERIAL
    Serial.println("[loopStateDisplayDtodResult] Hello");
#endif
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
    #if DECKINO_DEBUG_SERIAL
    Serial.println("[loopStateGenericRemoteScan] Hello");
    #endif
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
#if DECKINO_DEBUG_SERIAL
    Serial.println("[transitionStateMainMenuToDisplayDtodResult] Hello");
#endif
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
    display_oled.clearDisplay();
    display_oled.display();
  }
}

//---

void loopDtodServerStateOn()
{
  if (DtodServerMachine.executeOnce)
  {
    // TODO
    lastDtodServerStateChange = millis();
  }
  // TODO
}

void loopDtodServerStateOff()
{
  if (DtodServerMachine.executeOnce)
  {
    // TODO
  }
  // TODO
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

bool transitionDtodServerStateOnToDtodServerStateOff()
{
  return millis() - lastDtodServerStateChange > 1000 * 60; // Shutdown server 60 seconds after
}

bool transitionDtodServerStateOffToDtodServerStateOn()
{
  if (dtodServerRequest)
  {
    dtodServerRequest = false;
    return true;
  }
  return false;
}