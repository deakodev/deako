#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (binding = 0) uniform UniformDataShared 
{
	mat4 projection;
	mat4 view;
} uShared;

layout (location = 0) out vec3 outUVW;

void main() 
{
	outUVW = inPos;

	// Remove the translation component from the view matrix
    mat3 rotationOnly = mat3(uShared.view); // Extract rotation part
    mat4 viewRotationOnly = mat4(rotationOnly); // Convert back to mat4

    // Calculate gl_Position without translating the skybox
    gl_Position = uShared.projection * viewRotationOnly * vec4(inPos, 1.0);
}
