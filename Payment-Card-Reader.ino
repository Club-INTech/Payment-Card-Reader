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

  auto session_opt = make_session(key);

  session_opt.process(
      // Regular code
      [&](const auto &) {},
      // Error handler
      [](auto status) {
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
        default:
          Serial.print("Unknown error : ");
          Serial.println(static_cast<int>(status));
          return;
        }
      });
}
