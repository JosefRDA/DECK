#include "DeckDtodServer.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckDtodServer::DeckDtodServer(Adafruit_SSD1306 oled, DeckDatabase deckDatabase) { 

  _oled = oled;
  this->_deckDatabase = deckDatabase;

  
  // Set your Static IP address
  IPAddress local_IP(192, 168, 1, 184);
  // Set your Gateway IP address
  IPAddress gateway(192, 168, 1, 1);
  
  IPAddress subnet(255, 255, 0, 0);
  IPAddress primaryDNS(8, 8, 8, 8); // optional
  IPAddress secondaryDNS(8, 8, 4, 4); // optional
  
  /* Set these to your desired credentials. */
  String ssidSting =DECK_DTOD_SERVER_SSID_PREFIX +  String("_") + deckDatabase.getFirstLevelDataByKey("/config.json", "player_id");
  const char *ssid = ssidSting.c_str();
  const char *password = DECK_DTOD_SERVER_PASSWORD;
  
  ESP8266WebServer server(80);

  _webServer = new ESP8266WebServer(80);

  delay(1000);
  
  Serial.print("Configuring access point...");
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  std::function<void(void)> handleRootFunction = std::bind(&DeckDtodServer::handleRoot, this);
  _webServer->on("/", handleRootFunction);
  _webServer->begin();
  Serial.println("HTTP server started");

  
}

// CLASS MEMBER FUNCTIONS --------------------------------------------------

void DeckDtodServer::handleClient(void) {
  this->_webServer->handleClient();
}

void DeckDtodServer::handleRoot(void) {
  _oled.clearDisplay();
  _oled.setTextSize(1);
  _oled.setCursor(0,0);

  _oled.drawRoundRect(0, 5, 64, 43, 4, WHITE);

  //_oled.setCursor(4,2);
  //_oled.print("");
  _oled.setCursor(4,12);
  _oled.print("Accepter");
  _oled.setCursor(4,22);
  _oled.print("DTOD ?");
  _oled.setCursor(4,34);
  _oled.setTextColor(BLACK, WHITE);
  _oled.print("OUI");
  _oled.setTextColor(WHITE, BLACK);
  _oled.print("   ");
  _oled.print("NON");
  _oled.display();

  //deprecated : old behaviour
  //this->_webServer->send(200, "application/json", this->_deckDatabase.jsonFileToString("/used_stim_log.json"));
  //this->_deckDatabase.printJsonFile("/used_stim_log.json");

  //Todo : take from utilGetCurrentSporePercent
  int sporePercentInt = round(_deckDatabase.getFirstLevelDataByKey("/pers.json", "spore_actuel").toInt() * 100 / _deckDatabase.getFirstLevelDataByKey("/pers.json", "spore_max").toInt());

  String playerId = _deckDatabase.getFirstLevelDataByKey("/config.json", "player_id");
  String returnPayload = "{ \"spore_percent\" : \"" + String(sporePercentInt) + "\", \"player_id\" : \"" + String(playerId) + "\" }";
  this->_webServer->send(200, "application/json", returnPayload);

  this->_webServer->stop();
}
