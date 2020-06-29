#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include <albedo/accel/bvh.h>

namespace albedo
{

namespace debug
{

void
writeGraphiz(const accel::BVH& bvh, const std::string& filepath);

} // namespace debug

} // namespace albedo
