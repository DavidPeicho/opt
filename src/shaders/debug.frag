#version 450

#define PI_F 3.14159265359
#define TO_RAD_F (PI_F / 180.0)

// TODO: move to intersection pass.
struct Ray {
  vec3 origin;
  vec3 dir;
};

struct BVHNode
{
  vec3 min;
  uint nextNodeIndex;
  vec3 max;
  uint primitiveStartIndex;
};

layout (binding = 0) uniform RenderSettingsBuffer {
  uint width;
  uint height;
} RenderSettings;

layout (set = 0, binding = 1, std430) readonly buffer BVHNodeBuffer {
  BVHNode nodes[];
};

layout(location = 0) out vec4 outColor;

// TODO: move to intersection pass.
Ray
generateRay()
{
  Ray ray;

  float half_w = float(RenderSettings.width) * 0.5;
  float half_h = float(RenderSettings.height) * 0.5;

  float fovv = 45.0; // TODO: add to camera uniform buffer.
  float dist = half_h / tan(fovv * TO_RAD_F * 0.5);

  // TODO: use inverse projection matrix here instead.

  ray.origin.z = - 5.0;
  ray.dir.x = gl_FragCoord.x - half_w;
  ray.dir.y = RenderSettings.height - gl_FragCoord.y - half_h;
  ray.dir.z = - dist;
  ray.dir = normalize(ray.dir);

  return ray;
}

void main()
{
  Ray r = generateRay();

  float minDistance = 0xFFFFFFFF;

  uint nextIndex = 0;
  while (nextIndex != 0xFFFFFFFF)
  {
    BVHNodeBuffer node = nodes[nextIndex];
    // Node is a leaf.
    if (node.primitiveStartIndex != 0xFFFFFFFF)
    {
      nextIndex = node.nextIndex;
    }
    else if ()
    {
      
    }
    else
    {
      nextIndex++;
    }
  }

}
