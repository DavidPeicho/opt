#version 450

struct BVHNode
{
  vec3 min;
  uint nextNodeIndex;
  vec3 max;
  uint primitiveStartIndex;
};

layout (set = 0, binding = 0, std430) readonly buffer BVHNodeBuffer {
  BVHNode nodes[];
};

layout(location = 0) out vec4 outColor;

void main() {
  outColor = vec4(0.5, 0.0, 0.0, 1.0);
  if (nodes[11].primitiveStartIndex == 6) {
    outColor = vec4(0.0, 0.0, 0.5, 1.0);
  }
}
