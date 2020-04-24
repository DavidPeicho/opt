#version 450

layout(location=0) in vec2 vUv;

layout(std430, set = 0, binding = 0) readonly buffer buf {
    vec3 colors[ ];
};

layout(set=0, binding=1) uniform sampler uTextureSampler;
layout(set=0, binding=2) uniform texture2D uTexture;

layout(location = 0) out vec4 outColor;

void main() {
    // uint index = uint(gl_FragCoord.y * 640 + gl_FragCoord.x);
    // vec3 color = colors[index];
    outColor = texture(sampler2D(uTexture, uTextureSampler), vUv).rgba;
}
