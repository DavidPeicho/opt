#define JSON_NOEXCEPTION
#define TINYGLTF_NOEXCEPTION
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#include <albedoloader/gltf-loader.h>

#include <filesystem>
#include <type_traits>

#include <albedo/components/material.h>
#include <albedo/components/transform.h>
#include <albedoloader/gltf-loader.h>
#include <glm/common.hpp>
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
  return load(path, LoaderOptions {
    .resizePolicy = ResizePolicy::Smallest
  });
}

std::optional<Scene>
GLTFLoader::load(const std::string& path, const LoaderOptions& opts)
{
  tinygltf::Model model;
  std::string err;
  std::string warn;

  tinygltf::TinyGLTF loader;
  loader.SetPreserveImageChannels(false);

  bool success = false;
  std::filesystem::path filePath = path;
  if (filePath.extension() == ".glb") // Heed the dot.
  {
    success = loader.LoadBinaryFromFile(&model, &err, &warn, path);
  }
  else
  {
    success = loader.LoadASCIIFromFile(&model, &err, &warn, path);
  }

  if (!success)
  {
    std::cout << "[ ERROR ]: Failed to load" << std::endl;
    std::cout << "reason: " << err << std::endl;
    return std::nullopt;
  }

  Scene result;

  processMaterials(result, model);
  processMeshes(result, model);

  // TODO: remove when ressource are refactored out of the scene.
  result.addMeshes(m_meshes);

  for (const auto& glTFScene: model.scenes)
  {
    for (const auto nodeIndex: glTFScene.nodes)
    {
      const auto& node = model.nodes[nodeIndex];
      processNode(result, node, model);
    }
  }

  // Processes all textures.
  TexturesInfo texInfo;
  for (auto& image: model.images)
  {
    // TODO: handle image of different sizes.
    // TODO: tonemap high bit depth
    // TODO: convert float to uchar?
    texInfo.width = image.width;
    texInfo.height = image.height;
    if (image.image.size() == 0)
    {
      return std::nullopt;
    }

    texInfo.textures.emplace_back(std::move(image.image));
  }

  result.setTexturesInfo(std::move(texInfo));

  return std::make_optional(std::move(result));
}

void
GLTFLoader::processMaterials(Scene& scene, const tinygltf::Model& model)
{
  for (const auto& glTFMaterial: model.materials)
  {
    const auto& pbr = glTFMaterial.pbrMetallicRoughness;
    const auto& albedo = pbr.baseColorFactor;
    components::Material material;
    material.color = glm::vec4(albedo[0], albedo[1], albedo[2], albedo[3]);
    if (pbr.baseColorTexture.index >= 0)
    {
      material.albedoIndex = pbr.baseColorTexture.index;
    }
    if (pbr.metallicRoughnessTexture.index >= 0)
    {
      material.metalRoughnessIndex = pbr.metallicRoughnessTexture.index;
    }
    scene.addMaterial(std::move(material));
  }
}

void
GLTFLoader::processMeshes(Scene& scene, const tinygltf::Model& model)
{
  // TODO: add support for primitive and material...
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

      // TODO: compute face normal if no normal provided.
      const auto& normalAccessor = model.accessors[
        primitive.attributes.at("NORMAL")
      ];
      auto [ normalBuffer, normalStride ] = getAccessorDataPointerAndStride<glm::vec3>(
        model, normalAccessor
      );

      auto uvTuple = std::make_tuple<const glm::vec2*, size_t>(nullptr, 0);
      if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
      {
        const auto& uvAccessor = model.accessors[
          primitive.attributes.at("TEXCOORD_0")
        ];
        uvTuple = getAccessorDataPointerAndStride<glm::vec2>(
          model, uvAccessor
        );
      }

      uint32_t startIndex = mesh.getVertexBuffer().size();

      for (size_t i = 0; i < positionAccessor.count; ++i)
      {
        Vertex v;
        v.position = positionsBuffer[positionStride * i];
        v.normal = normalBuffer[normalStride * i];
        auto [ uvBuffer, uvStride ] = uvTuple;
        if (uvBuffer != nullptr)
        {
          v.uv = uvBuffer[uvStride * i];
        }
        // TODO: handle UVs
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

Entity
GLTFLoader::processNode(
  Scene& scene,
  const tinygltf::Node& node,
  const tinygltf::Model& model
)
{
  Entity entity;

  scene.addInstance(entity, InstanceData {
    .name = std::move(node.name) // Name will not be used again, this is safe.
  });

  auto& transforms = scene.transforms();
  components::Transform transform;

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

    // @todo: remove temporary allocations.
    auto scale = glm::scale(
      glm::mat4(1.0),
      s.size() > 0 ? glm::vec3(s[0], s[1], s[2]) : glm::vec3(1.0)
    );
    auto translate = glm::translate(
      glm::mat4(1.0),
      t.size() > 0 ? glm::vec3(t[0], t[1], t[2]) : glm::vec3(0.0)
    );
    auto rot = glm::mat4_cast(
      r.size() > 0 ? glm::quat(r[3], r[0], r[1], r[2]) : glm::quat()
    );
    transform.modelToLocal = translate * rot * scale;
  }

  if (node.mesh >= 0)
  {
    // TODO: add support for primitive and material...
    const auto& mesh = model.meshes[node.mesh];
    scene.addRenderable(entity, node.mesh, mesh.primitives[0].material);
  }

  // Traverse graph.
  for (const auto& child: node.children)
  {
    const auto childEntity = processNode(scene, model.nodes[child], model);
    transforms.attach(childEntity, entity);
  }
  transforms.createComponent(entity, std::move(transform));

  return entity;
}

} // namespace loader

} // namespace albedo
