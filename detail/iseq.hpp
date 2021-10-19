//
// Index pack deduction
//

#pragma once

namespace detail {

// Holds an argument pack of size_t values
template<size_t...>
struct iseq {};

template<size_t I, size_t... Is>
struct make_iseq_impl {
  using type = typename make_iseq_impl<I - 1, I - 1, Is...>::type;
};
template<size_t... Is>
struct make_iseq_impl<0, Is...> {
  using type = iseq<Is...>;
};

template<size_t I>
struct make_iseq : make_iseq_impl<I>::type {};

} // namespace detail
