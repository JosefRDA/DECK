#ifndef _DECKMTHRCLIENT_H_
#define _DECKMTHRCLIENT_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <CSV_Parser.h>

#include "DeckDatabase.h" //TODO : Exporter les adhérences vers une classe métier intermédiare pour éviter un couplage fort ?

#define DECKMTHRCLIENT_MAX_STIM 500

#define DECKMTHRCLIENT_DEBUG true

#define DECKMTHRCLIENT_DEBUG_SERIAL true

#ifdef DECKMTHRCLIENT_DEBUG_SERIAL()
  #define DECKMTHRCLIENT_DEBUG_SERIAL_PRINT(x) Serial.print(x)
  #define DECKMTHRCLIENT_DEBUG_SERIAL_PRINT_CST(x) Serial.print(F(x))
  #define DECKMTHRCLIENT_DEBUG_SERIAL_PRINTF(x,y) Serial.printf(x,y)
#else
  #define DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN(x)
  #define DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN_CST(x)
  #define DECKMTHRCLIENT_DEBUG_SERIAL_NEWLINE()
  #define DECKMTHRCLIENT_DEBUG_SERIAL_PRINT(x)
  #define DECKMTHRCLIENT_DEBUG_SERIAL_PRINT_CST(x)
  #define DECKMTHRCLIENT_DEBUG_SERIAL_PRINTF(x,y)
#endif 

struct RessourceResponse {
    int httpCode;
    String payload;
};
struct UpdateStimsResponse {
    bool error;
    String userMessage;
};

  #define DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN(x) Serial.println(x)
  #define DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN_CST(x) Serial.println(F(x))
  #define DECKMTHRCLIENT_DEBUG_SERIAL_NEWLINE() Serial.println
class DeckMthrClient {
  public:
  
    // CONSTRUCTORS ------------------------------------------------------------
    
    DeckMthrClient(String mthrSsid, String mthrPassword, String hostName);
    
    // CLASS MEMBER FUNCTIONS ----------------------------------------------

    RessourceResponse DownloadRessource(String relativePath);

    UpdateStimsResponse updateStims(String paddedPlayerId, bool forceUpdate = false);

  private:

    // PRIVATE PROPERTIES ----------------------------------------------
    ESP8266WiFiMulti* _wiFiMulti;
    String _hostName; //with port without final slash
    

    // CLASS PRIVATE FUNCTIONS ----------------------------------------------
    void updateStimIfNeeded(const char * paddedPlayerId, const char * stimUid, int32_t lastUpdateTimestamp, bool forceUpdate = false);
    bool checkIfStimUpdateIsNeeded(const char * stimUid, int32_t lastUpdateTimestamp);
    void updateStim(const char * paddedPlayerId, const char * stimUid);
    RessourceResponse downloadRessourceWithDebug(const String relativePath, const char * debugCallContext);

  
  
};

#endif // end _DECKMTHRCLIENT_H_