#include "session.hpp"

using namespace nfc;

// Hardcoded MFRC522 handler, because the program only deals with one reader
static MFRC522 mfrc522{10, 9};

// Initialize the remote MFRC522 chip
void nfc::init_mfrc522() { mfrc522.PCD_Init(); }

// Attempt to make a session
Optional<Session> nfc::make_session(const uint8_t *key) {
  MFRC522::MIFARE_Key mf_key;

  memcpy(mf_key.keyByte, key, sizeof mf_key.keyByte);

  ASSERT(mfrc522.PICC_IsNewCardPresent(), Status::NO_CARD_DETECTED);
  ASSERT(mfrc522.PICC_ReadCardSerial(), Status::NO_UID_RECEIVED);
  ASSERT(mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, 0, &mf_key, &(mfrc522.uid)) != MFRC522::STATUS_OK,
         Status::BAD_KEY);

  return Session{mfrc522.uid.uidByte, mf_key, detail::make_iseq<uid_size>{}};
}
