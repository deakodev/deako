#version 450 

layout(set = 0, binding = 0) uniform UniformDataPicker
{
    vec4 colorID;
} uPicker;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = uPicker.colorID;
} 
