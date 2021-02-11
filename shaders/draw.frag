#version 450

layout (location = 0) in vec4 color;
layout (location = 0) out vec4 outColor;
layout (push_constant) uniform colorBlock {
    int constColor;
    float mixerValue;
} pushColorBlock;

vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
vec4 green = vec4(0.0, 1.0, 0.0, 1.0);
vec4 blue = vec4(0.0, 0.0, 1.0, 1.0);

void main()
{
    vec4 mixColor = yellow;
    if (pushColorBlock.constColor == 1)
        mixColor = red;
    else if (pushColorBlock.constColor == 2)
        mixColor = green;
    else if (pushColorBlock.constColor == 3)
        mixColor = blue;

    outColor = color * (1.0 - pushColorBlock.mixerValue)+ mixColor * pushColorBlock.mixerValue;
}