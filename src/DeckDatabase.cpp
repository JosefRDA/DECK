#include "DeckDatabase.h"

// CONSTRUCTORS ------------------------------------------------------------

DeckDatabase::DeckDatabase() {}

// CLASS MEMBER FUNCTIONS --------------------------------------------------

void DeckDatabase::mountFS(void)
{
  DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Mount LittleFS");
  if (!LittleFS.begin())
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("LittleFS mount failed");
    return;
  }
}

void DeckDatabase::printJsonFile(const char *filename)
{
  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to open file for reading");
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to deserialize file");
    DECKDATABASE_DEBUG_SERIAL_PRINTLN(error.c_str());
  }
  else
  {
    serializeJsonPretty(doc, Serial);
    DECKDATABASE_DEBUG_SERIAL_NEWLINE();
  }
  // Close the file (File's destructor doesn't close the file)
  file.close();
}

String DeckDatabase::jsonFileToString(const char *filename)
{
  String result = "";
  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to open file for reading");
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to deserialize file");
    DECKDATABASE_DEBUG_SERIAL_PRINTLN(error.c_str());
  }
  else
  {
    serializeJsonPretty(doc, result);
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();

  return result;
}

void DeckDatabase::listDir(const char *dirname)
{
  DECKDATABASE_DEBUG_SERIAL_PRINTF("Listing directory: %s", dirname);
  DECKDATABASE_DEBUG_SERIAL_NEWLINE();

  Dir root = LittleFS.openDir(dirname);

  while (root.next())
  {
    File file = root.openFile("r");
    DECKDATABASE_DEBUG_SERIAL_PRINT_CST("  FILE: ");
    DECKDATABASE_DEBUG_SERIAL_PRINT(root.fileName());
    DECKDATABASE_DEBUG_SERIAL_PRINT_CST("  SIZE: ");
    DECKDATABASE_DEBUG_SERIAL_PRINT(file.size());
    DECKDATABASE_DEBUG_SERIAL_NEWLINE();
    file.close();
  }

  DECKDATABASE_DEBUG_SERIAL_NEWLINE();
}

DeckScanResult DeckDatabase::getStimResultByUid(String uid)
{
  DeckScanResult result;
  JsonObject stim = this->getStimByUid(uid);
  result.label = stim["label"].as<const char *>();
  result.usable = stim["usable"].as<const char *>() == "true";
  return result;
}

String DeckDatabase::getLabelByUid(String uid)
{
  return this->getStimByUid(uid)["label"].as<const char *>();
}

// TODO/Refactor : Handle return by custom return payload (struct) or exception (better !)
String DeckDatabase::getFieldValueByUid(String uid, String fieldKey)
{
  String result;

  JsonObject stim = this->getStimByUid(uid);
  if (stim)
  {
    result = stim[fieldKey].as<const char *>();
  }
  else
  {
    result = String("[UNKNOWN] :") + uid;
  }

  return result;
}

// unused ?
bool DeckDatabase::getUsableByUid(String uid)
{
  JsonObject stim = this->getStimByUid(uid);
  if (stim)
  {
    return stim["value"].as<bool>();
  }
  else
  {
    return false;
  }
}

JsonObject DeckDatabase::getStimByUid(String uid)
{
  return this->getRessourceByUid(DECKDATABASE_FOLDER_STIM_NAME, uid);
}

JsonObject DeckDatabase::getRessourceByUid(const char *folderPath, String uid) {
  JsonObject result;

  String filename = (String(folderPath) + DECKDATABASE_FOLDER_SEPARATOR + uid + DECKDATABASE_JSON_FILE_EXTENSION);

  // Checking if file exists
  if(this->fileExists(filename.c_str(), __func__)) {
    
    // Open file for reading
    File file = this->openFile(filename.c_str(), "r", __func__);
    if(file) {
       // StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;
      DynamicJsonDocument doc(ESP.getMaxFreeBlockSize() - DECKDATABASE_JSONDYNAMICDOCUMENT_SIZE_TO_LEFT);

      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, file);
      if (error) {
        this->logDocumentDeserializationError(error, filename.c_str(), __func__);
      } 
      else 
      {
        if(doc.is<JsonObject>()) {
          result = doc.as<JsonObject>();
        } else {
          DECKDATABASE_DEBUG_SERIAL_PRINT_CST("[DeckDatabase::getRessourceByUid] ERROR : Result is not a JsonObject !");
          this->rawPrintFile(filename.c_str());
        }
      }
    }
  } else {
    if(uid != DECKDATABASE_DEFAUT_STIM_UID) {
      result = this->getRessourceByUid(folderPath, DECKDATABASE_DEFAUT_STIM_UID);
    } else {
      DECKDATABASE_DEBUG_SERIAL_PRINT_CST("[DeckDatabase::getRessourceByUid] ERROR : No default ressource found !");
    }
  }
  
  return result;
}

//TODO : Transfer to filesystem utils class
bool DeckDatabase::fileExists(const char *fileFullPath, String debugCallContext)
{
  DECKDATABASE_DEBUG_SERIAL_PRINT_CST("[DeckDatabase::");
  DECKDATABASE_DEBUG_SERIAL_PRINT(debugCallContext);
  DECKDATABASE_DEBUG_SERIAL_PRINT_CST("] ");
  bool result = LittleFS.exists(fileFullPath);
  if(result) {
    DECKDATABASE_DEBUG_SERIAL_PRINT(fileFullPath);
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST(" exists");
  } else {
    DECKDATABASE_DEBUG_SERIAL_PRINT(fileFullPath);
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST(" does not exist");
  }
  return result;
}

//TODO : Transfer to filesystem utils class
File DeckDatabase::openFile(const char *filename, const char *mode, String debugCallContext)
{
  File file = LittleFS.open(filename, mode);
  if (!file)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINT_CST("[DeckDatabase::");
    DECKDATABASE_DEBUG_SERIAL_PRINT(debugCallContext);
    DECKDATABASE_DEBUG_SERIAL_PRINT_CST("] Failed to open file with mode ");
    DECKDATABASE_DEBUG_SERIAL_PRINTLN(mode);
  }
  return file;
}

// Only Debug
void DeckDatabase::logDocumentDeserializationError(DeserializationError error, const char *filename, String debugCallContext)
{
  DECKDATABASE_DEBUG_SERIAL_PRINT_CST("[DeckDatabase::");
  DECKDATABASE_DEBUG_SERIAL_PRINT(debugCallContext);
  DECKDATABASE_DEBUG_SERIAL_PRINT_CST("] Failed to deserialize file : ");
  DECKDATABASE_DEBUG_SERIAL_PRINT(String(filename));
  DECKDATABASE_DEBUG_SERIAL_PRINT_CST(" with error : ");
  DECKDATABASE_DEBUG_SERIAL_PRINTLN(error.c_str());
  this->rawPrintFile(filename);
}

String DeckDatabase::getFirstLevelDataByKey(const char *filename, String fieldKey)
{
  return this->getFirstLevelDataByKey(filename, fieldKey, "");
}

String DeckDatabase::getFirstLevelDataByKey(const char *filename, String fieldKey, String fallback)
{
  String result;

  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to open file for reading");
    return fallback;
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to deserialize file");
    DECKDATABASE_DEBUG_SERIAL_PRINTLN(error.c_str());
    return fallback;
  }
  else
  {
    JsonObject configArray = doc.as<JsonObject>();
    if (configArray.containsKey(fieldKey))
    {
      result = String(configArray[fieldKey].as<const char *>());
    }
    else
    {
      result = ""; // Return empty string if key not found
    }
  }

  file.close();

  return result;
}

String DeckDatabase::getMatchingLabelByRange(const char *filename, String fieldKey, int rangeValue)
{
  String result = "";

  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to open file for reading");
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to deserialize file");
    DECKDATABASE_DEBUG_SERIAL_PRINTLN(error.c_str());
  }
  else
  {
    JsonObject rootJson = doc.as<JsonObject>();
    JsonArray rangesArray = rootJson[fieldKey].as<JsonArray>();

    JsonObject matchingRangeJson;
    for (JsonVariant value : rangesArray)
    {
      JsonObject rangeJson = value.as<JsonObject>();
      if (String(rangeJson["min"].as<const char *>()).toInt() < rangeValue && String(rangeJson["max"].as<const char *>()).toInt() > rangeValue)
      {
        matchingRangeJson = rangeJson;
        break;
      }
    }

    result = String(matchingRangeJson["label"].as<const char *>());
  }

  file.close();

  return result;
}

void DeckDatabase::persistSporeActuel(String fieldValue)
{
  const char *filename = "/spor.json";
  const String sporeFiledName = "spore_actuel";
  if (!LittleFS.exists(filename))
  {
    File file = LittleFS.open(filename, "w+");
    file.print(String("{\n\t\"spore_actuel\" : \"" + fieldValue + "\"\n}").c_str());
  }
  else
  {
    this->persistFirstLevelDataByKeyValue(filename, sporeFiledName, fieldValue);
  }
}

void DeckDatabase::persistFirstLevelDataByKeyValue(const char *filename, String fieldKey, String fieldValue)
{
  JsonObject result;

  // If target file not exist, create an empty json file
  if (!LittleFS.exists(filename))
  {
    File file = LittleFS.open(filename, "w+");
    file.print(String("{\n}").c_str());
  }

  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to open file for reading");
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to deserialize file");
    DECKDATABASE_DEBUG_SERIAL_PRINTLN(error.c_str());
  }
  else
  {
    JsonObject configArray = doc.as<JsonObject>();
    configArray[fieldKey] = fieldValue;

    file.close();
    file = LittleFS.open(filename, "w");

    if (!file)
    {
      DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to open file for writing");
    }

    String targetJson;
    serializeJson(configArray, targetJson);
    file.print(targetJson.c_str());
  }

  file.close();
}

//Note : Override File if already exist
void DeckDatabase::persistFullFile(const char *filename, String fileContent)
{
  // Open file for reading
  File file = LittleFS.open(filename, "w+");
  if (!file)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to open file for writing");
  }

  DECKDATABASE_DEBUG_SERIAL_PRINT_CST("[DeckDatabase::persistFullFile] Try to write a ");
  DECKDATABASE_DEBUG_SERIAL_PRINT(String(fileContent.length()));
  DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST(" bytes file");

  file.print(fileContent.c_str());

  DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("[DeckDatabase::persistFullFile] File writen" );

  file.close();
}

// @obselete
void DeckDatabase::appendUsedStimLog(const char *filename, String usableStimCode)
{
  // Usage : deckDatabase.appendUsedStimLog("/used_stim_log.json", deckDatabase.getFieldValueByUid("/stim.json", rfidUidBufferToStringLastValue, "stim_code"));

  DynamicJsonDocument doc(8192); // TODO : Check Size

  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to open file for reading");
  }
  deserializeJson(doc, file);
  file.close();

  // Append new element
  JsonObject obj = doc.createNestedObject();
  char dateString[80]; // enough ?
  sprintf(dateString, "%lu", millis());
  obj["timestamp"] = dateString;
  obj["stim"] = usableStimCode.c_str();

  file = LittleFS.open(filename, "w");

  if (!file)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to open file for writing");
  }

  String targetJson;
  serializeJson(doc, targetJson);
  file.print(targetJson.c_str());
  serializeJson(doc, file);
  file.close();
}

void DeckDatabase::appendCsvLog(const char *filename, String line)
{
  // Open file for appending
  File file = LittleFS.open(filename, "a+");
  if (!file)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to open file for appending");
  }

  file.print((String(millis()) + "," + line + "\n").c_str());
  file.close();
}

//Todo : debug only
void DeckDatabase::rawPrintFile(const char *fileFullPath) {
  #if DECKDATABASE_DEBUG_PRINT_FILE_ON_ERROR
  File file = this->openFile(fileFullPath, "r", __func__);

  if(file) {
    DECKDATABASE_DEBUG_SERIAL_PRINT_CST("=== BEGIN OF FILE [");
    DECKDATABASE_DEBUG_SERIAL_PRINT(fileFullPath);
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("] ===");
    while (file.available())
    {
      String line = file.readStringUntil('\n');
      DECKDATABASE_DEBUG_SERIAL_PRINTLN(line);
    }
    DECKDATABASE_DEBUG_SERIAL_PRINT_CST("=== END OF FILE [");
    DECKDATABASE_DEBUG_SERIAL_PRINT(fileFullPath);
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("] ===");
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();
  #endif
}

//Todo : debug only
void DeckDatabase::printCsvLog(const char *filename)
{
  this->rawPrintFile(filename);
}

void DeckDatabase::emptyCsvLog(const char *filename)
{
  LittleFS.remove(filename);
  this->appendCsvLog(filename, "LOG CLEARED");
}

LinkedList<String> DeckDatabase::getSubNodesOfAFirstLevelNode(const char *filename, String firstLevelNodeName)
{
  const int maxResults = 2; // TODO : to delete once main menu will be abble to take more than 3 items

  LinkedList<String> result = LinkedList<String>();

  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to open file for reading");
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to deserialize file");
    DECKDATABASE_DEBUG_SERIAL_PRINTLN(error.c_str());
  }
  else
  {
    JsonObject rootJson = doc.as<JsonObject>();
    JsonArray nodeArray = rootJson[firstLevelNodeName].as<JsonArray>();
    // JsonObject configArray = doc.as<JsonObject>();
    // result = String(configArray[fieldKey].as<const char*>());

    for (JsonObject nodeArrayObject : nodeArray)
    {
      for (JsonPair nodeArrayPair : nodeArrayObject)
      {
        result.add(nodeArrayPair.key().c_str());
        if (result.size() >= maxResults)
        {
          break;
        }
      }
      if (result.size() >= maxResults)
      {
        break;
      }
    }
  }

  file.close();
  return result;
}

String DeckDatabase::getThirdLevelDataByKeys(const char *filename, String firstLevelKey, String secondLevelKey, String thirdLevelKey)
{

  String result = "";

  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to open file for reading");
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    DECKDATABASE_DEBUG_SERIAL_PRINTLN_CST("Failed to deserialize file");
    DECKDATABASE_DEBUG_SERIAL_PRINTLN(error.c_str());
  }
  else
  {
    JsonObject rootJson = doc.as<JsonObject>();
    JsonArray nodeArray = rootJson[firstLevelKey].as<JsonArray>();
    // JsonObject configArray = doc.as<JsonObject>();
    // result = String(configArray[fieldKey].as<const char*>());

    for (JsonObject nodeArrayObject : nodeArray)
    {
      for (JsonPair nodeArrayPair : nodeArrayObject)
      {
        if (String(nodeArrayPair.key().c_str()) == secondLevelKey)
        {

          JsonObject rmtScanValues = nodeArrayPair.value().as<JsonObject>();
          if (rmtScanValues.containsKey(thirdLevelKey))
          {
            result = String(rmtScanValues[thirdLevelKey].as<const char *>());
          }
          else
          {
            result = String(rmtScanValues["default"].as<const char *>());
          }
          break;
        }
      }
      if (result.length() > 0)
      {
        break;
      }
    }
  }

  file.close();
  return result;
}