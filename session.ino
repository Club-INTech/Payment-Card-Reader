#include "session.hpp"

#include "detail/error.hpp"

using namespace nfc;

// Hardcoded MFRC522 handler, because the program only deals with one reader
static MFRC522 mfrc522{10, 9};

// Indicates if a PICC is in ACTIVE state
static bool is_any_picc_active = false;

// Indicates if the PCD is currently authenticated to a PICC
static bool is_authenticated = false;

// Sector to which the PCD is authenticated
static uint8_t authenticated_to_trailing_block_index;

// Type of the key the PCD is currently authenticated with
static KeyType authenticated_key_type;

// Check if the PCD is already authenticated to the given sector of the handled PICC
static bool is_authenticated_to(uint8_t trailing_block_index, nfc::KeyType key_type) {
  return is_authenticated && authenticated_to_trailing_block_index == trailing_block_index &&
         key_type == authenticated_key_type;
}

// Authenticate to an ACTIVE PICC
static nfc::Status authenticate(uint8_t trailing_block_index, const uint8_t *key, nfc::KeyType key_type) {
  MFRC522::MIFARE_Key mf_key;

  memcpy(mf_key.keyByte, key, sizeof mf_key.keyByte);

  if (mfrc522.PCD_Authenticate(key_type == KeyType::KEY_A ? MFRC522::PICC_CMD_MF_AUTH_KEY_A
                                                          : MFRC522::PICC_CMD_MF_AUTH_KEY_B,
                               trailing_block_index,
                               &mf_key,
                               &mfrc522.uid) == MFRC522::STATUS_OK) {
    is_authenticated = true;
    authenticated_to_trailing_block_index = trailing_block_index;
    authenticated_key_type = key_type;
    return Status::OK;
  } else {
    is_authenticated = false;
    return Status::BAD_KEY;
  }
}

// Unauthenticate to the ACTIVE PICC
static void unauthenticate() {
  is_authenticated = false;
  mfrc522.PCD_StopCrypto1();
}

// Initialize the remote MFRC522 chip
void nfc::init_mfrc522() { mfrc522.PCD_Init(); }

// Select a PICC and make it available to read / write operation
Status nfc::select() {
  MFRC522::MIFARE_Key mf_key;

  // Halting the ACTIVE PICC
  if (is_any_picc_active) {
    ASSERT(mfrc522.PICC_HaltA() == MFRC522::STATUS_OK, Status::HALT_DENIED);
    unauthenticate();
    is_any_picc_active = false;
  }

  // Inviting idling PICCs
  ASSERT(mfrc522.PICC_IsNewCardPresent(), Status::NO_CARD_DETECTED);

  // Try to handle a PICC in READY state, if any available
  ASSERT(mfrc522.PICC_ReadCardSerial(), Status::NO_UID_RECEIVED);

  is_any_picc_active = true;
  return Status::OK;
}

// Get the UID of the ACTIVE PICC
Optional<uint8_t[uid_size]> nfc::get_uid() {
  ASSERT(is_any_picc_active, Status::NO_ACTIVE_PICC);
  return mfrc522.uid.uidByte;
}

// Read the content of a block on the PICC
Optional<uint8_t[block_size + crc_size]> nfc::read(uint8_t block_index, const uint8_t *key, KeyType key_type) {
  uint8_t buf[block_size + crc_size];
  uint8_t size = sizeof buf;
  uint8_t trailing_block_index = block_index - (block_index % blocks_per_sector_nb) + 3;

  ASSERT(is_any_picc_active, Status::NO_ACTIVE_PICC);

  if (!is_authenticated_to(trailing_block_index, key_type))
    PROPAGATE(authenticate(trailing_block_index, key, key_type));
  if (mfrc522.MIFARE_Read(block_index, buf, &size) != MFRC522::STATUS_OK) {
    unauthenticate();
    return Status::READ_ERROR;
  }

  return buf;
}

// Write a block of data to the PICC
Status nfc::write(uint8_t block_index, const uint8_t *key, KeyType key_type, const uint8_t *data) {
  uint8_t buf[block_size];
  uint8_t trailing_block_index = block_index - (block_index % blocks_per_sector_nb) + 3;

  memcpy(buf, data, sizeof buf);

  ASSERT(is_any_picc_active, Status::NO_ACTIVE_PICC);

  if (!is_authenticated_to(trailing_block_index, key_type))
    PROPAGATE(authenticate(trailing_block_index, key, key_type));
  if (mfrc522.MIFARE_Write(block_index, buf, block_size) != MFRC522::STATUS_OK) {
    unauthenticate();
    return Status::WRITE_ERROR;
  }

  return Status::OK;
}

// Format a block as a value block
Status nfc::write_value(uint8_t block_index, const uint8_t *key, KeyType key_type, int32_t value) {
  uint8_t trailing_block_index = block_index - (block_index % blocks_per_sector_nb) + 3;

  ASSERT(is_any_picc_active, Status::NO_ACTIVE_PICC);

  if (!is_authenticated_to(trailing_block_index, key_type))
    PROPAGATE(authenticate(trailing_block_index, key, key_type));
  auto status = mfrc522.MIFARE_SetValue(block_index, value);
  if (status != MFRC522::STATUS_OK) {
    unauthenticate();
    return Status::WRITE_ERROR;
  }

  return Status::OK;
}

// Decrement a value block
Status nfc::decrement(uint8_t block_index, const uint8_t *key, KeyType key_type, int32_t value) {
  uint8_t trailing_block_index = block_index - (block_index % blocks_per_sector_nb) + 3;

  ASSERT(is_any_picc_active, Status::NO_ACTIVE_PICC);

  if (!is_authenticated_to(trailing_block_index, key_type))
    PROPAGATE(authenticate(trailing_block_index, key, key_type));
  if (mfrc522.MIFARE_Decrement(block_index, value) != MFRC522::STATUS_OK) {
    unauthenticate();
    return Status::WRITE_ERROR;
  }
  if (mfrc522.MIFARE_Transfer(block_index) != MFRC522::STATUS_OK) {
    unauthenticate();
    return Status::TRANSFER_ERROR;
  }

  return Status::OK;
}

// Set access right of a sector
// Trailing block rights can't be modified.
Status nfc::set_access(uint8_t sector_index, const uint8_t *key, KeyType key_type, const Access *access_bytes) {
  uint8_t buf[block_size + crc_size];
  uint8_t size = sizeof buf;
  uint8_t trailing_block_index = blocks_per_sector_nb * sector_index + 3;

  ASSERT(is_any_picc_active, Status::NO_ACTIVE_PICC);

  memset(buf, 0xff, key_size);
  memset(buf + block_size - key_size, 0xff, key_size);

  mfrc522.MIFARE_SetAccessBits(buf + key_size,
                               static_cast<uint8_t>(access_bytes[0]),
                               static_cast<uint8_t>(access_bytes[1]),
                               static_cast<uint8_t>(access_bytes[2]),
                               3);
  if (mfrc522.MIFARE_Write(trailing_block_index, buf, block_size) != MFRC522::STATUS_OK) {
    unauthenticate();
    return Status::WRITE_ERROR;
  }

  return Status::OK;
}
