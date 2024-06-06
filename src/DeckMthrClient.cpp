#include "DeckMthrClient.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckMthrClient::DeckMthrClient(String ssid, String password, String hostName) { 
    this->_wiFiMulti = new ESP8266WiFiMulti();
    WiFi.mode(WIFI_STA);
    this->_wiFiMulti->addAP(ssid.c_str(), password.c_str());
    
    this->_hostName = hostName;

    #if DECKMTHRCLIENT_DEBUG
    Serial.println("[DeckMthrClient::DeckMthrClient] BEFORE CONNEXION TEST TO " + ssid + " " + password + " " + hostName);
    
    #endif

    while(this->_wiFiMulti->run() != WL_CONNECTED) {
        #if DECKMTHRCLIENT_DEBUG
        
        Serial.print("[DeckMthrClient::DeckMthrClient] TESTING CONNEXION STATUS : ");
        switch (this->_wiFiMulti->run())
        {
        case WL_NO_SSID_AVAIL:
            Serial.print("NO SSID AVAIL");
            break;
        case WL_CONNECT_FAILED:
            Serial.print("CONNECT FAILED");
            break;
        case WL_CONNECTION_LOST:
            Serial.print("CONNECTION_LOST");
            break;
        case WL_WRONG_PASSWORD:
            Serial.print("WRONG_PASSWORD");
            break;
        case WL_DISCONNECTED:
            Serial.print("DISCONNECTED");
            break;
        
        default:
            Serial.print("OTHER ERROR : " + String(this->_wiFiMulti->run()));
            break;
        }
        #endif
        delay(100);
    }
    #if DECKMTHRCLIENT_DEBUG
    Serial.print("[DeckMthrClient::DeckMthrClient] CONNECTED");
    #endif
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

                    #if DECKMTHRCLIENT_DEBUG
                    Serial.print("[DECKMTHRCLIENT][HTTP] Able to download request payload\n");
                    #endif

                    // get tcp stream
                    WiFiClient *stream = http.getStreamPtr();
                    while(stream->available()){
                        // read char by char to avoid large memory allocation
                        char c = stream->read();
                        result.payload += String(c);

                        #if DECKMTHRCLIENT_DEBUG
                        if(result.payload.length() % 100 == 0){
                            Serial.print("[DECKMTHRCLIENT][HTTP] Reading request payload ...\n");
                        }
                        #endif
                    }

                    #if DECKMTHRCLIENT_DEBUG
                    Serial.print("[DECKMTHRCLIENT][HTTP] Succes : End of request payload reached\n");
                    #endif

                } else {
                    #if DECKMTHRCLIENT_DEBUG
                    Serial.print("[DECKMTHRCLIENT][HTTP] Unable to download request payload\n");
                    #endif
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