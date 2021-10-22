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

// Remove cv-qualifier from type
template<typename T>
struct rm_cv {
  using type = T;
};
template<typename T>
struct rm_cv<const T> {
  using type = T;
};
template<typename T>
struct rm_cv<volatile T> {
  using type = T;
};
template<typename T>
struct rm_cv<const volatile T> {
  using type = T;
};
template<typename T>
using rm_cv_t = typename rm_cv<T>::type;

// Value-holding types
template<typename T, T Value>
struct integral_type {
  using type = T;
  constexpr static auto value = Value;
};
struct true_type : integral_type<bool, true> {};
struct false_type : integral_type<bool, false> {};

// Indicate if a type is an bounded array type
template<typename T>
struct is_bounded_array : false_type {};
template<typename T, size_t N>
struct is_bounded_array<T[N]> : true_type {};

// Require an expression to be true
template<bool Condition, typename = int>
struct require_impl {};
template<typename T>
struct require_impl<true, T> {
  using type = T;
};
template<bool Condition, typename T = int>
using require = typename require_impl<Condition, T>::type;

// Require a type to be a bounded array type or not
template<typename T, typename U = int>
using require_bounded_array = require<is_bounded_array<T>::value, U>;
template<typename T, typename U = int>
using require_not_bounded_array = require<!is_bounded_array<T>::value, U>;

// Move a value
template<typename T>
rm_ref_t<T> &&move(T &&x) {
  return static_cast<rm_ref_t<T> &&>(x);
}

// Behave like placement new
// It works for array as well with AVR-GCC by placing every elements
template<typename T, typename... Args, require_not_bounded_array<T> = 0>
void place(T &dest, Args &&... args) {
  new (&dest) T{FWD(args)...};
}
template<typename T, size_t N, typename U, require_bounded_array<rm_cv_t<rm_ref_t<U>>> = 0>
void place(T (&dest)[N], U &&src) {
  static_assert(N == sizeof src / sizeof *src);

  for (size_t i = 0; i < N; i++)
    place(dest[i], FWD(src[i]));
}
