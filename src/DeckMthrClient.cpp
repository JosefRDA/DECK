#include "DeckMthrClient.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckMthrClient::DeckMthrClient(String ssid, String password, String hostName) { 
    this->_wiFiMulti = new ESP8266WiFiMulti();
    WiFi.mode(WIFI_STA);
    this->_wiFiMulti->addAP(ssid.c_str(), password.c_str());
    
    this->_hostName = hostName;
} 

// CLASS MEMBER FUNCTIONS --------------------------------------------------

String DeckMthrClient::DownloadRessource(String relativePath)  {
    String result;
    if (this->_wiFiMulti->run() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;

        String fullRequest = this->_hostName + relativePath;

        if (http.begin(client, fullRequest )) {  // HTTP

            Serial.print("[HTTP] GET...\n");
            // start connection and send HTTP header
            int httpCode = http.GET();

            // httpCode will be negative on error
            if (httpCode > 0) {
                // HTTP header has been send and Server response header has been handled
                Serial.printf("[HTTP] GET... code: %d\n", httpCode);

                // file found at server
                if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
                    result = http.getString();
                }
            } else {
                Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
                Serial.printf("fullRequest: %s\n", fullRequest.c_str());
            }

            http.end();
            } else {
            Serial.printf("[HTTP} Unable to connect\n");
            }

    }
    return result;
} 