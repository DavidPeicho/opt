#version 450

#define EPSILON 0.000001
#define EPSILON1 1.000001

#define PI_F 3.14159265359
#define TO_RAD_F (PI_F / 180.0)
#define MAX_FLOAT 3.402823466e+38

#define INVALID_UINT 0xFFFFFFFF

// TODO: move to intersection pass.
struct Ray {
  vec3 origin;
  vec3 dir;
};

struct Intersection {
  vec2 uv;
  float dist;
  uint index;
};

struct Instance
{
  mat4 modelToWorld;
  uint bvhRootIndex;
  uint materialIndex;
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
bool
intersectTriangle(Ray ray, uint startIndex, inout Intersection intersection)
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
  if (abs(det) < EPSILON) { return false; }

  float invDet = 1.0 / det;

  // Computes Barycentric coordinates.
  vec3 centered = ray.origin - v0;

  float u = dot(centered, p) * invDet;
  if (u < EPSILON || u > EPSILON1) { return false; }

  vec3 q = cross(centered, e1);
  float v = dot(ray.dir, q) * invDet;
  if (v < EPSILON || u + v > EPSILON1) { return false; }

  intersection.uv = vec2(u, v);
  intersection.dist = dot(e2, q) * invDet;
  intersection.index = startIndex;

  return true;
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

void main()
{
  Ray ray = generateRay();
  ray.origin.z = 10.0;

  vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

  Intersection tmpIntersection;
  tmpIntersection.dist = -1.0;

  Intersection intersection;
  intersection.dist = MAX_FLOAT;

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
        if (intersectTriangle(ray, node.primitiveStartIndex, tmpIntersection))
        {
          if (tmpIntersection.dist < intersection.dist)
          {
            materialIndex = instance.materialIndex;
            intersection = tmpIntersection;
          }
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

  outColor = vec4(vec3(0.0), 1.0);

  if (intersection.dist < MAX_FLOAT) {
    vec2 uv = intersection.uv;
    float barycentricW = 1.0 - uv.x - uv.y;
    vec3 n0 = getVertex(intersection.index).normal;
    vec3 n1 = getVertex(intersection.index + 1).normal;
    vec3 n2 = getVertex(intersection.index + 2).normal;
    vec3 normal = barycentricW * n0 + uv.x * n1 + uv.y * n2;

    Material mat = materials[materialIndex];
    outColor = vec4(mat.albedo.rgb, 1.0);
    // outColor = vec4(normal, 1.0);
  }
}
