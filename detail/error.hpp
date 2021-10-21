//
// Error handling
//

#pragma once

#define ASSERT(CONDITION, STATUS)                                                                                      \
  if (!(CONDITION))                                                                                                    \
    return (STATUS);
#define PROPAGATE(EXPR)                                                                                                \
  if (true) {                                                                                                          \
    auto nfc__status = (EXPR);                                                                                         \
    if (nfc__status != ::nfc::Status::OK)                                                                              \
      return nfc__status;                                                                                              \
  }
