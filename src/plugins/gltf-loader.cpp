#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define STB_IMAGE_IMPLEMENTATION
#include "tiny_gltf.h"

#include <albedoloader/gltf-loader.h>

namespace albedo
{

namespace loader
{

namespace
{

  void
  processNode(Scene& scene, const tinygltf::Node& node)
  {

  }

}

std::optional<Scene>
glTFLoadScene(Renderer& renderer, const std::string& path)
{
  tinygltf::Model model;
  std::string err;
  std::string warn;

  tinygltf::TinyGLTF loader;

  if (!loader.LoadBinaryFromFile(&model, &err, &warn, path))
  {
    std::cout << "Failed to load" << std::endl;
    std::cout << err << std::endl;
    return std::nullopt;
  }

  Scene scene;

  std::cout << "NB SCENES " << model.scenes.size() << std::endl;

  if (model.scenes.size() == 0) {
    std::cout << model.nodes.size() << std::endl;
    for (const auto& node: model.nodes) {
      processNode(scene, node);
    }
    return std::optional{scene};
  }

  for (const auto& glTFScene: model.scenes) {
    for (const auto nodeIndex: glTFScene.nodes) {
      const auto& node = model.nodes[nodeIndex];
      processNode(scene, node);
    }
  }

  return std::optional{scene};
}

} // namespace loader

} // namespace albedo
