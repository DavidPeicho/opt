#include <iostream>
#include <albedo/scene.h>
#include <albedo/renderer.h>

namespace albedo
{

namespace loader
{

std::optional<Scene>
glTFLoadScene(Renderer& renderer, const std::string& path);

} // nanespace scene

} // nanespace albedo
