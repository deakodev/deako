#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV0;
layout (location = 3) in vec2 inUV1;
layout (location = 4) in uvec4 inJoint0;
layout (location = 5) in vec4 inWeight0;
layout (location = 6) in vec4 inColor0;

layout (set = 0, binding = 0) uniform UniformDynamicData
{
	mat4 model;
} uDynamic;

layout (set = 0, binding = 1) uniform UniformSharedData
{
	mat4 projection;
	mat4 view;
	vec3 camPos;
} uShared;

#define MAX_NUM_JOINTS 128

layout (set = 2, binding = 0) uniform UBONode 
{
	mat4 matrix;
	mat4 jointMatrix[MAX_NUM_JOINTS];
	uint jointCount;
} node;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV0;
layout (location = 3) out vec2 outUV1;
layout (location = 4) out vec4 outColor0;

mat4 CalculateSkinMatrix()
{
	return inWeight0.x * node.jointMatrix[inJoint0.x] +
		inWeight0.y * node.jointMatrix[inJoint0.y] +
		inWeight0.z * node.jointMatrix[inJoint0.z] +
		inWeight0.w * node.jointMatrix[inJoint0.w];
}

void main() 
{
	vec4 locPos;

	if (node.jointCount > 0) 
	{	// mesh is skinned
		mat4 skinMatrix = CalculateSkinMatrix();

		locPos = uDynamic.model * node.matrix * skinMatrix * vec4(inPos, 1.0);
		outNormal = normalize(transpose(inverse(mat3(uDynamic.model * node.matrix * skinMatrix))) * inNormal);
	} 
	else 
	{
		locPos = uDynamic.model * node.matrix * vec4(inPos, 1.0);
		outNormal = normalize(transpose(inverse(mat3(uDynamic.model * node.matrix))) * inNormal);
	}

	locPos.y = -locPos.y;
	outWorldPos = locPos.xyz / locPos.w;
	outUV0 = inUV0;
	outUV1 = inUV1;
	outColor0 = inColor0;

	gl_Position = uShared.projection * uShared.view * vec4(outWorldPos, 1.0);
}
