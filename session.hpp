//
// PCD - PICC communication
//

#pragma once

#include <MFRC522.h>

#include "detail/iseq.hpp"
#include "detail/optional.hpp"

namespace nfc {

constexpr size_t uid_size = 7;
constexpr size_t block_size = 18;
constexpr size_t blocks_per_sector_nb = 4;

enum class Status { OK, NO_CARD_DETECTED, NO_UID_RECEIVED, BAD_KEY, READ_ERROR };

template<typename T>
using Optional = detail::Optional_base<T, Status>;

// Stores PCD - PICC session information
class Session {
  friend Optional<Session> make_session();

public:
  Optional<uint8_t[block_size]> read(uint8_t, const uint8_t[block_size]) const;

private:
  explicit Session(const MFRC522::Uid &uid) : m_uid{uid} {}

  Status authenticate(uint8_t, const uint8_t[block_size]) const;

  MFRC522::Uid m_uid;
};

void init_mfrc522();
Optional<Session> make_session();

} // namespace nfc
