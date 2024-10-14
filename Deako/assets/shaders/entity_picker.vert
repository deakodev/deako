#version 450 

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec2 inUV1;
layout (location = 4) in uvec4 inJoint0;
layout (location = 5) in vec4 inWeight0;
layout (location = 6) in vec4 inColor0;


layout (set = 0, binding = 0) uniform UniformDataShared
{
	mat4 projection;
	mat4 view;
	vec3 camPos;
} uShared;

layout (set = 0, binding = 5) uniform UniformDataDynamic
{
	mat4 model;
} uDynamic;

void main()
{
    vec4 worldPos = uDynamic.model * vec4(inPos, 1.0);

	gl_Position = uShared.projection * uShared.view * worldPos;
}
