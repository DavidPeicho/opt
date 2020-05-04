#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define STB_IMAGE_IMPLEMENTATION
#include "tiny_gltf.h"

#include <albedoloader/gltf-loader.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

namespace albedo
{

namespace loader
{

namespace
{

  void
  processMesh()

  void
  processNode(Scene& scene, const tinygltf::Node& node)
  {
    Instance instance;

    scene.addInstance(instance, InstanceData {
      .name = std::move(node.name) // Name will not be used again, this is safe.
    });

    std::cout << "Processing node " << scene.data(instance).name << std::endl;

    // Process transform.
    if (node.matrix.size() != 0)
    {
      scene.transform(instance) = std::move(glm::make_mat4(&node.matrix[0]));
      std::cout << "Matrix equals to " << glm::to_string(scene.transform(instance)) << std::endl;
    }
    else
    {
      // TODO: handle pos + rot + scale.
    }

    if (node.mesh >= 0)
    {
      
    }


    // Traverse graph.
    for (const auto& child: node.children)
    {
      // TODO
    }
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
