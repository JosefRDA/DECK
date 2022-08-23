#include "DeckDtodServer.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckDtodServer::DeckDtodServer(Adafruit_SSD1306 oled, DeckDatabase deckDatabase)
{

  _oled = oled;
  this->_deckDatabase = deckDatabase;

  // Set your Static IP address
  IPAddress local_IP(192, 168, 1, 184);
  // Set your Gateway IP address
  IPAddress gateway(192, 168, 1, 1);

  IPAddress subnet(255, 255, 0, 0);
  IPAddress primaryDNS(8, 8, 8, 8);   // optional
  IPAddress secondaryDNS(8, 8, 4, 4); // optional

  String sporeActuelStr = _deckDatabase.getFirstLevelDataByKey("/pers.json", "spore_actuel");
  int sporeActuel = 0;
  if(sporeActuelStr.length() > 0) {
    sporeActuel = sporeActuelStr.toInt();
  } 
  String sporeMaxStr = _deckDatabase.getFirstLevelDataByKey("/pers.json", "spore_actuel");
  int sporeMax = 0; //default 10
  if(sporeMaxStr.length() > 0) {
    sporeMax = sporeMaxStr.toInt();
  } 
  if(sporeMax == 0) {
    sporeMax = 10; //default 10
  }
  int sporePercentInt = round(sporeActuel * 100 / sporeMax);

  //utilZeroPadPlayerId
  char playerIdBuffer[3];
  sprintf(playerIdBuffer, "%03d", deckDatabase.getFirstLevelDataByKey("/config.json", "player_id").toInt());
  String playerId = String(playerIdBuffer);

  char sporePercentBuffer[2];
  sprintf(sporePercentBuffer,"%02X",sporePercentInt + 7);
  String sporePercentStr = String(sporePercentBuffer);
  
  /* Set these to your desired credentials. */
  String ssidSting = DECK_DTOD_SERVER_SSID_PREFIX;
  ssidSting += String("_") + playerId;
  ssidSting += String("_") + sporePercentStr;
  
  const char *ssid = ssidSting.c_str();
  const char *password = DECK_DTOD_SERVER_PASSWORD;



  Serial.println("Configuring access point " + ssidSting + "... ");
  while(!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS))
  {
    Serial.println("STA Failed to configure");
  }

  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);
}

// CLASS MEMBER FUNCTIONS --------------------------------------------------

void DeckDtodServer::handleClient(void)
{
}

void DeckDtodServer::handleRoot(void)
{
}

void DeckDtodServer::close(void)
{
  WiFi.softAPdisconnect(true);
}
