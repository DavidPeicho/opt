#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define STB_IMAGE_IMPLEMENTATION
#include <albedoloader/gltf-loader.h>

#include <type_traits>

#include <albedo/components/transform.h>
#include <albedoloader/gltf-loader.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

  template<typename T>
  void
  copyAccessorData(
    std::vector<Mesh::IndexType>& indices,
    const size_t indexOffset,
    const tinygltf::Accessor& accessor,
    const tinygltf::Model& model
  )
  {
    auto [ indicesBuffer, indicesStride ] = getAccessorDataPointerAndStride<T>(
      model, accessor
    );
    for (size_t i = 0; i < accessor.count; ++i)
    {
      indices.push_back(indicesBuffer[indicesStride * i] + indexOffset);
    }
  }

}

std::optional<Scene>
GLTFLoader::load(const std::string& path)
{
  tinygltf::Model model;
  std::string err;
  std::string warn;

  tinygltf::TinyGLTF loader;

  // if (loader.LoadASCIIFromFile(&model, &err, &warn, path))
  if (!loader.LoadBinaryFromFile(&model, &err, &warn, path))
  {
    std::cout << "Failed to load" << std::endl;
    std::cout << err << std::endl;
    return std::nullopt;
  }

  Scene result;

  std::cout << "NB SCENES " << model.scenes.size() << std::endl;

  processMeshes(result, model);

  // TODO: remove when ressource are refactored out of the scene.
  result.addMeshes(m_meshes);

  if (model.scenes.size() == 0) {
    std::cout << model.nodes.size() << std::endl;
    for (const auto& node: model.nodes) {
      processNode(result, node, model);
    }
    return std::make_optional(std::move(result));
  }

  for (const auto& glTFScene: model.scenes) {
    for (const auto nodeIndex: glTFScene.nodes) {
      const auto& node = model.nodes[nodeIndex];
      processNode(result, node, model);
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
      auto [ positionsBuffer, positionStride ] = getAccessorDataPointerAndStride<glm::vec3>(
        model, positionAccessor
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

      const auto& indexAccessor = model.accessors[primitive.indices];
      switch (indexAccessor.componentType)
      {
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
          copyAccessorData<uint8_t>(indices, startIndex, indexAccessor, model);
          break;
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
          copyAccessorData<uint16_t>(indices, startIndex, indexAccessor, model);
					break;
        case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
          copyAccessorData<uint32_t>(indices, startIndex, indexAccessor, model);
					break;
      }
      mesh.setIndices(std::move(indices));
    }
    m_meshes.emplace_back(std::make_shared<Mesh>(std::move(mesh)));
  }
}

void
GLTFLoader::processNode(
  Scene& scene,
  const tinygltf::Node& node,
  const tinygltf::Model& model
)
{
  Instance instance;

  scene.addInstance(instance, InstanceData {
    .name = std::move(node.name) // Name will not be used again, this is safe.
  });

  std::cout << "Processing node " << scene.data<InstanceData>(instance).name << std::endl;

  Transform transform;

  // Process transform.
  if (node.matrix.size() != 0)
  {
    transform.modelToLocal = std::move(glm::make_mat4(&node.matrix[0]));
  }
  else
  {
    const auto& t = node.translation;
    const auto& r = node.rotation;
    const auto& s = node.scale;

    transform.modelToLocal = glm::translate(
      glm::mat4(1.0),
      t.size() > 0 ? glm::vec3(t[0], t[1], t[2]) : glm::vec3(0.0)
    );
    transform.modelToLocal = glm::rotate(
      transform.modelToLocal,
      r.size() > 0 ? glm::quat(r[3], r[0], r[1], r[2])
    );
    transform.modelToLocal = glm::scale(
      transform.modelToLocal,
      s.size() > 0 ? glm::vec3(s[0], s[1], s[2]) : glm::vec3(0.0)
    );
  }

  scene.getTransformManager().createComponent(instance, std::move(transform));

  if (node.mesh >= 0)
  {
    scene.addRenderable(instance, node.mesh);
  }

  // Traverse graph.
  for (const auto& child: node.children)
  {
    processNode(scene, model.nodes[child], model);
  }
}

} // namespace loader

} // namespace albedo
