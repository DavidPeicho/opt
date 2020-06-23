#version 450

layout( location = 0 ) in vec2 vUv;

layout ( binding = 0 ) uniform RenderSettingsBuffer {
  uint width;
  uint height;
  uint instanceCount;
  uint frameCount;
} RenderSettings;

layout( set = 0, binding = 1 ) uniform sampler uTextureSampler;
layout( set = 0, binding = 2 ) uniform texture2D uTexture;

layout(location = 0) out vec4 outColor;

void main() {
  vec4 color = texture(sampler2D(uTexture, uTextureSampler), vUv).rgba;
  outColor = color.rgba / float(RenderSettings.frameCount);
  // outColor = color;
}
