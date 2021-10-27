#include <MFRC522.h>
#include <SPI.h>

#include "detail/error.hpp"
#include "session.hpp"

constexpr uint8_t first_quest_index = 16;

static nfc::Status error_handler(nfc::Status status) {
  using namespace nfc;

  switch (status) {
  case Status::OK:
    break;
  case Status::NO_CARD_DETECTED:
    Serial.println("No card detected");
    break;
  case Status::NO_UID_RECEIVED:
    Serial.println("No uid received");
    break;
  case Status::BAD_KEY:
    Serial.println("Bad key");
    break;
  case Status::NO_ACTIVE_PICC:
    Serial.println("No PICC selected");
    break;
  case Status::HALT_DENIED:
    Serial.println("PICC denied the halt request");
    break;
  case Status::ACCESS_DENIED:
    Serial.println("PICC didn't grant access to this block");
    break;
  case Status::FATAL:
    Serial.println("An unrecoverable error has occured and connection has been terminated");
    break;
  default:
    Serial.print("Unknown error : ");
    Serial.println(static_cast<int>(status));
    break;
  }

  return status;
}

static void print_byte_sequence(const uint8_t *sequence, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (sequence[i] <= 0xf)
      Serial.print('0');
    Serial.print(sequence[i], HEX);
    Serial.print(' ');
  }
  Serial.println();
}

template<size_t N>
static void print_byte_sequence(const uint8_t (&array)[N]) {
  print_byte_sequence(array, N);
}

static nfc::Status configure_picc(uint8_t sector_index) {
  using namespace nfc;

  constexpr static uint8_t key[key_size] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  constexpr Access access_rights[] = {Access::RELOADABLE_VALUE_BLOCK, Access::READ_ONLY, Access::READ_ONLY};

  uint8_t heading_block_index = blocks_per_sector_nb * sector_index;
  auto print = [](const auto &value) { print_byte_sequence(value); };

  PROPAGATE(select());
  PROPAGATE(get_uid().process([](const auto &uid) {
    Serial.print("I'm currently handling PICC ");
    print_byte_sequence(uid);
  }));
  Serial.println("Content of the sector :");
  for (size_t i = heading_block_index; i < heading_block_index + blocks_per_sector_nb; i++) {
    read(i, key, KeyType::KEY_B).process(print);
  }
  Serial.println("Formating the sector...");
  PROPAGATE(set_access(sector_index, key, KeyType::KEY_B, access_rights));
  PROPAGATE(write_value(heading_block_index, key, KeyType::KEY_B, first_quest_index));
  Serial.println("New content of the sector :");
  for (size_t i = heading_block_index; i < heading_block_index + blocks_per_sector_nb; i++) {
    PROPAGATE(read(i, key, KeyType::KEY_B).process(print));
  }

  return Status::OK;
}

static nfc::Status update_quest_counter(uint8_t sector_index, int32_t quest_index, const uint8_t *key) {
  using namespace nfc;

  uint8_t heading_block_index = blocks_per_sector_nb * sector_index;
  int32_t quest_advancement;
  auto store_quest_advancement = [&](const auto &value) { quest_advancement = value; };

  PROPAGATE(read_value(heading_block_index, key, KeyType::KEY_A).process(store_quest_advancement));
  if (quest_advancement == quest_index) {
    PROPAGATE(decrement(heading_block_index, key, KeyType::KEY_A));
    Serial.println("Updated quest advancement !");
  } else if (quest_advancement < quest_index) {
    Serial.print("Already reached step ");
    Serial.println(quest_index);
  } else {
    Serial.print("Haven't fullfilled the prerequisite steps before ");
    Serial.println(quest_index);
  }

  PROPAGATE(read_value(heading_block_index, key, KeyType::KEY_A).process(store_quest_advancement));
  Serial.print("Quest is at stage : ");
  Serial.println(quest_advancement);

  return Status::OK;
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

  constexpr static uint8_t sector_index = 2;
  constexpr static uint8_t key[key_size] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

  delay(500);
  error_handler([&]() {
    PROPAGATE(configure_picc(sector_index));
    PROPAGATE(update_quest_counter(sector_index, 16, key));
    PROPAGATE(update_quest_counter(sector_index, 15, key));
    PROPAGATE(update_quest_counter(sector_index, 14, key));
    PROPAGATE(update_quest_counter(sector_index, 13, key));
    PROPAGATE(update_quest_counter(sector_index, 10, key));
  }());
}
