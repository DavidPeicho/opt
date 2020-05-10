#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define STB_IMAGE_IMPLEMENTATION
#include <albedoloader/gltf-loader.h>

#include <type_traits>

#include <albedo/mesh.h>
#include <albedoloader/gltf-loader.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

namespace albedo
{

namespace loader
{

namespace
{

  template<typename T>
  std::tuple<const T*, size_t>
  getAccessorDataPointerAndStride(
    const tinygltf::Model& model,
    const tinygltf::Accessor& accessor
  )
  {
    const auto& view = model.bufferViews[accessor.bufferView];
    const auto& buffer = model.buffers[view.buffer];
    const auto startByte = accessor.byteOffset + view.byteOffset;
    const T* data = reinterpret_cast<const T*>(&buffer.data[startByte]);
    const auto componentSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);

    assert(startByte % componentSize == 0);

    const int stride = accessor.ByteStride(view);
    assert(stride != -1);

    return std::make_tuple(data, stride / sizeof (T));
  }

}

std::optional<Scene>
GLTFLoader::load(Renderer& renderer, const std::string& path)
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

  Scene result;

  Scene b(std::move(result));

  std::cout << "NB SCENES " << model.scenes.size() << std::endl;

  processMeshes(result, model);

  if (model.scenes.size() == 0) {
    std::cout << model.nodes.size() << std::endl;
    for (const auto& node: model.nodes) {
      processNode(result, node);
    }
    return std::make_optional(std::move(result));
  }

  for (const auto& glTFScene: model.scenes) {
    for (const auto nodeIndex: glTFScene.nodes) {
      const auto& node = model.nodes[nodeIndex];
      processNode(result, node);
    }
  }

  return std::make_optional(std::move(result));
}

void
GLTFLoader::processMeshes(Scene& scene, const tinygltf::Model& model)
{
  for (const auto& gltfMesh: model.meshes)
  {
    Mesh mesh;
    mesh.name(std::move(gltfMesh.name));

    for (const auto& primitive: gltfMesh.primitives)
    {
      std::vector<uint32_t> indices;

      assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

      const auto& positionAccessor = model.accessors[
        primitive.attributes.at("POSITION")
      ];
      // TODO: handle the case where no indices are provided.
      const auto& indexAccessor = model.accessors[primitive.indices];

      auto [ positionsBuffer, positionStride ] = getAccessorDataPointerAndStride<glm::vec3>(
        model, positionAccessor
      );
      auto [ indicesBuffer, indicesStride ] = getAccessorDataPointerAndStride<uint32_t>(
        model, indexAccessor
      );

      uint32_t startIndex = mesh.getVertexBuffer().size();

      for (size_t i = 0; i < positionAccessor.count; ++i)
      {
        Vertex v;
        v.position = positionsBuffer[positionStride * i];
        // TODO: handle normal and UVs
        // TODO: compute face normal if no normal provided.
        mesh.getVertexBuffer().emplace_back(std::move(v));
      }

      for (size_t i = 0; i < indexAccessor.count; ++i)
      {
        indices.push_back(indicesBuffer[indicesStride * i] + startIndex);
      }

      mesh.addPrimitive(std::move(indices));
    }
    m_meshes.emplace_back(std::make_shared<Mesh>(std::move(mesh)));
  }
}

void
GLTFLoader::processNode(Scene& scene, const tinygltf::Node& node)
{
  Instance instance;

  scene.addInstance(instance, InstanceData {
    .name = std::move(node.name) // Name will not be used again, this is safe.
  });

  std::cout << "Processing node " << scene.data<InstanceData>(instance).name << std::endl;

  // Process transform.
  if (node.matrix.size() != 0)
  {
    scene.data<Transform>(instance).modelToLocal = std::move(glm::make_mat4(&node.matrix[0]));
  }
  else
  {
    // TODO: handle pos + rot + scale.
  }

  if (node.mesh >= 0)
  {
    scene.addMesh(instance, m_meshes[node.mesh]);
  }

  // Traverse graph.
  for (const auto& child: node.children)
  {
    // TODO
  }
}

} // namespace loader

} // namespace albedo
