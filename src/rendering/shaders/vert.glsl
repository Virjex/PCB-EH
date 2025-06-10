#version 450

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 outColor;

void main() {
    outColor = inColor;

    // Map coordinates to NDC (-1 .. +1) → you can change this later to camera transform!
    // For now: just simple scale to [-1, 1] space:
    float scale = 1.0 / 500.0; // Example scale — adjust for your coordinate system

    gl_Position = vec4(inPos.x * scale, inPos.y * scale, 0.0, 1.0);
}
