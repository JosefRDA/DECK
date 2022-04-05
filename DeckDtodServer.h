#ifndef _DECKDTODSERVER_H_
#define _DECKDTODSERVER_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef DECK_DTOD_SERVER_SSID
#define DECK_DTOD_SERVER_SSID "DECK"
//#define DECK_DTOD_SERVER_PASSWORD  "987L6NU2"
#define DECK_DTOD_SERVER_PASSWORD  "aaaaaaaa"
#endif


class DeckDtodServer {
  public:
  
  // CONSTRUCTORS ------------------------------------------------------------
  
  DeckDtodServer();
  
  // CLASS MEMBER FUNCTIONS ----------------------------------------------

  void handleClient(void);
  void handleRoot(void);

   private:

  // PRIVATE PROPERTIES ----------------------------------------------
  ESP8266WebServer* _webServer;

  // CLASS PRIVATE FUNCTIONS ----------------------------------------------


  
  
};

#endif // end _DECKDTODSERVER_H_
