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
  Optional_base(const T &object) : m_is_valid{true}, m_content{Status_T::OK} { place(m_content.object, object); }
  Optional_base(T &&object) : m_is_valid{true}, m_content{Status_T::OK} { place(m_content.object, move(object)); }
  Optional_base(Status_T status) : m_is_valid{false}, m_content{status} {}

  // If containing an object, call a functor on the content; call an handler on the status code instead
  template<typename F, typename Handler_F>
  void process(F &&ftor, Handler_F &&handler_ftor) & {
    if (m_is_valid) {
      FWD(ftor)(m_content.object);
    } else {
      FWD(handler_ftor)(m_content.status);
    }
  }
  template<typename F, typename Handler_F>
  void process(F &&ftor, Handler_F &&handler_ftor) && {
    if (m_is_valid) {
      FWD(ftor)(move(m_content.object));
    } else {
      FWD(handler_ftor)(m_content.status);
    }
  }
  template<typename F, typename Handler_F>
  void process(F &&ftor, Handler_F &&handler_ftor) const & {
    if (m_is_valid) {
      FWD(ftor)(m_content.object);
    } else {
      FWD(handler_ftor)(m_content.status);
    }
  }

  operator bool() const { return m_is_valid; }

private:
  bool m_is_valid;
  union Content {
    Status_T status;
    T object;
  } m_content;
};

} // namespace detail
