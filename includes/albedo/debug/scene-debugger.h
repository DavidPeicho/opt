#pragma once

#include <string>
#include <fstream>
#include <sstream>

#include <albedo/accel/bvh.h>
#include <albedo/scene.h>

namespace albedo
{

namespace debug
{

class SceneDebugger
{
  public:

    static void
    writeGraphiz(const accel::BVH& bvh, const std::string& filepath);

    static void
    writeGraphiz(const Scene&, uint32_t meshIndex, const std::string& filepath);

};

} // namespace debug

} // namespace albedo
