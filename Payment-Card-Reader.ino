#include <MFRC522.h>
#include <SPI.h>

#include "session.hpp"

constexpr uint8_t key[nfc::uid_size] = {0xff};

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

  auto session_opt = make_session();

  session_opt.process(
      [&](const auto &session) {
        auto data_opt = session.read(0, key);
        data_opt.process(
            [](const auto &data) {
              for (auto c : data)
                Serial.print(c, HEX);
              Serial.println();
            },
            error_handler);
      },
      error_handler);
}
