//
// Error handling
//

#pragma once

#define ASSERT(CONDITION, STATUS)                                                                                      \
  if (!(CONDITION))                                                                                                    \
    return (STATUS);
#define PROPAGATE(EXPR)                                                                                                \
  if (true) {                                                                                                          \
    auto status = (EXPR);                                                                                              \
    if (status != ::nfc::Status::OK)                                                                                   \
      return status;                                                                                                   \
  }
