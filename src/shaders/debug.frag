#version 450

#define EPSILON 0.000001
#define EPSILON1 1.000001

#define PI_F 3.14159265359
#define TWO_PI 6.28318530718
#define TO_RAD_F (PI_F / 180.0)
#define MAX_FLOAT 3.402823466e+38

#define VEC3_UP vec3(0.0, 1.0, 0.0)

#define MAX_UINT 0xFFFFFFFF
#define INVALID_UINT MAX_UINT

// TODO: move to intersection pass.
struct Ray {
  vec3 origin;
  vec3 dir;
};

struct Intersection {
  vec2 uv;
  uint index;
  uint materialIndex;
};

struct Instance
{
  mat4 modelToWorld;
  uint bvhRootIndex;
  uint materialIndex;
  uint padding_0;
  uint padding_1;
};

struct BVHNode
{
  vec3 min;
  uint nextNodeIndex;
  vec3 max;
  uint primitiveStartIndex;
};

// TODO: split into multiple arrays?
struct Vertex
{
  vec3 position;
  uint padding_0;
  vec3 normal;
  uint padding_1;
};

struct Material
{
  vec4 albedo;
};

layout (binding = 0) uniform RenderSettingsBuffer {
  uint width;
  uint height;
  uint instanceCount;
} RenderSettings;

layout (set = 0, binding = 1, std430) readonly buffer InstanceBuffer {
  Instance instances[];
};

layout (set = 0, binding = 2, std430) readonly buffer BVHNodeBuffer {
  BVHNode nodes[];
};

layout (set = 0, binding = 3, std430) readonly buffer IndexBuffer {
  uint indices[];
};

layout (set = 0, binding = 4, std430) readonly buffer VertexBuffer {
  Vertex vertices[];
};

layout (set = 0, binding = 5, std430) readonly buffer MaterialBuffer {
  Material materials[];
};

layout(location = 0) out vec4 outColor;

uint gRandState;

/* Utils */

uint
WangHash(inout uint seed)
{
  seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
  seed *= uint(9);
  seed = seed ^ (seed >> 4);
  seed *= uint(0x27d4eb2d);
  seed = seed ^ (seed >> 15);
  return seed;
}

float rand(inout uint seed)
{
  return float(WangHash(seed)) / float(MAX_UINT);
}

vec3
transformDirection(vec3 direction, mat4 transform)
{
  return normalize((transform * vec4(direction, 0.0)).xyz);
}

// Implementation of Hammersley Points on the Hemisphere
vec3
randomCosineWeightedVector(uint seed)
{
  // To avoid to use a second sine and a normalization, it's possible to
  // use directly the random number in [0.0; 1.0] and scale the generated
  // `x` and `z` coordinates by it to obtain a normalized vector.
  // The code below is equivalent to:
  //   x = cos(theta), y = sin(phi), z = sin(theta);
  //   normalize(x, y, z);

  float theta = rand(seed) * TWO_PI;
  float r = rand(seed);
  float rLen = sqrt(1.0 - r);

  float z = sqrt(r); // weights the samples to tend the normal
  float x = cos(theta) * rLen; // weights the x value to preserve normalization
  float y = sin(theta) * rLen; // weights the y value to preserve normalization
  return vec3(x, y, z);
}

/* Shading */

float LambertBRDF()
{
  return 1.0 / PI_F;
}



/* Intersection */

Vertex
getVertex(uint index)
{
  return vertices[indices[index]];
}

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

  ray.origin = vec3(0.0, 0.0, 0.0);

  ray.dir.x = gl_FragCoord.x - half_w;
  ray.dir.y = RenderSettings.height - gl_FragCoord.y - half_h;
  ray.dir.z = - dist;
  ray.dir = normalize(ray.dir);

  return ray;
}

// TODO: implement watertight version of Ray-Triangle intersection, available
// behind a flag

// Implementation of:
// Möller, Tomas; Trumbore, Ben (1997). "Fast, Minimum Storage Ray-Triangle Intersection"
float
intersectTriangle(Ray ray, uint startIndex, inout vec2 uv)
{
  // TODO: pre-process edge?
  // Maybe not really useful if decide to add skinning in shader.
  vec3 v0 = getVertex(startIndex).position;
  vec3 v1 = getVertex(startIndex + 1).position;
  vec3 v2 = getVertex(startIndex + 2).position;

  vec3 e1 = v1 - v0;
  vec3 e2 = v2 - v0;

  vec3 p = cross(ray.dir, e2);
  float det = dot(e1, p);

  // Ray is parralel to edge.
  // if (det <= - EPSILON) { return false; }
  if (abs(det) < EPSILON) { return MAX_FLOAT; }

  float invDet = 1.0 / det;

  // Computes Barycentric coordinates.
  vec3 centered = ray.origin - v0;

  float u = dot(centered, p) * invDet;
  if (u < EPSILON || u > EPSILON1) { return MAX_FLOAT; }

  vec3 q = cross(centered, e1);
  float v = dot(ray.dir, q) * invDet;
  if (v < EPSILON || u + v > EPSILON1) { return MAX_FLOAT; }

  uv = vec2(u, v);
  return dot(e2, q) * invDet;
}

bool
intersectAABB(Ray ray, vec3 aabbMin, vec3 aabbMax)
{
  // Ray is assumed to be in local coordinates, ie:
  // ray = inverse(objectMatrix * invCameraMatrix) * ray

  // Equation of ray: O + D * t

  vec3 invRay = 1.0 / ray.dir;
  vec3 tbottom = invRay * (aabbMin - ray.origin);
  vec3 ttop = invRay * (aabbMax - ray.origin);

  vec3 tmin = min(ttop, tbottom);
  vec3 tmax = max(ttop, tbottom);

  float largestMin = max(max(tmin.x, tmin.y), max(tmin.x, tmin.z));
  float smallestMax = min(min(tmax.x, tmax.y), min(tmax.x, tmax.z));

  return smallestMax > largestMin;
}

float
computeClosestHit(Ray ray, inout Intersection intersection)
{
  float dist = MAX_FLOAT;
  vec2 uv = vec2(0.0);
  uint materialIndex = INVALID_UINT;

  for (uint i = 0; i < RenderSettings.instanceCount; ++i)
  {
    Instance instance = instances[i];
    uint nextIndex = instance.bvhRootIndex;
    while (nextIndex != INVALID_UINT)
    {
      BVHNode node = nodes[nextIndex];

      // Node is a leaf.
      if (node.primitiveStartIndex != INVALID_UINT)
      {
        float t = intersectTriangle(ray, node.primitiveStartIndex, uv);
        if (t < dist)
        {
          intersection.uv = uv;
          intersection.index = node.primitiveStartIndex;
          intersection.materialIndex = instance.materialIndex;
          dist = t;
        }
        nextIndex = node.nextNodeIndex;
      }
      else if (!intersectAABB(ray, node.min, node.max))
      {
        nextIndex = node.nextNodeIndex;
      }
      else
      {
        nextIndex++;
      }
    }
  }
  return dist;
}

void main()
{
  gRandState = (uint(fragCoord.x) * uint(1973) + uint(fragCoord.y) * uint(9277)) | uint(1);

  Ray ray = generateRay();
  ray.origin.z = 10.0;

  outColor = vec4(vec3(0.0), 1.0);

  Intersection intersection;
  for (uint bounces = 0; bounces < 2; ++bounces)
  {
    float t = computeClosestHit(ray, intersection);
    if (t != MAX_FLOAT)
    {
      vec2 uv = intersection.uv;
      float barycentricW = 1.0 - uv.x - uv.y;
      vec3 n0 = getVertex(intersection.index).normal;
      vec3 n1 = getVertex(intersection.index + 1).normal;
      vec3 n2 = getVertex(intersection.index + 2).normal;
      vec3 normal = barycentricW * n0 + uv.x * n1 + uv.y * n2;

      // 𝐿𝑟 𝛚𝑟 ≈ 𝑁 𝑓𝑟 𝛚𝑖,𝛚𝑟 𝐿𝑖 𝛚𝑖 cos𝜃𝑖⁡

      // We sample in a cosine weighted hemisphere, so basially we remove
      // the 2PI term and the multiplication by cos(𝜃) as the samples are already
      // sampled in a cosine hemisphere.

      // Bounce

      vec3 tangent = cross(normal, VEC3_UP);
      vec3 bitangent = cross(normal, right);

      // Diffuse bounce.
      vec3 dir = randomCosineWeightedVector(gRandState);
      ray.origin = t * ray.dir;
      ray.dir = normalize(tangent * dir.x + bitangent * dir.y + dir.z * normal);

      // Material mat = materials[intersection.materialIndex];
      // outColor = vec4(mat.albedo.rgb, 1.0);
      // outColor = vec4(normal, 1.0);
    }
  }

}
