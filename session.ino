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

// Check if the PCD is already authenticated to the given sector of the handled PICC
static bool is_authenticated_to(uint8_t trailing_block_index) {
  return is_authenticated && authenticated_to_trailing_block_index == trailing_block_index;
}

// Authenticate to an ACTIVE PICC
static nfc::Status authenticate(uint8_t trailing_block_index, const uint8_t *key) {
  MFRC522::MIFARE_Key mf_key;

  memcpy(mf_key.keyByte, key, sizeof mf_key.keyByte);

  if (is_authenticated) {
    mfrc522.PCD_StopCrypto1();
  }
  if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailing_block_index, &mf_key, &mfrc522.uid) ==
      MFRC522::STATUS_OK) {
    is_authenticated = true;
    authenticated_to_trailing_block_index = trailing_block_index;
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

// Read the content of a block on the PICC
Optional<uint8_t[block_size]> nfc::read(uint8_t block_index, const uint8_t *key) {
  uint8_t buf[block_size];
  uint8_t size = block_size;
  uint8_t trailing_block_index = block_index - (block_index % blocks_per_sector_nb) + 3;

  ASSERT(is_any_picc_active, Status::NO_ACTIVE_PICC);

  if (!is_authenticated_to(trailing_block_index))
    PROPAGATE(authenticate(trailing_block_index, key));
  if (mfrc522.MIFARE_Read(block_index, buf, &size) != MFRC522::STATUS_OK) {
    unauthenticate();
    return Status::READ_ERROR;
  }

  return buf;
}
