//
// PCD - PICC communication
//

#pragma once

#include <MFRC522.h>

#include "detail/iseq.hpp"
#include "detail/optional.hpp"

namespace nfc {

constexpr size_t uid_size = 7;

enum class Status { OK, NO_CARD_DETECTED, NO_UID_RECEIVED, BAD_KEY };

template<typename T>
using Optional = detail::Optional_base<T, Status>;

// Stores PCD - PICC session information
class Session {
public:
  friend Optional<Session> make_session(const uint8_t *);

private:
  template<size_t... Is>
  explicit Session(const uint8_t *uid, const MFRC522::MIFARE_Key &mf_key, detail::iseq<Is...>)
      : m_uid{uid[Is]...}, m_mf_key{mf_key} {}

  uint8_t m_uid[uid_size];
  MFRC522::MIFARE_Key m_mf_key;
};

void init_mfrc522();
Optional<Session> make_session(const uint8_t *);

} // namespace nfc
