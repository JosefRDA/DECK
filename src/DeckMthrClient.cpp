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
        Serial.println();
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
        Serial.print("[DECKMTHRCLIENT]BEFORE http.begin ");
        Serial.print(fullRequest);
        Serial.println();
        #endif

        if (http.begin(client, fullRequest )) {  // HTTP

            #if DECKMTHRCLIENT_DEBUG
            Serial.println("[DECKMTHRCLIENT][HTTP] GET ...");
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

UpdateStimsResponse DeckMthrClient::updateStims(String paddedPlayerId, bool forceUpdate) {
   DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN_CST("[DeckMthrClient::updateStims] begin");
   UpdateStimsResponse result;

   RessourceResponse motherResponse = this->downloadRessourceWithDebug("/HTTP/pers/" + paddedPlayerId + "/stim/list.csv", "updateStims");

   if(motherResponse.httpCode == t_http_codes::HTTP_CODE_OK) {
    //TODO : if force clear whole stim folder

    CSV_Parser stimListCsvParser(motherResponse.payload.c_str(), "sL");

    char **uid = (char**)stimListCsvParser["uid"];
    int32_t *lastUpdateTimestamp = (int32_t*)stimListCsvParser["lastUpdateTimestamp"];

    DECKMTHRCLIENT_DEBUG_SERIAL_PRINT_CST("[DeckMthrClient::updateStims] Number of lines : ");
    DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN(stimListCsvParser.getRowsCount()+1);

    for (int i = 0; i <= stimListCsvParser.getRowsCount(); i++) {
      DECKMTHRCLIENT_DEBUG_SERIAL_PRINT_CST("uid : ");
      DECKMTHRCLIENT_DEBUG_SERIAL_PRINT(uid[i]);
      DECKMTHRCLIENT_DEBUG_SERIAL_PRINT_CST(" - lastUpdateTimestamp : ");
      DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN(lastUpdateTimestamp[i]);

      this->updateStimIfNeeded(paddedPlayerId.c_str(), uid[i], lastUpdateTimestamp[i], forceUpdate);
    }
    result.error = false;
    result.userMessage = "STIMs : DATA UPDATED";
   } else if(motherResponse.httpCode == t_http_codes::HTTP_CODE_NOT_FOUND) {
    result.error = false;
    result.userMessage = "STIMs : PERSONNAGE NOT FOUND";
   } else {
    result.error = true;
    result.userMessage = "STIMs : ERROR HTTP " + String(motherResponse.httpCode);
   }
   
   DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN_CST("[DeckMthrClient::updateStims] end");
   return result;
}

void DeckMthrClient::updateStimIfNeeded(const char * paddedPlayerId, const char * stimUid, int32_t lastUpdateTimestamp, bool forceUpdate) {
    if(forceUpdate || checkIfStimUpdateIsNeeded(stimUid, lastUpdateTimestamp)) {
        this->updateStim(paddedPlayerId, stimUid);
    }
}

//TODO
bool DeckMthrClient::checkIfStimUpdateIsNeeded(const char * stimUid, int32_t lastUpdateTimestamp) {
    return true;
}

void DeckMthrClient::updateStim(const char * paddedPlayerId, const char * stimUid) {
  DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN_CST("[DeckMthrClient::updateStim] begin");

  RessourceResponse motherResponse = this->downloadRessourceWithDebug("/HTTP/pers/" + String(paddedPlayerId) + "/stim/" + String(stimUid) + ".json", "updateStim");

  if(motherResponse.httpCode == t_http_codes::HTTP_CODE_OK) {
    String fileFullPath = "/stim/" + String(stimUid) + ".json";
    DeckDatabase::Instance()->persistFullFile(fileFullPath.c_str(), motherResponse.payload);
  }

  DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN_CST("[DeckMthrClient::updateStim] end");
}

RessourceResponse DeckMthrClient::downloadRessourceWithDebug(const String relativePath, const char * debugCallContext) {
  RessourceResponse motherResponse = this->DownloadRessource(relativePath);

  if (motherResponse.httpCode == t_http_codes::HTTP_CODE_NOT_FOUND)
  {
    DECKMTHRCLIENT_DEBUG_SERIAL_PRINT_CST("[DeckMthrClient::");
    DECKMTHRCLIENT_DEBUG_SERIAL_PRINT(debugCallContext);
    DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN_CST("] HTTP 404 : RESSOURCE NOT FOUND");
  }
  else if (motherResponse.httpCode != t_http_codes::HTTP_CODE_OK)
  {
    DECKMTHRCLIENT_DEBUG_SERIAL_PRINT_CST("[DeckMthrClient::");
    DECKMTHRCLIENT_DEBUG_SERIAL_PRINT(debugCallContext);
    DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN_CST("] HTTP ");
    DECKMTHRCLIENT_DEBUG_SERIAL_PRINT(motherResponse.httpCode);
    DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN_CST(" : ERROR");
  }
  else
  {
    DECKMTHRCLIENT_DEBUG_SERIAL_PRINT_CST("[DeckMthrClient::");
    DECKMTHRCLIENT_DEBUG_SERIAL_PRINT(debugCallContext);
    DECKMTHRCLIENT_DEBUG_SERIAL_PRINTLN_CST("] HTTP 200 : SUCCESS");
  }
  return motherResponse;
}