#version 450

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inColor;

layout(push_constant) uniform PushConstants {
    mat4 viewProj;
} pc;

layout(location = 0) out vec3 fragColor;

void main() {
    vec4 worldPos = vec4(inPos.x, inPos.y, 0.0, 1.0); // z=0, 2D locked
    gl_Position = pc.viewProj * worldPos;
    fragColor = inColor;
}
