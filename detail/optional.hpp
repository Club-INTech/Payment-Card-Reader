//
// Option type with error values implementation
//

#pragma once

#include "utility.hpp"

namespace detail {

// Contains either a value or a status code
// This suboptimal and hackish implementation has been choosen because of a bug with AVR-GCC : union ctors are not
// working
template<typename T, typename Status_T>
class Optional_base {
public:
  Optional_base(const T &object) : m_is_valid{true}, m_content{Status_T::OK} { new (&m_content.object) T{object}; }
  Optional_base(T &&object) : m_is_valid{true}, m_content{Status_T::OK} { new (&m_content.object) T{FWD(object)}; }
  Optional_base(Status_T status) : m_is_valid{false}, m_content{Status_T::OK} {}

  Status_T status() const { return m_content.status; }

  T &operator*() { return m_content.object; }
  T *operator->() { return &m_content.object; }
  operator bool() const { return m_is_valid; }

private:
  bool m_is_valid;
  union Content {
    Status_T status;
    T object;
  } m_content;
};

} // namespace detail
