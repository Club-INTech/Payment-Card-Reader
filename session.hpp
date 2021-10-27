//
// PCD - PICC communication
//

#pragma once

#include <MFRC522.h>

#include "detail/iseq.hpp"
#include "detail/optional.hpp"

namespace nfc {

constexpr size_t uid_size = 10;
constexpr size_t key_size = 6;
constexpr size_t block_size = 16;
constexpr size_t crc_size = 2;
constexpr size_t blocks_per_sector_nb = 4;

enum class Status { OK, NO_CARD_DETECTED, NO_UID_RECEIVED, BAD_KEY, NO_ACTIVE_PICC, HALT_DENIED, ACCESS_DENIED, FATAL };

enum class Access {
  FREE = 0b000,
  RAW_READ_WRITE = 0b100,
  RELOADABLE_VALUE_BLOCK = 0b110,
  READ_ONLY = 0b101,
  NONE = 0b111
};

enum class KeyType { KEY_A, KEY_B };

template<typename T>
using Optional = detail::Optional_base<T, Status>;

void init_mfrc522();
Status select();
Optional<uint8_t[uid_size]> get_uid();
Optional<uint8_t[block_size + crc_size]> read(uint8_t, const uint8_t *, KeyType);
Status write(uint8_t, const uint8_t *, KeyType, const uint8_t *);
Optional<int32_t> read_value(uint8_t, const uint8_t *, KeyType);
Status write_value(uint8_t, const uint8_t *, KeyType, int32_t);
Status increment(uint8_t, const uint8_t *, KeyType, int32_t = 1);
Status decrement(uint8_t, const uint8_t *, KeyType, int32_t = 1);
Status set_access(uint8_t, const uint8_t *, KeyType, const Access *);

} // namespace nfc
