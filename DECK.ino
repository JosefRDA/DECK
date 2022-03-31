
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
DebouncedButton okButton(PIN_BUTTON_OK);
DebouncedButton upButton(PIN_BUTTON_UP);
DebouncedButton downButton(PIN_BUTTON_DOWN);

#include "DeckMenu.h"

unsigned long lastDisplayOledTime = 0;
#define OLED_CLS_DELAY 10000

#define PIN_VIBRATION_MOTOR D5
unsigned long lastVibrationMotorStartTime = 0;
#define VIBRATION_MOTOR_DELAY 2000

#define SSID_PREFIX "Bbox"
//#define SSID_PREFIX "TOTO"

#include "ClusterLogo.h"

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
}

void loopOkButton(void){
  if(okButton.read() == BUTTON_SHORT_PRESS) {
    Serial.println("START SCAN");
    pn532ReadRfidLoop();
    Serial.println("END SCAN");
  }
}

void loopUpButton(void){
  if(upButton.read() == BUTTON_SHORT_PRESS) {
    //Serial.println("UP BUTTON");
    pocScanWifi();
  }
}

void loopDownButton(void){
  uint8_t buttonValue = downButton.read();
  if(buttonValue == BUTTON_SHORT_PRESS) {
    Serial.println("SHORT DOWN BUTTON");

    DeckMenuItem mainMenuItems[] = { 
        { .label = "SCAN", .value = "SCAN", .selected = true },
        { .label = "STIM", .value = "STIM", .selected = false },
        { .label = "DTOD", .value = "DTOD", .selected = false }
      };
    
    DeckMenu mainMenu(mainMenuItems, 3);

    Serial.println(mainMenu.getSelected().label);
    
    // Menu Test
    display_oled.clearDisplay();
    display_oled.setTextSize(1);
    display_oled.setCursor(0,0);
    
    /* OLD MENU
    display_oled.println("+-[DECK]-+");
    display_oled.print("| ");
    display_oled.setTextColor(BLACK, WHITE);
    display_oled.print(" SCAN ");
    display_oled.setTextColor(WHITE, BLACK);
    display_oled.println(" |");
    display_oled.println("|  DTOD  |");
    display_oled.println("|  CONF  |");
    display_oled.println("+--------+");*/

    //MENU
    display_oled.drawRoundRect(0, 5, 64, 43, 4, WHITE);
    display_oled.setCursor(15,2);
    display_oled.setTextColor(WHITE, BLACK);
    display_oled.print(" DECK ");
    display_oled.setCursor(15,13);
    display_oled.setTextColor(WHITE, BLACK);
    display_oled.print(" SCAN ");
    display_oled.setCursor(15,24);
    display_oled.setTextColor(BLACK, WHITE);
    display_oled.print(" DTOD ");
    display_oled.setCursor(15,35);
    display_oled.setTextColor(WHITE, BLACK);
    display_oled.print(" CONF ");
    

    /* CONFIRMATION POP UP
    display_oled.drawRoundRect(0, 0, 64, 48, 4, WHITE);
    display_oled.setCursor(4,2);
    display_oled.print("Diviser");
    display_oled.setCursor(4,12);
    display_oled.print("mes XP");
    display_oled.setCursor(4,22);
    display_oled.print("par 2 ?");
    display_oled.setCursor(4,34);
    display_oled.setTextColor(BLACK, WHITE);
    display_oled.print("OUI");
    display_oled.setTextColor(WHITE, BLACK);
    display_oled.print("   ");
    display_oled.print("NON");
    */
    
    display_oled.display();
  } else { 
    if(buttonValue == BUTTON_LONG_PRESS) {
      Serial.println("LONG DOWN BUTTON");
    } 
  }
}

void pocScanWifi(void) {
  //INIT
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

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
      dtodLabel = deckDatabase.getLabelByUid("/dtod.json", closestDeckName);
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
    String stimLabel = deckDatabase.getLabelByUid("/stim.json", rfidUidBufferToString(uid));
    Serial.println(stimLabel);
    Serial.println("");
    
    display_oled.clearDisplay();
    display_oled.setCursor(0,0);
    display_oled.println(stimLabel);
    display_oled.display();
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
