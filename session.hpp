//
// PCD - PICC communication
//

#pragma once

#include <MFRC522.h>

#include "detail/iseq.hpp"
#include "detail/optional.hpp"

namespace nfc {

constexpr size_t uid_size = 7;
constexpr size_t key_size = 6;
constexpr size_t block_size = 18;
constexpr size_t blocks_per_sector_nb = 4;

enum class Status { OK, NO_CARD_DETECTED, NO_UID_RECEIVED, BAD_KEY, READ_ERROR, NO_ACTIVE_PICC };

template<typename T>
using Optional = detail::Optional_base<T, Status>;

void init_mfrc522();
Status select();
Optional<uint8_t[block_size]> read(uint8_t, const uint8_t *);

} // namespace nfc
