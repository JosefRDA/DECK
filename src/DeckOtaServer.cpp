#include "DeckOtaServer.h"

#pragma region singleton

DeckOtaServer* DeckOtaServer::pInstance = nullptr;

DeckOtaServer *DeckOtaServer::Instance(){
	//static Cleanup cleanup;
 if(pInstance==nullptr){
        pInstance = new DeckOtaServer();
    }
    return pInstance;
}

#pragma endregion

// CONSTRUCTORS ------------------------------------------------------------

DeckOtaServer::DeckOtaServer() {}

// CLASS MEMBER FUNCTIONS --------------------------------------------------

void DeckOtaServer::setup() {
	this->connectToMthr();
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  // ArduinoOTA.onStart([]() {
  //   Serial.println("Start");
  // });
  // ArduinoOTA.onEnd([]() {
  //   Serial.println("\nEnd");
  // });
  // ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  //   Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  // });
  // ArduinoOTA.onError([](ota_error_t error) {
  //   Serial.printf("Error[%u]: ", error);
  //   if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
  //   else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
  //   else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
  //   else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
  //   else if (error == OTA_END_ERROR) Serial.println("End Failed");
  // });
  ArduinoOTA.begin(WiFi.localIP(), "DECK", "Cluster1", InternalStorage);
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

bool DeckOtaServer::connectToMthr() {
	this->_wiFiMulti = new ESP8266WiFiMulti();

	WiFi.mode(WIFI_STA);

	const String ssid = DeckDatabase::Instance()->getFirstLevelDataByKey("/wifi.json", "mthr_ssid");
	const String password = DeckDatabase::Instance()->getFirstLevelDataByKey("/wifi.json", "mthr_password");
	this->_hostName = DeckDatabase::Instance()->getFirstLevelDataByKey("/wifi.json", "mthr_uri");

	this->_wiFiMulti->addAP(ssid.c_str(), password.c_str());
	

	DECKOTASERVER_DEBUG_SERIAL_PRINT_CST("[DeckOtaServer::connectToMthr] BEFORE CONNEXION TEST TO ");
	DECKOTASERVER_DEBUG_SERIAL_PRINT(ssid);
	DECKOTASERVER_DEBUG_SERIAL_PRINT_CST(" ");
	DECKOTASERVER_DEBUG_SERIAL_PRINT(password);
	DECKOTASERVER_DEBUG_SERIAL_PRINT_CST(" ");
	DECKOTASERVER_DEBUG_SERIAL_PRINTLN(this->_hostName);

	while(this->_wiFiMulti->run() != WL_CONNECTED) {
			
		DECKOTASERVER_DEBUG_SERIAL_PRINT_CST("[DeckOtaServer::connectToMthr] TESTING CONNEXION STATUS : ");
		switch (this->_wiFiMulti->run())
		{
		case WL_NO_SSID_AVAIL:
				DECKOTASERVER_DEBUG_SERIAL_PRINT_CST("NO SSID AVAIL");
				break;
		case WL_CONNECT_FAILED:
				DECKOTASERVER_DEBUG_SERIAL_PRINT_CST("CONNECT FAILED");
				break;
		case WL_CONNECTION_LOST:
				DECKOTASERVER_DEBUG_SERIAL_PRINT_CST("CONNECTION_LOST");
				break;
		case WL_WRONG_PASSWORD:
				DECKOTASERVER_DEBUG_SERIAL_PRINT_CST("WRONG_PASSWORD");
				break;
		case WL_DISCONNECTED:
				DECKOTASERVER_DEBUG_SERIAL_PRINT_CST("DISCONNECTED");
				break;
		
		default:
				DECKOTASERVER_DEBUG_SERIAL_PRINT_CST("OTHER ERROR : ");
				DECKOTASERVER_DEBUG_SERIAL_PRINT(String(this->_wiFiMulti->run()));
				break;
		}
		DECKOTASERVER_DEBUG_SERIAL_PRINTLN();
		delay(100);
	}
	
	DECKOTASERVER_DEBUG_SERIAL_PRINTLN_CST("[DeckOtaServer::connectToMthr] CONNECTED");
	return true;
}