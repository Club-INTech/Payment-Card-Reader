#include <MFRC522.h>
#include <SPI.h>

#include "session.hpp"

constexpr uint8_t key[nfc::uid_size] = {0xff};

void setup() {
  using namespace nfc;

  Serial.begin(9600);
  SPI.begin();
  init_mfrc522();

  Serial.println("BEGIN");
}

void loop() {
  using namespace nfc;

  delay(500);

  uint8_t buf[18] = {0};
  uint8_t buf_size = sizeof buf;
  Serial.println("hey");

  auto session_opt = make_session(key);

  if (!session_opt) {
    switch (session_opt.status()) {
    case Status::NO_CARD_DETECTED:
      Serial.println("No card detected");
      break;
    case Status::NO_UID_RECEIVED:
      Serial.println("No uid received");
      break;
    case Status::BAD_KEY:
      Serial.println("Bad key");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }
}
