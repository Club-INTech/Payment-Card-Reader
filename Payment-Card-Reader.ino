#include <MFRC522.h>
#include <SPI.h>

#include "Arduino-SHA-256/sha256.h"

#include "detail/error.hpp"
#include "display.hpp"
#include "quest.hpp"
#include "session.hpp"

constexpr int debug_pin = 2;
constexpr uint8_t default_key[nfc::key_size] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

struct KeyPair {
  uint8_t key_a[nfc::key_size];
  uint8_t key_b[nfc::key_size];
};

static nfc::Optional<KeyPair> generate_keys() {
  using namespace nfc;

  constexpr static char salt[] = TO_STRING(SALT);

  char uid[uid_size];
  Sha256 sha256;
  KeyPair key_pair;

  PROPAGATE(get_uid().process([&](const auto &data) { memcpy(uid, data, sizeof uid); }));

  sha256.init();
  sha256.print(salt);
  sha256.print(uid);
  memcpy(key_pair.key_a, sha256.result(), sizeof key_pair.key_a);
  memcpy(key_pair.key_b, sha256.result() + sizeof key_pair.key_a, sizeof key_pair.key_b);

  return key_pair;
}

static nfc::Status configure_picc(uint8_t sector_index, uint8_t quests_nb, const uint8_t *key_a, const uint8_t *key_b) {
  using namespace nfc;

  constexpr static Access access_rights[] = {Access::READ_ONLY_A, Access::FREE, Access::FREE};

  uint8_t heading_block_index = blocks_per_sector_nb * sector_index;
  uint8_t block_to_write[block_size] = {};
  auto dump_data = [](const auto &value) { print_byte_sequence(value); };

  PROPAGATE(get_uid().process([](const auto &uid) {
    print("I'm currently handling PICC ");
    print_byte_sequence(uid);
  }));
  println("Content of the sector :");
  for (size_t i = heading_block_index; i < heading_block_index + blocks_per_sector_nb; i++) {
    error_handler(read(i, default_key, KeyType::KEY_B).process(dump_data));
  }
  println("Formating the sector...");
  block_to_write[0] = quests_nb;
  PROPAGATE(set_access(sector_index, default_key, KeyType::KEY_B, access_rights));
  PROPAGATE(write(heading_block_index, default_key, KeyType::KEY_B, block_to_write));
  println("New content of the sector :");
  for (size_t i = heading_block_index; i < heading_block_index + blocks_per_sector_nb; i++) {
    PROPAGATE(read(i, default_key, KeyType::KEY_B).process(dump_data));
  }
#ifdef NDEBUG
  println("Seal sector...");
  seal_sector(sector_index, default_key, KeyType::KEY_B, key_a, key_b, access_rights);
#endif

  return Status::OK;
}

static nfc::Status update_quest_counter(uint8_t sector_index, uint8_t step, const uint8_t *key) {
  using namespace nfc;

  uint8_t heading_block_index = blocks_per_sector_nb * sector_index;
  uint8_t block_to_update[block_size];
  uint8_t &picc_advancement = block_to_update[0];
  auto store_picc_advancement = [&](const auto &block) { memcpy(block_to_update, block, sizeof block_to_update); };

  PROPAGATE(read(heading_block_index, key, KeyType::KEY_B).process(store_picc_advancement));
  if (picc_advancement == step) {
    picc_advancement--;
    PROPAGATE(write(heading_block_index, key, KeyType::KEY_B, block_to_update));
    println("Updated quest advancement !");
  } else if (picc_advancement < step) {
    print("Already reached step ");
    println(step);
  } else {
    print("Haven't fullfilled the prerequisite steps before ");
    println(step);
  }

  PROPAGATE(read(heading_block_index, key, KeyType::KEY_B).process(store_picc_advancement));
  print("Quest is at stage : ");
  println(picc_advancement);

  return Status::OK;
}

static nfc::Status reset_quest_counter(uint8_t sector_index, uint8_t value, const uint8_t *key) {
  using namespace nfc;

  uint8_t heading_block_index = blocks_per_sector_nb * sector_index;
  uint8_t block_to_reset[block_size];

  PROPAGATE(write(heading_block_index, key, KeyType::KEY_B, block_to_reset));

  println("Value written : ");
  PROPAGATE(read(heading_block_index, key, KeyType::KEY_B).process([](const auto &block) { println(block[0]); }));

  return Status::OK;
}

void setup() {
  using namespace nfc;

  pinMode(debug_pin, INPUT_PULLUP);

  Serial.begin(9600);
  enable_display(digitalRead(debug_pin) == LOW);
  SPI.begin();
  init_mfrc522();

  attachInterrupt(
      digitalPinToInterrupt(debug_pin),
      []() {
        if (digitalRead(debug_pin) == LOW) {
          Serial.println("DEBUG MODE ENABLED");
          enable_display(true);
        } else {
          Serial.println("DEBUG MODE DISABLED");
          enable_display(false);
        }
      },
      CHANGE);
}

void loop() {
  using namespace nfc;

  constexpr static uint8_t sector_index = 1;

  uint8_t key_a[key_size];
  uint8_t key_b[key_size];

  if (select() != Status::OK)
    return;

  if (digitalRead(debug_pin) == LOW) {
    error_handler([&]() {
      PROPAGATE(generate_keys().process([&](const auto &key_pair) {
        memcpy(key_a, key_pair.key_a, sizeof key_a);
        memcpy(key_b, key_pair.key_b, sizeof key_b);
      }));

      println("ATTEMPT TO CONFIGURE PICC IF NEEDED");
      auto configure_code = error_handler(configure_picc(sector_index, quests_nb, default_key, key_b));
#ifdef NDEBUG
      ASSERT(configure_code != Status::OK, Status::OK);
#endif

      println("TEST THE QUEST ADVANCEMENT");
      PROPAGATE(select());
      for (size_t i = quests_nb; i > 0; i--)
        PROPAGATE(update_quest_counter(sector_index, i, key_b));

      println("RESET THE QUEST ADVANCEMENT");
      PROPAGATE(reset_quest_counter(sector_index, quests_nb, key_b));

      return Status::OK;
    }());

    delay(500);
  } else {
    [&]() {
      PROPAGATE(generate_keys().process([&](const auto &key_pair) { memcpy(key_a, key_pair.key_a, sizeof key_a); }));
      PROPAGATE(update_quest_counter(sector_index, quest_index, key_a));
    }();
  }
}
