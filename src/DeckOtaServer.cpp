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
	if(!this->_active) {
		this->connectToMthr(); //TODO : Handle connexion error

		ArduinoOTA.onStart([]() {
			DECKOTASERVER_DEBUG_SERIAL_PRINTLN_CST("[DeckOtaServer::ArduinoOTA.onStart] Start updating ");
		});
		ArduinoOTA.onError([](int code, const char* msg) {
			DECKOTASERVER_DEBUG_SERIAL_PRINT_CST("[DeckOtaServer::ArduinoOTA.onError] ERROR : CODE ");
			DECKOTASERVER_DEBUG_SERIAL_PRINT(code);
			DECKOTASERVER_DEBUG_SERIAL_PRINT_CST(" MSG ");
			DECKOTASERVER_DEBUG_SERIAL_PRINTLN(msg);
		});
		ArduinoOTA.beforeApply([]() {
			DECKOTASERVER_DEBUG_SERIAL_PRINTLN_CST("[DeckOtaServer::ArduinoOTA.beforeApply] Start applying update");
		});

		ArduinoOTA.begin(WiFi.localIP(), "DECK", "Cluster1", InternalStorage);

		this->_active = true;
		DECKOTASERVER_DEBUG_SERIAL_PRINT_CST("[DeckOtaServer::setup] Ready IP address: ");
		DECKOTASERVER_DEBUG_SERIAL_PRINTLN(WiFi.localIP());
	} else {
		DECKOTASERVER_DEBUG_SERIAL_PRINTLN_CST("[DeckOtaServer::setup] ALREADY ACTIVE");
	}
}
	

bool DeckOtaServer::connectToMthr() {
	this->_wiFiMulti = new ESP8266WiFiMulti();

	WiFi.mode(WIFI_STA);

	const String ssid = DeckDatabase::Instance()->getFirstLevelDataByKey("/wifi.json", "mthr_ssid");
	const String password = DeckDatabase::Instance()->getFirstLevelDataByKey("/wifi.json", "mthr_password");

	this->_wiFiMulti->addAP(ssid.c_str(), password.c_str());
	

	DECKOTASERVER_DEBUG_SERIAL_PRINT_CST("[DeckOtaServer::connectToMthr] BEFORE CONNEXION TEST TO ");
	DECKOTASERVER_DEBUG_SERIAL_PRINT(ssid);
	DECKOTASERVER_DEBUG_SERIAL_PRINT_CST(" ");
	DECKOTASERVER_DEBUG_SERIAL_PRINT(password);
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
		delay(DECKOTASERVER_WAITING_FOR_MTHR_WIFI_LOOP_DELAY);
	}
	
	DECKOTASERVER_DEBUG_SERIAL_PRINTLN_CST("[DeckOtaServer::connectToMthr] CONNECTED");
	return true;
}

void DeckOtaServer::loop() {
	if(this->_active) {
		if(millis() - this->_lastLoopTimestamp > DECKOTASERVER_WAITING_FOR_OTA_UPDATE_LOOP_DELAY) {
			if(this->_wiFiMulti->run() == WL_CONNECTED) {
				DECKOTASERVER_DEBUG_SERIAL_PRINTLN_CST("[DeckOtaServer::loop] WAITING FOR OTA UPDATE");
				ArduinoOTA.handle();
			} else {
				DECKOTASERVER_DEBUG_SERIAL_PRINTLN_CST("[DeckOtaServer::loop] NOT CONNECTED TO MTHR");
			}
			this->_lastLoopTimestamp = millis();
		}
	}
}

void DeckOtaServer::stop() {
	if(this->_active) {
		ArduinoOTA.end();
		WiFi.disconnect(true);
		this->_active = false;
		DECKOTASERVER_DEBUG_SERIAL_PRINTLN_CST("[DeckOtaServer::stop] STOPPED");
	} else {
		DECKOTASERVER_DEBUG_SERIAL_PRINTLN_CST("[DeckOtaServer::stop] ALREADY STOPPED");
	}
}

bool DeckOtaServer::isActive() {
	return this->_active;
}