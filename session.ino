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
static uint8_t authenticated_to_sector_index;

// Check if the PCD is already authenticated to the given sector of the handled PICC
static bool is_authenticated_to(uint8_t sector_index) {
  return is_authenticated && authenticated_to_sector_index == sector_index;
}

// Authenticate to an ACTIVE PICC
static nfc::Status authenticate(uint8_t sector_index, const uint8_t key[nfc::key_size]) {
  MFRC522::MIFARE_Key mf_key;

  memcpy(mf_key.keyByte, key, key_size);

  if (is_authenticated) {
    mfrc522.PCD_StopCrypto1();
  }
  if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, sector_index, &mf_key, &mfrc522.uid) ==
      MFRC522::STATUS_OK) {
    is_authenticated = true;
    authenticated_to_sector_index = sector_index;
    return Status::OK;
  } else {
    is_authenticated = false;
    return Status::BAD_KEY;
  }
}

// Initialize the remote MFRC522 chip
void nfc::init_mfrc522() { mfrc522.PCD_Init(); }

// Select a PICC and make it available to read / write operation
Status nfc::select() {
  MFRC522::MIFARE_Key mf_key;

  // Inviting idling PICCs
  ASSERT(mfrc522.PICC_IsNewCardPresent(), Status::NO_CARD_DETECTED);

  // Try to handle a PICC in READY state, if any available
  ASSERT(mfrc522.PICC_ReadCardSerial(), Status::NO_UID_RECEIVED);

  is_any_picc_active = true;
  return Status::OK;
}

// Read the content of a block on the PICC
Optional<uint8_t[block_size]> nfc::read(uint8_t block_index, const uint8_t key[key_size]) {
  uint8_t buf[block_size];
  uint8_t size = block_size;
  uint8_t sector_index = block_index / blocks_per_sector_nb;

  ASSERT(is_any_picc_active, Status::NO_ACTIVE_PICC);

  if (!is_authenticated_to(sector_index))
    PROPAGATE(authenticate(sector_index, key));
  ASSERT(mfrc522.MIFARE_Read(block_index, buf, &size) == MFRC522::STATUS_OK, Status::READ_ERROR);

  return buf;
}
