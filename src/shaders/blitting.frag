#version 450

layout(location=0) in vec2 vUv;

layout(set=0, binding=0) uniform sampler uTextureSampler;
layout(set=0, binding=1) uniform texture2D uTexture;

layout(location = 0) out vec4 outColor;

void main() {
    // uint index = uint(gl_FragCoord.y * 640 + gl_FragCoord.x);
    // vec3 color = colors[index];
    outColor = texture(sampler2D(uTexture, uTextureSampler), vUv).rgba;
}
