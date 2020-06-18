#version 450

#define EPSILON 0.000001
#define EPSILON1 1.000001

#define PI_F 3.14159265359
#define TWO_PI 6.28318530718
#define TO_RAD_F (PI_F / 180.0)
#define MAX_FLOAT 3.402823466e+38

#define VEC3_UP vec3(0.0, 0.999999995, 0.0001)

#define MAX_UINT 0xFFFFFFFF
#define INVALID_UINT MAX_UINT

// TODO: move to intersection pass.
struct Ray {
  vec3 origin;
  vec3 dir;
};

struct Intersection {
  vec3 emissive;
  uint materialIndex;
  vec2 uv;
  uint index;
  bool emitter;
};

struct BRDFSample
{
  float pdf;
  vec3 dir;
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

layout (set = 0, binding = 6) uniform UniformsBuffer {
  float iFrame;
} Uniforms;

layout(location = 0) out vec4 radiance;

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
  return float(WangHash(seed)) / 4294967296.0;
}

vec3
transformDirection(vec3 direction, mat4 transform)
{
  return normalize((transform * vec4(direction, 0.0)).xyz);
}

// Implementation of Hammersley Points on the Hemisphere
vec3
randomCosineWeightedVector(inout uint seed)
{
  // To avoid to use a second sine and a normalization, it's possible to
  // use directly the random number in [0.0; 1.0] and scale the generated
  // `x` and `z` coordinates by it to obtain a normalized vector.
  // The code below is equivalent to:
  //   x = cos(theta), y = sin(phi), z = sin(theta);
  //   normalize(x, y, z);

  float theta = rand(seed) * TWO_PI;
  float r = rand(seed);
  float rLen = sqrt(max(EPSILON, 1.0 - r));

  float z = sqrt(r); // weights the samples to tend the normal
  float x = cos(theta) * rLen; // weights to preserve normalization
  float y = sin(theta) * rLen; // weights to preserve normalization

  return vec3(x, y, z);
}

/* Intersection */

Vertex
getVertex(uint index)
{
  return vertices[indices[index]];
}

/* Shading */

BRDFSample
LambertBRDF(vec3 normal, inout uint randState)
{
  vec3 tangent = normalize(cross(VEC3_UP, normal));
  vec3 bitangent = cross(normal, tangent);
  vec3 localDir = randomCosineWeightedVector(randState);

  BRDFSample brdf;
  brdf.dir = normalize(tangent * localDir.x + bitangent * localDir.y + localDir.z * normal);
  brdf.pdf = 0.5; // Should be PI/2 but cancels out with cosine sampling.
  return brdf;
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

float
intersectPlane(Ray ray, vec3 normal, vec3 origin, vec3 edge01, vec3 edge02)
{
  float NdotD = dot(normal, ray.dir);
  if (NdotD < EPSILON) { return MAX_FLOAT; }

  float t = dot(normal, origin - ray.origin) / NdotD;
  if (t < EPSILON) { return MAX_FLOAT; }

  vec3 intersection = (ray.origin + ray.dir * t) - origin;

  // Check not before first edge.
  float interProj = dot(edge01, intersection);
  if (interProj < EPSILON || interProj > dot(edge01, edge01)) { return MAX_FLOAT; }

  interProj = dot(edge02, intersection);
  if (interProj < EPSILON || interProj > dot(edge02, edge02)) { return MAX_FLOAT; }
  return t;
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
sceneHit(Ray ray, inout Intersection intersection)
{
  float dist = MAX_FLOAT;
  vec2 uv = vec2(0.0);
  uint materialIndex = INVALID_UINT;

  // DEBUG with one area light
  #if 1
  float xLen = 3.0;
  float zLen = 1.0;
  vec3 normal = vec3(.0, -1., 0.);
  vec3 bitangent = vec3(0.0, 0.0, -1.0) * zLen;
  vec3 tangent = vec3(1.0, 0.0, 0.0) * xLen;
  vec3 o = vec3(0.0, 3.0, 0.0) - 0.5 * tangent - 0.5 * bitangent;
  // ray, normal, origin, edge0, edge1
  dist = intersectPlane(
    ray,
    - normal,
    o,
    tangent,
    bitangent
  );
  if (dist < MAX_FLOAT)
  {
    intersection.emissive = vec3(1.0, 0.9, 0.8) * 30.0;
    intersection.emitter = true;
  }
  #endif

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
          intersection.emitter = false;
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
  uint randState = uint(uint(gl_FragCoord.x) * uint(1973) + uint(gl_FragCoord.y) * uint(9277) + uint(Uniforms.iFrame) * uint(26699)) | uint(1);

  Ray ray = generateRay();
  ray.origin.z = 10.0;

  vec3 throughput = vec3(1.0);
  radiance = vec4(vec3(0.0), 1.0);

  Intersection intersection;
  for (uint bounces = 0; bounces < 4; ++bounces)
  {
    intersection.emissive = vec3(0.0);
    intersection.emitter = false;

    float t = sceneHit(ray, intersection);

    radiance.rgb += throughput * intersection.emissive;

    if (t >= MAX_FLOAT) { break; }

    if (intersection.emitter)
    {
      // Sample emitter here.
      break;
    }

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
    BRDFSample brdf = LambertBRDF(normal, randState);

    Material mat = materials[intersection.materialIndex];
    vec3 directRadiance = mat.albedo.rgb * brdf.pdf;
    throughput *= directRadiance;

    // Diffuse bounce.
    ray.origin += t * ray.dir + EPSILON * normal;
    ray.dir = brdf.dir;
  }

}
