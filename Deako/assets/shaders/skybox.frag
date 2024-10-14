#version 450
#extension GL_GOOGLE_include_directive : require

layout (location = 0) in vec3 inUVW;
layout (location = 0) out vec4 outColor;

layout (set = 0, binding = 1) uniform UniformLightData 
{
	vec4 _pad0;
	float exposure;
	float gamma;
} uLight;

layout (set = 0, binding = 2) uniform samplerCube samplerEnv;

#include "includes/tonemapping.glsl"
#include "includes/srgbtolinear.glsl"

void main() 
{
	vec3 color = SRGBtoLINEAR(tonemap(textureLod(samplerEnv, inUVW, 1.5))).rgb;	
	outColor = vec4(color * 1.0, 1.0);
}
