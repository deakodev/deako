#version 450 

layout(set 0, binding = 0) uniform UniformDataEntity
{
    vec3 colorID;
} uEntity;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(uEntity.colorID, 1.0);
}
