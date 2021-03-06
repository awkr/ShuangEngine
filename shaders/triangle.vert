#version 450

layout(binding = 0) uniform vs_ubo_t {
    mat4 model;
    mat4 view;
    mat4 proj;
} vs_ubo;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = vs_ubo.proj * vs_ubo.view * vs_ubo.model * vec4(position, 1.0);
    //    gl_Position.y = -gl_Position.y;
    fragColor = color;
}
