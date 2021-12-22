#include <Adafruit_TinyUSB.h> // for Serial
#include "Adafruit_nRFCrypto.h"
#include "nRF52840_DHKE.h"
#include <ArduinoJson.h>
#include "Secrets.h"

using namespace std;
template class basic_string<char>; // https://github.com/esp8266/Arduino/issues/1136
// Required or the code won't compile!
namespace std _GLIBCXX_VISIBILITY(default) {
_GLIBCXX_BEGIN_NAMESPACE_VERSION
void __throw_bad_alloc() {}
void __throw_length_error(char const*) {
}
void __throw_out_of_range(char const*) {
}
void __throw_logic_error(char const*) {
}
void __throw_out_of_range_fmt(char const*, ...) {
}
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  while (!Serial) delay(10);
  delay(1000);
  DeserializationError error = deserializeJson(doc, pubKeys);
  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  char buff[128];
  JsonObject root = doc.as<JsonObject>();
  for (JsonPair keyValue : root) {
    const char* value = doc[keyValue.key().c_str()];
    if (strcmp((char*)keyValue.key().c_str(), myUUID) == 0) {
      Serial.print(" --> ");
      Serial.print(myUUID);
      Serial.println(": This is me!");
    } else {
      pair<char *, char *>myItem((char*)keyValue.key().c_str(), (char*)value);
      sprintf(buff, "* Inserting %s :: %s\n", (char*)keyValue.key().c_str(), (char*)value);
      Serial.print(buff);
      myBuddies.insert(myItem);
    }
  }
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  
}
