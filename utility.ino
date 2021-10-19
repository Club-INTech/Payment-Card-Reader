#include "detail/utility.hpp"

// Placement new operator
void *operator new(size_t, void *ptr) { return ptr; }
