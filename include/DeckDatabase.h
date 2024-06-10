#ifndef _DECKDATABASE_H_
#define _DECKDATABASE_H_

#include <ArduinoJson.h> // arduino json v6  // https://github.com/bblanchon/ArduinoJson
#include <LittleFS.h>
#include <LinkedList.h>
#include "DeckScanResult.h"

// to upload config file : https://github.com/earlephilhower/arduino-esp8266littlefs-plugin/releases

#define DECKDATABASE_DEBUG_PRINT_FILE_ON_ERROR true
#define SIZE_ARRAY 20
#define MAX_SIZE_CODE 9
#define STATIC_JSON_DOCUMENT_SIZE 2048
#define DECKDATABASE_JSONDYNAMICDOCUMENT_SIZE_TO_LEFT 512

#define DECKDATABASE_FOLDER_SEPARATOR "/"
#define DECKDATABASE_JSON_FILE_EXTENSION ".json"

#define DECKDATABASE_FOLDER_STIM_NAME "stim"
#define DECKDATABASE_DEFAUT_STIM_UID "default"

#define DECKDATABASE_DEBUG_SERIAL true

#ifdef DECKDATABASE_DEBUG_SERIAL
  #define DECKDATABASE_DEBUG_SERIAL_PRINTLN(x) Serial.println(x)
  #define DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST(x) Serial.println(F(x))
  #define DECKDATABASE_DEBUG_SERIAL_NEWLINE() Serial.println()
  #define DECKDATABASE_DEBUG_SERIAL_PRINT(x) Serial.print(x)
  #define DECKDATABASE_DEBUG_SERIAL_PRINT_CST(x) Serial.print(F(x))
  #define DECKDATABASE_DEBUG_SERIAL_PRINTF(x,y) Serial.printf(x,y)
#else
  #define DECKDATABASE_DEBUG_SERIAL_PRINTLN(x)
  #define DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST(x)
  #define DECKDATABASE_DEBUG_SERIAL_NEWLINE()
  #define DECKDATABASE_DEBUG_SERIAL_PRINT(x)
  #define DECKDATABASE_DEBUG_SERIAL_PRINT_CST(x)
  #define DECKDATABASE_DEBUG_SERIAL_PRINTF(x,y)
#endif 


class DeckDatabase {
  private:

    #pragma region singleton
    static DeckDatabase* pInstance; // Static variable holding the pointer to the only instance of this

    // CONSTRUCTORS ------------------------------------------------------------
    
    DeckDatabase();
    #pragma endregion

  public:

    #pragma region singleton
    /**
     * Singletons should not be cloneable.
     */
    DeckDatabase(DeckDatabase &other) = delete;
    /**
     * Singletons should not be assignable.
     */
    void operator=(const DeckDatabase &) = delete;
    
    static DeckDatabase *Instance();
    
    #pragma endregion
    
    // CLASS MEMBER FUNCTIONS ----------------------------------------------

    void mountFS(void);

    void printJsonFile(const char * filename);

    String jsonFileToString(const char * filename);

    void listDir(const char * dirname);

    DeckScanResult getStimResultByUid(String uid);

    String getLabelByUid(String uid);

    String getFieldValueByUid(String uid, String fieldKey);
    
    bool getUsableByUid(String uid);

    JsonObject getStimByUid(String uid);

    void persistFirstLevelDataByKeyValue(const char * filename, String fieldKey, String fieldValue);

    void persistSporeActuel(String fieldValue);

    JsonObject getRessourceByUid(const char *folderPath, String uid);

    bool fileExists(const char *fileFullPath, String debugCallContext = "");

    File openFile(const char *filename, const char *mode, String debugCallContext = "");

    void logDocumentDeserializationError(DeserializationError error, const char *filename, String debugCallContext = "");

    String getFirstLevelDataByKey(const char * filename, String fieldKey);

    String getFirstLevelDataByKey(const char * filename, String fieldKey, String fallback);

    String getMatchingLabelByRange(const char * filename, String fieldKey, int rangeValue);
    
    void persistFullFile(const char * filename, String fileContent);

    // @obselete
    void appendUsedStimLog(const char * filename, String contient);

    LinkedList<String> getSubNodesOfAFirstLevelNode(const char * filename, String firstLevelNodeName);

    String getThirdLevelDataByKeys(const char * filename, String firstLevelKey, String secondLevelKey, String thirdLevelKey);

    void appendCsvLog(const char * filename, String line);

    void rawPrintFile(const char * filename);

    void printCsvLog(const char * filename);

    void emptyCsvLog(const char * filename);
  
};


#endif // end _DECKDATABASE_H_
