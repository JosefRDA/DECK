#include "DeckDtodServer.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckDtodServer::DeckDtodServer() { 
  // Set your Static IP address
  IPAddress local_IP(192, 168, 1, 184);
  // Set your Gateway IP address
  IPAddress gateway(192, 168, 1, 1);
  
  IPAddress subnet(255, 255, 0, 0);
  IPAddress primaryDNS(8, 8, 8, 8); // optional
  IPAddress secondaryDNS(8, 8, 4, 4); // optional
  
  /* Set these to your desired credentials. */
  const char *ssid = DECK_DTOD_SERVER_SSID;
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
  std::function<void(void)> f = std::bind(&DeckDtodServer::handleRoot, this);
  _webServer->on("/", f);
  _webServer->begin();
  Serial.println("HTTP server started");
  
}

// CLASS MEMBER FUNCTIONS --------------------------------------------------

void DeckDtodServer::handleClient(void) {
  this->_webServer->handleClient();
}

void DeckDtodServer::handleRoot(void) {
  this->_webServer->send(200, "text/html", "<h1>You are connected</h1>");
}
