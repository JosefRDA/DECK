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

  StaticJsonDocument<1024> doc;

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

  // Close the file (File's destructor doesn't close the file)
  file.close();
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

String DeckDatabase::getLabelByUid(const char * filename, String uid) {
  JsonObject stim = getStimByUid(filename, uid);
  if(stim) {
    return stim["label"].as<char*>();
  } else {
    return String("[NULL]");
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

  StaticJsonDocument<1024> doc;

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

  // Close the file (File's destructor doesn't close the file)
  file.close();

  return result;
}
