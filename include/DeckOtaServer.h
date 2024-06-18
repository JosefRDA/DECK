#ifndef _DECKOTASERVER_H_
#define _DECKOTASERVER_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "DeckDatabase.h"

#define DECKOTASERVER_SSID 

#define DECKOTASERVER_DEBUG_SERIAL true

#ifdef DECKOTASERVER_DEBUG_SERIAL
  #define DECKOTASERVER_DEBUG_SERIAL_PRINTLN(x) Serial.println(x)
  #define DECKOTASERVER_DEBUG_SERIAL_PRINTLN_CST(x) Serial.println(F(x))
  #define DECKOTASERVER_DEBUG_SERIAL_NEWLINE() Serial.println()
  #define DECKOTASERVER_DEBUG_SERIAL_PRINT(x) Serial.print(x)
  #define DECKOTASERVER_DEBUG_SERIAL_PRINT_CST(x) Serial.print(F(x))
  #define DECKOTASERVER_DEBUG_SERIAL_PRINTF(x,y) Serial.printf(x,y)
#else
  #define DECKOTASERVER_DEBUG_SERIAL_PRINTLN(x)
  #define DECKOTASERVER_DEBUG_SERIAL_PRINTLN_CST(x)
  #define DECKOTASERVER_DEBUG_SERIAL_NEWLINE()
  #define DECKOTASERVER_DEBUG_SERIAL_PRINT(x)
  #define DECKOTASERVER_DEBUG_SERIAL_PRINT_CST(x)
  #define DECKOTASERVER_DEBUG_SERIAL_PRINTF(x,y)
#endif 


class DeckOtaServer {

  private:

    #pragma region singleton
    static DeckOtaServer* pInstance; // Static variable holding the pointer to the only instance of this

    // CONSTRUCTORS ------------------------------------------------------------
    
    DeckOtaServer();
    #pragma endregion

    // PRIVATE PROPERTIES ------------------------------------------------------

    ESP8266WiFiMulti* _wiFiMulti;
    String _hostName; //with port without final slash

    // PRIVATE FUNCTIONS -------------------------------------------------------

    bool connectToMthr();

  public:

    #pragma region singleton
    /**
     * Singletons should not be cloneable.
     */
    DeckOtaServer(DeckOtaServer &other) = delete;
    /**
     * Singletons should not be assignable.
     */
    void operator=(const DeckOtaServer &) = delete;
    
    static DeckOtaServer *Instance();
    
    #pragma endregion
  
    // CLASS MEMBER FUNCTIONS ----------------------------------------------

    void setup();
    
    void loop();
  
};

#endif // end _DECKOTASERVER_H_
