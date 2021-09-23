#ifndef _DECKDATABASE_H_
#define _DECKDATABASE_H_

#include <ArduinoJson.h> // arduino json v6  // https://github.com/bblanchon/ArduinoJson

// to upload config dile : https://github.com/earlephilhower/arduino-esp8266littlefs-plugin/releases
#define SIZE_ARRAY 20
#define MAX_SIZE_CODE 9

class DeckDatabase {
  public:
  
  // CONSTRUCTORS ------------------------------------------------------------
  
  DeckDatabase();
  
  // CLASS MEMBER FUNCTIONS ----------------------------------------------

  void mountFS(void);

  void printJsonFile(const char * filename);

  void listDir(const char * dirname);

  String getLabelByUid(const char * filename, String uid);

  JsonObject getStimByUid(const char * filename, String uid);
  
  
};



#endif // end _DECKDATABASE_H_
