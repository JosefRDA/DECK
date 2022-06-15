#ifndef _DECKDATABASE_H_
#define _DECKDATABASE_H_

#include <ArduinoJson.h> // arduino json v6  // https://github.com/bblanchon/ArduinoJson
#include "DeckScanResult.h"

// to upload config file : https://github.com/earlephilhower/arduino-esp8266littlefs-plugin/releases
#define SIZE_ARRAY 20
#define MAX_SIZE_CODE 9

class DeckDatabase {
  public:
  
  // CONSTRUCTORS ------------------------------------------------------------
  
  DeckDatabase();
  
  // CLASS MEMBER FUNCTIONS ----------------------------------------------

  void mountFS(void);

  void printJsonFile(const char * filename);

  String jsonFileToString(const char * filename);

  void listDir(const char * dirname);

  DeckScanResult getLabelByUid(const char * filename, String uid);

  String getFieldValueByUid(const char * filename, String uid, String fieldKey);
  
  bool getUsableByUid(const char * filename, String uid);

  JsonObject getStimByUid(const char * filename, String uid);

  void persistFirstLevelDataByKeyValue(const char * filename, String fieldKey, String fieldValue);

  String getFirstLevelDataByKey(const char * filename, String fieldKey);

  String getMatchingLabelByRange(const char * filename, String fieldKey, int rangeValue);
  
  void persistFullFile(const char * filename, String fileContent);

  void appendUsedStimLog(const char * filename, String contient);
  
};



#endif // end _DECKDATABASE_H_
