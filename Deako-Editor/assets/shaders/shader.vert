#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout (location = 3) in int inTexIndex;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outTexCoord;


layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 projection;
} ubo;

void main() 
{
    gl_Position = ubo.projection * ubo.view * ubo.model * vec4(inPosition, 1.0);
    outColor = inColor;
	outTexCoord = vec3(inTexCoord, inTexIndex);
}
