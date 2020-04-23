#version 450

layout(std430, set = 0, binding = 0) readonly buffer buf {
    vec3 colors[ ];
};

layout(location = 0) out vec4 outColor;

void main() {
    uint index = uint(gl_FragCoord.y * 640 + gl_FragCoord.x);
    vec3 color = colors[index];
    outColor = vec4(color, 1.0);
}
