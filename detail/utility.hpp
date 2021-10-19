//
// Utility definitions
//

#pragma once

#define FWD(X) static_cast<decltype(X) &&>(X)
#define ASSERT(CONDITION, STATUS)                                                                                      \
  if (!(CONDITION))                                                                                                    \
  return (STATUS)

void *operator new(size_t, void *);
