#version 450

layout (std140, binding = 0) uniform BufferVars {
    mat4 mvp;
} bv;

layout (location = 0) in vec4 pos;
layout (location = 1) in vec2 inUV;
layout (location = 0) out vec2 outUV;

void main() {
    outUV = inUV;
    gl_Position = bv.mvp * pos;
}
