#include "session.hpp"

#include "detail/error.hpp"

// Hardcoded MFRC522 handler, because the program only deals with one reader
static MFRC522 mfrc522{10, 9};

// Indicates if a session is currently active
static bool is_any_session_active;

// Read the content of a block on the PICC
nfc::Optional<uint8_t[nfc::block_size]> nfc::Session::read(uint8_t block_index,
                                                           const uint8_t key[nfc::block_size]) const {
  uint8_t buf[block_size];
  uint8_t size = block_size;

  PROPAGATE(authenticate(block_index / blocks_per_sector_nb, key));
  ASSERT(mfrc522.MIFARE_Read(block_index, buf, &size) != MFRC522::STATUS_OK, Status::READ_ERROR);

  return move(buf);
}

// Authenticate to the designated sector of the PICC
nfc::Status nfc::Session::authenticate(uint8_t sector_index, const uint8_t key[block_size]) const {
  MFRC522::MIFARE_Key mf_key;

  memcpy(mf_key.keyByte, key, sizeof mf_key.keyByte);

  if (is_any_session_active)
    mfrc522.PCD_StopCrypto1();
  if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, sector_index, &mf_key, &m_uid) == MFRC522::STATUS_OK) {
    is_any_session_active = true;
    return Status::OK;
  } else {
    is_any_session_active = false;
    return Status::BAD_KEY;
  }
}

// Initialize the remote MFRC522 chip
void nfc::init_mfrc522() { mfrc522.PCD_Init(); }

// Attempt to make a session out of a not yet handled PICC (ie, PICC in IDLE state)
nfc::Optional<nfc::Session> nfc::make_session() {
  MFRC522::MIFARE_Key mf_key;

  ASSERT(mfrc522.PICC_IsNewCardPresent(), Status::NO_CARD_DETECTED);
  ASSERT(mfrc522.PICC_ReadCardSerial(), Status::NO_UID_RECEIVED);

  return Session{mfrc522.uid};
}
