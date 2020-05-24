#version 450

layout(location=0) in vec2 vUv;

layout(set=0, binding=0) uniform sampler uTextureSampler;
layout(set=0, binding=1) uniform texture2D uTexture;

layout(location = 0) out vec4 outColor;

void main() {
  outColor = vec4(1.0, 0.0, 0.0, 1.0);
    // outColor = texture(sampler2D(uTexture, uTextureSampler), vUv).rgba;
}
