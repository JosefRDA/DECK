#ifndef _DECKMTHRCLIENT_H_
#define _DECKMTHRCLIENT_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#ifndef DECK_DTOD_SERVER_SSID
#define DECK_DTOD_SERVER_SSID "DECK"
//#define DECK_DTOD_SERVER_PASSWORD  "987L6NU2"
#define DECK_DTOD_SERVER_PASSWORD  "aaaaaaaa"
#endif


class DeckMthrClient {
  public:
  
    // CONSTRUCTORS ------------------------------------------------------------
    
    DeckMthrClient(String mthrSsid, String mthrPassword, String hostName);
    
    // CLASS MEMBER FUNCTIONS ----------------------------------------------

    String DownloadRessource(String relativePath);

  private:

    // PRIVATE PROPERTIES ----------------------------------------------
    ESP8266WiFiMulti* _wiFiMulti;
    String _hostName; //with port without final slash
    

    // CLASS PRIVATE FUNCTIONS ----------------------------------------------


  
  
};

#endif // end _DECKMTHRCLIENT_H_