#include "DeckMthrClient.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckMthrClient::DeckMthrClient(String ssid, String password, String hostName) { 
    this->_wiFiMulti = new ESP8266WiFiMulti();
    WiFi.mode(WIFI_STA);
    this->_wiFiMulti->addAP(ssid.c_str(), password.c_str());
    
    this->_hostName = hostName;

    #if DECKMTHRCLIENT_DEBUG
    Serial.println("[DeckMthrClient::DeckMthrClient] BEFORE CONNEXION TEST TO " + ssid + " " + password + " " + hostName);
    
    Serial.println("[DECKMTHRCLIENT]this->_wiFiMulti->run() = " + String(this->_wiFiMulti->run()));
    #endif

    while(this->_wiFiMulti->run() != WL_CONNECTED) {
        #if DECKMTHRCLIENT_DEBUG
        Serial.println("[DeckMthrClient::DeckMthrClient] TESTING CONNEXION");
        #endif
        delay(100);
    }
} 

// CLASS MEMBER FUNCTIONS --------------------------------------------------

RessourceResponse DeckMthrClient::DownloadRessource(String relativePath)  {

    #if DECKMTHRCLIENT_DEBUG
    Serial.println("[DECKMTHRCLIENT]HELLO");
    #endif

    RessourceResponse result;
    result.payload = "";
    result.httpCode = 0;

    #if DECKMTHRCLIENT_DEBUG
    Serial.println("[DECKMTHRCLIENT]BEFORE _wiFiMulti");
    Serial.println("[DECKMTHRCLIENT]this->_wiFiMulti->run() = " + String(this->_wiFiMulti->run()));
    #endif

    if (this->_wiFiMulti->run() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;

        String fullRequest = this->_hostName + relativePath;

        #if DECKMTHRCLIENT_DEBUG
        Serial.println("[DECKMTHRCLIENT]BEFORE http.begin");
        #endif

        if (http.begin(client, fullRequest )) {  // HTTP

            #if DECKMTHRCLIENT_DEBUG
            Serial.print("[DECKMTHRCLIENT][HTTP] GET...\n");
            #endif

            // start connection and send HTTP header
            result.httpCode = http.GET();

            // httpCode will be negative on error
            if (result.httpCode > 0) {
                // HTTP header has been send and Server response header has been handled

                #if DECKMTHRCLIENT_DEBUG
                Serial.printf("[DECKMTHRCLIENT][HTTP] GET... code: %d\n", result.httpCode);
                #endif

                // file found at server
                if (result.httpCode == HTTP_CODE_OK || result.httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                    result.payload = http.getString();
                }
            } else {
                #if DECKMTHRCLIENT_DEBUG
                Serial.printf("[DECKMTHRCLIENT][HTTP] GET... failed, error: %s\n", http.errorToString(result.httpCode).c_str());
                Serial.printf("[DECKMTHRCLIENT]fullRequest: %s\n", fullRequest.c_str());
                #endif
            }

            http.end();
            } else {
                #if DECKMTHRCLIENT_DEBUG
                Serial.printf("[DECKMTHRCLIENT][HTTP] Unable to connect\n");
                #endif
            }

    }
    return result;
} 