#version 450

layout (std140, binding = 0) uniform bufferVars {
    mat4 mvp;
} buf;

layout (location = 0) in vec4 pos;
layout (location = 1) in vec4 inColor;
layout (location = 0) out vec4 outColor;

void main()
{
    outColor = inColor;
    gl_Position = buf.mvp * pos;

    // GL -> Vulkan
    //gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
}