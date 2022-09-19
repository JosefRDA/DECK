#include "DeckDatabase.h"


// CONSTRUCTORS ------------------------------------------------------------

DeckDatabase::DeckDatabase() { }

// CLASS MEMBER FUNCTIONS --------------------------------------------------

void DeckDatabase::mountFS(void) {
  Serial.println(F("Mount LittleFS"));
  if (!LittleFS.begin())
  {
    Serial.println(F("LittleFS mount failed"));
    return;
  }
}

void DeckDatabase::printJsonFile(const char * filename){
  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file) 
  {
    Serial.println(F("Failed to open file for reading"));
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println(F("Failed to deserialize file"));
    Serial.println(error.c_str());
  }
  else
  {
    serializeJsonPretty(doc, Serial);
    Serial.println();
  }

  doc.garbageCollect();
  doc.clear();
  // Close the file (File's destructor doesn't close the file)
  file.close();
}

String DeckDatabase::jsonFileToString(const char * filename){
  String result = "";
    // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file) 
  {
    Serial.println(F("Failed to open file for reading"));
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println(F("Failed to deserialize file"));
    Serial.println(error.c_str());
  }
  else
  {
    serializeJsonPretty(doc, result);
  }

  doc.garbageCollect();
  doc.clear();

  // Close the file (File's destructor doesn't close the file)
  file.close();

  return result;
}

void DeckDatabase::listDir(const char * dirname) {
  Serial.printf("Listing directory: %s", dirname);
  Serial.println();

  Dir root = LittleFS.openDir(dirname);

  while (root.next())
  {
    File file = root.openFile("r");
    Serial.print(F("  FILE: "));
    Serial.print(root.fileName());
    Serial.print(F("  SIZE: "));
    Serial.print(file.size());
    Serial.println();
    file.close();
  }

  Serial.println();
}

DeckScanResult DeckDatabase::getLabelByUid(const char * filename, String uid) {
  DeckScanResult result;
  JsonObject stim = getStimByUid(filename, uid);
  if(stim) {
    result.label = stim["label"].as<char*>();
    result.usable = false;
    return result;
  } else {
    result.label = String("[UNKNOWN] :") + uid;
    result.usable = false;
    return result;
  }
}

//TODO/Refactor : Handle return by custom return payload (struct) or exception (better !)
String DeckDatabase::getFieldValueByUid(const char * filename, String uid, String fieldKey) {
  String result;

  JsonObject stim = getStimByUid(filename, uid);
  if(stim) {
    result = stim[fieldKey].as<char*>();
  } else {
    result = String("[UNKNOWN] :") + uid;
  }

  return result;
}

bool DeckDatabase::getUsableByUid(const char * filename, String uid) {
  JsonObject stim = getStimByUid(filename, uid);
  if(stim) {
    return stim["value"].as<bool>();
  } else {
    return false;
  }
}

JsonObject DeckDatabase::getStimByUid(const char * filename, String uid) {
  JsonObject result;

  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file) 
  {
    Serial.println(F("Failed to open file for reading"));
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println(F("Failed to deserialize file"));
    Serial.println(error.c_str());
  }
  else
  {
    JsonArray stimArray = doc.as<JsonArray>();
    for(JsonVariant value : stimArray) {
      JsonObject stimValue = value.as<JsonObject>();
      if(uid.equals(stimValue["uid"].as<char*>())) {
        result = stimValue;
      }
    }
  }

  doc.garbageCollect();
  doc.clear();

  // Close the file (File's destructor doesn't close the file)
  file.close();

  return result;
}

String DeckDatabase::getFirstLevelDataByKey(const char * filename, String fieldKey) {
  String result;

  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file) 
  {
    Serial.println(F("Failed to open file for reading"));
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println(F("Failed to deserialize file"));
    Serial.println(error.c_str());
  }
  else
  {
    JsonObject configArray = doc.as<JsonObject>();
    if(configArray.containsKey(fieldKey)) {
      result = String(configArray[fieldKey].as<char*>());
    } else {
      result = ""; //Return empty string if key not found
    }
  }

  doc.garbageCollect();
  doc.clear();

  file.close();

  return result;
}

String DeckDatabase::getMatchingLabelByRange(const char * filename, String fieldKey, int rangeValue) {
  String result = "";

  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file) 
  {
    Serial.println(F("Failed to open file for reading"));
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println(F("Failed to deserialize file"));
    Serial.println(error.c_str());
  }
  else
  {
    JsonObject rootJson = doc.as<JsonObject>();
    JsonArray rangesArray = rootJson[fieldKey].as<JsonArray>();

    JsonObject matchingRangeJson;
    for(JsonVariant value : rangesArray) {
      JsonObject rangeJson = value.as<JsonObject>();
      if(String(rangeJson["min"].as<char*>()).toInt() <  rangeValue && String(rangeJson["max"].as<char*>()).toInt() >  rangeValue) {
        matchingRangeJson = rangeJson;
        break;
      }
    }

    result = String(matchingRangeJson["label"].as<char*>());
  }

  doc.garbageCollect();
  doc.clear();

  file.close();

  return result;
}

void DeckDatabase::persistFirstLevelDataByKeyValue(const char * filename, String fieldKey, String fieldValue) {
  JsonObject result;

  //If target file not exist, create an empty json file
  if(!LittleFS.exists(filename)) {
    File file = LittleFS.open(filename, "w+");
    file.print(String("{\n}").c_str());
  }

  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file) 
  {
    Serial.println(F("Failed to open file for reading"));
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println(F("Failed to deserialize file"));
    Serial.println(error.c_str());
  }
  else
  {
    JsonObject configArray = doc.as<JsonObject>();
    configArray[fieldKey] = fieldValue;

    file.close();
    file = LittleFS.open(filename, "w");

    if (!file) 
    {
      Serial.println(F("Failed to open file for writing"));
    }

    String targetJson;
    serializeJson(configArray, targetJson);
    file.print(targetJson.c_str());

  }

  doc.garbageCollect();
  doc.clear();

  file.close();
}

void DeckDatabase::persistFullFile(const char * filename, String fileContent) {
  // Open file for reading
  File file = LittleFS.open(filename, "w+");
  if (!file) 
  {
    Serial.println(F("Failed to open file for writing"));
  }

  file.print(fileContent.c_str());

  file.close();
}

// @obselete
void DeckDatabase::appendUsedStimLog(const char * filename, String usableStimCode) {
  //Usage : deckDatabase.appendUsedStimLog("/used_stim_log.json", deckDatabase.getFieldValueByUid("/stim.json", rfidUidBufferToStringLastValue, "stim_code"));
  
  DynamicJsonDocument doc(8192); //TODO : Check Size

  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file) 
  {
    Serial.println(F("Failed to open file for reading"));
  }
  deserializeJson(doc, file);
  file.close();

  // Append new element
  JsonObject obj = doc.createNestedObject();
  char dateString[80]; //enough ?
  sprintf(dateString,"%lu",millis());
  obj["timestamp"] = dateString;
  obj["stim"] = usableStimCode.c_str();


  file = LittleFS.open(filename, "w");

  if (!file) 
  {
    Serial.println(F("Failed to open file for writing"));
  }
  
  String targetJson;
  serializeJson(doc, targetJson);
  file.print(targetJson.c_str());
  serializeJson(doc, file);
  file.close();
}

void DeckDatabase::appendCsvLog(const char * filename, String line) {
  // Open file for appending
  File file = LittleFS.open(filename, "a+");
  if (!file) 
  {
    Serial.println(F("Failed to open file for appending"));
  }

  file.print((String(millis()) + "," + line + "\n").c_str());
  file.close();
  
}

void DeckDatabase::printCsvLog(const char * filename) {
  File file = LittleFS.open(filename, "r");
  if (!file) 
  {
    Serial.println(F("[printCsvLog] Failed to open file for reading"));
  }
  
  while(file.available()) {
    String line = file.readStringUntil('\n');
    Serial.println(line);
  }

  // Close the file (File's destructor doesn't close the file)
  file.close();
}

void DeckDatabase::emptyCsvLog(const char * filename) {
  LittleFS.remove(filename);
  this->appendCsvLog(filename, "LOG CLEARED");
}

LinkedList<String> DeckDatabase::getSubNodesOfAFirstLevelNode(const char * filename, String firstLevelNodeName) {
  const int maxResults = 2; //TODO : to delete once main menu will be abble to take more than 3 items

  LinkedList<String> result = LinkedList<String>();
  
  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file) 
  {
    Serial.println(F("Failed to open file for reading"));
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println(F("Failed to deserialize file"));
    Serial.println(error.c_str());
  }
  else
  {
    JsonObject rootJson = doc.as<JsonObject>();
    JsonArray nodeArray = rootJson[firstLevelNodeName].as<JsonArray>();
    //JsonObject configArray = doc.as<JsonObject>();
    //result = String(configArray[fieldKey].as<char*>());
  
    for(JsonObject nodeArrayObject : nodeArray) {
      for(JsonPair nodeArrayPair : nodeArrayObject) {
        result.add(nodeArrayPair.key().c_str());
        if(result.size() >= maxResults) {
          break;
        }
      }
      if(result.size() >= maxResults) {
        break;
      }
    }
  }

  doc.garbageCollect();
  doc.clear();

  file.close();
  return result;
}

String DeckDatabase::getThirdLevelDataByKeys(const char * filename, String firstLevelKey, String secondLevelKey, String thirdLevelKey) {

  String result = "";
  
  // Open file for reading
  File file = LittleFS.open(filename, "r");
  if (!file) 
  {
    Serial.println(F("Failed to open file for reading"));
  }

  StaticJsonDocument<STATIC_JSON_DOCUMENT_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
  {
    Serial.println(F("Failed to deserialize file"));
    Serial.println(error.c_str());
  }
  else
  {
    JsonObject rootJson = doc.as<JsonObject>();
    JsonArray nodeArray = rootJson[firstLevelKey].as<JsonArray>();
    //JsonObject configArray = doc.as<JsonObject>();
    //result = String(configArray[fieldKey].as<char*>());
  
    for(JsonObject nodeArrayObject : nodeArray) {
      for(JsonPair nodeArrayPair : nodeArrayObject) {
        if(String(nodeArrayPair.key().c_str()) == secondLevelKey) {

          JsonObject rmtScanValues = nodeArrayPair.value().as<JsonObject>();
          if(rmtScanValues.containsKey(thirdLevelKey)) {
            result = String(rmtScanValues[thirdLevelKey].as<char*>());
          } else {
            result = String(rmtScanValues["default"].as<char*>());
          }
          break;
        }
      }
      if(result.length() > 0) {
        break;
      }
    }
  }

  doc.garbageCollect();
  doc.clear();

  file.close();
  return result;
}