#pragma once

#include <functional>

#include <albedo/utils/index.h>

namespace albedo
{

class Entity: public GlobalIdentifier { };

} // namespace albedo

namespace std
{

template<>
struct hash<albedo::Entity> {
  typedef albedo::Entity argument_type;
  typedef size_t result_type;
  result_type operator()(argument_type const& e) const {
      return e.getValue();
  }
};

} // namespace std
