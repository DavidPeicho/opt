#pragma once

#include <albedo/utils/index.h>

namespace albedo
{

template <typename Tag>
struct Component
{
  using Id = Index<Tag>;
};

}
