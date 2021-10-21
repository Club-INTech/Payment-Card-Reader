//
// Utility definitions
//

#pragma once

#define FWD(X) static_cast<decltype(X) &&>(X)

// Forward declarations
void *operator new(size_t, void *);

// Remove reference from type
template<typename T>
struct rm_ref {
  using type = T;
};
template<typename T>
struct rm_ref<T &> {
  using type = T;
};
template<typename T>
struct rm_ref<T &&> {
  using type = T;
};
template<typename T>
using rm_ref_t = typename rm_ref<T>::type;

// Move a value
template<typename T>
rm_ref_t<T> &&move(T &&x) {
  return static_cast<rm_ref_t<T> &&>(x);
}
