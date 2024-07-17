#version 450

// Vertex attributes
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

// Instanced attributes
layout (location = 3) in vec3 instancePosition;
layout (location = 4) in vec3 instanceRotation;
layout (location = 5) in float instanceScale;
layout (location = 6) in int instanceTexIndex;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outTexCoord;


layout(binding = 0) uniform UniformBufferObject
{
    mat4 viewProjection;
} ubo;

void main() 
{
	vec4 position = vec4((inPosition.xyz * instanceScale) + instancePosition, 1.0);

    gl_Position = ubo.viewProjection * position;
    outColor = inColor;
	outTexCoord = vec3(inTexCoord, instanceTexIndex);
}
