#include <MFRC522.h>
#include <SPI.h>

#include "session.hpp"

constexpr uint8_t key[nfc::key_size] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
constexpr uint8_t block_data[nfc::block_size] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};

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

static void print_byte_sequence(const uint8_t *sequence, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (sequence[i] <= 0xf)
      Serial.print('0');
    Serial.print(sequence[i], HEX);
    Serial.print(' ');
  }
}

template<size_t N>
static void print_byte_sequence(const uint8_t (&array)[N]) {
  print_byte_sequence(array, N);
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

  error_handler(select());
  get_uid().process(
      [](const auto &uid) {
        Serial.print("I'm currently handling PICC ");
        print_byte_sequence(uid);
        Serial.println();
      },
      error_handler);
  for (auto i = 4; i < 8; i++) {
    read(i, key).process(
        [](const auto &data) {
          print_byte_sequence(data);
          Serial.println();
        },
        error_handler);
  }
  Serial.println("Writing the first block of the second sector...");
  error_handler(write(4, key, block_data));
  Serial.println("See what we've got now :");
  for (auto i = 4; i < 8; i++) {
    read(i, key).process(
        [](const auto &data) {
          print_byte_sequence(data);
          Serial.println();
        },
        error_handler);
  }
}
