#version 450
#extension GL_GOOGLE_include_directive : require

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 outColor;

layout (binding = 1) uniform UniformParams {
	vec4 _pad0;
	float exposure;
	float gamma;
} uParams;

layout (binding = 2) uniform samplerCube samplerEnv;

#include "includes/tonemapping.glsl"
#include "includes/srgbtolinear.glsl"

void main() 
{
	vec3 color = SRGBtoLINEAR(tonemap(textureLod(samplerEnv, inUVW, 1.5))).rgb;	
	outColor = vec4(color * 1.0, 1.0);
}
