#include <MFRC522.h>
#include <SPI.h>

#include "session.hpp"

constexpr uint8_t key[nfc::key_size] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static void error_handler(nfc::Status status) {
  using namespace nfc;

  switch (status) {
  case Status::NO_CARD_DETECTED:
    Serial.println("No card detected");
    return;
  case Status::NO_UID_RECEIVED:
    Serial.println("No uid received");
    return;
  case Status::BAD_KEY:
    Serial.println("Bad key");
    return;
  case Status::READ_ERROR:
    Serial.println("Couldn't read the PICC");
    return;
  case Status::NO_ACTIVE_PICC:
    Serial.println("No PICC selected");
    return;
  default:
    Serial.print("Unknown error : ");
    Serial.println(static_cast<int>(status));
    return;
  }
}

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

  select();
  for (auto i = 4; i < 8; i++) {
    read(i, key).process(
        [](const auto &data) {
          for (auto c : data)
            Serial.print(c, HEX);
          Serial.println();
        },
        error_handler);
  }
}
