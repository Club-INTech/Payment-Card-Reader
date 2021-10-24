#include <MFRC522.h>
#include <SPI.h>

#include "session.hpp"

constexpr uint8_t sector_index = 2;
constexpr uint8_t heading_block_index = 8;
constexpr uint8_t value_block_index = 9;

constexpr uint8_t key[nfc::key_size] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
constexpr uint8_t block_data[nfc::block_size] = {
    0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
constexpr int32_t value_block_data = 64;
constexpr nfc::Access access_rights[] = {nfc::Access::RELOADABLE_VALUE_BLOCK, nfc::Access::FREE, nfc::Access::FREE};

static void error_handler(nfc::Status status) {
  using namespace nfc;

  switch (status) {
  case Status::OK:
    return;
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
  case Status::WRITE_ERROR:
    Serial.println("Couldn't write the PICC");
    return;
  case Status::NO_ACTIVE_PICC:
    Serial.println("No PICC selected");
    return;
  case Status::HALT_DENIED:
    Serial.println("PICC denied the halt request");
    return;
  case Status::TRANSFER_ERROR:
    Serial.println("Couldn't transfer staged changed on PICC");
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

  auto status = select();
  if (status != Status::OK) {
    error_handler(status);
    return;
  }
  get_uid().process(
      [](const auto &uid) {
        Serial.print("I'm currently handling PICC ");
        print_byte_sequence(uid);
        Serial.println();
      },
      error_handler);
  for (size_t i = heading_block_index; i < heading_block_index + blocks_per_sector_nb; i++) {
    read(i, key, KeyType::KEY_B)
        .process(
            [](const auto &data) {
              print_byte_sequence(data);
              Serial.println();
            },
            error_handler);
  }
  Serial.println("Writing the first block of the second sector...");
  error_handler(write(heading_block_index, key, KeyType::KEY_B, block_data));
  Serial.println("See what we've got now :");
  for (size_t i = heading_block_index; i < heading_block_index + blocks_per_sector_nb; i++) {
    read(i, key, KeyType::KEY_B)
        .process(
            [](const auto &data) {
              print_byte_sequence(data);
              Serial.println();
            },
            error_handler);
  }
  Serial.println("Making the second block a value block...");
  error_handler(write_value(value_block_index, key, KeyType::KEY_B, value_block_data));
  Serial.println("See what we've got now :");
  for (size_t i = heading_block_index; i < heading_block_index + blocks_per_sector_nb; i++) {
    read(i, key, KeyType::KEY_B)
        .process(
            [](const auto &data) {
              print_byte_sequence(data);
              Serial.println();
            },
            error_handler);
  }
  Serial.println("Setting access of non-trailing blocks...");
  error_handler(set_access(sector_index, key, KeyType::KEY_B, access_rights));
  Serial.println("Decrementing the first block of the second sector 5 times...");
  error_handler(decrement(value_block_index, key, KeyType::KEY_B, 5));
  Serial.println("See what we've got now :");
  for (size_t i = heading_block_index; i < heading_block_index + blocks_per_sector_nb; i++) {
    read(i, key, KeyType::KEY_B)
        .process(
            [](const auto &data) {
              print_byte_sequence(data);
              Serial.println();
            },
            error_handler);
  }
}
